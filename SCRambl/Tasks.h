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

		template<typename TEvent>
		struct event_id {
			static std::vector<std::type_index> vec;
		};

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

				inline void SetName(std::string v) { if (m_Name.empty()) m_Name = v; }

				// Add event handler
				template<typename Func>
				void AddHandler(Func handler) {
					// sum magic
					auto function = new decltype(to_function(handler))(to_function(handler));
					// add the handler
					m_Handlers.emplace_back(static_cast<void*>(function));
				}
				// Call handlers, returns number of successful calls
				template<typename TEvent>
				size_t CallHandler(TEvent& event) {
					// this is surely impossible...?
					if (m_Handlers.empty()) return false;
					size_t calls = 0;
					event.SetName(m_Name);
					for (auto fun : m_Handlers) {
						auto func = static_cast<std::function<bool(TEvent&)>*>(fun);
						if (!(*func)(event)) break;
						++calls;
					}
					return calls;
				}

			private:
				std::vector<void*> m_Handlers;
				std::string m_Name;
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
			enum State { init, running, error, finished };

			virtual ~Task() { };

			// Add an event
			template<typename TEvent>
			inline bool AddEvent(std::string name) {
				static std::type_index event_id = typeid(TEvent);
				if (std::is_base_of<task_event, TEvent>()) {
					auto& v = m_Events[event_id];
					v.first = name;
					v.second.SetName(name);
					return true;
				}
				return false;
			}
			// Add an event handler
			template<typename TEvent, typename Func>
			inline void AddEventHandler(Func func) {
				static std::type_index event_id = typeid(TEvent);
				m_Events[event_id].second.AddHandler<Func>(std::forward<Func>(std::ref(func)));
			}
			// Call all handlers for an event - returns number of successful calls
			template<typename TEvent>
			inline size_t CallEvent(TEvent& event) {
				// validate as derived event class
				static std::type_index event_id = typeid(TEvent);
				auto b = std::is_base_of<task_event, TEvent>();
				if (event_id == typeid(task_event)) throw(task_bad_event());
				if (!m_Events.empty()) {
					for (auto id : event) {
						auto it = m_Events.find(id);
						if (it != m_Events.end()) {
							// pass the message
							return it->second.second.CallHandler(event);
						}
					}
				}
				// nothing called, try a higher-up
				return m_Parent ? m_Parent->CallEvent<TEvent>(event) : false;
			}
			// Gets the current state
			inline State GetState()	const { return m_State; }

			// Convenience event functions
			template<typename TEvent>
			inline size_t Event(TEvent& event) { return CallEvent<TEvent>(event); }
			template<typename TEvent, typename... TArgs>
			inline size_t Event(TArgs&&... args) {
				return CallEvent<TEvent>(TEvent(std::forward<TArgs>(args)...));
			}
			template<typename TEvent>
			inline size_t operator()(TEvent& event) const { return CallEvent<TEvent>(event); }

		protected:
			// a running start!
			Task() : m_State(running)
			{ }
			Task(Task* parent) : m_State(running), m_Parent(parent)
			{ }

			// When you say run, I say go fuck yourself...
			inline const Task& Run() {
				do {
					if (m_State == init) m_State = running;
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
			State m_State = State::init;
			Task* m_Parent = nullptr;

			// events can be handled by the implementor
			std::unordered_map<std::type_index, std::pair<std::string, TaskEvent>> m_Events;
		};
	}

	struct task_event {
		using LinkEventDeque = std::set<const std::type_index>;
		friend TaskSystem::TaskEvent;
		task_event() {
			LinkEvent<task_event>("task event");
		}
		virtual ~task_event() { }

		virtual size_t Send(TaskSystem::Task& task) {
			return task.Event(*this);
		}

		inline const std::string& Name() const { return m_Name; }
		LinkEventDeque::const_iterator begin() const { return GetLinkEventDeque().begin(); }
		LinkEventDeque::const_iterator end() const { return GetLinkEventDeque().end(); }

	protected:
		template<typename TEvent>
		void LinkEvent(std::string name) {
			m_Name = name;
			if (LinkLock<TEvent>() != 'O')
				GetLinkEventDeque().emplace(typeid(TEvent));
		}

		template<> void LinkEvent<task_event>(std::string name) {
			m_Name = name;
			if (LinkLock<task_event>() != 'O') {
				if (LinkLock<task_event>() != 'I') LinkLock<task_event>() = 'I';
				else LinkLock<task_event>() = 'O';
				GetLinkEventDeque().emplace(typeid(task_event));
			}
		}

	private:
		template<typename TEvent>
		static char& LinkLock() {
			static char b = '\0';
			return b;
		}

		static LinkEventDeque& GetLinkEventDeque() {
			static LinkEventDeque deque;
			return deque;
		}

		inline void SetName(std::string name) { if (m_Name.empty()) m_Name = name; }

	private:
		std::string m_Name;
	};
}