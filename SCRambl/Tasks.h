/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <queue>
#include <memory>
#include <set>
#include <functional>
#include <typeinfo>
#include "Reporting.h"

namespace SCRambl
{
	class Engine;
	
	namespace TaskSystem
	{
		// secret stash of functionage
		namespace
		{
			template <typename Function>
			struct function_traits : public function_traits<decltype(&Function::operator())>
			{};

			template <typename ClassType, typename ReturnType>
			struct function_traits<ReturnType(ClassType::*)() const>
			{
				typedef ReturnType(*pointer)();
				typedef std::function<ReturnType()> function;
			};

			template <typename ClassType, typename ReturnType, typename... Args>
			struct function_traits<ReturnType(ClassType::*)(Args...) const>
			{
				typedef ReturnType(*pointer)(std::reference_wrapper<Args>...);
				typedef std::function<ReturnType(std::reference_wrapper<Args>...)> function;
			};

			template <typename Function>
			typename function_traits<Function>::pointer to_function_pointer(Function& lambda)
			{
				return static_cast<typename function_traits<Function>::pointer>(lambda);
			}

			template <typename Function>
			typename function_traits<Function>::function to_function(Function& lambda)
			{
				return static_cast<typename function_traits<Function>::function>(lambda);
			}
		}
		
		template<typename EventType>
		class Event
		{
			template<typename>
			friend class Task;

			// imagine if there was a way to store functions by how they need to be called...
			std::multimap<const std::type_info*, void*>	m_Handlers;

		protected:
			EventType			m_ID;

		public:
			Event() = default;
			Event(EventType id) : m_ID(id)
			{ }

			// Add event handler
			template<typename Func>
			void AddHandler(Func handler)
			{
				// do some weird shit
				auto function = new decltype(to_function(handler))(to_function(handler));
				// add the function to the list and associate its type
				m_Handlers.emplace(&typeid(function), static_cast<void*>(function));
			}

			// Call capable handlers, returns false if none were called
			template<typename... Args>
			bool CallHandler(Args&&... args)
			{
				// this is surely impossible...?
				if (m_Handlers.empty()) return false;

				// try to find some handlers capable of accepting Args
				auto it_pair = m_Handlers.equal_range(&typeid(std::function<bool(Args...)>*));

				// no? really?
				if (it_pair.first == m_Handlers.end()) return false;

				// call all handlers, or until the first one that returns 'false'
				for (auto it = it_pair.first; it != it_pair.second; ++it)
				{
					// cast it back to the function
					auto func = static_cast<std::function<bool(Args...)>*>(it->second);

					// if it returns false, abort in case it did something that could screw up future calls
					if (!(*func)(std::forward<Args...>(args)...)) break;
				}
				return true;
			}

			virtual ~Event()
			{
				// delete the function pointers
				for (auto pair : m_Handlers) {
					delete static_cast<std::function<bool(void)>*>(pair.second);
				}
			}
		};

		class ITask
		{
		public:
			virtual void ResetTask() = 0;
			virtual void RunTask() = 0;
			virtual bool IsTaskFinished() = 0;
		};

		template<typename EventType>
		class Task : public ITask
		{
			friend class SCRambl::Engine;

		public:
			enum State { running, error, finished };

		private:
			State									m_State;

			// events can be handled by the implementor
			std::map<EventType, Event<EventType>>	m_Events;

		protected:
			inline State & TaskState()				{ return m_State; }

		public:
			inline State GetState()	const			{ return m_State; }

			// don't need this - instead we'll do it when a handler is added which is way more efficient in many ways
			/*template<EventType id>
			inline void AddEvent() {
				m_Events.emplace(id, id);
			}*/

			// Add an event handler
			template<EventType id, typename Func>
			inline void AddEventHandler(Func func) {
				auto & ev = m_Events[id];
				ev.AddHandler<Func>(std::forward<Func>(std::ref(func)));
			}

			template<typename... Args>
			inline bool CallEventHandler(EventType id, Args&&... args) {
				// if no event is found, there mustn't be a handler for it anyway
				if (m_Events.empty()) return false;
				auto it = m_Events.find(id);
				if (it != m_Events.end())
				{
					// pass the message
					if(it->second.CallHandler(std::ref(args)...))
						return true;
				}
				return false;
			}

		protected:
			Task() : m_State(running)
			{ }
			virtual ~Task() { };

			const Task & Run()
			{
				do
				{
					switch (m_State)
					{
					case error:
						m_State = running;
					case running:
						try
						{
							RunTask();

							if (IsTaskFinished()) m_State = finished;
						}
						catch (...)
						{
							m_State = error;
						}
						break;
					case finished:
						ResetTask();
						if (!IsTaskFinished()) m_State = running;
						continue;
					}
				} while (false);

				return *this;
			};
		};
	}
}