/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Tasks.h"
#include "Scripts.h"
#include "Parser.h"

namespace SCRambl
{
	namespace Compiling
	{
		class Task;
		class Error {
		public:
			enum ID {
				// involuntary errors (errors that should be impossible!!) (500+)

				// normal errors (1000+)

				// fatal errors (4000+)
				fatal_begin = 4000,
				//include_failed = 4000,
				fatal_end,
			};

			Error(ID id) : m_ID(id)
			{ }
			inline operator ID() const			{ return m_ID; }

		private:
			ID			m_ID;
		};
		class Compiler {
		public:
			enum State {
				init, compiling, finished,
				bad_state, max_state = bad_state,
			};

			Compiler(Task& task, Engine& engine, Build* build);

			inline bool IsFinished() const { return m_State == finished; }
			inline bool IsRunning()	const { return m_State == init || m_State == compiling; }
			void Init();
			void Run();
			void Finish();
			void Reset();
			void Compile();

		protected:
			inline size_t GetNumTokens() const {
				return m_Tokens.Size();
			}
			inline size_t GetCurrentToken() const {
				return m_TokenIt.Index();
			}
			inline Tokens::Token GetToken() const {
				return m_TokenIt.Get();
			}
			template<typename T, typename U = T>
			inline void Output(const U& v) {
				m_File.write((char*)&v, sizeof(T));
			}
			template<typename T>
			inline void Output(const T& v, size_t n) {
				m_File.write((char*)&v, n);
			}
			template<typename T, typename U = T>
			inline void Output(const U* v) {
				m_File.write((char*)v, sizeof(T));
			}
			template<typename T>
			inline void Output(const T* v, size_t n) {
				m_File.write((char*)v, n);
			}

		private:
			union DataVal {
				uint64_t uint64;
				uint32_t uint32;
				uint16_t uint16;
				uint8_t uint8;
				float flt;
			};

			void CompileTranslation(Types::Translation::Ref, Types::Xlation);
			Types::Xlation FormArgumentXlate(const Types::Xlation&, const Tokens::CommandArgs::Arg&) const;
			inline void AddCommandName(std::string name, size_t id) {
				m_CommandNames.emplace(name, id);
				m_CommandNameVec.emplace_back(name, id);
			}
			inline int32_t AddCommandName(std::string name) {
				int32_t i = m_CommandNameVec.size();
				m_CommandNames.emplace(name, i);
				m_CommandNameVec.emplace_back(name, i);
				return i;
			}
			DataVal RawifyValue(XMLValue value, size_t size, Types::DataType type) {
				DataVal v;
				v.uint64 = 0;
				switch (type) {
				case Types::DataType::Int:
					if (size <= 8)
						v.uint8 = value.AsNumber<uint8_t>();
					else if (size <= 16)
						v.uint16 = value.AsNumber<uint16_t>();
					else if (size <= 32)
						v.uint32 = value.AsNumber<uint32_t>();
					else
						v.uint64 = value.AsNumber<uint64_t>();
					break;
				case Types::DataType::Float:
					v.flt = value.AsNumber<float>();
					break;
				default: BREAK();
				}
				return v;
			}
			bool OutputValue(DataVal value, size_t size) {
				if (size == 8)
					Output<uint8_t>(value.uint8);
				else if (size == 16)
					Output<uint16_t>(value.uint16);
				else if (size == 32)
					Output<uint32_t>(value.uint32);
				else if (size == 64)
					Output<uint64_t>(value.uint64);
				else return false;
				return true;
			}

		private:
			State m_State;
			Task& m_Task;
			Engine& m_Engine;
			Build* m_Build;
			Build::Xlations::const_iterator m_XlationIt;
			
			Tokens::Storage& m_Tokens;
			Tokens::Iterator m_TokenIt;
			std::ofstream m_File;

			std::map<std::string, int32_t> m_CommandNames;
			std::vector<std::pair<std::string, int32_t>> m_CommandNameVec;
		};

		/*\ Compiler::Event \*/
		enum class Event
		{
			Begin, Finish,
			Warning,
			Error,
		};
		template<typename T>
		struct event : public build_event {
			explicit event(std::string name, const Engine& engine) : build_event(engine) {
				LinkEvent<T>(name);
			}
		};
		struct event_begin : public event<event_begin> {
			event_begin(const Engine& engine) : event("begin", engine)
			{ }
		};
		struct event_finish : public event<event_finish> {
			event_finish(const Engine& engine) : event("finish", engine)
			{ }
		};
		struct event_warning : public event<event_warning> {
			event_warning(const Engine& engine) : event("warning", engine)
			{ }
		};
		template<Error::ID TID, typename... TArgs>
		struct event_error : public error_event_data<TArgs...> {
			event_error(const Engine& engine, TArgs... args) : error_event_data(Basic::Error(engine, TID), std::forward<TArgs>(args)...)
			{ }
		};

		/*\ Compiler::Task \*/
		class Task : public TaskSystem::Task, Compiler {
			friend Compiler;
			Engine& m_Engine;

		public:
			Task(Engine& engine, Build* build) : TaskSystem::Task(build), Compiler(*this, engine, build),
				m_Engine(engine)
			{ }

			inline size_t GetProgressCurrent() const { return GetCurrentToken(); }
			inline size_t GetProgressTotal() const { return GetNumTokens(); }

			bool IsRunning() const { return Compiler::IsRunning(); }
			bool IsTaskFinished() final override { return Compiler::IsFinished(); }

			template<typename TEvent, typename... TArgs>
			inline size_t Event(TArgs&&... args) {
				return CallEvent(TEvent(m_Engine, std::forward<TArgs>(args)...));
			}

		protected:
			void RunTask() final override { Compiler::Run(); }
			void ResetTask() final override { Compiler::Reset(); }
		};
	}
}