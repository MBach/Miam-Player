#ifndef FETCHDIALOG_H
#define FETCHDIALOG_H

#include <QDialog>

#include <cover.h>
#include "providers/coverartprovider.h"
#include "miamcoverfetcher_global.hpp"
#include "ui_fetchdialog.h"

/**
 * \brief       The FetchDialog class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCOVERFETCHER_LIBRARY FetchDialog : public QDialog, public Ui::FetchDialog
{
	Q_OBJECT
private:
	Q_ENUMS(ListWidgetUserType)

public:
	explicit FetchDialog(const QList<CoverArtProvider*> &providers, QWidget *parent = nullptr);

	virtual ~FetchDialog();

protected:
	virtual void closeEvent(QCloseEvent *e) override;

private:
	bool copyCoverToFolder(Cover *cover, QString artistAlbum, QString album);

	bool integrateCoverToFile(Cover *cover, QString artistAlbum, QString album);

public slots:
	void addCover(const QString &album, const QByteArray &coverByteArray);

private slots:
	void applyChanges();

	void updateCoverSize(int value);

signals:
	void refreshView();
};

#endif // FETCHDIALOG_H
