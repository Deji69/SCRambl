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
#include "Tokens.h"
#include "Standard.h"

namespace SCRambl
{
	enum class BuildEvent {

	};
	enum class BuildSymbolType {
		None, Command, Data
	};

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

	/* build events */
	struct build_event : public task_event {
		explicit build_event(const Engine& engine) : m_Engine(engine) {
			LinkEvent<build_event>("build_event");
		}

	private:
		const Engine& m_Engine;
	};
	struct token_event : public build_event {
	public:
		explicit token_event(const Engine& engine, Scripts::Range rg) : build_event(engine), m_ScriptRange(rg) {
			LinkEvent<token_event>("token_event");
		};
		inline Scripts::Range TokenRange() const { return m_ScriptRange; }

	private:
		Scripts::Range m_ScriptRange;
	};
	struct error_event : public build_event {
		explicit error_event(const Engine& engine, Basic::Error&& error, std::string params) : build_event(engine), Error(std::move(error)) {
			LinkEvent<error_event>("error_event");
			Params.emplace_back(params);
		}
		explicit error_event(const Engine& engine, Basic::Error&& error, std::vector<std::string> params) : build_event(engine), Error(std::move(error)), Params(params) {
			LinkEvent<error_event>("error_event");
		}

		Basic::Error Error;
		std::vector<std::string> Params;
	};
	template<typename... TArgs>
	struct error_event_data : public error_event {
	protected:
		explicit error_event_data(Basic::Error&& error, TArgs&&... args) : error_event(error.GetEngine(), std::forward<Basic::Error>(error), error.GetEngine().FormatVec(args...))
		{ }

		std::tuple<TArgs...> Data;
	};
	struct event_added_token : public token_event {
	public:
		explicit event_added_token(const Engine& engine, Scripts::Range rg) : token_event(engine, rg) {
			LinkEvent<event_added_token>("added_token");
		};
	};
	struct event_parsed_token : public token_event {
	public:
		explicit event_parsed_token(const Engine& engine, Scripts::Range rg) : token_event(engine, rg) {
			LinkEvent<event_parsed_token>("parsed_token");
		}
	};

	class BuildEnvironment {
		Engine& m_Engine;
		mutable std::map<std::string, BuildVariable> m_Variables;

	public:
		BuildEnvironment(Engine&);

		BuildVariable& Set(XMLValue id, XMLValue v);
		BuildVariable& Get(XMLValue id);
		const BuildVariable& Get(XMLValue id) const;

		XMLValue Val(XMLValue v) const;
	
		void DoAction(const ParseObjectConfig::Action& action, XMLValue v);
	};
	class BuildCommand {
	public:
		BuildCommand(BuildEnvironment& env, VecRef<ParseObjectConfig::Action> act, XMLValue val) : m_Environment(env), m_Action(act), m_Value(val)
		{ }

	private:
		BuildEnvironment& m_Environment;
		VecRef<ParseObjectConfig::Action> m_Action;
		XMLValue m_Value;
	};
	class BuildSymbol {
	public:
		BuildSymbol(BuildEnvironment& env) : m_Env(env)
		{ }

	protected:
		inline BuildEnvironment& Environment() const { return m_Env; }

	private:
		BuildEnvironment& m_Env;
	};
	class Build : public TaskSystem::Task {
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
		inline Operators::Master& GetOperators() { return m_Operators; }
		inline const Operators::Master& GetOperators() const { return m_Operators; }

		// Types
		inline Types::Types& GetTypes() { return m_Types; }
		inline const Types::Types& GetTypes() const { return m_Types; }

		// Variables
		inline ScriptObjects<Variable>& GetVariables() { return m_Variables; }
		inline const ScriptObjects<Variable>& GetVariables() const { return m_Variables; }
		inline const ScriptVariable::Scope& OpenVarScope() { return m_Variables.BeginLocal(); }
		inline const ScriptVariable::Scope& CloseVarScope() { return m_Variables.EndLocal(); }
		inline const ScriptLabel::Scope& OpenLabelScope() { return m_Labels.BeginLocal(); }
		inline const ScriptLabel::Scope& CloseLabelScope() { return m_Labels.EndLocal(); }
		ScriptVariable* AddScriptVariable(std::string name, VecRef<Types::Type>, size_t array_size);
		ScriptVariable* GetScriptVariable(std::string);

		// Labels
		inline ScriptObjects<Label>& GetLabels() { return m_Labels; }
		inline const ScriptObjects<Label>& GetLabels() const { return m_Labels; }
		ScriptLabel* AddScriptLabel(std::string name, size_t offset = -1);
		ScriptLabel* GetScriptLabel(std::string);
		ScriptLabel* GetScriptLabel(Label*);

		Xlations::const_iterator GetXlationsBegin() const {
			return m_Xlations.begin();
		}
		Xlations::const_iterator GetXlationsEnd() const {
			return m_Xlations.end();
		}

		template<typename TTokenType, typename... TArgs>
		VecRef<Tokens::Token> CreateToken(Scripts::Range range, TArgs&&... args) {
			auto token = m_Script.GetTokens().Add<TTokenType>(range.Begin(), args...);
			m_CurrentTask->second->CallEvent(event_added_token(m_Engine, range));
			return token;
		}
		template<typename TTokenType, typename... TArgs>
		VecRef<Tokens::Token> ParseToken(Scripts::Range range, TArgs&&... args) {
			auto token = m_Script.GetParseTokens().Add<TTokenType>(range.Begin(), args...);
			m_CurrentTask->second->CallEvent(event_parsed_token(m_Engine, range));
			return token;
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
		void ParseCommands(const std::multimap<const std::string, Tokens::Iterator>& map);

		template<typename T, typename ID, typename... Params>
		T* AddTask(ID id, Params&&... prms) {
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
		inline int GetCurrentTaskID() const { return !IsTaskFinished() ? m_CurrentTask->first : -1; }
		inline size_t GetNumTasks() const { return m_Tasks.size(); }
		inline void ClearTasks() { m_Tasks.clear(); }

		const TaskSystem::Task& Run();

	protected:
		bool IsTaskFinished() const override { return m_CurrentTask == std::end(m_Tasks); }
		void ResetTask() override { Init(); }
		void RunTask() override { Run(); }

	private:
		void Setup();
		void Init();
		void LoadDefinitions();

		Engine& m_Engine;
		Constants m_Constants;
		Commands m_Commands;
		Operators::Master m_Operators;
		Types::Types m_Types;
		BuildEnvironment m_Env;
		BuildConfig* m_Config;
		ConfigMap m_ConfigMap;

		Script m_Script;
		std::vector<BuildScript> m_BuildScripts;
		std::vector<BuildInput> m_BuildInputs;
		std::vector<std::string> m_Files;
		Xlations m_Xlations;

		//
		ScriptObjects<Variable> m_Variables;
		ScriptObjects<Label> m_Labels;
		std::map<Scripts::Position, ScriptLabel*> m_LabelPosMap;

		// Tasks
		using TaskMap = std::map<int, std::unique_ptr<TaskSystem::Task>>;
		TaskMap m_Tasks;
		TaskMap::iterator m_CurrentTask;
		bool m_HaveTask;
	};
	class Builder {
	public:
		Builder(Engine&);
		
		Scripts::FileRef LoadFile(Build*, std::string);
		bool LoadDefinitions(Build*);
		bool LoadScriptFile(std::string, Script&);
		bool SetConfig(std::string) const;
		BuildConfig* GetConfig() const;

	private:
		Engine& m_Engine;
		XMLConfiguration* m_Configuration = nullptr;
		XMLConfig* m_Config = nullptr;
		BuildConfig* m_BuildConfig = nullptr;
		std::unordered_map<std::string, BuildConfig> m_BuildConfigurations;
	};
}