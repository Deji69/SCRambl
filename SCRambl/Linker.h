/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Tasks.h"
#include "Scripts.h"
#include "Compiler.h"

namespace SCRambl
{
	namespace Linking
	{
		class Task;

		class Linker
		{
		public:
			enum State {
				init, linking, finished,
				bad_state, max_state = bad_state,
			};

			Linker(Task& task, Engine& engine, Build* build);

			void Run();

			inline bool IsFinished() const { return m_State == finished; }
			inline bool IsRunning()	const { return m_State == init || m_State == linking; }

		protected:
			void Init();
			void Finish();
			void Reset();
			void Link();

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
			State m_State = init;
			Engine& m_Engine;
			Task& m_Task;
			Build* m_Build;
			std::ifstream m_Input;
			std::ofstream m_File;
		};

		/*\
		 * Linker::Event
		\*/
		enum class Event
		{
			Begin, Finish,
			Warning,
			Error,
		};

		/*\
		 * Linker::Task
		\*/
		class Task : public TaskSystem::Task, private Linker
		{
		public:
			Task(Engine& engine, Build* build) :
				Linker(*this, engine, build),
				m_Engine(engine)
			{ }

			bool IsRunning() const { return Linker::IsRunning(); }
			bool IsTaskFinished() final override { return Linker::IsFinished(); }

		protected:
			void RunTask() final override			{ Linker::Run(); }
			void ResetTask() final override			{ Linker::Reset(); }

		private:
			friend Linker;
			
			Engine& m_Engine;
		};
	}
}