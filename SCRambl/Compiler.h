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
	namespace Compiler
	{
		class Task;

		class Compiler
		{
		public:
			enum State {
				init, compiling, finished,
				bad_state, max_state = bad_state,
			};

			Compiler(Task& task, Engine& engine, Build::Shared build);

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
				return m_TokenIt.GetIndex();
			}
			inline Scripts::Token GetToken() const {
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
			State m_State;
			Task& m_Task;
			Engine& m_Engine;
			Build::Shared m_Build;
			Scripts::Tokens& m_Tokens;
			Scripts::Tokens::Iterator m_TokenIt;
			std::ofstream m_File;

			std::map<std::string, size_t> m_CommandNames;
		};

		/*\
		 * Compiler::Event
		\*/
		enum class Event
		{
			Begin, Finish,
			Warning,
			Error,
		};

		/*\
		 * Compiler::Task
		\*/
		class Task : public TaskSystem::Task<Event>, private Compiler
		{
			friend Compiler;
			Engine& m_Engine;

			inline bool operator()(Event id) { return CallEventHandler(id); }
			template<typename... Args>
			inline bool operator()(Event id, Args&&... args) { return CallEventHandler(id, std::forward<Args>(args)...); }

		public:
			Task(Engine& engine, Build::Shared build) :
				Compiler(*this, engine, build),
				m_Engine(engine)
			{ }

			inline size_t GetProgressCurrent() const { return GetCurrentToken(); }
			inline size_t GetProgressTotal() const { return GetNumTokens(); }
			//inline Scripts::Token GetToken() const { return Compiler::GetToken(); }

			bool IsRunning() const { return Compiler::IsRunning(); }
			bool IsTaskFinished() final override { return Compiler::IsFinished(); }

		protected:
			void RunTask() final override { Compiler::Run(); }
			void ResetTask() final override { Compiler::Reset(); }
		};
	}
}