//**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <queue>
#include <memory>
#include <map>
#include <set>
#include <functional>
#include <typeinfo>
#include "utils\function_traits.h"

namespace SCRambl
{
	class Engine;

	// This was HELL to write...
	namespace TaskSystem {
		// Events & handlers
		template<typename EventType>
		class Event {
			template<typename>
			friend class Task;

		public:
			Event() = default;
			Event(EventType id) : m_ID(id)
			{ }
			virtual ~Event() {
				// delete the function pointers
				for (auto pair : m_Handlers) {
					// looks really simple...
					delete static_cast<std::function<bool(void)>*>(pair.second);
				}
			}

			// Add event handler
			template<typename Func>
			void AddHandler(Func handler) {
				// do some weird, but very necessary shit
				auto function = new decltype(to_function(handler))(to_function(handler));

				// add the function to the list and associate its type
				m_Handlers.emplace(&typeid(function), static_cast<void*>(function));
			}
			// Call capable handlers, returns false if none were called
			template<typename... Args>
			bool CallHandler(Args&&... args) {
				// this is surely impossible...?
				if (m_Handlers.empty()) return false;

				// try to find some handlers capable of accepting Args...
				static auto s_p = &typeid(std::function<bool(Args...)>*);
				auto it_pair = m_Handlers.equal_range(s_p);

				// no? really?
				if (it_pair.first == m_Handlers.end()) return false;

				// call all handlers, or until the first one that returns 'false'
				for (auto it = it_pair.first; it != it_pair.second; ++it) {
					// cast it back to the function
					auto func = static_cast<std::function<bool(Args...)>*>(it->second);
					// if it returns false, abort in case it did something that could screw up future calls
					if (!(*func)(std::forward<Args>(args)...)) break;
				}
				return true;
			}
		
		private:
			EventType m_ID;
			// imagine if there was a way to store functions by how they need to be called...
			std::multimap<const std::type_info*, void*>	m_Handlers;
		};

		// Task interface - Task placeholder
		class ITask
		{
		public:
			// task controllers
			virtual void RunTask() = 0;
			virtual bool IsTaskFinished() = 0;
			virtual void ResetTask() = 0;
		};

		// Task - Kinda the point of this whole file
		template<typename EventType>
		class Task : public ITask {
			friend class SCRambl::Engine;

		public:
			enum State { running, error, finished };

			// Add an event handler, plus the event if it doesn't exist already
			template<EventType id, typename Func>
			inline void AddEventHandler(Func func) {
				auto & ev = m_Events[id];
				ev.AddHandler<Func>(std::forward<Func>(std::ref(func)));
			}
			// Call all handlers for an event
			// - Returns false if none were called
			template<typename... Args>
			inline bool CallEventHandler(EventType id, Args&&... args) {
				// if no event is found, there must not be a handler for it anyway
				if (m_Events.empty()) return false;
				auto it = m_Events.find(id);
				if (it != m_Events.end()) {
					// pass the message
					if(it->second.CallHandler(std::forward<Args>(args)...))
						return true;
				}
				// nothing called
				return false;
			}
			// Gets the current state
			inline State GetState()	const { return m_State; }

		protected:
			// a running start!
			Task() : m_State(running)
			{ }
			virtual ~Task()
			{ };

			// When you say run, I say go fuck yourself...
			inline const Task& Run() {
				do {
					// continue from where we left off...
					switch (m_State) {
					case error:
						// error last time? oh well, moving on...
						m_State = running;
					case running:
						try {
							// great, now redirect to the REAL task
							RunTask();

							// finished? what d'yeh want, a cookie?
							if (IsTaskFinished()) m_State = finished;
						}
						catch (...) {
							// I doubt this will ever happen...
							m_State = error;
						}
						break;
					case finished:
						// run again (y/n)?
						ResetTask();
						if (!IsTaskFinished()) {
							m_State = running;
							continue;
						}
						break;
					}
				} while (false);
				return *this;
			};
			inline State& TaskState() { return m_State; }

		private:
			State m_State;

			// events can be handled by the implementor
			std::map<EventType, Event<EventType>> m_Events;
		};
	}
}