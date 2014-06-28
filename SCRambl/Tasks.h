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
#include <typeinfo>
#include <typeindex>
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

			template <typename ClassType, typename ReturnType, typename... Args>
			struct function_traits<ReturnType(ClassType::*)(Args...) const>
			{
				typedef ReturnType(*pointer)(Args...);
				typedef std::function<ReturnType(Args...)> function;
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
		
		class Event
		{
			template<typename>
			friend class Task;

			std::multimap<std::type_index, function_traits<std::function<bool(void)>>::function>	m_Handlers;

		protected:
			class Result
			{
			};

		public:
			Event() = default;

			template<typename Func>
			void AddHandler(Func handler)
			{
				m_Handlers.emplace(typeid(to_function(handler)), handler);
			}

			template<typename... Args>
			bool CallHandler(Args&&... args)
			{
				auto it = m_Handlers.find(typeid(std::function<bool(Args...)>));
				if (it != m_Handlers.end())
				{
					static_cast<std::function<bool(Args...)>>(it->second)();
					return true;
				}
				return false;
			}

			template<typename... Args>
			void operator()(Args&&... args)
			{
				for (auto handler : m_Handlers)
				{
					*handler(std::forward<Args>(args)...);
				}
			}

			virtual ~Event()
			{ }
		};

		class TaskBase
		{
		public:
			virtual void ResetTask() = 0;
			virtual void RunTask() = 0;
			virtual bool IsTaskFinished() = 0;
		};

		template<typename EventType>
		class Task : public TaskBase
		{
			friend class SCRambl::Engine;

		public:
			enum State { running, error, finished };

		private:
			State									m_State;

			std::map<EventType, Event>				m_Events;

		protected:
			inline State & TaskState()				{ return m_State; }

		public:
			inline State GetState()	const			{ return m_State; }

			template<EventType id>
			void AddEvent() {
				m_Events.emplace(id, Event());
			}

			template<EventType id, typename Func>
			bool AddEventHandler(Func func) {
				auto it = m_Events.find(id);
				if (it != m_Events.end())
				{
					it->second.AddHandler(func);
					return true;
				}
				return false;
			}

			template<EventType id, typename... Args>
			bool CallEventHandler(Args&&... args) {
				static EventType s_id = id;
				auto it = m_Events.find(s_id);
				if (it != m_Events.end())
				{
					if(it->second.CallHandler(std::forward(args)...))
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