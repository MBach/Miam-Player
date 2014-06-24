#ifndef CUSTOMIZEOPTIONSDIALOG_H
#define CUSTOMIZEOPTIONSDIALOG_H

#include <QDir>
#include <QDialog>
#include <QToolButton>
#include <QTranslator>

#include "ui_customizeoptionsdialog.h"

class CustomizeOptionsDialog : public QDialog, public Ui::CustomizeOptionsDialog
{
	Q_OBJECT
private:
	QTranslator customTranslator, defaultQtTranslator;
	QMap<QString, QString> languages;

public:
	CustomizeOptionsDialog(QWidget *parent = 0);

	/** Is it necessary to redefined this from the UI class just for this init label? */
	void retranslateUi(CustomizeOptionsDialog *dialog);

protected:
	/** Redefined to add custom behaviour. */
	virtual void closeEvent(QCloseEvent *);

signals:
	/** Signal sent whether the music locations have changed or not. */
	void musicLocationsHaveChanged(bool libraryIsEmpty);

public slots:
	/** Adds a new music location in the library. */
	void addMusicLocation(const QString &musicLocation);

	/** Adds a external music locations in the library (Drag & Drop). */
	void addMusicLocations(const QList<QDir> &dirs);

	/** Change language at runtime. */
	void changeLanguage(QModelIndex);

	//void checkShortcut(ShortcutWidget *, int typedKey);

	/** Redefined to initialize theme from settings. */
	void open();

private slots:
	/** Delete a music location previously chosen by the user. */
	void deleteSelectedLocation();

	void initCloseActionForPlaylists();
	void initDragDropAction();

	/** Open a dialog for letting the user to choose a music directory. */
	void openLibraryDialog();

	void updateMusicLocations();
};

#endif // CUSTOMIZEOPTIONSDIALOG_H
