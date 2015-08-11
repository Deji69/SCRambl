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

		class Compiler
		{
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
			void OutputValue(XMLValue value, size_t size) {
				if (size <= 8)
					Output<uint8_t>(value.AsNumber<uint8_t>());
				else if (size <= 16)
					Output<uint16_t>(value.AsNumber<uint16_t>());
				else if (size <= 32)
					Output<uint32_t>(value.AsNumber<uint32_t>());
				else
					Output<uint64_t>(value.AsNumber<uint64_t>());
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
			Task(Engine& engine, Build* build) : Compiler(*this, engine, build),
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