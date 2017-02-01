#ifndef ACOUSTID_PLUGIN
#define ACOUSTID_PLUGIN

#include <interfaces/tageditorplugin.h>
#include "qchromaprint.h"
#include "requestpool.h"
#include "matchingrecordswidget.h"

#include <QPushButton>

/**
 * \brief		The AcoustIdPlugin class can fetch tags automatically from Webservice
 * \details		This plugin uses Chromaprint library and MusicBrainz
 * \author      Matthieu Bachelier
 * \version     0.1
 * \copyright   GNU General Public License v3
 */
class AcoustIdPlugin : public TagEditorPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID TagEditorPlugin_iid)
	Q_INTERFACES(TagEditorPlugin)
private:
	static QString _apiKey;
	static QString _wsAcoustID;

	RequestPool *_requestPool;
	QChromaprint *_chromaprint;
	MatchingRecordsWidget *_matchingRecordsWidget;

	/// From TagEditorPlugin
	QPushButton *_analyzeButton;
	SelectedTracksModel *_selectedTracksModel;
	QTableWidget *_tableWidget;
	QStackedWidget *_stackedWidget;

public:
	explicit AcoustIdPlugin();

	virtual ~AcoustIdPlugin();

	/// From BasicPlugin
	inline virtual QWidget *configPage() override { return nullptr; }

	inline virtual bool isConfigurable() const override { return false; }

	inline virtual QString name() const override { return "AcoustIdPlugin"; }

	inline virtual QString version() const override { return "0.1"; }

	/// From TagEditorPlugin
	virtual void setSelectedTracksModel(SelectedTracksModel *selectedTracksModel) override;

	virtual void setExtensibleLayout(QHBoxLayout *layout) override;

	virtual void setStackWidget(QStackedWidget *sw) override;

	virtual void setTagEditorWidget(QTableWidget *tableWidget) override;

private slots:
	void start();

signals:
	void releaseFound(const MusicBrainz::Release &);
	void tracksAnalyzed();
};

#endif // ACOUSTID_PLUGIN
