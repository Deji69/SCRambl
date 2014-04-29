#include "stdafx.h"
#include "Parser.h"
#include "Scripts.h"
#include "Directives.h"
#include "Literals.h"

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
					PreprocessLine(ln);

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

	void Script::PreprocessLine(std::string & code)
	{
		// initialise the comment offset, as we might be already in a comment from a previous line
		// if that comment ends (and it wasn't nested), remove up to the end of it on this line.
		// the main preprocessor handles completely removing lines that are commented out
		size_t comment_offset = 0;

		bool is_separated = false;

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
						std::string val = GetIdentifier(ltrim(def).substr(name.length()));
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
					PushSourceControl(EvaluateExpression(def) != 0);
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

	void Script::Error(int code, const std::string & msg)
	{
		//ScriptError()

	}

	long long Script::EvaluateExpression(const std::string & expr, int depth) const
	{
		bool bGotLVal = false;
		int lVal = 0, rVal = 0;
		int nStartDepth = depth;

		for (size_t i = 0; i < expr.length(); ++i)
		{
			char c = expr[i];

			// Nothings
			if (IsSpace(c))
				continue;

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
					// ERROR
				}
				continue;
			}

			// Numeros
			try
			{
				IntConst value(expr.substr(i));
				i += value.Pos();
				continue;
			}
			catch (...)
			{
			}

			// Operations
			switch (c)
			{
			case '(':
				++depth;
				if (bGotLVal)
				{
				}
				else
				{
				}
				break;
			}
		}

		if (nStartDepth != depth)
		{
			// ERROR
		}

		return lVal;
	}

	void Script::LoadFile(const std::string & path)
	{
		std::ifstream file(path, std::ios::in);

		if (file.is_open())
		{
			std::copy(std::istream_iterator<ScriptLine>(file), std::istream_iterator<ScriptLine>(), std::back_inserter(m_Code));
		}

		Init();
	}

	Script::Script(const std::string & path)
	{
		LoadFile(path);
	}
}