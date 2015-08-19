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
#include <typeindex>
#include "utils\function_traits.h"

namespace SCRambl
{
	class Engine;

	struct task_exception : public std::exception {
		using std::exception::exception;
	};
	struct task_bad_event : public task_exception {
		using task_exception::task_exception;
	};

	namespace TaskSystem {
		class Task;

		namespace {
			// Events & handlers
			class TaskEvent {
				friend class Task;

			public:
				TaskEvent() = default;
				virtual ~TaskEvent() {
					// delete the function pointers
					for (auto fun : m_Handlers) {
						delete static_cast<std::function<bool(void)>*>(fun);
					}
				}

				// Add event handler
				template<typename Func>
				void AddHandler(Func handler) {
					// sum magic
					auto function = new decltype(to_function(handler))(to_function(handler));
					// add the handler
					m_Handlers.emplace_back(static_cast<void*>(function));
				}
				// Call handlers, returns false if none were called
				template<typename TEvent>
				bool CallHandler(const TEvent& event) const {
					// this is surely impossible...?
					if (m_Handlers.empty()) return false;
					for (auto fun : m_Handlers) {
						auto func = static_cast<std::function<bool(const TEvent&)>*>(fun);
						if (!(*func)(event)) break;
					}
					return false;
				}

			private:
				std::vector<void*> m_Handlers;
			};

			// Task interface - Task runner
			class ITask
			{
			public:
				// task controllers
				virtual void RunTask() = 0;
				virtual bool IsTaskFinished() = 0;
				virtual void ResetTask() = 0;
			};
		}

		// Task - tasks and events
		class Task : public ITask {
		public:
			enum State { running, error, finished };

			virtual ~Task() { };

			// Add an event
			template<typename TEvent>
			inline bool AddEvent(std::string name) {
				if (std::is_base_of<task_event, TEvent>()) {
					m_Events[typeid(TEvent)].first = name;
					return true;
				}
				return false;
			}
			// Add an event handler
			template<typename TEvent, typename Func>
			inline void AddEventHandler(Func func) {
				m_Events.at(typeid(TEvent)).second.AddHandler<Func>(std::forward<Func>(std::ref(func)));
			}
			// Call all handlers for an event - returns false if none were called
			template<typename TEvent>
			inline bool CallEvent(const TEvent& event) const {
				// validate as derived event class
				auto b = std::is_base_of<task_event, TEvent>();
				if (typeid(TEvent) == typeid(task_event)) throw(task_bad_event());
				if (m_Events.empty()) return false;
				auto it = m_Events.find(typeid(TEvent));
				if (it != m_Events.end()) {
					// pass the message
					if(it->second.second.CallHandler(event))
						return true;
				}
				// nothing called
				return false;
			}
			// Gets the current state
			inline State GetState()	const { return m_State; }

			// Convenience event functions
			template<typename TEvent>
			inline bool Event(const TEvent& event) const { return CallEvent<TEvent>(event); }
			template<typename TEvent, typename... TArgs>
			inline bool Event(TArgs&&... args) const {
				return CallEvent<TEvent>(TEvent(std::forward<TArgs>(args)...));
			}
			template<typename TEvent>
			inline bool operator()(const TEvent& event) const { return CallEvent<TEvent>(event); }

		protected:
			// a running start!
			Task() : m_State(running)
			{ }

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
			std::unordered_map<std::type_index, std::pair<std::string, TaskEvent>> m_Events;
		};
	}


	struct task_event {
		friend TaskSystem::Task;
		virtual ~task_event() { }

		virtual bool Send(TaskSystem::Task& task) const {
			return task.Event(*this);
		}
	};
}