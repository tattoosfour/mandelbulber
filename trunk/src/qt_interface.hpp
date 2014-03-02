/*********************************************************
 /                   MANDELBULBER
 / Qt user interface
 /
 /
 / author: Krzysztof Marczak
 / contact: buddhi1980@gmail.com
 / licence: GNU GPL v3.0
 /
 / many improvements done by Rayan Hitchman
 ********************************************************/

#ifndef QT_INTERFACE_HPP_
#define QT_INTERFACE_HPP_

#include <QtGui>
#include <QtUiTools>

class InterfaceSlots: public QObject
{
Q_OBJECT
public:
	InterfaceSlots(void);

public slots:
	void slotek();
	void load();
};

class RenderedImage: public QWidget
{
Q_OBJECT

public:
	RenderedImage(QWidget *parent = 0);

protected:
	void paintEvent(QPaintEvent *event);
};

struct sRGBA
{
	uchar b;
	uchar g;
	uchar r;
	uchar a;

};

int QtStart(int argc, char* argv[]);


#endif /* QT_INTERFACE_HPP_ */
