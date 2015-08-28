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

				inline void SetName(std::string v, bool b = false) { if (m_Name.empty() || b) m_Name = v; }

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
				virtual bool IsTaskFinished() const = 0;
				virtual void ResetTask() = 0;
			};
		}

		// Task - tasks and events
		class Task : public ITask {
			using EventPair = std::pair<std::string, TaskEvent>;
			using Events = std::deque<EventPair>;

		public:
			enum State { init, running, error, finished };

			virtual ~Task() { };

			// Add an event
			template<typename TEvent>
			inline bool AddEvent(std::string name = "") {
				static std::type_index event_id = typeid(TEvent);
				ASSERT((std::is_base_of<task_event, TEvent>()) == true);
				if (std::is_base_of<task_event, TEvent>()) {
					auto it = m_EventMap.find(event_id);
					if (it != std::end(m_EventMap))
						it->second->second.SetName(!name.empty() ? name : event_id.name(), true);
					else {
						auto& v = MyAddEvent<TEvent>();
						v.second.SetName(!name.empty() ? name : event_id.name());
						m_EventMap[event_id] = &v;
					}
					return true;
				}
				return false;
			}
			// Add an event handler
			template<typename TEvent, typename Func>
			inline void AddEventHandler(Func func) {
				static std::type_index event_id = typeid(TEvent);
				auto it = m_EventMap.find(event_id);
				if (it != std::end(m_EventMap)) {
					it->second->second.AddHandler<Func>(std::forward<Func>(std::ref(func)));
				}
				else {
					auto& ev = MyAddEvent<TEvent>();
					ev.second.AddHandler<Func>(std::forward<Func>(std::ref(func)));
					m_EventMap[event_id] = &ev;
				}
			}
			// Call all handlers for an event - returns number of successful calls
			template<typename TEvent>
			inline size_t CallEvent(TEvent& event) {
				// validate as derived event class
				static std::type_index event_id = typeid(TEvent);
				auto b = std::is_base_of<task_event, TEvent>();
				if (event_id == typeid(task_event)) throw(task_bad_event());
				if (!m_EventMap.empty()) {
					for (auto id : event) {
						auto iter = m_EventMap.find(id);
						if (iter != std::end(m_EventMap)) {
							// pass the message
							return iter->second->second.CallHandler(event);
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
			template<typename TEvent>
			EventPair& MyAddEvent() {
				static std::type_index event_id = typeid(TEvent);
				m_Events.emplace_front();
				auto& ref = m_Events.front();
				ref.first = event_id.name();
				return ref;
			}

		private:
			State m_State = State::init;
			Task* m_Parent = nullptr;

			// events can be handled by the implementor
			Events m_Events;
			std::unordered_map<std::type_index, EventPair*> m_EventMap;
		};
	}

	struct task_event {
		using LinkEventDeque = std::deque<const std::type_index>;
		friend TaskSystem::TaskEvent;
		task_event() {
			LinkEvent<task_event>("task_event");
		}
		virtual ~task_event() { }

		virtual size_t Send(TaskSystem::Task& task) {
			return task.Event(*this);
		}

		inline const std::string& Name() const { return m_Name; }
		LinkEventDeque::const_iterator begin() const { return m_LinkEvents.begin(); }
		LinkEventDeque::const_iterator end() const { return m_LinkEvents.end(); }

	protected:
		template<typename TEvent>
		void LinkEvent(std::string name) {
			m_Name = name;
			m_LinkEvents.emplace_front(typeid(TEvent));
		}

	private:
		inline void SetName(std::string name, bool b = false) { if (m_Name.empty() || b) m_Name = name; }

	private:
		std::string m_Name;
		LinkEventDeque m_LinkEvents;
	};
}