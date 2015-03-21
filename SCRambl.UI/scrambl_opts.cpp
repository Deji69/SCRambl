#include "stdafx.h"
#include <functional>
#include "scrambl_opts.h"
#include "config.h"
#include "editor.h"
#include "parser.h"

bool occupyDefinitionsList(const QString& str) {
	QFile file(str);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QXmlStreamReader xml;
	xml.setDevice(&file);

	while (auto v = xml.readNext()) {
		if (v == QXmlStreamReader::TokenType::StartElement)
		{

		}
	}

	return true;
}

QGroupBox* createDefinitionsGroup() {
	GetProcessor();
	auto config = GlobalConfig().Get<Processor::Configuration>("Processor");
	auto group = new QGroupBox(QCoreApplication::tr("SCRambl Definitions"));
	auto box = new QVBoxLayout();
	{
		// Definitions folder
		auto group = new QWidget();
		auto layout = new QVBoxLayout();
		auto buildConfPathLabel = new QLabel(QCoreApplication::tr("Build.xml Path"));
		auto lineEdit = new QLineEdit();
		lineEdit->setText(config->BuildConfigFilePath);

		auto buildConfigSelectLabel = new QLabel(QCoreApplication::tr("Default Configuration"));
		auto buildConfigSelect = new QComboBox();
		buildConfigSelect->addItems(config->AllBuildConfigs);
		buildConfigSelect->setCurrentIndex(config->SelectedBuildConfig);

		auto button = new QPushButton("...");
		QFontMetrics metrics(button->font());
		button->setMaximumWidth(metrics.width("...") * 3);
		QCoreApplication::connect(button, &QPushButton::pressed, [group, lineEdit, config, buildConfigSelect](){
			buildConfigSelect->setDisabled(true);
			buildConfigSelect->clear();
			QFileInfo fi(config->BuildConfigFilePath);
			QString fd;
			if (fi.exists()) {
				fd = fi.dir().absolutePath();
			}
			QString fp = QFileDialog::getOpenFileName(group, "Locate SCRambl Build.xml", fd, "*.xml");
			config->BuildConfigFilePath = fp;
			config->onBuildConfigFileChange();
			if (!config->BuildConfig.isEmpty()) {
				if (!fp.isEmpty()) {
					lineEdit->setText(fp);
				}
				buildConfigSelect->addItems(config->AllBuildConfigs);
				buildConfigSelect->setDisabled(false);
			}
			/*if (!occupyDefinitionsList(lineEdit->text())) {
				QMessageBox::warning(nullptr, QCoreApplication::tr("Error"), QCoreApplication::tr("Unable to read Build.xml file"), QMessageBox::Ok);
				lineEdit->setText("");
			}*/
		});
		QCoreApplication::connect(lineEdit, &QLineEdit::textChanged, [buildConfigSelect,config](const QString& fp){
			buildConfigSelect->setDisabled(true);

			if (QFile::exists(fp)) {
				if (config->BuildConfigFilePath != fp) {
					config->BuildConfigFilePath = fp;
					config->onBuildConfigFileChange();
					if (!config->BuildConfig.isEmpty()) {
						buildConfigSelect->clear();
						buildConfigSelect->addItems(config->AllBuildConfigs);
					}
				}
				buildConfigSelect->setDisabled(false);
			}
		});
		QCoreApplication::connect(buildConfigSelect, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, [config](int index){
			config->selectBuildConfig(index);
		});

		buildConfPathLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
		layout->addWidget(buildConfPathLabel);
		{
			auto hlayout = new QHBoxLayout;
			hlayout->addWidget(lineEdit);
			button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
			hlayout->addWidget(button);
			hlayout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
			layout->addItem(hlayout);
		}

		buildConfigSelectLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
		layout->addWidget(buildConfigSelectLabel);
		buildConfigSelect->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
		layout->addWidget(buildConfigSelect);
		layout->setAlignment(Qt::AlignTop);

		group->setLayout(layout);
		group->setContentsMargins(0, 10, 0, 10);
		group->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
		box->addWidget(group, 1);
	}
	{
		// 
	}
	group->setFlat(false);
	group->setLayout(box);
	group->setContentsMargins(0, 10, 0, 10);
	return group;
}

QGroupBox* createHighlightingGroup() {
	QGroupBox *group = new QGroupBox(QCoreApplication::tr("Fonts && Colours"));
	auto box = new QVBoxLayout();
	auto config = GlobalConfig().Get<CodeEditor::Configuration>("Editor");
	auto hlconfig = GlobalConfig().Get<Highlighter::Configuration>("Highlighter");
	auto textfield = new CodeEditorPreview(nullptr, config, hlconfig);
	{
		auto group = new QGroupBox(QCoreApplication::tr("Configure for..."));
		auto layout = new QGridLayout();
		auto dropbox = new QComboBox();
		
		static QPair<QString, QTextCharFormat*> fontConfigsList[] = {
				{ "Default", &config->DefaultFont },
				{ "Commands", &hlconfig->CommandFormat },
				{ "Type Names", &hlconfig->TypeNameFormat },
				{ "Directives", &hlconfig->DirectiveFormat },
				{ "Inline Comments", &hlconfig->LineCommentFormat },
				{ "Multiline Comments", &hlconfig->BlockCommentFormat },
				{ "Keywords", &hlconfig->KeywordFormat },
				{ "Literal Strings", &hlconfig->LiteralFormat },
				{ "Literal Numbers", &hlconfig->NumberFormat }
		};
		static CodeEditor::Configuration defaultEditorConfig = CodeEditor::Configuration(nullptr, true);
		static Highlighter::Configuration defaultHighlightConfig = Highlighter::Configuration(nullptr, true);
		static QTextCharFormat fontDefaults[] = {
			defaultEditorConfig.DefaultFont,
			defaultHighlightConfig.CommandFormat,
			defaultHighlightConfig.TypeNameFormat,
			defaultHighlightConfig.DirectiveFormat,
			defaultHighlightConfig.LineCommentFormat,
			defaultHighlightConfig.BlockCommentFormat,
			defaultHighlightConfig.KeywordFormat,
			defaultHighlightConfig.LiteralFormat,
			defaultHighlightConfig.NumberFormat
		};

		int j = sizeof(fontConfigsList) / sizeof(*fontConfigsList);
		for (int i = 0; i < j; ++i)
		{
			dropbox->addItem(fontConfigsList[i].first, i);
		}
		
		auto button = new QPushButton(QCoreApplication::tr("Select &Font..."));
		QCoreApplication::connect(button, &QPushButton::pressed, [group, dropbox, textfield](){
			bool ok;
			QTextCharFormat * ptr = fontConfigsList[dropbox->itemData(dropbox->currentIndex()).toInt()].second;
			QFont newfont = QFontDialog::getFont(&ok, ptr->font(), group);
			if (ok) {
				ptr->setFont(newfont);
				textfield->onConfigUpdate();
			}
		});
		auto colbutton = new QPushButton(QCoreApplication::tr("Select &Colour..."));
		QCoreApplication::connect(colbutton, &QPushButton::pressed, [group, dropbox, textfield](){
			QTextCharFormat * ptr = fontConfigsList[dropbox->itemData(dropbox->currentIndex()).toInt()].second;
			QColor col = QColorDialog::getColor(ptr->foreground().color(), group);
			if (col.isValid()) {
				ptr->setForeground(QBrush(col));
				textfield->onConfigUpdate();
			}
		});
		auto bgcolbutton = new QPushButton(QCoreApplication::tr("Select &Background..."));
		QCoreApplication::connect(bgcolbutton, &QPushButton::pressed, [group, dropbox, textfield](){
			QTextCharFormat * ptr = fontConfigsList[dropbox->itemData(dropbox->currentIndex()).toInt()].second;
			QColor col = QColorDialog::getColor(ptr->background().color(), group);
			if (col.isValid()) {
				ptr->setBackground(QBrush(col));
				textfield->onConfigUpdate();
			}
		});

		layout->addWidget(dropbox, 0, 0);
		layout->addWidget(button, 0, 1);
		layout->addWidget(colbutton, 1, 0);
		layout->addWidget(bgcolbutton, 1, 1);

		auto dfbutton = new QPushButton(QCoreApplication::tr("&Default Font"));
		dfbutton->setWhatsThis(QCoreApplication::tr("Applies the 'Default' font configuration"));
		QCoreApplication::connect(dfbutton, &QPushButton::pressed, [dropbox, textfield](){
			if (dropbox->itemData(dropbox->currentIndex()).toInt() != 0) {
				QTextCharFormat * ptr = fontConfigsList[dropbox->itemData(dropbox->currentIndex()).toInt()].second;
				ptr->setFont(fontConfigsList[0].second->font());
			}
			textfield->onConfigUpdate();
		});
		auto dcbutton = new QPushButton(QCoreApplication::tr("D&efault Colours"));
		dcbutton->setWhatsThis(QCoreApplication::tr("Applies the 'Default' colour configuration"));
		QCoreApplication::connect(dcbutton, &QPushButton::pressed, [dropbox, textfield](){
			if (dropbox->itemData(dropbox->currentIndex()).toInt() != 0) {
				QTextCharFormat * ptr = fontConfigsList[dropbox->itemData(dropbox->currentIndex()).toInt()].second;
				ptr->setForeground(fontConfigsList[0].second->foreground());
				ptr->setBackground(fontConfigsList[0].second->background());
			}
			textfield->onConfigUpdate();
		});
		auto dbutton = new QPushButton(QCoreApplication::tr("C&ustom Default"));
		dbutton->setWhatsThis(QCoreApplication::tr("Applies the 'Default' font and colour configurations"));
		QCoreApplication::connect(dbutton, &QPushButton::pressed, [dropbox, textfield](){
			QTextCharFormat * ptr = fontConfigsList[dropbox->itemData(dropbox->currentIndex()).toInt()].second;
			if (dropbox->itemData(dropbox->currentIndex()).toInt() != 0) *ptr = fontDefaults[0];
			textfield->onConfigUpdate();
		});
		auto rbutton = new QPushButton(QCoreApplication::tr("&Application Default"));
		rbutton->setWhatsThis(QCoreApplication::tr("Resets the configuration to application default"));
		QCoreApplication::connect(rbutton, &QPushButton::pressed, [dropbox, textfield](){
			QTextCharFormat * ptr = fontConfigsList[dropbox->itemData(dropbox->currentIndex()).toInt()].second;
			*ptr = fontDefaults[dropbox->itemData(dropbox->currentIndex()).toInt()];
			textfield->onConfigUpdate();
		});

		layout->addWidget(dfbutton, 2, 0);
		layout->addWidget(dcbutton, 2, 1);
		layout->addWidget(dbutton, 3, 0);
		layout->addWidget(rbutton, 3, 1);
		
		group->setFlat(true);
		group->setLayout(layout);
		group->setContentsMargins(0, 10, 0, 10);
		box->addWidget(group, 1, Qt::AlignTop);
	}
	{
		auto group = new QGroupBox(QCoreApplication::tr("Preview"));
		auto layout = new QHBoxLayout;
		layout->addWidget(textfield);
		textfield->setPlainText("Default\nWAIT\n:label\n#directive\n// Inline Comment \n/* Multiline Comment */\nKEYWORD\n\"STRING\" 'STRING'\n800 0x6AA 3.14");
		textfield->setReadOnly(true);
		group->setFlat(true);
		group->setLayout(layout);
		group->setContentsMargins(0, 10, 0, 10);
		box->addWidget(group, 1, Qt::AlignTop);
	}
	group->setFlat(false);
	group->setLayout(box);
	group->setContentsMargins(0, 10, 0, 10);
	textfield->onConfigUpdate();
	return group;
}

struct {
	const char						*	name;
	std::function<QLayout*()>			setup;
} optionsCategories[] = {
	{ "General", [](){
		auto grid = new QGridLayout();
		grid->addWidget(createDefinitionsGroup(), 0, 0);
		return grid;
	} },
	{ "Fonts & Colours", [](){
		auto grid = new QGridLayout();
		grid->addWidget(createHighlightingGroup(), 0, 0);
		return grid;
	} }
};

void SCRamblOptions::setupCategories()
{
	proxyModel = new QSortFilterProxyModel();
	optionsList = optionsDialogUi.listView;

	auto model = createCategoryModel(proxyModel);
	proxyModel->setSourceModel(model);
	//optionsList->setSelectionModel();
	optionsList->setModel(proxyModel);
	optionsList->setEditTriggers(QAbstractItemView::NoEditTriggers);

	optionsDialogUi.frame->setLayout(optionsCategories[0].setup());

	connect(optionsList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(categoryChange(const QItemSelection&, const QItemSelection&)));
	connect(optionsDialogUi.lineEdit, SIGNAL(textChanged(QString)), this, SLOT(optionsFilterUpdate(QString)));
}

QAbstractItemModel * SCRamblOptions::createCategoryModel(QObject * parent)
{
	auto model = new QStandardItemModel(0, 1, parent);
	for (int i = 0; i < (sizeof(optionsCategories) / sizeof(*optionsCategories)); ++i) {
		if (!*(optionsCategories[i].name)) break;
		model->insertRow(i);
		model->setData(model->index(i, 0), optionsCategories[i].name);
	}
	return model;
}

void SCRamblOptions::optionsFilterUpdate(QString str)
{
	proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	proxyModel->setFilterFixedString(str);
}

void DeleteWidgetsWidgets(QLayout * layout) {
	while (auto item = layout->takeAt(0)) {
		if (auto sub = item->layout()) DeleteWidgetsWidgets(sub);
		else if (auto widget = item->widget()) {
			widget->hide();
			delete widget;
		}
		else delete item;
	}
}

void SCRamblOptions::updateConfigDisplay()
{
	if (auto layout = optionsDialogUi.frame->layout()) {
		DeleteWidgetsWidgets(layout);
		delete layout;
	}

	auto row = optionsList->currentIndex().row();
	if (row != -1)
		optionsDialogUi.frame->setLayout(optionsCategories[row].setup());
	else
		optionsDialogUi.frame->setLayout(optionsCategories[0].setup());
}

void SCRamblOptions::categoryChange(const QItemSelection &, const QItemSelection &)
{
	updateConfigDisplay();
}

SCRamblOptions::SCRamblOptions(QWidget *parent) : QDialog(parent)
{
	optionsDialogUi.setupUi(this);
	setupCategories();
}

SCRamblOptions::~SCRamblOptions()
{
	QDialog::close();
}