/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Configuration.h"

namespace SCRambl
{
	class Engine;

	namespace Constructing
	{
		class DataCode {
		public:
			virtual ~DataCode() = default;

		private:
		};

		class CommandCode : public DataCode {
		public:
			enum class ConditionType {
				Name
			};

		public:
			CommandCode() { }

			void AddCondition(ConditionType type, XMLValue value) {
				m_Conditions.emplace_back(type, value);
			}

		private:
			std::vector<std::pair<ConditionType, XMLValue>> m_Conditions;
			std::vector<std::string> m_Args;
		};

		enum class DataPosition {
			Block, Conditions,
		};

		class Data {
		public:
			template<typename T, typename... TArgs>
			inline T& AddCode(TArgs&&... args) {
				m_Code.emplace_back(std::make_unique<T>(std::forward<TArgs>(args)...));
				return *static_cast<T*>(m_Code.back().get());
			}

		private:
			std::vector<std::unique_ptr<DataCode>> m_Code;
		};

		class ConditionList {
		public:
			void SetMinConditions(size_t v) { m_Min = v; }
			void SetMaxConditions(size_t v) { m_Max = v; }
			void SetDisableMixedLogic(bool b) { m_DisableMixedLogic = b; }

		private:
			size_t	m_Min = 0,
					m_Max = 0;
			bool	m_DisableMixedLogic = false;
		};

		class Block {
		public:
			Block(std::string begin, std::string end, bool repeat = false) : m_Begin(begin), m_End(end), m_Repeatable(repeat)
			{ }
			Block(const Block& v) = delete;
			Block(Block&& v) : m_Begin(v.m_Begin), m_End(v.m_End),
				m_Blocks(std::move(v.m_Blocks)), m_Default(v.m_Default), m_Repeatable(v.m_Repeatable),
				m_Data(std::move(v.m_Data))
			{
			}

			Block& operator=(const Block&) = delete;
			Block& operator=(Block&& v) {
				if (this != &v) {
					m_Begin = v.m_Begin;
					m_End = v.m_End;
					m_Blocks = std::move(v.m_Blocks);
					m_Default = v.m_Default;
					m_Repeatable = v.m_Repeatable;
					m_Data = std::move(v.m_Data);
				}
				return *this;
			}

			template<typename... TArgs>
			inline Block& AddBlock(TArgs&&... args) {
				m_Blocks.emplace_back(std::make_unique<Block>(std::forward<TArgs>(args)...));
				return *m_Blocks.back();
			}

			inline Data& AddData(DataPosition pos, bool before = false) {
				auto it = m_Data.emplace(pos, std::make_pair(std::make_unique<Data>(), before));
				return *it->second.first;
			}
			size_t GetDataAt(std::vector<Data*>& vec, DataPosition pos, bool before = false) {
				auto rg = m_Data.equal_range(pos);
				size_t n = 0;
				for (auto it = rg.first; it != rg.second; ++it) {
					if (it->second.second == before) {
						++n;
						vec.push_back(it->second.first.get());
					}
				}
				return n;
			}

			inline ConditionList& AddConditionList() {
				ASSERT(!m_ConditionList);
				if (!m_ConditionList)
					m_ConditionList = std::make_unique<ConditionList>();
				return *m_ConditionList.get();
			}
			inline ConditionList& GetConditionList() const {
				return *m_ConditionList;
			}
			inline bool HasConditionList() const { return m_ConditionList != false; }

		private:
			std::string m_Begin = "{", m_End = "}";
			std::vector<std::unique_ptr<Block>> m_Blocks;
			std::multimap<DataPosition, std::pair<std::unique_ptr<Data>, bool>> m_Data;
			std::unique_ptr<ConditionList> m_ConditionList;
			bool m_Default = false,
				 m_Repeatable = false;
		};

		class Construct {
		public:
			Construct(std::string name) : m_Name(name)
			{ }
			Construct(const Construct&) = delete;

			template<typename... TArgs>
			Block& AddBlock(TArgs&&... args) {
				m_Blocks.emplace_back(std::make_unique<Block>(std::forward<TArgs>(args)...));
				return *m_Blocks.back();
			}
			size_t NumBlocks() const { return m_Blocks.size(); }
			Block& GetBlock(size_t i) const { return *m_Blocks[i]; }

			inline const std::string& Name() const { return m_Name; }

		private:
			std::string m_Name;
			std::vector<std::unique_ptr<Block>> m_Blocks;
		};

		class Constructs {
			using Map = std::unordered_multimap<std::string, std::unique_ptr<Construct>>;

		public:
			Constructs();
			Constructs(const Constructs&) = delete;

			void Init(Build& build);

			Construct* AddConstruct(std::string name) {
				auto pr = m_Constructs.emplace(name, std::make_unique<Construct>(name));
				return pr->second.get();
			}
			Construct* GetConstruct(std::string name) const {
				auto it = m_Constructs.find(name);
				return it != m_Constructs.end() ? it->second.get() : nullptr;
			}

		private:
			void AddDataConfig(XMLConfig*, const char *);

		private:
			Map m_Constructs;
			XMLConfiguration* m_Config;
		};
	}
}