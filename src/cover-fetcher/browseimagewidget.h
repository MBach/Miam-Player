#ifndef BROWSEIMAGEWIDGET_H
#define BROWSEIMAGEWIDGET_H

#include <miamcoverfetcher_global.hpp>
#include <QStackedWidget>

/**
 * \brief       BrowseImageWidget class can switch between images.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCOVERFETCHER_LIBRARY BrowseImageWidget : public QWidget
{
	Q_OBJECT
public:
	explicit BrowseImageWidget(QStackedWidget *parent);

protected:
	virtual void paintEvent(QPaintEvent *) override;
};

#endif // BROWSEIMAGEWIDGET_H
