#ifndef CUSTOMIZEOPTIONSDIALOG_H
#define CUSTOMIZEOPTIONSDIALOG_H

#include <QDialog>
#include <QToolButton>
#include <QTranslator>

#include "ui_customizeoptionsdialog.h"

class CustomizeOptionsDialog : public QDialog, public Ui::CustomizeOptionsDialog
{
	Q_OBJECT
private:
	QTranslator t;
	QMap<QString, QString> languages;
	//bool musicLocationsChanged;

public:
	CustomizeOptionsDialog(QWidget *parent = 0);

	/** Is it necessary to redefined this from the UI class just for this init label? */
	void retranslateUi(CustomizeOptionsDialog *dialog);

protected:
	/** Redefined to add custom behaviour. */
	void closeEvent(QCloseEvent *);

signals:
	/** Signal sent whether the music locations have changed or not. */
	void musicLocationsHaveChanged(bool libraryIsEmpty);

public slots:
	/** Adds a new music location in the library. */
	void addMusicLocation(const QString &musicLocation);

	void checkShortcut(ShortcutWidget *, int typedKey);

	/** Change language at runtime. */
	void changeLanguage(QModelIndex);

	/** Redefined to initialize theme from settings. */
	void show();

	void setExternalDragDropPreference(QToolButton *toolButton);

private slots:
	void initCloseActionForPlaylists();

	/** Open a dialog for letting the user to choose a music directory. */
	void openLibraryDialog();

	/** Delete a music location previously chosen by the user. */
	void deleteSelectedLocation();
};

#endif // CUSTOMIZEOPTIONSDIALOG_H
