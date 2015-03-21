#ifndef SCRAMBL_OPTS_H
#define SCRAMBL_OPTS_H

#include <QtWidgets/QDialog>
#include "ui_scrambl_opts.h"
#include "config.h"

class SCRamblOptions : public QDialog
{
	Q_OBJECT

public:
	SCRamblOptions(QWidget * = 0);
	~SCRamblOptions();

	void updateConfigDisplay();

public slots:
	void categoryChange(const QItemSelection&, const QItemSelection&);
	void optionsFilterUpdate(QString);

private:
	QAbstractItemModel * createCategoryModel(QObject *);
	void setupCategories();

	Ui::OptionsDialog			optionsDialogUi;
	QListView				*	optionsList;
	QListView				*	proxyView;
	QSortFilterProxyModel	*	proxyModel;
};

#endif