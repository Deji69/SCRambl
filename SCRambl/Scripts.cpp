#include "stdafx.h"
#include "Parser.h"
#include "Scripts.h"
#include "Directives.h"
#include "Literals.h"
#include "Expressions.h"

namespace SCRambl
{
	bool Script::Preprocess()
	{
		try
		{
			for (auto it = m_Code.begin(); it != m_Code.end(); ++it)
			{
				auto & ln = *it;

				try
				{
					//PreprocessLine(ln);

					// we're keeping blank lines, merely so we know the line numbers for errors
					// we've allocated the useless stuff already - the damage is done :)
					/*// Remove an unnecessary line
					if (ln.empty())
					{
					auto new_it = std::prev(it);
					m_Code.erase(it);
					it = new_it;
					}*/
				}
				catch (...)
				{
				}
			}
			return true;
		}
		catch (...)
		{
			
		}
		return false;
	}

	void Script::Init()
	{
		m_nCommentDepth = 0;
		m_PreprocessorHistory.push_back(true);
	}

	void Script::Init(const CodeList & code)
	{
		m_Code = code;
		Init();
	}

#if FALSE
	void Script::PreprocessLine(ScriptLine & line)
	{
		// initialise the comment offset, as we might be already in a comment from a previous line
		// if that comment ends (and it wasn't nested), remove up to the end of it on this line.
		// the main preprocessor handles completely removing lines that are commented out
		size_t comment_offset = 0;
		std::string & code = line;

		bool is_separated = false;

		/* Phase 1 */

		// look for comments, macros to replace, etc.
		for (size_t i = 0; i < code.length(); ++i)
		{
			// Check for comment
			if (i < code.length() - 1)
			{
				if (!m_nCommentDepth || code[i + 1] == '*')
				{
					if (code[i] == '/')
					{
						if (!m_nCommentDepth && code[i + 1] == '/')
						{
							code.erase(i);
							continue;
						}
						else if (m_nCommentDepth || code[i + 1] == '*')
						{
							// nested comments are allowed, so we need to keep track of where the topmost comment started
							if (!m_nCommentDepth)
								comment_offset = i;

							++m_nCommentDepth;
							++i;
							continue;
						}
					}
				}
				else
				{
					if (code[i] == '*')
					{
						if (code[i + 1] == '/')
						{
							--m_nCommentDepth;

							// If this comment wasn't nested, remove it and everything inside
							if (!m_nCommentDepth)
							{
								// erase all from /* to */ on this line
								code.erase(comment_offset, i + 2);

								// go back to the comment beginning offset, as the comment was removed anyway
								i = comment_offset;
							}
							continue;
						}
					}
				}
			}

			if (!InComment())
			{
				// un-necessary spaces are annoying, get rid of them now to optimise parsing
				if (IsSpace(code[i]))
				{
					if (is_separated || (i < code.length() - 1 && IsSeparator(code[i + 1])))
					{
						code.erase(i, 1);
						--i;
					}
				}

				if (IsSeparator(code[i]))
					is_separated = true;
				else
					is_separated = false;
			}
		}

		// If the block comment hasn't ended by the end of the line, remove everything up to the end
		// and any lines after this one which dont terminate the comment will be completely removed
		if (m_nCommentDepth)
			code.erase(comment_offset, code.length());

		// There might not even be anything left by now...
		if (!code.empty())
		{
			// Trim excess whitespaces
			code = trim(code);

			// Check for directive
			if (IsDirectivePrefix(code[0]) && code.length() > 1)
			{
				if (!IsIdentifierStart(code[1]))
				{
					std::string err("");
					Error(SCR_BAD_DIRECTIVE_CHAR, err);
				}

				//std::string dir(code.begin(), std::find_if(code.begin()+1, code.end(), IsIdentifier));
				std::string dir = GetIdentifier(code.substr(1));
				std::string def(code.begin() + 1 + dir.length(), code.end());

				if (dir.empty())
				{
					// error
				}
				if (def.empty())
				{
					// error
				}
				
				switch (GetDirective(dir))
				{
				case DIRECTIVE_DEFINE:
					{
						std::string name = GetIdentifier(def);
						std::string val = ltrim(def).substr(name.length());
						Macros().Define(name, val);
					}
					break;
				case DIRECTIVE_IFDEF:
					{
						std::string name = GetIdentifier(def);
						PushSourceControl(Macros().Get(name) != nullptr);
					}
					break;
				case DIRECTIVE_IF:
					PushSourceControl(EvaluateExpression(ltrim(def)) != 0);
					break;
				case DIRECTIVE_ELIF:
					if (!GetSourceControl() && EvaluateExpression(def))
						InvertSourceControl();
					break;
				case DIRECTIVE_ELSE:
					InvertSourceControl();
					break;
				case DIRECTIVE_ENDIF:
					PopSourceControl();
					break;
				}

				//code.find()
				//identifier = code.substr(code)

				// done, prevent it from being parsed
				code = "";
			}
		}
	}
#endif

	void Script::Error(int code, const std::string & msg)
	{
		//ScriptError()

	}

#if FALSE
	long long Script::EvaluateExpression(const std::string & expr, int depth)
	{
		IntExpression<long long, unsigned long long> eval;

		int nStartDepth = depth;

		for (size_t i = 0; i < expr.length(); ++i)
		{
			char c = expr[i];
			char next = (i + 1) < expr.length() ? expr[i + 1] : 0;
			if (next && IsSpace(next)) next = 0;

			// Nothings
			if (IsSpace(c)) continue;

			// Identifiers
			if (IsIdentifierStart(c))
			{
				auto id = GetIdentifier(expr.substr(i));
				i += id.length();

				if (auto pCode = Macros().Get(id))
				{
					EvaluateExpression(*pCode, depth);
				}
				else
				{
					// ERROR ?
				}
				continue;
			}

			// Symbols
			switch (c)
			{
			case '(':
			{
				auto n = expr.find_first_of(')', i + 1);
				eval.Evaluate(EvaluateExpression(expr.substr(i + 1, n - 1), depth + 1));
				i += n;
				continue;
			}
			case ')':
				if (depth) --depth;
				return eval.Result<long long>();

				// Operators
			case '!':
				eval.Logicate(EXPR_LOG_NOT);
				continue;
			case '+':
				if (next == '+')
					Error(SCR_BAD_CONSTEXPR_OPERATOR, "invalid operator '++' in constant expression");
				eval.Operate(EXPR_OP_ADD);
				continue;
			case '-':
				if (next == '-')
					Error(SCR_BAD_CONSTEXPR_OPERATOR, "invalid operator '--' in constant expression");
				eval.Operate(EXPR_OP_SUBTRACT);
				continue;
			case '*':
				continue;
			}

			// Numeros
			try
			{
				IntConst value(expr.substr(i));
				i += value.Pos();
				if (IsIdentifier(expr[i]))
				{
					switch (expr[i])
					{
					case 'U':
						eval.Evaluate(value.Value<unsigned long long>());
						break;
					default:
						Error(SCR_BAD_NUMBER_SUFFIX, "bad number suffix '" + std::string(&expr[i], &expr[i + 1]) + "'");
						break;
					}
				}
				else eval.Evaluate(value.Value<long long>());
				continue;
			}
			catch (...)
			{
			}
		}


		if (nStartDepth != depth)
		{
			// ERROR
		}

		return eval.Result<long long>();
	}

	void Script::LoadFile(const std::string & path)
	{
		if (!m_Code.empty()) m_Code.clear();
		if (!m_Files.empty()) m_Files.clear();
		m_Files.emplace_back(path, m_Code);
		m_pFile = m_Files.begin() != m_Files.end() ? &*m_Files.begin() : nullptr;
		ASSERT(m_pFile);
		/*std::ifstream file(path, std::ios::in);
		
		if (file.is_open())
		{
			std::string code;
			long i = 0;
			while (std::getline(file, code))
			{
				m_Code.emplace_back( ++i, code );
			}
		}
		*/
		//std::copy(std::istream_iterator<ScriptLine>(file), std::istream_iterator<ScriptLine>(), std::back_inserter(m_Code));

		Init();
	}

	void Script::IncludeFile(const std::string & path, CodeList::iterator line)
	{
		CodeList code;
		m_Files.emplace_back(path, code, m_pFile);
		/*std::ifstream file(path, std::ios::in);

		if (file.is_open())
		{
			std::string code;
			long i = line != m_Code.end() ? line->Line : m_Code.back().Line;

			while (std::getline(file, code))
			{
				m_Code.emplace(line, ++i, code);
			}
		}*/
	}
#endif

	Script::Script(const CodeList & code) : m_Code(code)
	{
		//LoadFile(path);
	}

	ScriptFile::ScriptFile(std::string path, CodeList & code, const ScriptFile * parent) :
		m_FilePath(path), m_Parent(parent), m_NumLines(0)
	{
		std::ifstream file(path, std::ios::in);
		if (file)
		{
			std::string line;
			while (std::getline(file, line))
			{
				//code.emplace_back(++m_NumLines, line, this);
			}
		}
	}
}