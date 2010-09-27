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
#include "smartptr.h"

smart_ptr<cTimeline> timeline;

cTimeline::cTimeline()
{
	database.reset(new cDatabase(1));
	keyframeCount = 0;
	isCreated = false;
}

cTimeline::~cTimeline()
{
}

int cTimeline::Initialize(char *keyframesPath)
{
	int numberOfKeyframes = CheckNumberOfKeyframes(keyframesPath);
	printf("Found %d keyframes\n", numberOfKeyframes);

	smart_ptr<cImage> thumbnail;

	CreateInterface(numberOfKeyframes);

	char filename2[1000];
	smart_ptr<sTimelineRecord> record(new sTimelineRecord);
	for (int i = 0; i < numberOfKeyframes; i++)
	{
		thumbnail.reset(new cImage(128, 128));
		IndexFilename(filename2, keyframesPath, (char*) "fract", i);
		ThumbnailRender(filename2, thumbnail.ptr());
		thumbnail->CreatePreview(1.0);
		thumbnail->ConvertTo8bit();
		thumbnail->UpdatePreview();
		memcpy(record->thumbnail, thumbnail->GetPreviewPtr(), sizeof(sRGB8) * 128 * 128);
		record->index = i; //only for testing database

		if (i == 0) database->SetRecord(0, (char*) record.ptr(), sizeof(sTimelineRecord));
		else database->AddRecord((char*) record.ptr(), sizeof(sTimelineRecord));

		DisplayInDrawingArea(i, timelineInterface.darea[i]);
		while (gtk_events_pending())
			gtk_main_iteration();
		gtk_signal_connect(GTK_OBJECT(timelineInterface.darea[i]), "expose-event", GTK_SIGNAL_FUNC(thumbnail_expose), NULL);
		gtk_signal_connect(GTK_OBJECT(timelineInterface.darea[i]), "button_press_event", GTK_SIGNAL_FUNC(PressedKeyframeThumbnail), NULL);
		gtk_widget_add_events(timelineInterface.darea[i], GDK_BUTTON_PRESS_MASK);
	}

	isCreated = true;
	keyframeCount = numberOfKeyframes;
	return numberOfKeyframes;
}

int cTimeline::CheckNumberOfKeyframes(char *keyframesPath)
{
	char filename2[1000];
	int maxKeyNumber = 0;
	do
	{
		IndexFilename(filename2, keyframesPath, (char*) "fract", maxKeyNumber);
		maxKeyNumber++;
	} while (FileIfExist(filename2));
	maxKeyNumber--;
	return maxKeyNumber;
}

bool cTimeline::GetImage(int index, sRGB8 *image)
{
	errorCode err;
	smart_ptr<sTimelineRecord> record(new sTimelineRecord);
	err = database->GetRecord(index, (char*) record.ptr());
	if (!err)
	{
		memcpy(image, record->thumbnail, sizeof(sRGB8) * 128 * 128);
		return true;
	}
	return false;
}

void cTimeline::DisplayInDrawingArea(int index, GtkWidget *darea)
{
	smart_array<sRGB8> image(new sRGB8[128 * 128]);
	if (timeline->GetImage(index, image.ptr()))
	{
		gdk_draw_rgb_image(darea->window, darea->style->fg_gc[GTK_STATE_NORMAL], 0, 0, 128, 128, GDK_RGB_DITHER_MAX, (unsigned char*) image.ptr(), 128 * 3);
	}
}

void cTimeline::CreateInterface(int numberOfKeyframes)
{
	timelineInterface.scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	timelineInterface.windowHadjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(timelineInterface.scrolledWindow));
	timelineInterface.windowVadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(timelineInterface.scrolledWindow));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(timelineInterface.scrolledWindow), GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);
	timelineInterface.mainBox = gtk_hbox_new(false, 1);
	timelineInterface.table = gtk_table_new(2, numberOfKeyframes + 1, false);

	gtk_box_pack_start(GTK_BOX(timelineInterface.mainBox), timelineInterface.table, false, false, 1);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(timelineInterface.scrolledWindow), timelineInterface.mainBox);
	gtk_container_add(GTK_CONTAINER(timeLineWindow), timelineInterface.scrolledWindow);
	gtk_widget_show(timelineInterface.mainBox);
	gtk_widget_show(timelineInterface.table);
	gtk_widget_show(timelineInterface.scrolledWindow);

	timelineInterface.darea = new GtkWidget*[numberOfKeyframes];
	timelineInterface.label = new GtkWidget*[numberOfKeyframes];

	char widgetName[7];
	char labelText[20];

	for (int i = 0; i < numberOfKeyframes; i++)
	{
		timelineInterface.darea[i] = gtk_drawing_area_new();
		sprintf(widgetName, "da%d", i);
		sprintf(labelText,"keyframe %d",i);
		timelineInterface.label[i] = gtk_label_new(labelText);
		gtk_widget_set_size_request(timelineInterface.darea[i], 128, 128);
		gtk_widget_set_name(timelineInterface.darea[i], widgetName);

		gtk_table_attach(GTK_TABLE(timelineInterface.table), timelineInterface.label[i], i, i + 1, 0, 1, GTK_EXPAND, GTK_EXPAND, 1, 0);
		gtk_table_attach(GTK_TABLE(timelineInterface.table), timelineInterface.darea[i], i, i + 1, 1, 2, GTK_EXPAND, GTK_EXPAND, 1, 0);
		gtk_widget_show(timelineInterface.darea[i]);
		gtk_widget_show(timelineInterface.label[i]);
	}
}

void cTimeline::RebulidTimelineWindow(void)
{
	CreateInterface(keyframeCount);
	for (int i = 0; i < keyframeCount; i++)
	{
		DisplayInDrawingArea(i, timelineInterface.darea[i]);
		gtk_signal_connect(GTK_OBJECT(timelineInterface.darea[i]), "expose-event", GTK_SIGNAL_FUNC(thumbnail_expose), NULL);
	}
}

void cTimeline::RecordKeyframe(int index, char *keyframeFile)
{

	smart_ptr<sTimelineRecord> record(new sTimelineRecord);
	smart_ptr<cImage> thumbnail(new cImage(128, 128));
	ThumbnailRender(keyframeFile, thumbnail.ptr());
	thumbnail->CreatePreview(1.0);
	thumbnail->ConvertTo8bit();
	thumbnail->UpdatePreview();
	memcpy(record->thumbnail, thumbnail->GetPreviewPtr(), sizeof(sRGB8) * 128 * 128);
	record->index = index;
	if (index < keyframeCount)
	{
		database->SetRecord(index, (char*) record.ptr(), sizeof(sTimelineRecord));
	}
	else
	{
		database->AddRecord((char*) record.ptr(), sizeof(sTimelineRecord));
		Resize(keyframeCount+1);
		keyframeCount++;
	}
	DisplayInDrawingArea(index, timelineInterface.darea[index]);
}

void cTimeline::Resize(int newsize)
{
	gtk_table_resize(GTK_TABLE(timelineInterface.table),2,newsize + 1);

	//resize array with drawing area widgets
	GtkWidget **darea_new = new GtkWidget*[newsize];
	int max;
	if(newsize<keyframeCount) max = newsize; else max = keyframeCount;
	for (int i = 0; i < max; i++)
	{
		darea_new[i] = timelineInterface.darea[i];
	}
	delete[] timelineInterface.darea;
	timelineInterface.darea = darea_new;

	char widgetName[7];
	char labelText[20];
	if(newsize>keyframeCount)
	{
		for(int i=keyframeCount; i<newsize; i++)
		{
			timelineInterface.darea[i] = gtk_drawing_area_new();
			sprintf(widgetName, "da%d", i);
			sprintf(labelText,"keyframe %d",i);
			gtk_widget_set_size_request(timelineInterface.darea[i], 128, 128);
			gtk_widget_set_name(timelineInterface.darea[i], widgetName);
			gtk_table_attach(GTK_TABLE(timelineInterface.table), timelineInterface.label[i], i, i + 1, 0, 1, GTK_EXPAND, GTK_EXPAND, 1, 0);
			gtk_table_attach(GTK_TABLE(timelineInterface.table), timelineInterface.darea[i], i, i + 1, 1, 2, GTK_EXPAND, GTK_EXPAND, 1, 0);
			gtk_widget_show(timelineInterface.darea[i]);
			gtk_widget_show(timelineInterface.label[i]);
			gtk_signal_connect(GTK_OBJECT(timelineInterface.darea[i]), "expose-event", GTK_SIGNAL_FUNC(thumbnail_expose), NULL);
		}
	}
	else
	{
		for(int i=newsize; i<keyframeCount; i++)
		{
			gtk_widget_destroy(timelineInterface.darea[i]);
		}
	}
}

gboolean thumbnail_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	const char* widgetName = gtk_widget_get_name(widget);
	int number = 0;
	sscanf(widgetName, "da%d", &number);
	timeline->DisplayInDrawingArea(number, widget);
	while (gtk_events_pending())
		gtk_main_iteration();
	return true;
}

void PressedKeyframeThumbnail(GtkWidget *widget, GdkEventButton *event)
{
	if (event->type == GDK_2BUTTON_PRESS)
	{
		const char* widgetName = gtk_widget_get_name(widget);
		int index = 0;
		sscanf(widgetName, "da%d", &index);
		printf("Clicked on keyframe %d\n", index);

		gtk_entry_set_text(GTK_ENTRY(Interface.edit_animationKeyNumber), IntToString(index));

		char filename2[1000];

		IndexFilename(filename2, Interface_data.file_keyframes, (char*) "fract", index);
		if (FileIfExist(filename2))
		{
			sParamRender fractParamLoaded;
			ParamsAllocMem(&fractParamLoaded);
			LoadSettings(filename2, fractParamLoaded);
			WriteInterface(&fractParamLoaded);
			last_keyframe_position = fractParamLoaded.doubles.vp;
			ParamsReleaseMem(&fractParamLoaded);

			Interface_data.animMode = false;
			Interface_data.playMode = false;
			Interface_data.recordMode = false;
			Interface_data.continueRecord = false;
			Interface_data.keyframeMode = false;

			programClosed = true;
			isPostRendering = false;
			renderRequest = true;
		}
		else
		{
			GtkWidget *dialog =	gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL, "Error! Keyframe not exists: %s", filename2);
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			gtk_entry_set_text(GTK_ENTRY(Interface.edit_animationKeyNumber), IntToString(index));
		}
	}
}
