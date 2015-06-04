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
#include "Constants.h"
#include "Types.h"

namespace SCRambl
{
	enum class BuildEvent {

	};

	using ScriptVariable = ScriptObject<Variable>;
	using ScriptLabel = ScriptObject<Scripts::Label>;

	template<typename TTokenType>
	TTokenType* CreateToken() {

	}

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
		BuildVariable& Get(XMLValue id) {
			return m_Variables[Val(id).AsString()];
		}
		const BuildVariable& Get(XMLValue id) const {
			return m_Variables[Val(id).AsString()];
		}

		XMLValue Val(XMLValue v) const;
	
		void DoAction(const ParseObjectConfig::Action& action, XMLValue v) {
			using ActionType = ParseObjectConfig::ActionType;
			switch (action.Type) {
			case ActionType::Clear:
				Set(action.Var, "");
				break;
			case ActionType::Set:
				Set(action.Var, v);
				break;
			case ActionType::Inc:
				++Get(action.Var);
				break;
			case ActionType::Dec:
				++Get(action.Var);
				break;
			case ActionType::Add:
				Get(action.Var) += XMLValue(Val(v)).AsNumber<long long>();
				break;
			case ActionType::Sub:
				Get(Val(action.Var)) -= XMLValue(Val(v)).AsNumber<long long>();
				break;
			case ActionType::Mul:
				Get(Val(action.Var)) *= XMLValue(Val(v)).AsNumber<long long>();
				break;
			case ActionType::Div:
				Get(Val(action.Var)) /= XMLValue(Val(v)).AsNumber<long long>();
				break;
			case ActionType::Mod:
				Get(Val(action.Var)) %= XMLValue(Val(v)).AsNumber<long long>();
				break;
			case ActionType::And:
				Get(Val(action.Var)) &= XMLValue(Val(v)).AsNumber<long long>();
				break;
			case ActionType::Or:
				Get(Val(action.Var)) |= XMLValue(Val(v)).AsNumber<long long>();
				break;
			case ActionType::Xor:
				Get(Val(action.Var)) ^= XMLValue(Val(v)).AsNumber<long long>();
				break;
			case ActionType::Shl:
				Get(Val(action.Var)) <<= XMLValue(Val(v)).AsNumber<long long>();
				break;
			case ActionType::Shr:
				Get(Val(action.Var)) >>= XMLValue(Val(v)).AsNumber<long long>();
				break;
			}
		}
	};

	class Build : public TaskSystem::Task<BuildEvent>
	{
		friend class Builder;

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
			Scripts::File::Shared Input;

			BuildInput() = default;
			BuildInput(std::string type, std::string value) : ScriptType(type), Value(value)
			{ }
		};

		using ConfigMap = std::map<std::string, XMLConfiguration>;

		Engine& m_Engine;
		Constants m_Constants;
		Commands m_Commands;
		Types::Types m_Types;
		BuildEnvironment m_Env;
		BuildConfig* m_Config;
		ConfigMap m_ConfigMap;

		Script m_Script;
		std::vector<BuildScript> m_BuildScripts;
		std::vector<BuildInput> m_BuildInputs;
		std::vector<std::string> m_Files;
		std::vector<IToken*> m_Tokens;
		std::vector<Tokens::Symbol*> m_Symbols;

		//
		ScriptObjects<Variable> m_Variables;
		ScriptObjects<Scripts::Label> m_Labels;

		// Tasks
		using TaskMap = std::map<int, std::shared_ptr<TaskSystem::ITask>>;
		TaskMap m_Tasks;
		TaskMap::iterator m_CurrentTask;
		bool m_HaveTask;

		void Init();
		void LoadDefinitions();

	public:
		using Shared = std::shared_ptr<Build>;

		Build(Engine&, BuildConfig*);
		~Build() {
			for (auto ptr : m_Symbols) {
				delete ptr;
			}
		}

		Scripts::File::Shared AddInput(std::string);
		XMLConfiguration* AddConfig(const std::string& name);
		bool LoadXML(std::string path);
		XMLValue GetEnvVar(std::string var) const { return m_Env.Get(var).Value; }

		bool IsCommandArgParsed(Command*, unsigned long arg_index) const;

		// Script
		inline Script& GetScript() { return m_Script; }
		inline const Script& GetScript() const { return m_Script; }

		// Commands
		inline Commands& GetCommands() { return m_Commands; }
		inline const Commands& GetCommands() const { return m_Commands; }

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
		inline ScriptObjects<Scripts::Label>& GetLabels() { return m_Labels; }
		inline const ScriptObjects<Scripts::Label>& GetLabels() const { return m_Labels; }
		ScriptLabel* AddScriptLabel(Types::Type* type, std::string name) {
			return m_Labels.Add(type, name);
		}
		ScriptLabel* GetScriptLabel(std::string name) {
			return m_Labels.Find(name);
		}
		ScriptLabel* GetScriptLabel(Scripts::Label* label) {
			return m_Labels.Find(label);
		}

		template<typename TTokenType, typename... TArgs>
		Scripts::Token* CreateToken(Scripts::Position pos, TArgs&&... args) {
			return m_Script.GetTokens().Add<TTokenType>(pos, args...);
		}
		template<typename TSymbolType, typename... TArgs>
		TSymbolType* CreateSymbol(TArgs&&... args) {
			auto ptr = new TSymbolType(args...);
			m_Symbols.emplace_back(ptr);
			return ptr;
		}

		void DoParseActions(std::string val, const ParseObjectConfig::ActionVec& vec) {
			for (auto& action : vec) {
				m_Env.DoAction(action, val);
			}
		}

		//template<typename TMap = std::multimap<const std::string, Scripts::Tokens::Iterator>>
		//void ParseCommands(const TMap& map) {
		void ParseCommands(const std::multimap<const std::string, Scripts::Tokens::Iterator>& map) {
			for (auto& parsecmd : m_Config->GetParseCommands()) {
				auto v = m_Env.Val(parsecmd.first).AsString();
				auto rg = map.equal_range(v);
				if (rg.first != rg.second) {
					for (auto it = rg.first; it != rg.second; ++it) {
						bool check_arg = false, check_type = false;
						bool found_type = false;
						auto type = m_Env.Val(parsecmd.second->Type).AsString();
						if (!type.empty()) {
							check_type = true;
						}

						auto argit = it->second;
						++argit;
						auto& cmdtok = it->second->GetToken()->Get<Tokens::Command::Info<Command>>();
						auto cmd = cmdtok.GetValue<Tokens::Command::CommandType>();
						for (unsigned long i = 0; i < cmd->GetNumArgs(); ++i, ++argit) {
							if (cmd->GetArg(i).GetType()->GetName() == type) {
								found_type = true;
								check_type = false;
								break;
							}
						}

						if (!check_type && !check_arg) {
							if (found_type) {
								auto& tok = argit->GetToken()->Get<Tokens::Identifier::Info<>>();
								auto range = tok.GetValue<Tokens::Identifier::ScriptRange>();
								DoParseActions(range.Format(), parsecmd.second->Actions);
							}
						}
					}
				}
				else if (parsecmd.second->Required.AsBool()) {
					BREAK();
				}
			}
		}

		template<typename T, typename ID, typename... Params>
		const std::shared_ptr<T> AddTask(ID id, Params&&... prms) {
			auto task = std::shared_ptr<T>(new T(m_Engine, std::forward<Params>(prms)...));
			m_Tasks.emplace(id, task);
			if (!m_HaveTask) {
				Init();
				m_HaveTask = true;
			}
			return task;
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
		inline T & GetCurrentTask() { return reinterpret_cast<T&>(*CurrentTask->second); }
		inline int GetCurrentTaskID() const { return std::ref(m_CurrentTask->first); }
		inline size_t GetNumTasks() const { return m_Tasks.size(); }
		inline void ClearTasks() { m_Tasks.clear(); }

		const TaskSystem::Task<BuildEvent> & Run();

	protected:
		bool IsTaskFinished() override { return m_CurrentTask == std::end(m_Tasks); }
		void ResetTask() override { Init(); }
		void RunTask() override { Run(); }
	};

	class Builder
	{
		Engine& m_Engine;
		XMLConfiguration* m_Configuration;
		XMLConfig& m_Config;
		std::unordered_map<std::string, BuildConfig> m_BuildConfigurations;
		BuildConfig* m_BuildConfig;

	public:
		Builder(Engine&);
		
		Scripts::File::Shared LoadFile(Build*, std::string);
		bool LoadDefinitions(Build*);

		bool LoadScriptFile(std::string, Script&);
		bool SetConfig(std::string) const { return true; }
		BuildConfig* GetConfig() const { return m_BuildConfig; }
	};
}