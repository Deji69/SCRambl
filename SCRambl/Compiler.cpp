#include "stdafx.h"
#include "Standard.h"
#include "Compiler.h"
#include "Scripts.h"
#include "TokensB.h"

#include <cctype>

namespace SCRambl
{
	namespace Compiling
	{
		void Compiler::Init() {
			m_TokenIt = m_Tokens.Begin();
			m_XlationIt = m_Build->GetXlationsBegin();
			m_Task(Event::Begin);
			m_State = compiling;

			auto name = m_Build->GetEnvVar("ScriptName").AsString();
			if (name.empty()) {
				BREAK();
				name = "main";
			}
			m_File.open(name + ".scrmbl", std::ios::out | std::ios::binary);
		}
		void Compiler::Reset() {

		}
		void Compiler::Run()
		{
			switch (m_State) {
			case init:
				Init();
				return;
			case compiling:
				if (m_XlationIt != m_Build->GetXlationsEnd())
					Compile();
				else
					Finish();
				return;
			}
		}
		void Compiler::Compile() {
			auto xlate = *m_XlationIt;
			CompileTranslation(xlate.GetTranslation(), xlate);
			++m_XlationIt;
		}
		void Compiler::CompileTranslation(Types::Translation::Ref translation, Types::Xlation xlate) {
			for (size_t y = 0; y < translation->GetDataCount(); ++y) {
				auto data = translation->GetData(y);

				size_t data_size = 0, totes_size = 0;
				Types::DataType data_type = Types::DataType::INVALID;
				DataVal dval;
				dval.uint64 = 0;
				std::string str;
				bool init = true, args = false;

				for (size_t x = 0; x < data->GetNumFields(); !args ? ++x : x = x) {
					auto field = data->GetField(x);
					bool cval = field->GetDataSource() == Types::DataSourceID::None && field->GetDataAttribute() == Types::DataAttributeID::None;
					auto value = xlate.GetAttribute(field->GetDataSource(), field->GetDataAttribute());
					size_t size = field->HasSizeLimit() ? field->GetSizeLimit() : 0;

					bool isstr = false;
					auto type = field->GetDataType();

					switch (type) {
					case Types::DataType::Float:
					case Types::DataType::Fixed:
					case Types::DataType::Int:
						if (cval) {
							value = field->GetValue();
						}
						if (!field->HasSizeLimit()) {
							size = CountBitOccupation(value.AsNumber<size_t>());
							if (size > 32) size = 64;
							else if (size > 16) size = 32;
							else if (size > 8) size = 16;
							else size = 8;
						}
						break;
					case Types::DataType::Char:
						if (cval) {
							value = field->GetValue();
						}
						size = 8;
						break;
					case Types::DataType::String:
						if (cval) {
							str = field->GetValue();
						}
						size = field->HasSizeLimit() ? size : value.AsString().size();
						isstr = true;
						break;
					case Types::DataType::Args:
						args = true;
						break;
					default: BREAK();
					}

					if (args) {
						auto type = m_TokenIt->GetToken()->GetType<Tokens::Type>();
						auto& vec = Tokens::CommandArgs::GetVector(*m_TokenIt->GetToken());
						++m_TokenIt;
						for (auto v : vec) {
							CompileTranslation(v.second->GetTranslation(), FormArgumentXlate(xlate, v));
						}
						args = false;
						continue;
						BREAK();
					}

					size_t valsize = 0;
					if (!isstr) {
						if (size > 64) BREAK();
						else if (size > 32)
							valsize = 64;
						else if (size > 16)
							valsize = 32;
						else if (size > 8)
							valsize = 16;
						else
							valsize = 8;
					}
					else valsize = size;

					if (init) {
						data_size = valsize;

						if (data_size == size) {
							if (isstr) {
								str = value.AsString();
								Output(str.c_str(), str.size());
								for (auto i = str.size(); i < data_size; ++i) {
									Output<uint8_t>(0);
								}
							}
							else {
								OutputValue(RawifyValue(value, data_size, type), data_size);
							}
							continue;
						}

						init = false;
						dval.uint64 = 0;
						totes_size = 0;
					}

					size_t req_size = totes_size + size,
						left_size = data_size - totes_size;
					if (left_size < size) {
						bool success = OutputValue(dval, data_size);
						ASSERT(success);
						ASSERT(!init);
						totes_size = size;
						if (!isstr) {
							dval = RawifyValue(value, valsize, type);
						}
						else str = value.AsString().substr(0, size);
						continue;
					}
					else {
						if (!isstr) {
							if (!totes_size) {
								dval = RawifyValue(value, valsize, type);
							}
							else {
								dval.uint64 |= RawifyValue(value, valsize, type).uint64 << totes_size;
							}

							if (size != left_size)
								totes_size += size;
							else
							{
								OutputValue(dval, data_size);
								init = true;
							}
						}
						else str = value.AsString().substr(0, size);
					}
				}
			}
		}
		Types::Xlation Compiler::FormArgumentXlate(const Types::Xlation& xlate, const Tokens::CommandArgs::Arg& arg) const {
			Types::Xlation r = xlate;
			r.SetAttributes(Types::DataSourceID::Number, arg.first.GetNumberAttributes());
			r.SetAttributes(Types::DataSourceID::Text, arg.first.GetTextAttributes());
			return r;
		}
		void Compiler::Finish() {
			m_Task(Event::Finish);
			m_State = finished;
			m_File.close();
		}

		Compiler::Compiler(Task& task, Engine& engine, Build* build) :
			m_State(init), m_Task(task), m_Engine(engine), m_Build(build), m_Tokens(build->GetScript().GetParseTokens())
		{ }
	}
}