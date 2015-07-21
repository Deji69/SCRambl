/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <list>
#include <memory>
#include <map>
#include <iostream>
#include "utils.h"
#include "Builder.h"
#include "Tasks.h"
#include "Formatter.h"
#include "Reporting.h"
#include "Configuration.h"
#include "Commands.h"
#include "Types.h"
#include "Constants.h"
#include "Scripts.h"

namespace SCRambl
{
	enum class EngineEvent {
		ConfigurationError,
	};

	class Engine
	{
		using TaskMap = std::map<int, std::unique_ptr<TaskSystem::ITask>>;
		using FormatMap = std::map<const std::type_info*, std::unique_ptr<IFormatter>>;
		using ConfigMap = std::map<std::string, XMLConfiguration>;

		// Configuration
		ConfigMap m_Config;

		// BuildSystem
		Builder	m_Builder;

		// Message formatting
		FormatMap Formatters;
		
	public:
		Engine();
		virtual ~Engine();

		// Initiate the build
		Build* InitBuild(std::vector<std::string> files);
		// Free the build
		void FreeBuild(Build*);
		// Run the build
		bool BuildScript(Build*);
		// Load XML configuration/definition file
		bool LoadXML(const std::string& path);

		// Obtain current build configuration
		BuildConfig* GetBuildConfig() const;
		// Set current build configuration
		bool SetBuildConfig(const std::string& name);

		// Add configuration (name = XML section)
		XMLConfiguration* AddConfig(const std::string& name);

		// Load file based on extension and type
		bool LoadFile(const std::string& path, Script& script);
		// Load build XML file and apply specified build config
		bool LoadBuildFile(const std::string& path, const std::string& buildConfig = "");
		
		// Set, override or cancel override of a string formatter (T = source type)
		template<typename T, typename F> inline void SetFormatter(F& func) {
			Formatters[&typeid(T)] = std::make_unique<Formatter<T>>(func);
		}

		// SCRambl string formatting for generic and SCRambl types
		template<typename T>
		inline std::string Format(const T& param) const {
			if (!Formatters.empty()) {
				auto k = &typeid(T);
				auto it = Formatters.find(&typeid(T));
				if (it != Formatters.end()) {
					return it->second->Qualify<T>()(param);
				}
			}
			return "";
		}

		// Specialisations for easy generic types
		template<> inline std::string Format(const std::string& param) const { return param; }
		template<> inline std::string Format(const int& param) const { return std::to_string(param); }
		template<> inline std::string Format(const unsigned int& param) const { return std::to_string(param); }
		template<> inline std::string Format(const long& param) const { return std::to_string(param); }
		template<> inline std::string Format(const unsigned long& param) const { return std::to_string(param); }
		template<> inline std::string Format(const long long& param) const { return std::to_string(param); }
		template<> inline std::string Format(const unsigned long long& param) const { return std::to_string(param); }
		template<> inline std::string Format(const float& param) const { return std::to_string(param); }
		template<> inline std::string Format(const double& param) const { return std::to_string(param); }
		template<> inline std::string Format(const long double& param) const { return std::to_string(param); }

		// String format multiple types (sprintf)
		template<typename First, typename... Args>
		inline void Format(std::vector<std::string>& out, First&& first, Args&&... args) {
			// do one
			out.push_back(Format(first));
			// continue
			Format(out, args...);
		}
		
	private:
		// String format multiple types (sprintf) (pt2)
		template<typename Last>
		inline void Format(std::vector<std::string>& out, Last&& last) {
			// finale
			out.emplace_back(Format(std::forward<Last>(last)));
		}
	};
}