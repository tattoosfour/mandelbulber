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

#include <stdio.h>
#include "qt_interface.hpp"
#include "qt_interface.moc.hpp"

QWidget *mainWindow;
QApplication *application;
QImage *qimage = NULL;


InterfaceSlots::InterfaceSlots(void)
{
}

RenderedImage::RenderedImage(QWidget *parent)
    : QWidget(parent)
{ }

void RenderedImage::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

	QPen pen(Qt::white, 2, Qt::SolidLine);
	painter.setPen(pen);
	painter.drawLine(0, 0, 500, 20);

	printf("paint\n");
	if(qimage)
	{
		printf("paintImage\n");
		painter.drawImage(QRect(0,0,512,512), *qimage, QRect(0,0,512,512));
	}
}

void InterfaceSlots::slotek(void)
{
	printf("Hello World!\n");
	QLineEdit *edit = qFindChild<QLineEdit*>(mainWindow, "lineEdit");
	int value = edit->text().toInt();
	printf("Value is %d\n", value);

	int width = 512;
	int height = 512;
	sRGBA *img = new sRGBA[width*height];
	qimage = new QImage((const uchar*)img, width, height, width*sizeof(sRGBA), QImage::Format_ARGB32);


	for (int index = 0; index < 500; index++)
	{

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				img[x + y * width].r = (x*y+index)/3;
				img[x + y * width].g = y+index/2;
				img[x + y * width].b = x*y+index;
				img[x + y * width].a = (float)(x+y)/(width+height)*255;
			}

		}

		application->processEvents();
		mainWindow->update();

	}
}

void InterfaceSlots::load(void)
{
	printf("load\n");
}

int QtStart(int argc, char* argv[])
{
	printf("Hello World!\n");

	application = new QApplication(argc, argv);
	InterfaceSlots *slot = new InterfaceSlots;

	QUiLoader loader;

	mainWindow = new QWidget;

	std::string sharedDir("/usr/share/mandelbulber");
	QFile file((sharedDir + "/qt/render_window.ui").c_str());
	file.open(QFile::ReadOnly);
	mainWindow = loader.load(&file, NULL);
	file.close();

	mainWindow->show();

	QVBoxLayout *vboxlayout = qFindChild<QVBoxLayout*>(mainWindow, "verticalLayout");

	RenderedImage *renderedImage = new RenderedImage(mainWindow);
	vboxlayout->addWidget(renderedImage);
	renderedImage->show();

	QWidget *button = qFindChild<QPushButton*>(mainWindow, "pushButton");
	QApplication::connect(button, SIGNAL(clicked()), slot, SLOT(slotek()));

	QAction *actionQuit = qFindChild<QAction*>(mainWindow, "actionQuit");
	QApplication::connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

	QAction *actionLoad = qFindChild<QAction*>(mainWindow, "actionLoad");
	QApplication::connect(actionLoad, SIGNAL(triggered()), slot, SLOT(load()));

	return application->exec();
}


