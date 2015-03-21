#include "stdafx.h"
#include "highlighter.h"
#include "parser.h"
#include "editor.h"

Highlighter::Highlighter(QTextDocument* doc, Configuration* config) : QSyntaxHighlighter(doc),
	m_Highlighted(false), m_Config(config ? config : new Configuration(this))
{
	HighlightRule rule;

	// Numbers in hex...
	rule.Pattern = QRegExp("\\b0[xX][0-9a-fA-F]+\\b");
	rule.Format = &Config()->NumberFormat;
	m_HighlightRules.append(rule);
	// ... in dec
	rule.Pattern = QRegExp("\\b[a-fA-F]*[0-9][\\.0-9a-fA-F]*\\b");
	m_HighlightRules.append(rule);

	QStringList keywords;
	keywords << "\\bkeyword\\b";

	foreach(const QString& pat, keywords) {
		rule.Pattern = QRegExp(pat);
		rule.Format = &Config()->KeywordFormat;
		m_HighlightRules.append(rule);
	}
}

void Highlighter::onConfigUpdate()
{
	if (m_Highlighted) rehighlight();
}

QString Highlighter::generateTooltipForKeyword(KeywordType type, const QString& str)
{
	if (type == KeywordType::command) {
		return "command: " + str;
	}
	return "";
}

const QTextCharFormat& Highlighter::getFormattingForKeyword(KeywordType type)
{
	if (type == KeywordType::command)
		return m_Config->CommandFormat;
	
	ASSERT(type != KeywordType::none);
}

void Highlighter::highlightBlock(const QString& txt)
{
	enum { Default = -1, BlockComment, StringLiteral };
	
	int state = previousBlockState();
	int start = 0;

	auto editor = GlobalConfig().Get<CodeEditor::Configuration>("Editor");
	if (editor) {
		setFormat(0, txt.length(), editor->DefaultFont);
	}

	// Follow rules
	if (state == Default) {
		foreach(const HighlightRule& rule, m_HighlightRules) {
			QRegExp expr(rule.Pattern);
			int len;
			for (int idx = expr.indexIn(txt) + start; idx >= 0; idx = expr.indexIn(txt, idx + len + start)) {
				len = expr.matchedLength();
				setFormat(idx, len, *rule.Format);
			}
		}
		//setCurrentBlockState(0);

		static QRegExp identifier_regex("\\b[A-Za-z_\\.][A-Za-z0-9_\\.]*\\b");
		auto& processor = GetProcessor();
		int len;
		for (int idx = identifier_regex.indexIn(txt) + start; idx >= 0; idx = identifier_regex.indexIn(txt, idx + len + start)) {
			len = identifier_regex.matchedLength();
			auto type = processor.getKeywordType(txt.mid(idx, len));
			if (type != KeywordType::none) {
				QTextCharFormat fmt = getFormattingForKeyword(type);
				//fmt.setToolTip(generateTooltipForKeyword(type, txt));
				setFormat(idx, len, fmt);
			}
		}
	}

	// Handle syntax
	bool quoEscape = false;
	for (int i = 0; i < txt.length(); ++i) {
		// Continue string
		if (state == StringLiteral) {
			auto ch = txt.at(i);

			if (ch == '"' && !quoEscape) {
				state = Default;
				setFormat(start, i - start + 1, Config()->LiteralFormat);
			}
			else if (quoEscape)
				quoEscape = false;
			else if (ch == '\\')
				quoEscape = true;
		}
		// Continue comment
		else if (state == BlockComment) {
			if (txt.mid(i, 2) == "*/") {
				if (m_NumNestedComments-- <= 0) {
					state = Default;
					setFormat(start, i - start + 2, Config()->BlockCommentFormat);
				}
			}
			else if (txt.mid(i, 2) == "/*") {
				++m_NumNestedComments;
			}
		}
		// Activate string..?
		else if (txt.at(i) == '"') {
			start = i;
			state = StringLiteral;
			quoEscape = false;
		}
		// Activate comment..?
		else if (txt.at(i) == '/' && txt.length() > (i+1)) {
			if (txt.at(i + 1) == '/') {
				setFormat(i, txt.length(), Config()->LineCommentFormat);
				start = i + txt.length();
				break;
			}
			else if (txt.at(i + 1) == '*') {
				start = i;
				state = BlockComment;
				m_NumNestedComments = 0;
			}
		}
	}

	if (state == BlockComment) setFormat(start, txt.length(), Config()->BlockCommentFormat);
	else if (state == StringLiteral) setFormat(start, txt.length(), Config()->LiteralFormat);

	setCurrentBlockState(state);
	m_Highlighted = true;

	/*int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = commentStartExpression.indexIn(text);

	while (startIndex >= 0) {
		int endIndex = commentEndExpression.indexIn(text, startIndex);
		int commentLength;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		}
		else {
			commentLength = endIndex - startIndex
				+ commentEndExpression.matchedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
	}*/
}