#ifndef UNIQUELIBRARY_H
#define UNIQUELIBRARY_H

#include <QWidget>

#include <miamuniquelibrary_global.h>

namespace Ui {
class UniqueLibrary;
}

class FlowLayout;

class MIAMUNIQUELIBRARY_LIBRARY UniqueLibrary : public QWidget/*, public Ui::UniqueLibrary*/
{
	Q_OBJECT

public:
	explicit UniqueLibrary(QWidget *parent = 0);

private:
	Ui::UniqueLibrary *ui;

	FlowLayout *_flowLayout;

/// Proof of concept
private slots:
	void changeNumberOfButtons(int max);
	void changeWidthOfButtons(int max);
	void changeHeightOfButtons(int max);

private:
	int _maxW;
	int _maxH;
};

#endif // UNIQUELIBRARY_H
