#ifndef VIEWLOADER_H
#define VIEWLOADER_H

#include <QMenuBar>
#include <QString>
#include <abstractview.h>
#include "pluginmanager.h"

/**
 * \brief		The ViewLoader class is an Helper class designed to load complex views (subclasses of QWidget).
 * \details		This class can instanciate ViewPlaylists and UniqueLibrary classes of this player, but it also has the ability to load
 *				views from any plugin, if this plugin provides an implementation of AbstractView.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class ViewLoader
{
private:
	MediaPlayer *_mediaPlayer;
	PluginManager *_pluginManager;
	QWidget *_parent;

public:
	explicit ViewLoader(MediaPlayer *mediaPlayer, PluginManager *pluginManager, QWidget *parent = nullptr);

	AbstractView *load(AbstractView *currentView, const QString &menuAction);

private:
	/** Attach plugins if views allow to receive some. */
	void attachPluginToBuiltInView(AbstractView *view);

	AbstractView *loadFromPlugin(AbstractView *currentView, const QString &menuAction);
};

#endif // VIEWLOADER_H
