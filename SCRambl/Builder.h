/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <unordered_map>
#include "Tasks.h"
#include "Configuration.h"
#include "Scripts.h"
#include "ScriptObjects.h"
#include "BuildConfig.h"
#include "Commands.h"
#include "Operators.h"
#include "Constants.h"
#include "Types.h"

namespace SCRambl
{
	enum class BuildEvent {

	};

	using ScriptVariable = ScriptObject<Variable>;
	using ScriptLabel = ScriptObject<Label>;

	struct BuildVariable {
		XMLValue Value;
		BuildVariable() = default;
		BuildVariable(XMLValue v) : Value(v) { }

		template<typename T> BuildVariable& operator=(const T& v) {
			Value = v;
			return *this;
		}
		template<typename T> BuildVariable& operator+(T v) {
			Value = Value.AsNumber<T>() + v;
			return *this;
		}
		template<typename T> BuildVariable& operator-(T v) {
			Value = Value.AsNumber<T>() - v;
			return *this;
		}
		template<typename T> BuildVariable& operator*(T v) {
			Value = Value.AsNumber<T>() * v;
			return *this;
		}
		template<typename T> BuildVariable& operator/(T v) {
			Value = Value.AsNumber<T>() / v;
			return *this;
		}
		template<typename T> BuildVariable& operator%(T v) {
			Value = Value.AsNumber<T>() % v;
			return *this;
		}
		template<typename T> BuildVariable& operator&(T v) {
			Value = Value.AsNumber<T>() & v;
			return *this;
		}
		template<typename T> BuildVariable& operator|(T v) {
			Value = Value.AsNumber<T>() | v;
			return *this;
		}
		template<typename T> BuildVariable& operator^(T v) {
			Value = Value.AsNumber<T>() & v;
			return *this;
		}
		template<typename T> BuildVariable& operator<<(T v) {
			Value = Value.AsNumber<T>() << v;
			return *this;
		}
		template<typename T> BuildVariable& operator>>(T v) {
			Value = Value.AsNumber<T>() >> v;
			return *this;
		}
		template<typename T> BuildVariable& operator+=(T v) {
			return (*this = *this + v);
		}
		template<typename T> BuildVariable& operator-=(T v) {
			return (*this = *this - v);
		}
		template<typename T> BuildVariable& operator*=(T v) {
			return (*this = *this * v);
		}
		template<typename T> BuildVariable& operator/=(T v) {
			return (*this = *this / v);
		}
		template<typename T> BuildVariable& operator%=(T v) {
			return (*this = *this % v);
		}
		template<typename T> BuildVariable& operator&=(T v) {
			return (*this = *this & v);
		}
		template<typename T> BuildVariable& operator|=(T v) {
			return (*this = *this | v);
		}
		template<typename T> BuildVariable& operator^=(T v) {
			return (*this = *this ^ v);
		}
		template<typename T> BuildVariable& operator<<=(T v) {
			return (*this = *this << v);
		}
		template<typename T> BuildVariable& operator>>=(T v) {
			return (*this = *this >> v);
		}
		BuildVariable& operator++() {
			Value = Value.AsNumber<long long>() + 1;
			return *this;
		}
		BuildVariable operator++(int) {
			BuildVariable v = *this;
			++(*this);
			return v;
		}
		BuildVariable& operator--() {
			Value = Value.AsNumber<long long>() - 1;
			return *this;
		}
		BuildVariable operator--(int) {
			BuildVariable v = *this;
			--(*this);
			return v;
		}
	};

	class BuildEnvironment
	{
		Engine& m_Engine;
		mutable std::map<std::string, BuildVariable> m_Variables;

	public:
		BuildEnvironment(Engine&);

		template<typename T>
		BuildVariable& Set(XMLValue id, const T& v) {
			auto it = m_Variables.find(Val(id).AsString());
			if (it != std::end(m_Variables))
				it->second.Value = Val(v);
			else
				it = m_Variables.emplace(Val(id).AsString(), Val(v)).first;
			return it->second;
		}
		BuildVariable& Get(XMLValue id);
		const BuildVariable& Get(XMLValue id) const;

		XMLValue Val(XMLValue v) const;
	
		void DoAction(const ParseObjectConfig::Action& action, XMLValue v);
	};

	class Build : public TaskSystem::Task<BuildEvent>
	{
		friend class Builder;
		using ConfigMap = std::map<std::string, XMLConfiguration>;

		struct BuildScript {
			std::string ID;
			std::string Output;
			std::vector<std::string> Inputs;

			BuildScript() = default;
			BuildScript(std::string id, std::string output) : ID(id), Output(output)
			{ }
		};
		struct BuildInput {
			std::string ScriptType;
			std::string Value;
			Scripts::FileRef Input;

			BuildInput() = default;
			BuildInput(std::string type, std::string value) : ScriptType(type), Value(value)
			{ }
		};

	public:
		using Symbols = std::vector<TokenSymbol*>;
		using Xlations = std::vector<Types::Xlation>;

		Build(Engine&, BuildConfig*);
		Build(const Build&) = delete;
		~Build();

		Scripts::FileRef AddInput(std::string);
		XMLConfiguration* AddConfig(const std::string& name);
		bool LoadXML(std::string path);
		XMLValue GetEnvVar(std::string var) const;

		bool IsCommandArgParsed(Command*, unsigned long arg_index) const;

		// Script
		inline Script& GetScript() { return m_Script; }
		inline const Script& GetScript() const { return m_Script; }

		// Commands
		inline Commands& GetCommands() { return m_Commands; }
		inline const Commands& GetCommands() const { return m_Commands; }

		// Operators
		inline Operators::Operators& GetOperators() { return m_Operators; }
		inline const Operators::Operators& GetOperators() const { return m_Operators; }

		// Types
		inline Types::Types& GetTypes() { return m_Types; }
		inline const Types::Types& GetTypes() const { return m_Types; }

		// Variables
		inline ScriptObjects<Variable>& GetVariables() { return m_Variables; }
		inline const ScriptObjects<Variable>& GetVariables() const { return m_Variables; }
		inline const ScriptVariable::Scope& OpenVarScope() { return m_Variables.BeginLocal(); }
		inline const ScriptVariable::Scope& CloseVarScope() { return m_Variables.EndLocal(); }
		ScriptVariable* AddScriptVariable(std::string name, Types::Type* type, size_t array_size);
		ScriptVariable* GetScriptVariable(std::string name);

		// Labels
		inline ScriptObjects<Label>& GetLabels() { return m_Labels; }
		inline const ScriptObjects<Label>& GetLabels() const { return m_Labels; }
		ScriptLabel* AddScriptLabel(std::string name, Scripts::Position);
		ScriptLabel* GetScriptLabel(std::string name);
		ScriptLabel* GetScriptLabel(Label* label);

		// "Declarations"
		inline std::vector<TokenSymbol*> & GetDeclarations() { return m_Declarations; }
		inline const std::vector<TokenSymbol*> & GetDeclarations() const { return m_Declarations; }
		void AddDeclaration(TokenSymbol* tok);

		Xlations::const_iterator GetXlationsBegin() const {
			return m_Xlations.begin();
		}
		Xlations::const_iterator GetXlationsEnd() const {
			return m_Xlations.end();
		}
		Symbols::const_iterator GetSymbolsBegin() const {
			return m_Symbols.begin();
		}
		Symbols::const_iterator GetSymbolsEnd() const {
			return m_Symbols.end();
		}

		template<typename TTokenType, typename... TArgs>
		Scripts::Token CreateToken(Scripts::Position pos, TArgs&&... args) {
			return m_Script.GetTokens().Add<TTokenType>(pos, args...);
		}
		template<typename TSymbolType, typename... TArgs>
		TSymbolType* CreateSymbol(TArgs&&... args) {
			auto ptr = new TSymbolType(args...);
			m_Symbols.emplace_back(ptr);
			return ptr;
		}
		VecRef<Types::Xlation> AddSymbol(Types::Translation::Ref translation) {
			m_Xlations.emplace_back(translation, [this](Types::DataSource src, Types::DataAttribute attr)->XMLValue{
				switch (src.ID()) {
				case Types::DataSourceID::Env:
					return m_Env.Get(attr.Name()).Value;
				}
				return "";
			});
			return {m_Xlations, -1};
		}

		void DoParseActions(std::string val, const ParseObjectConfig::ActionVec& vec);
		void ParseCommands(const std::multimap<const std::string, Scripts::Tokens::Iterator>& map);

		template<typename T, typename ID, typename... Params>
		const T* AddTask(ID id, Params&&... prms) {
			auto pr = m_Tasks.emplace(id, std::make_unique<T>(m_Engine, std::forward<Params>(prms)...));
			if (pr.second) {
				auto task = pr.first->second.get();
				if (!m_HaveTask) {
					Init();
					m_HaveTask = true;
				}
				return static_cast<T*>(task);
			}
			return nullptr;
		}
		template<typename ID>
		bool RemoveTask(ID id) {
			if (!m_Tasks.empty()) {
				auto it = std::find(std::begin(m_Tasks), std::end(m_Tasks), id);
				if (it != std::end(Tasks)) {
					delete it->second;
					m_Tasks.erase(it);

					if (m_Tasks.empty()) {
						m_HaveTask = false;
						m_CurrentTask = std::end(m_Tasks);
					}
					return true;
				}
			}
			return false;
		}

		template<typename T>
		inline T& GetCurrentTask() { return *std::static_pointer_cast<T>(CurrentTask->second); }
		inline int GetCurrentTaskID() const { return m_CurrentTask->first; }
		inline size_t GetNumTasks() const { return m_Tasks.size(); }
		inline void ClearTasks() { m_Tasks.clear(); }

		const TaskSystem::Task<BuildEvent>& Run();

	protected:
		bool IsTaskFinished() override { return m_CurrentTask == std::end(m_Tasks); }
		void ResetTask() override { Init(); }
		void RunTask() override { Run(); }

	private:
		Engine& m_Engine;
		Constants m_Constants;
		Commands m_Commands;
		Operators::Operators m_Operators;
		Types::Types m_Types;
		BuildEnvironment m_Env;
		BuildConfig* m_Config;
		ConfigMap m_ConfigMap;

		Script m_Script;
		std::vector<BuildScript> m_BuildScripts;
		std::vector<BuildInput> m_BuildInputs;
		std::vector<std::string> m_Files;
		TokenPtrVec m_Tokens;
		Xlations m_Xlations;
		SymbolPtrVec m_Symbols;
		SymbolPtrVec m_Declarations;

		//
		ScriptObjects<Variable> m_Variables;
		ScriptObjects<Label> m_Labels;
		std::map<Scripts::Position, ScriptLabel*> m_LabelPosMap;

		// Tasks
		using TaskMap = std::map<int, std::unique_ptr<TaskSystem::ITask>>;
		TaskMap m_Tasks;
		TaskMap::iterator m_CurrentTask;
		bool m_HaveTask;

		void Setup();
		void Init();
		void LoadDefinitions();
	};

	class Builder
	{
		Engine& m_Engine;
		XMLConfiguration* m_Configuration;
		XMLConfig* m_Config;
		std::unordered_map<std::string, BuildConfig> m_BuildConfigurations;
		BuildConfig* m_BuildConfig;

	public:
		Builder(Engine&);
		
		Scripts::FileRef LoadFile(Build*, std::string);
		bool LoadDefinitions(Build*);

		bool LoadScriptFile(std::string, Script&);
		bool SetConfig(std::string) const;
		BuildConfig* GetConfig() const;
	};
}