#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QHash>
#include "config.h"
#include "parser.h"

class QTextDocument;

class HighlighterSettings
{

};

class Highlighter : public QSyntaxHighlighter, Configurable
{
	Q_OBJECT

public:
	enum Setting {
		KeywordFormat, InlineCommentFormat, MultilineCommentFormat
	};

	class Configuration : public ConfigBase {
	public:
		QTextCharFormat				KeywordFormat;				// keywords (?)
		QTextCharFormat				LineCommentFormat;			// // line comments
		QTextCharFormat				BlockCommentFormat;			// /* multiline, nestable comments */
		QTextCharFormat				DirectiveFormat;			// #directive
		QTextCharFormat				NumberFormat;				// 1337
		QTextCharFormat				LiteralFormat;				// "literals" / 'literals'
		QTextCharFormat				CommandFormat;				// commands (WAIT)
		QTextCharFormat				TypeNameFormat;				// type_names (INT)
		QTextCharFormat				ConstantFormat;				// constants (true / false)
		QTextCharFormat				OperatorFormat;				// += operators (including text ones like "NOT")

		Configuration(Configurable *configurable, bool temp = false) : ConfigBase("Highlighter", configurable, temp) {
			// apply defaults
			KeywordFormat.setForeground(QColor(0, 0, 255));
			KeywordFormat.setFontWeight(QFont::Bold);
			KeywordFormat.setFontFamily("Courier");
			KeywordFormat.setFontPointSize(10);
			
			LineCommentFormat.setForeground(QColor(0, 128, 0));
			LineCommentFormat.setFontItalic(true);
			LineCommentFormat.setFontFamily("Courier");
			LineCommentFormat.setFontPointSize(10);
			
			BlockCommentFormat.setForeground(QColor(0, 128, 0));
			BlockCommentFormat.setFontItalic(true);
			BlockCommentFormat.setFontFamily("Courier");
			BlockCommentFormat.setFontPointSize(10);
			
			DirectiveFormat.setForeground(QColor(128, 0, 0));
			DirectiveFormat.setFontWeight(QFont::Bold);
			DirectiveFormat.setFontFamily("Courier");
			DirectiveFormat.setFontPointSize(10);
			
			NumberFormat.setForeground(QColor(64, 0, 128));
			NumberFormat.setFontFamily("Courier");
			NumberFormat.setFontPointSize(10);

			LiteralFormat.setForeground(QColor(255, 0, 128));
			LiteralFormat.setFontFamily("Courier");
			LiteralFormat.setFontPointSize(10);
			
			CommandFormat.setForeground(QColor(0, 0, 0));
			CommandFormat.setFontWeight(QFont::Bold);
			CommandFormat.setFontFamily("Courier");
			CommandFormat.setFontPointSize(10);

			TypeNameFormat.setForeground(QColor(0, 0, 255));
			TypeNameFormat.setFontWeight(QFont::Bold);
			TypeNameFormat.setFontFamily("Courier");
			TypeNameFormat.setFontPointSize(10);

			ConstantFormat.setForeground(QColor(0, 0, 255));
			ConstantFormat.setFontWeight(QFont::Bold);
			ConstantFormat.setFontFamily("Courier");
			ConstantFormat.setFontPointSize(10);

			ConfigBase::Load();
			ConfigBase::Save();
		}

		~Configuration()
		{
			ConfigBase::Save();
		}

		void LoadXML(pugi::xml_node node)
		{
			auto formatting = node.find_child([](pugi::xml_node node){ return !strcmp(node.name(), "Formatting"); });
			for (auto it : formatting)
			{
				std::string name = it.name();
				if (name == "Keyword")
					LoadFormatXML(it, KeywordFormat);
				else if (name == "LineComment")
					LoadFormatXML(it, LineCommentFormat);
				else if (name == "BlockComment")
					LoadFormatXML(it, BlockCommentFormat);
				else if (name == "Directive")
					LoadFormatXML(it, DirectiveFormat);
				else if (name == "Number")
					LoadFormatXML(it, NumberFormat);
				else if (name == "Literal")
					LoadFormatXML(it, LiteralFormat);
				else if (name == "Command")
					LoadFormatXML(it, CommandFormat);
				else if (name == "TypeName")
					LoadFormatXML(it, TypeNameFormat);
				else if (name == "Constant")
					LoadFormatXML(it, ConstantFormat);
			}
		}

		void SaveXML(pugi::xml_node node)
		{
			auto formatting = node.child("Formatting");
			if (!formatting) formatting = node.append_child("Formatting");

			SaveFormatXML(formatting, "Keyword", KeywordFormat);
			SaveFormatXML(formatting, "LineComment", LineCommentFormat);
			SaveFormatXML(formatting, "BlockComment", BlockCommentFormat);
			SaveFormatXML(formatting, "Directive", DirectiveFormat);
			SaveFormatXML(formatting, "Number", NumberFormat);
			SaveFormatXML(formatting, "Literal", LiteralFormat);
			SaveFormatXML(formatting, "Command", CommandFormat);
			SaveFormatXML(formatting, "TypeName", TypeNameFormat);
			SaveFormatXML(formatting, "Constant", ConstantFormat);
		}

	private:
		void Load(const QSettings&) override
		{
			
		}
		void Save(QSettings&) override
		{

		}

		template<typename T>
		inline void SaveFormatAttributeXML(pugi::xml_node node, const char* name, const T& val)
		{
			auto attr = node.attribute(name);
			if (!attr) attr = node.append_attribute(name);
			attr = val;
		}
		void SaveFormatXML(pugi::xml_node parent, const char* name, QTextCharFormat& fmt)
		{
			auto child = parent.child(name);
			if (!child) child = parent.append_child(name);
			{
				auto col = child.child("Colour");
				if (!col) col = child.append_child("Colour");
				int r, g, b;
				fmt.foreground().color().getRgb(&r, &g, &b);
				SaveFormatAttributeXML(col, "R", r);
				SaveFormatAttributeXML(col, "G", g);
				SaveFormatAttributeXML(col, "B", b);
			}
			{
				auto col = child.child("BGColour");
				if (!col) col = child.append_child("BGColour");
				int r, g, b;
				fmt.background().color().getRgb(&r, &g, &b);
				SaveFormatAttributeXML(col, "R", r);
				SaveFormatAttributeXML(col, "G", g);
				SaveFormatAttributeXML(col, "B", b);
			}
			{
				auto style = child.child("FontStyle");
				if (!style) style = child.append_child("FontStyle");

				auto font = fmt.font();
				SaveFormatAttributeXML(style, "Bold", fmt.fontWeight());
				SaveFormatAttributeXML(style, "Italic", font.italic());
				SaveFormatAttributeXML(style, "Italic", font.underline());
			}
		}

		void LoadFormatSettings(QTextCharFormat& fmt)
		{
			QSettings settings;
			settings.beginGroup("Highlighter/Formatting");
			if (settings.value("Used", true).toBool())
			{
				fmt.setForeground(QBrush(settings.value("Colour", fmt.foreground().color()).value<QColor>()));
				fmt.setBackground(QBrush(settings.value("BGColour", fmt.background().color()).value<QColor>()));
				fmt.setFontWeight(settings.value("Bold", fmt.font().weight()).toInt());
				fmt.setFontItalic(settings.value("Italic", fmt.font().italic()).toBool());
				fmt.setFontUnderline(settings.value("Underline", fmt.font().underline()).toBool());
			}
		}

		void LoadFormatXML(pugi::xml_node node, QTextCharFormat& fmt)
		{
			if (node.attribute("Used").as_bool(true))
			{
				for (auto v : node)
				{
					std::string name = v.name();
					if (name == "Colour")
					{
						int r, g, b;
						fmt.foreground().color().getRgb(&r, &g, &b);
						fmt.setForeground(QBrush(QColor(v.attribute("R").as_int(r), v.attribute("G").as_int(g), v.attribute("B").as_int(b))));
					}
					else if (name == "BGColour")
					{
						int r, g, b;
						fmt.background().color().getRgb(&r, &g, &b);
						fmt.setBackground(QBrush(QColor(v.attribute("R").as_int(r), v.attribute("G").as_int(g), v.attribute("B").as_int(b))));
					}
					else if (name == "FontStyle")
					{
						fmt.setFontWeight(v.attribute("Bold").as_int(fmt.font().weight()));
						fmt.setFontItalic(v.attribute("Italic").as_bool(fmt.font().italic()));
						fmt.setFontUnderline(v.attribute("Underline").as_bool(fmt.font().underline()));
					}
				}
			}
		}
	};

	Highlighter(QTextDocument* = 0, Configuration* = nullptr);

	inline Configuration*		Config() 						{ return m_Config; }
	inline void					Config(Configuration *config)	{ m_Config = config; }

	inline void					SetDefaultFormat(const QTextCharFormat &v)	{ setFormat(0, document()->characterCount(), v); }

protected:
	void highlightBlock(const QString &) override;

private:
	bool						m_Highlighted;
	Configuration			*	m_Config;

	struct HighlightRule {
		QRegExp				Pattern;
		QTextCharFormat	*	Format;
	};
	
	QVector<HighlightRule>		m_HighlightRules;
	//QTextCharFormat				m_KeywordFormat;
	//QTextCharFormat				m_InlineComment;
	//QTextCharFormat				m_MultilineComment;

	int							m_NumNestedComments;

	QString generateTooltipForKeyword(KeywordType, const QString&);
	const QTextCharFormat& getFormattingForKeyword(KeywordType);
	void onConfigUpdate() override;
};

#endif