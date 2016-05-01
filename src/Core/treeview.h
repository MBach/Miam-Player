#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QMessageBox>
#include <QTreeView>
#include <model/selectedtracksmodel.h>
#include <model/trackdao.h>

/**
 * \brief		The TreeView class is the base class for displaying trees in the player.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY TreeView : public QTreeView, public SelectedTracksModel
{
	Q_OBJECT
public:
	explicit TreeView(QWidget *parent = nullptr);

	virtual ~TreeView();

	/** Scan nodes and its subitems before dispatching tracks to a specific widget (playlist or tageditor). */
	virtual void findAll(const QModelIndex &index, QList<QUrl> *tracks) const = 0;

	virtual QList<QUrl> selectedTracks() override;

protected:
	/** Explore items to count leaves (tracks). */
	virtual int countAll(const QModelIndexList &indexes) const = 0;

	/** Redefined to override shortcuts that are mapped on simple keys. */
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

	virtual void startDrag(Qt::DropActions supportedActions) override;

private:
	/** Alerts the user if there's too many tracks to add. */
	QMessageBox::StandardButton beforeSending(const QString &target, QList<QUrl> *tracks);

public slots:
	/** Sends folders or tracks to the end of a playlist. */
	void appendToPlaylist();

	/** Sends folders or tracks to the tag editor. */
	void openTagEditor();

signals:
	/** Adds tracks to the current playlist at a specific position. */
	void aboutToInsertToPlaylist(int rowIndex, const QList<QUrl> &tracks);

	/** Adds tracks to the tag editor. */
	void aboutToSendToTagEditor(const QList<QUrl> &tracks);
};

#endif // TREEVIEW_H
