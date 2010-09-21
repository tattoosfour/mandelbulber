/*
 * timeline.cpp
 *
 *  Created on: 2010-09-20
 *      Author: krzysztof
 */

#include <gtk-2.0/gtk/gtk.h>
#include "timeline.hpp"
#include "Render3D.h"
#include "interface.h"
#include "settings.h"
#include "cimage.hpp"
#include "callbacks.h"

cTimeline *timeline;

cTimeline::cTimeline()
{
	database = new cDatabase(0);
	keyframeCount = 0;
}

cTimeline::~cTimeline()
{
	delete database;
}

int cTimeline::Initialize(char *keyframesPath)
{
	char filename2[1000];
	int maxKeyNumber = 0;
	do
	{
		IndexFilename(filename2, keyframesPath, (char*) "fract", maxKeyNumber);
		maxKeyNumber++;
	} while (FileIfExist(filename2));
	maxKeyNumber--;
	printf("Found %d keyframes\n", maxKeyNumber);
	cImage *thumbnail;


	timelineInterface.darea = new GtkWidget*[maxKeyNumber];
	timelineInterface.table = gtk_table_new(2, maxKeyNumber + 1, false);
	gtk_container_add(GTK_CONTAINER(timeLineWindow), timelineInterface.table);
	gtk_widget_show(timelineInterface.table);

	char widgetName[7];

	sTimelineRecord *record = new sTimelineRecord;
	for (int i = 0; i < maxKeyNumber; i++)
	{
		thumbnail = new cImage(128, 128);
		IndexFilename(filename2, keyframesPath, (char*) "fract", i);
		ThumbnailRender(filename2, thumbnail);
		thumbnail->CreatePreview(1.0);
		thumbnail->ConvertTo8bit();
		thumbnail->UpdatePreview();
		memcpy(record->thumbnail, thumbnail->GetPreviewPtr(), sizeof(sRGB8) * 128 * 128);
		record->index = i;
		database->AddRecord((char*) record, sizeof(sTimelineRecord));

		timelineInterface.darea[i] = gtk_drawing_area_new();
		sprintf(widgetName, "da%d", i);
		gtk_widget_set_size_request(timelineInterface.darea[i], 128, 128);
		gtk_widget_set_name(timelineInterface.darea[i], widgetName);
		gtk_table_attach_defaults(GTK_TABLE(timelineInterface.table), timelineInterface.darea[i], i, i + 1, 1, 2);
		gtk_widget_show(timelineInterface.darea[i]);

		gdk_draw_rgb_image(timelineInterface.darea[i]->window, timelineInterface.darea[i]->style->fg_gc[GTK_STATE_NORMAL], 0, 0, 128, 128, GDK_RGB_DITHER_MAX, thumbnail->GetPreviewPtr(), 128 * 3);
		while (gtk_events_pending())
			gtk_main_iteration();
		gtk_signal_connect(GTK_OBJECT(timelineInterface.darea[i]), "expose-event", GTK_SIGNAL_FUNC(thumbnail_expose), NULL);
		delete thumbnail;
	}

	delete record;
	return maxKeyNumber;
}

void cTimeline::GetImage(int index, sRGB8 *image)
{
	sTimelineRecord *record = new sTimelineRecord;
	database->GetRecord(index, (char*) record);
	memcpy(image, record->thumbnail, sizeof(sRGB8) * 128 * 128);
	delete record;
}

void ThumbnailRender(char *settingsFile, cImage *miniImage)
{

	printf("Rendering keyframe preview: %s\n", settingsFile);

	if (FileIfExist(settingsFile))
	{
		sParamRender fractParamLoaded;
		ParamsAllocMem(&fractParamLoaded);
		LoadSettings(settingsFile, fractParamLoaded);

		fractParamLoaded.image_width = miniImage->GetWidth();
		fractParamLoaded.image_height = miniImage->GetHeight();
		fractParamLoaded.doubles.resolution = 1.0 / fractParamLoaded.image_width;
		fractParamLoaded.doubles.max_y = 20.0 / fractParamLoaded.doubles.zoom;
		if (fractParamLoaded.formula == trig_DE || fractParamLoaded.formula == menger_sponge || fractParamLoaded.formula == kaleidoscopic || fractParamLoaded.formula == tglad) fractParamLoaded.analitycDE
				= true;
		else fractParamLoaded.analitycDE = false;
		RecalculateIFSParams(&fractParamLoaded);
		CreateFormulaSequence(&fractParamLoaded);

		miniImage->ClearImage();
		miniImage->SetImageParameters(fractParamLoaded.doubles.imageAdjustments, fractParamLoaded.effectColours, fractParamLoaded.imageSwitches);
		miniImage->SetPalette(fractParamLoaded.palette);

		LoadTextures(&fractParamLoaded);

		programClosed = false;
		isPostRendering = false;
		Render(fractParamLoaded, miniImage, NULL);

		ParamsReleaseMem(&fractParamLoaded);
	}
}

gboolean thumbnail_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	const char* widgetName = gtk_widget_get_name(widget);
	printf("Widget name: %s\n", widgetName);
	int number = 0;
	sscanf(widgetName, "da%d", &number);
	printf("Widget number: %d\n", number);
	sRGB8 *image = new sRGB8[128 * 128];
	timeline->GetImage(number, image);
	gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, 128, 128, GDK_RGB_DITHER_MAX, (unsigned char*) image, 128 * 3);
	delete image;
	while (gtk_events_pending())
		gtk_main_iteration();
}

