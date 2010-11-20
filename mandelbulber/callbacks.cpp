/********************************************************
 /                   MANDELBULBER                        *
 /                                                       *
 / author: Krzysztof Marczak                             *
 / contact: buddhi1980@gmail.com                         *
 / licence: GNU GPL                                      *
 ********************************************************/

/*
 * callbacks.cpp
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof
 */

#include <gtk-2.0/gtk/gtk.h>
#include "callbacks.h"
#include "algebra.hpp"
#include "interface.h"
#include "image.h"
#include "Render3D.h"
#include "settings.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fractal.h"
#include "shaders.h"
#include "fractparams.h"
#include "files.h"
#include "undo.hpp"
#include "loadsound.hpp"
#include "timeline.hpp"

double last_navigator_step;
CVector3 last_keyframe_position;
bool renderRequest = false;

gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	x_mouse = event->x;
	y_mouse = event->y;

	return TRUE;
}

gboolean CallerTimerLoop(GtkWidget *widget)
{
	if (isRendering && renderRequest && !Interface_data.animMode)
	{
		programClosed = true;
		isPostRendering = false;
	}

	if (!isRendering && renderRequest && !Interface_data.animMode)
	{
		renderRequest = false;
		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		gdk_threads_enter();
		MainRender();
		gdk_threads_leave();
	}
	return true;
}

gboolean pressed_button_on_image(GtkWidget *widget, GdkEventButton *event)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkZoomClickEnable)) && !Interface_data.animMode)
	{
		int x = event->x;
		int z = event->y;
		x = x / mainImage->GetPreviewScale();
		z = z / mainImage->GetPreviewScale();

		int width = mainImage->GetWidth();
		int height = mainImage->GetHeight();

		double closeUpRatio = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mouse_click_distance)));
		if(event->button == 3)
		{
			closeUpRatio = 1.0/closeUpRatio;
		}

		sParamRender params;
		ParamsAllocMem(&params);
		ReadInterface(&params);
		undoBuffer.SaveUndo(&params);

		sFractal calcParam;

		double x2, z2;
		double aspectRatio = (double) width / height;

		bool fishEye = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkFishEye));
		if (fishEye)
		{
			x2 = M_PI * ((double) x / width - 0.5) * aspectRatio;
			z2 = M_PI * ((double) z / height - 0.5);
		}
		else
		{
			x2 = ((double) x / width - 0.5) * params.doubles.zoom * aspectRatio;
			z2 = ((double) z / height - 0.5) * params.doubles.zoom;
		}
		//rotation matrix
		CRotationMatrix mRot;
		mRot.RotateZ(params.doubles.alfa);
		mRot.RotateX(params.doubles.beta);
		mRot.RotateY(params.doubles.gamma);
		double y = mainImage->GetPixelZBuffer(x, z);
		if (y < 1e19)
		{
			double persp_factor = 1.0 + y * params.doubles.persp;
			CVector3 vector, vector2;
			if (fishEye)
			{
				double y2 = y * (1.0 - 1.0 / closeUpRatio);
				vector.x = sin(params.doubles.persp * x2) * y2;
				vector.z = sin(params.doubles.persp * z2) * y2;
				vector.y = cos(params.doubles.persp * x2) * cos(params.doubles.persp * z2) * y2;
			}
			else
			{
				double delta_y;
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorGoToSurface)))
				{
					delta_y = 0;
				}
				else
				{
					delta_y = (y + (1.0 / params.doubles.persp)) / closeUpRatio;
				}
				double y2 = ((y - delta_y) * params.doubles.zoom);
				vector.x = x2 * persp_factor;
				vector.y = y2;
				vector.z = z2 * persp_factor;
			}
			vector2 = mRot.RotateVector(vector);
			params.doubles.vp = calcParam.point = vector2 + params.doubles.vp;

			params.doubles.zoom /= closeUpRatio;

			sFractal calcParam;
			sFractal_ret calcRet;
			CopyParams(&params, &calcParam);
			char distanceString[1000];
			double distance = CalculateDistance(calcParam, calcRet);
			sprintf(distanceString, "Estimated viewpoint distance to the surface: %g", distance);
			gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

			WriteInterface(&params);
			ParamsReleaseMem(&params);
			Interface_data.animMode = false;
			Interface_data.playMode = false;
			Interface_data.recordMode = false;
			Interface_data.continueRecord = false;

			programClosed = true;
			isPostRendering = false;
			renderRequest = true;

			printf("Event finished\n");
		}
	}
	else
	{
		if (Interface_data.animMode && Interface_data.recordMode)
		{

			double DESpeed = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_animationDESpeed)));
			if (event-> button == 1)
			{
				DESpeed *= 1.1;
			}
			if (event-> button == 3)
			{
				DESpeed *= 0.9;
			}
			gtk_entry_set_text(GTK_ENTRY(Interface.edit_animationDESpeed), DoubleToString(DESpeed));
		}
	}
	return true;
}

//----------- close program
gboolean StopRenderingAndQuit(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	programClosed = true;
	StopRendering(widget, data);
	gtk_main_quit();
	printf("Quitting\n");
	return true;
}

gboolean WindowReconfigured(GtkWindow *window, GdkEvent *event, gpointer data)
{
	int width = event->configure.width;
	int height = event->configure.height;

	if (width != lastWindowWidth || height != lastWindowHeight)
	{
		ChangedComboScale(Interface.combo_imageScale, NULL);
	}
	return false;
}

gboolean on_darea_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	if (mainImage->IsPreview())
	{
		mainImage->RedrawInWidget(darea);
		return TRUE;
	}
	else return false;
}

gboolean on_dareaPalette_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	paletteViewCreated = true;
	//srand(Interface_data.coloring_seed);
	//NowaPaleta(paleta, 1.0);
	DrawPalette(mainImage->GetPalettePtr());
	return TRUE;
}

gboolean on_dareaSound_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	char *userdata = (char*) user_data;
	double w = 640;
	double h = 40;
	if (sound.IsLoaded())
	{
		double colWidth = sound.animframes / w;
		GdkGC *GC = gdk_gc_new(widget->window);
		//GdkColor gdk_color_red = { 0, 65535, 0, 0 };
		GdkColor gdk_color_white = { 0, 65535, 65535, 65535 };
		GdkColor gdk_color_black = { 0, 0, 0, 0 };

		gdk_gc_set_rgb_fg_color(GC, &gdk_color_black);
		gdk_draw_line(widget->window, GC, 0, 0, w, 0);

		double seconds = sound.GetLength();

		for (int i = 0; i < seconds; i++)
		{
			double x = w / seconds * i;
			gdk_gc_set_rgb_fg_color(GC, &gdk_color_white);
			gdk_draw_line(widget->window, GC, x, 0, x, h);
		}

		for (int i = 0; i < w; i++)
		{
			double value = 0;
			if (userdata[0] == '0') value = sound.GetEnvelope(i * colWidth);
			else if (userdata[0] == 'A') value = sound.GetSpectrumA(i * colWidth);
			else if (userdata[0] == 'B') value = sound.GetSpectrumB(i * colWidth);
			else if (userdata[0] == 'C') value = sound.GetSpectrumC(i * colWidth);
			else if (userdata[0] == 'D') value = sound.GetSpectrumD(i * colWidth);

			GdkColor gdk_color = { 0, value * 0, value * 65535, value * 0 };
			gdk_gc_set_rgb_fg_color(GC, &gdk_color);
			gdk_draw_line(widget->window, GC, i, h - value * h, i, h);
		}
	}
	return true;
}

void StartRendering(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	ParamsReleaseMem(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;

	programClosed = true;
	isPostRendering = false;
	renderRequest = true;
}

void PressedAnimationRecord(GtkWidget *widget, gpointer data)
{
	if (!isRendering)
	{
		sParamRender params;
		ParamsAllocMem(&params);
		ReadInterface(&params);
		undoBuffer.SaveUndo(&params);
		ParamsReleaseMem(&params);

		Interface_data .animMode = true;
		Interface_data.playMode = false;
		Interface_data.recordMode = true;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;
		MainRender();
	}
}

void PressedAnimationContinueRecording(GtkWidget *widget, gpointer data)
{
	if (!isRendering)
	{
		sParamRender params;
		ParamsAllocMem(&params);
		ReadInterface(&params);
		undoBuffer.SaveUndo(&params);
		ParamsReleaseMem(&params);

		Interface_data .animMode = true;
		Interface_data.playMode = false;
		Interface_data.recordMode = true;
		Interface_data.continueRecord = true;
		MainRender();
	}
}

void PressedAnimationRender(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	ParamsReleaseMem(&params);

	if (!isRendering)
	{
		Interface_data .animMode = true;
		Interface_data.playMode = true;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;
		MainRender();
	}
}

void StopRendering(GtkWidget *widget, gpointer data)
{
	programClosed = true;
	isPostRendering = false;
	printf("Rendering terminated\n");
}

void PressedApplyBrigtness(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	ParamsReleaseMem(&params);

	GdkColor color;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorGlow1), &color);

	gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorGlow2), &color);

	gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorMainLight), &color);

	//generating color palette
	//srand(Interface_data.coloring_seed);
	//NowaPaleta(paleta, 1.0);
	DrawPalette(mainImage->GetPalettePtr());

	if (!isRendering)
	{
		mainImage->CompileImage();
		mainImage->ConvertTo8bit();
		mainImage->UpdatePreview();
		mainImage->RedrawInWidget(darea);
		while (gtk_events_pending())
			gtk_main_iteration();
	}
}

void PressedLoadSettings(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Load fractal settings", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilenameSettings);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		sParamRender fractParamLoaded;
		sParamSpecial fractParamSpecial;
		ParamsAllocMem(&fractParamLoaded);
		LoadSettings(filename, fractParamLoaded, &fractParamSpecial);
		Params2InterfaceData(&fractParamLoaded);
		WriteInterface(&fractParamLoaded, &fractParamSpecial);
		ParamsReleaseMem(&fractParamLoaded);

		strcpy(lastFilenameSettings, filename);
		char windowTitle[1000];
		sprintf(windowTitle, "Mandelbulber (%s)", filename);
		gtk_window_set_title(GTK_WINDOW(window_interface), windowTitle);

		timeline.reset(new cTimeline);
	}
	gtk_widget_destroy(dialog);

}

void PressedSaveSettings(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Save fractal settings", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
			GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilenameSettings);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		sParamRender fractParamToSave;
		sParamSpecial fractParamSpecial;
		ParamsAllocMem(&fractParamToSave);
		ReadInterface(&fractParamToSave, &fractParamSpecial);
		SaveSettings(filename, fractParamToSave, &fractParamSpecial);
		ParamsReleaseMem(&fractParamToSave);

		strcpy(lastFilenameSettings, filename);
		char windowTitle[1000];
		sprintf(windowTitle, "Mandelbulber (%s)", filename);
		gtk_window_set_title(GTK_WINDOW(window_interface), windowTitle);
	}
	gtk_widget_destroy(dialog);

}

void PressedSaveImage(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Save image as...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
			GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilenameImage);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		SaveJPEG(filename, 100, mainImage->GetWidth(), mainImage->GetHeight(), (JSAMPLE*) mainImage->ConvertTo8bit());
		strcpy(lastFilenameImage, filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedSaveImagePNG(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Save image as...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
			GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilenameImage);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		SavePNG(filename, 100, mainImage->GetWidth(), mainImage->GetHeight(), (JSAMPLE*) mainImage->ConvertTo8bit());
		strcpy(lastFilenameImage, filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedSaveImagePNG16(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Save image as...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
			GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilenameImage);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		SavePNG16(filename, 100, mainImage->GetWidth(), mainImage->GetHeight(), mainImage);
		strcpy(lastFilenameImage, filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedSaveImagePNG16Alpha(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Save image as...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
			GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilenameImage);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		SavePNG16Alpha(filename, 100, mainImage->GetWidth(), mainImage->GetHeight(), mainImage);
		strcpy(lastFilenameImage, filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedOkDialogFiles(GtkWidget *widget, gpointer data)
{
	sDialogFiles *dialog = (sDialogFiles*) data;
	printf("Files OK\n");

	strcpy(Interface_data.file_destination, gtk_entry_get_text(GTK_ENTRY(dialog->edit_destination)));
	strcpy(Interface_data.file_background, gtk_entry_get_text(GTK_ENTRY(dialog->edit_background)));
	strcpy(Interface_data.file_envmap, gtk_entry_get_text(GTK_ENTRY(dialog->edit_envmap)));
	strcpy(Interface_data.file_lightmap, gtk_entry_get_text(GTK_ENTRY(dialog->edit_lightmap)));
	strcpy(Interface_data.file_path, gtk_entry_get_text(GTK_ENTRY(dialog->edit_path)));
	strcpy(Interface_data.file_keyframes, gtk_entry_get_text(GTK_ENTRY(dialog->edit_keyframes)));
	strcpy(Interface_data.file_sound, gtk_entry_get_text(GTK_ENTRY(dialog->edit_sound)));

	gtk_widget_destroy(dialog->window_files);
	delete dialog;

	timeline.reset(new cTimeline);
}

void PressedCancelDialogFiles(GtkWidget *widget, gpointer data)
{
	sDialogFiles *dialog = (sDialogFiles*) data;
	printf("Files cancel\n");
	gtk_widget_destroy(dialog->window_files);
}

void PressedNavigatorUp(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	double rotation_step = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_rotation))) / 180.0 * M_PI;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkStraightRotation)))
	{
		params.doubles.beta -= rotation_step;
	}
	else
	{
		CRotationMatrix mRot;
		mRot.RotateZ(params.doubles.alfa);
		mRot.RotateX(params.doubles.beta);
		mRot.RotateY(params.doubles.gamma);
		mRot.RotateX(-rotation_step);

		params.doubles.alfa = -mRot.GetAlfa();
		params.doubles.beta = -mRot.GetBeta();
		params.doubles.gamma = -mRot.GetGamma();
	}

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void PressedNavigatorDown(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	double rotation_step = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_rotation))) / 180.0 * M_PI;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkStraightRotation)))
	{
		params.doubles.beta += rotation_step;
	}
	else
	{
		CRotationMatrix mRot;
		mRot.RotateZ(params.doubles.alfa);
		mRot.RotateX(params.doubles.beta);
		mRot.RotateY(params.doubles.gamma);
		mRot.RotateX(rotation_step);

		params.doubles.alfa = -mRot.GetAlfa();
		params.doubles.beta = -mRot.GetBeta();
		params.doubles.gamma = -mRot.GetGamma();
	}

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void PressedNavigatorLeft(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	double rotation_step = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_rotation))) / 180.0 * M_PI;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkStraightRotation)))
	{
		params.doubles.alfa += rotation_step;
	}
	else
	{
		CRotationMatrix mRot;
		mRot.RotateZ(params.doubles.alfa);
		mRot.RotateX(params.doubles.beta);
		mRot.RotateY(params.doubles.gamma);
		mRot.RotateZ(rotation_step);

		params.doubles.alfa = -mRot.GetAlfa();
		params.doubles.beta = -mRot.GetBeta();
		params.doubles.gamma = -mRot.GetGamma();
	}

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void PressedNavigatorRight(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	double rotation_step = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_rotation))) / 180.0 * M_PI;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkStraightRotation)))
	{
		params.doubles.alfa -= rotation_step;
	}
	else
	{
		CRotationMatrix mRot;
		mRot.RotateZ(params.doubles.alfa);
		mRot.RotateX(params.doubles.beta);
		mRot.RotateY(params.doubles.gamma);
		mRot.RotateZ(-rotation_step);

		params.doubles.alfa = -mRot.GetAlfa();
		params.doubles.beta = -mRot.GetBeta();
		params.doubles.gamma = -mRot.GetGamma();
	}

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void PressedNavigatorMoveUp(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	sFractal calcParam;
	sFractal_ret calcRet;

	CopyParams(&params, &calcParam);

	double distance = 0;
	double speed = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_forward)));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorAbsoluteDistance)))
	{
		distance = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_NavigatorAbsoluteDistance)));
	}
	else
	{
		distance = CalculateDistance(calcParam, calcRet) * speed;
	}

	CVector3 vDelta;
	CRotationMatrix mRot;
	mRot.RotateZ(params.doubles.alfa);
	mRot.RotateX(params.doubles.beta);
	mRot.RotateY(params.doubles.gamma);
	CVector3 directionVector(0, 0, -distance);
	vDelta = mRot.RotateVector(directionVector);
	params.doubles.vp += vDelta;

	CopyParams(&params, &calcParam);
	char distanceString[1000];
	distance = CalculateDistance(calcParam, calcRet);
	double key_distance = (params.doubles.vp - last_keyframe_position).Length();
	sprintf(distanceString, "Estimated viewpoint distance to the surface: %g\nDistance from last keyframe: %g", distance, key_distance);
	gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void PressedNavigatorMoveDown(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	sFractal calcParam;
	sFractal_ret calcRet;

	CopyParams(&params, &calcParam);

	double distance = 0;
	double speed = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_forward)));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorAbsoluteDistance)))
	{
		distance = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_NavigatorAbsoluteDistance)));
	}
	else
	{
		distance = CalculateDistance(calcParam, calcRet) * speed;
	}

	CVector3 vDelta;
	CRotationMatrix mRot;
	mRot.RotateZ(params.doubles.alfa);
	mRot.RotateX(params.doubles.beta);
	mRot.RotateY(params.doubles.gamma);
	CVector3 directionVector(0, 0, distance);
	vDelta = mRot.RotateVector(directionVector);
	params.doubles.vp += vDelta;

	CopyParams(&params, &calcParam);
	char distanceString[1000];
	distance = CalculateDistance(calcParam, calcRet);
	double key_distance = (params.doubles.vp - last_keyframe_position).Length();
	sprintf(distanceString, "Estimated viewpoint distance to the surface: %g\nDistance from last keyframe: %g", distance, key_distance);
	gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void PressedNavigatorMoveLeft(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	sFractal calcParam;
	sFractal_ret calcRet;

	CopyParams(&params, &calcParam);

	double distance = 0;
	double speed = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_forward)));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorAbsoluteDistance)))
	{
		distance = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_NavigatorAbsoluteDistance)));
	}
	else
	{
		distance = CalculateDistance(calcParam, calcRet) * speed;
	}

	CVector3 vDelta;
	CRotationMatrix mRot;
	mRot.RotateZ(params.doubles.alfa);
	mRot.RotateX(params.doubles.beta);
	mRot.RotateY(params.doubles.gamma);
	CVector3 directionVector(-distance, 0, 0);
	vDelta = mRot.RotateVector(directionVector);
	params.doubles.vp += vDelta;

	CopyParams(&params, &calcParam);
	char distanceString[1000];
	distance = CalculateDistance(calcParam, calcRet);
	double key_distance = (params.doubles.vp - last_keyframe_position).Length();
	sprintf(distanceString, "Estimated viewpoint distance to the surface: %g\nDistance from last keyframe: %g", distance, key_distance);
	gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void PressedNavigatorMoveRight(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	sFractal calcParam;
	sFractal_ret calcRet;

	CopyParams(&params, &calcParam);

	double distance = 0;
	double speed = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_forward)));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorAbsoluteDistance)))
	{
		distance = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_NavigatorAbsoluteDistance)));
	}
	else
	{
		distance = CalculateDistance(calcParam, calcRet) * speed;
	}

	CVector3 vDelta;
	CRotationMatrix mRot;
	mRot.RotateZ(params.doubles.alfa);
	mRot.RotateX(params.doubles.beta);
	mRot.RotateY(params.doubles.gamma);
	CVector3 directionVector(distance, 0, 0);
	vDelta = mRot.RotateVector(directionVector);
	params.doubles.vp += vDelta;

	CopyParams(&params, &calcParam);
	char distanceString[1000];
	distance = CalculateDistance(calcParam, calcRet);
	double key_distance = (params.doubles.vp - last_keyframe_position).Length();
	sprintf(distanceString, "Estimated viewpoint distance to the surface: %g\nDistance from last keyframe: %g", distance, key_distance);
	gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void PressedNavigatorForward(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	sFractal calcParam;
	sFractal_ret calcRet;

	CopyParams(&params, &calcParam);

	double distance = 0;
	double speed = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_forward)));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorAbsoluteDistance)))
	{
		distance = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_NavigatorAbsoluteDistance)));
	}
	else
	{
		distance = CalculateDistance(calcParam, calcRet) * speed;
	}
	last_navigator_step = distance;

	CVector3 vDelta;
	CRotationMatrix mRot;
	mRot.RotateZ(params.doubles.alfa);
	mRot.RotateX(params.doubles.beta);
	mRot.RotateY(params.doubles.gamma);
	CVector3 directionVector(0, distance, 0);
	vDelta = mRot.RotateVector(directionVector);
	params.doubles.vp += vDelta;

	CopyParams(&params, &calcParam);
	char distanceString[1000];
	distance = CalculateDistance(calcParam, calcRet);
	double key_distance = (params.doubles.vp - last_keyframe_position).Length();
	sprintf(distanceString, "Estimated viewpoint distance to the surface: %g\nDistance from last keyframe: %g", distance, key_distance);
	gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void PressedNavigatorBackward(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	double distance = 0;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorAbsoluteDistance)))
	{
		distance = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_NavigatorAbsoluteDistance)));
	}
	else
	{
		distance = last_navigator_step;
	}

	CVector3 vDelta;
	CRotationMatrix mRot;
	mRot.RotateZ(params.doubles.alfa);
	mRot.RotateX(params.doubles.beta);
	mRot.RotateY(params.doubles.gamma);
	CVector3 directionVector(0, distance, 0);
	vDelta = mRot.RotateVector(directionVector);
	params.doubles.vp -= vDelta;

	sFractal calcParam;
	sFractal_ret calcRet;
	CopyParams(&params, &calcParam);
	char distanceString[1000];
	distance = CalculateDistance(calcParam, calcRet);
	double key_distance = (params.doubles.vp - last_keyframe_position).Length();
	sprintf(distanceString, "Estimated viewpoint distance to the surface: %g\nDistance from last keyframe: %g", distance, key_distance);
	gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void PressedNavigatorInit(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	params.doubles.zoom = 1e-7;
	params.doubles.vp.x = 10;
	params.doubles.vp.y = -10;
	params.doubles.vp.z = -10;
	params.doubles.alfa = 45 * 2 * M_PI / 360.0;
	params.doubles.beta = 38 * 2 * M_PI / 360.0;
	params.doubles.persp = 1.2;
	params.doubles.max_y = 10.0 / params.doubles.zoom;

	sFractal calcParam;
	sFractal_ret calcRet;
	CopyParams(&params, &calcParam);
	char distanceString[1000];
	double distance = CalculateDistance(calcParam, calcRet);
	sprintf(distanceString, "Estimated viewpoint distance to the surface: %g", distance);
	gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

	WriteInterface(&params);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;
	programClosed = true;
	isPostRendering = false;
	renderRequest = true;

	ParamsReleaseMem(&params);
}

void ChangedComboScale(GtkWidget *widget, gpointer data)
{
	GtkComboBox *combo = GTK_COMBO_BOX(widget);
	int scale = gtk_combo_box_get_active(combo);
	double imageScale;
	if (scale == 0) imageScale = 1.0 / 10.0;
	if (scale == 1) imageScale = 1.0 / 8.0;
	if (scale == 2) imageScale = 1.0 / 6.0;
	if (scale == 3) imageScale = 1.0 / 4.0;
	if (scale == 4) imageScale = 1.0 / 3.0;
	if (scale == 5) imageScale = 1.0 / 2.0;
	if (scale == 6) imageScale = 1.0;
	if (scale == 7) imageScale = 2.0;
	if (scale == 8) imageScale = 4.0;
	if (scale == 9) imageScale = 6.0;
	if (scale == 10) imageScale = 8.0;
	if (scale == 11)
	{
		int winWidth;
		int winHeight;
		gtk_window_get_size(GTK_WINDOW(window2), &winWidth, &winHeight);
		winWidth -= scrollbarSize;
		winHeight -= scrollbarSize;
		imageScale = (double) winWidth / mainImage->GetWidth();
		if (mainImage->GetHeight() * imageScale > winHeight) imageScale = (double) winHeight / mainImage->GetHeight();
	}
	Interface_data.imageScale = imageScale;

	mainImage->CreatePreview(imageScale);
	gtk_widget_set_size_request(darea, mainImage->GetPreviewWidth(), mainImage->GetPreviewHeight());

	mainImage->ConvertTo8bit();
	mainImage->UpdatePreview();
	mainImage->RedrawInWidget(darea);

	while (gtk_events_pending())
		gtk_main_iteration();
}

void ChangedComboFormula(GtkWidget *widget, gpointer data)
{
	int formula = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboFractType));

	if (formula == 0 || formula == 1)
	{
		gtk_widget_set_sensitive(Interface.checkFastAmbientOcclusion, true);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkFastAmbientOcclusion), false);
		gtk_widget_set_sensitive(Interface.checkFastAmbientOcclusion, false);
	}

	if (formula == 8)
	{
		gtk_widget_set_sensitive(Interface.tab_box_mandelbox, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.tab_box_mandelbox, false);
	}

	if ((formula == 9) || gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSFoldingMode)))
	{
		gtk_widget_set_sensitive(Interface.tab_box_IFS, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.tab_box_IFS, false);
	}

	if (formula == 0 || formula == 1 || formula == 4 || formula == 12)
	{
		gtk_widget_set_sensitive(Interface.edit_power, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.edit_power, false);
	}

	if ((formula == 13))
	{
		gtk_widget_set_sensitive(Interface.tab_box_hybrid, true);
		gtk_widget_set_sensitive(Interface.tab_box_IFS, true);
		gtk_widget_set_sensitive(Interface.tab_box_mandelbox, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.tab_box_hybrid, false);
	}
}

void ChangedTgladFoldingMode(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkTgladMode)))
	{
		gtk_widget_set_sensitive(Interface.edit_tglad_folding_1, true);
		gtk_widget_set_sensitive(Interface.edit_tglad_folding_2, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.edit_tglad_folding_1, false);
		gtk_widget_set_sensitive(Interface.edit_tglad_folding_2, false);
	}
}

void ChangedSphericalFoldingMode(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkSphericalFoldingMode)))
	{
		gtk_widget_set_sensitive(Interface.edit_spherical_folding_1, true);
		gtk_widget_set_sensitive(Interface.edit_spherical_folding_2, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.edit_spherical_folding_1, false);
		gtk_widget_set_sensitive(Interface.edit_spherical_folding_2, false);
	}
}

void ChangedIFSFoldingMode(GtkWidget *widget, gpointer data)
{
	int formula = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboFractType));
	if ((formula == 9) || gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSFoldingMode)))
	{
		gtk_widget_set_sensitive(Interface.tab_box_IFS, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.tab_box_IFS, false);
	}
}

void ChangedJulia(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkJulia)))
	{
		gtk_widget_set_sensitive(Interface.edit_julia_a, true);
		gtk_widget_set_sensitive(Interface.edit_julia_b, true);
		gtk_widget_set_sensitive(Interface.edit_julia_c, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.edit_julia_a, false);
		gtk_widget_set_sensitive(Interface.edit_julia_b, false);
		gtk_widget_set_sensitive(Interface.edit_julia_c, false);
	}
}

void ChangedLimits(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkLimits)))
	{
		gtk_widget_set_sensitive(Interface.edit_amin, true);
		gtk_widget_set_sensitive(Interface.edit_amax, true);
		gtk_widget_set_sensitive(Interface.edit_bmin, true);
		gtk_widget_set_sensitive(Interface.edit_bmax, true);
		gtk_widget_set_sensitive(Interface.edit_cmin, true);
		gtk_widget_set_sensitive(Interface.edit_cmax, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.edit_amin, false);
		gtk_widget_set_sensitive(Interface.edit_amax, false);
		gtk_widget_set_sensitive(Interface.edit_bmin, false);
		gtk_widget_set_sensitive(Interface.edit_bmax, false);
		gtk_widget_set_sensitive(Interface.edit_cmin, false);
		gtk_widget_set_sensitive(Interface.edit_cmax, false);
	}
}

void ChangedMandelboxRotations(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkMandelboxRotationsEnable)))
	{
		gtk_widget_set_sensitive(Interface.tableMandelboxRotations, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.tableMandelboxRotations, false);
	}
}

void ChangedSliderFog(GtkWidget *widget, gpointer data)
{
	if (Interface_data.disableInitRefresh) return;
	GdkColor color;

	gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorFog), &color);
	sRGB color2 = { color.red, color.green, color.blue };

	sImageAdjustments *adj = mainImage->GetImageAdjustments();
	adj->fogVisibility = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepth));
	adj->fogVisibilityFront = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepthFront));
	sImageSwitches *sw = mainImage->GetImageSwitches();
	sw->fogEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkFogEnabled));
	sEffectColours *ecol = mainImage->GetEffectColours();
	ecol->fogColor = color2;

	if (!isRendering)
	{
		mainImage->CompileImage();
		mainImage->ConvertTo8bit();
		mainImage->UpdatePreview();
		mainImage->RedrawInWidget(darea);
		while (gtk_events_pending())
			gtk_main_iteration();
	}
}

void PressedSSAOUpdate(GtkWidget *widget, gpointer data)
{
	if (Interface_data.disableInitRefresh) return;
	double SSAOQuality = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentSSAOQuality));
	bool SSAOEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkSSAOEnabled));
	double persp = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_persp)));
	bool fishEye = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkFishEye));
	if (!isRendering)
	{
		if (SSAOEnabled)
		{
			PostRendering_SSAO(mainImage, persp, SSAOQuality, fishEye, false);
		}
		mainImage->CompileImage();
		mainImage->ConvertTo8bit();
		mainImage->UpdatePreview();
		mainImage->RedrawInWidget(darea);
		while (gtk_events_pending())
			gtk_main_iteration();
	}
}

void PressedDOFUpdate(GtkWidget *widget, gpointer data)
{
	if (Interface_data.disableInitRefresh) return;
	double DOFFocus = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentDOFFocus));
	double DOFRadius = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentDOFRadius));
	bool DOFEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkDOFEnabled));
	double persp = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_persp)));
	if (!isRendering)
	{
		mainImage->CompileImage();
		if (DOFEnabled)
		{
			double DOF_focus = pow(10, DOFFocus / 10.0 - 2.0) - 1.0 / persp;
			PostRendering_DOF(mainImage, DOFRadius, DOF_focus, persp);
		}
		mainImage->ConvertTo8bit();
		mainImage->UpdatePreview();
		mainImage->RedrawInWidget(darea);
		while (gtk_events_pending())
			gtk_main_iteration();
	}
}

void PressedDistributeLights(GtkWidget *widget, gpointer data)
{
	sParamRender fractParams;
	ParamsAllocMem(&fractParams);
	ReadInterface(&fractParams);
	undoBuffer.SaveUndo(&fractParams);
	PlaceRandomLights(&fractParams);
	ParamsReleaseMem(&fractParams);
}

void CopyParams(sParamRender *src, sFractal *dest)
{
	dest->amin = src->doubles.amin;
	dest->amax = src->doubles.amax;
	dest->bmin = src->doubles.bmin;
	dest->bmax = src->doubles.bmax;
	dest->cmin = src->doubles.cmin;
	dest->cmax = src->doubles.cmax;
	dest->limits_enabled = src->limits_enabled;
	dest->N = src->N;
	dest->minN = src->minN;
	dest->power = src->doubles.power;
	dest->analitycDE = src->analitycDE;
	dest->formula = src->formula;
	dest->iterThresh = src->iterThresh;
	dest->julia = src->doubles.julia;
	dest->point = src->doubles.vp;
	dest->juliaMode = src->juliaMode;
	dest->DE_threshold = 0;
	dest->tgladFoldingMode = src->tgladFoldingMode;
	dest->sphericalFoldingMode = src->sphericalFoldingMode;
	dest->IFSFoldingMode = src->IFSFoldingMode;
	dest->foldingLimit = src->doubles.foldingLimit;
	dest->foldingValue = src->doubles.foldingValue;
	dest->foldingSphericalFixed = src->doubles.foldingSphericalFixed;
	dest->foldingSphericalMin = src->doubles.foldingSphericalMin;

	dest->IFSDirection = src->IFSDirection;
	dest->IFSDistance = src->IFSDistance;
	dest->IFSOffset = src->doubles.IFSOffset;
	dest->IFSRotationAlfa = src->doubles.IFSRotationAlfa;
	dest->IFSRotationBeta = src->doubles.IFSRotationBeta;
	dest->IFSRotationGamma = src->doubles.IFSRotationGamma;
	dest->IFSfoldingCount = src->IFSfoldingCount;
	dest->IFSEnabled = src->IFSEnabled;
	dest->IFSAlfa = src->IFSAlfa;
	dest->IFSBeta = src->IFSBeta;
	dest->IFSGamma = src->IFSGamma;
	dest->IFSIntensity = src->IFSIntensity;
	dest->IFSScale = src->doubles.IFSScale;
	dest->IFSRot = src->IFSRot;
	dest->IFSMainRot = src->IFSMainRot;
	dest->IFSAbsX = src->IFSAbsX;
	dest->IFSAbsY = src->IFSAbsY;
	dest->IFSAbsZ = src->IFSAbsZ;

	dest->hybridFormula1 = src->hybridFormula1;
	dest->hybridFormula2 = src->hybridFormula2;
	dest->hybridFormula3 = src->hybridFormula3;
	dest->hybridFormula4 = src->hybridFormula4;
	dest->hybridFormula5 = src->hybridFormula5;
	dest->hybridIters1 = src->hybridIters1;
	dest->hybridIters2 = src->hybridIters2;
	dest->hybridIters3 = src->hybridIters3;
	dest->hybridIters4 = src->hybridIters4;
	dest->hybridIters5 = src->hybridIters5;
	dest->hybridPower1 = src->doubles.hybridPower1;
	dest->hybridPower2 = src->doubles.hybridPower2;
	dest->hybridPower3 = src->doubles.hybridPower3;
	dest->hybridPower4 = src->doubles.hybridPower4;
	dest->hybridPower5 = src->doubles.hybridPower5;
	dest->formulaSequence = src->formulaSequence;
	dest->hybridPowerSequence = src->hybridPowerSequence;
	dest->hybridCyclic = src->hybridCyclic;

	dest->mandelboxMainRot = src->mandelboxMainRot;
	dest-> mandelboxRot1X = src->mandelboxRot1X;
	dest-> mandelboxRot1Y = src->mandelboxRot1Y;
	dest-> mandelboxRot1Z = src->mandelboxRot1Z;
	dest-> mandelboxRot2X = src->mandelboxRot2X;
	dest-> mandelboxRot2Y = src->mandelboxRot2Y;
	dest-> mandelboxRot2Z = src->mandelboxRot2Z;
	dest-> mandelboxRot1Xinv = src->mandelboxRot1Xinv;
	dest-> mandelboxRot1Yinv = src->mandelboxRot1Yinv;
	dest-> mandelboxRot1Zinv = src->mandelboxRot1Zinv;
	dest-> mandelboxRot2Xinv = src->mandelboxRot2Xinv;
	dest-> mandelboxRot2Yinv = src->mandelboxRot2Yinv;
	dest-> mandelboxRot2Zinv = src->mandelboxRot2Zinv;

	dest->mandelboxScale = src->doubles.mandelboxScale;
	dest->mandelboxFoldingLimit = src->doubles.mandelboxFoldingLimit;
	dest->mandelboxFoldingValue = src->doubles.mandelboxFoldingValue;
	dest->mandelboxFoldingSphericalMin = src->doubles.mandelboxFoldingSphericalMin;
	dest->mandelboxFoldingSphericalFixed = src->doubles.mandelboxFoldingSphericalFixed;

	dest->mandelboxColorFactorR = src->doubles.mandelboxColorFactorR;
	dest->mandelboxColorFactorX = src->doubles.mandelboxColorFactorX;
	dest->mandelboxColorFactorY = src->doubles.mandelboxColorFactorY;
	dest->mandelboxColorFactorZ = src->doubles.mandelboxColorFactorZ;
	dest->mandelboxColorFactorSp1 = src->doubles.mandelboxColorFactorSp1;
	dest->mandelboxColorFactorSp2 = src->doubles.mandelboxColorFactorSp2;

	dest->mandelboxRotationsEnabled  = src->mandelboxRotationsEnabled;

	dest->interiorMode = src->interiorMode;
}

void RecalculateIFSParams(sParamRender *params)
{
	params->IFSMainRot.Null();
	params->IFSMainRot.RotateZ(params->doubles.IFSRotationAlfa);
	params->IFSMainRot.RotateY(params->doubles.IFSRotationBeta);
	params->IFSMainRot.RotateX(params->doubles.IFSRotationGamma);
	for (int i = 0; i < IFS_number_of_vectors; i++)
	{
		params->IFSRot[i].Null();
		params->IFSRot[i].RotateZ(params->IFSAlfa[i]);
		params->IFSRot[i].RotateY(params->IFSBeta[i]);
		params->IFSRot[i].RotateX(params->IFSGamma[i]);
		params->IFSDirection[i].Normalize();
	}

	params->mandelboxMainRot.Null();
	params->mandelboxMainRot.RotateZ(params->doubles.mandelboxRotationMainGamma);
	params->mandelboxMainRot.RotateY(params->doubles.mandelboxRotationMainBeta);
	params->mandelboxMainRot.RotateX(params->doubles.mandelboxRotationMainAlfa);

	params->mandelboxRot1X.Null();
	params->mandelboxRot1X.RotateZ(params->doubles.mandelboxRotationX1Gamma);
	params->mandelboxRot1X.RotateY(params->doubles.mandelboxRotationX1Beta);
	params->mandelboxRot1X.RotateX(params->doubles.mandelboxRotationX1Alfa);
	params->mandelboxRot2X.Null();
	params->mandelboxRot2X.RotateZ(params->doubles.mandelboxRotationX2Gamma);
	params->mandelboxRot2X.RotateY(params->doubles.mandelboxRotationX2Beta);
	params->mandelboxRot2X.RotateX(params->doubles.mandelboxRotationX2Alfa);
	params->mandelboxRot1Y.Null();
	params->mandelboxRot1Y.RotateZ(params->doubles.mandelboxRotationY1Gamma);
	params->mandelboxRot1Y.RotateY(params->doubles.mandelboxRotationY1Beta);
	params->mandelboxRot1Y.RotateX(params->doubles.mandelboxRotationY1Alfa);
	params->mandelboxRot2Y.Null();
	params->mandelboxRot2Y.RotateZ(params->doubles.mandelboxRotationY2Gamma);
	params->mandelboxRot2Y.RotateY(params->doubles.mandelboxRotationY2Beta);
	params->mandelboxRot2Y.RotateX(params->doubles.mandelboxRotationY2Alfa);
	params->mandelboxRot1Z.Null();
	params->mandelboxRot1Z.RotateZ(params->doubles.mandelboxRotationZ1Gamma);
	params->mandelboxRot1Z.RotateY(params->doubles.mandelboxRotationZ1Beta);
	params->mandelboxRot1Z.RotateX(params->doubles.mandelboxRotationZ1Alfa);
	params->mandelboxRot2Z.Null();
	params->mandelboxRot2Z.RotateZ(params->doubles.mandelboxRotationZ2Gamma);
	params->mandelboxRot2Z.RotateY(params->doubles.mandelboxRotationZ2Beta);
	params->mandelboxRot2Z.RotateX(params->doubles.mandelboxRotationZ2Alfa);

	params->mandelboxRot1Xinv = params->mandelboxRot1X.Transpose();
	params->mandelboxRot1Yinv = params->mandelboxRot1Y.Transpose();
	params->mandelboxRot1Zinv = params->mandelboxRot1Z.Transpose();
	params->mandelboxRot2Xinv = params->mandelboxRot2X.Transpose();
	params->mandelboxRot2Yinv = params->mandelboxRot2Y.Transpose();
	params->mandelboxRot2Zinv = params->mandelboxRot2Z.Transpose();
}

void CreateFormulaSequence(sParamRender *params)
{

	if (params->formulaSequence != 0) delete[] params->formulaSequence;
	if (params->hybridPowerSequence != 0) delete[] params->hybridPowerSequence;
	params->formulaSequence = new enumFractalFormula[params->N];
	params->hybridPowerSequence = new double[params->N];
	WriteLog("Allocated memory for hybrid formula sequence");

	int number = 0;
	while (number < params->N)
	{
		for (int i = 0; i < params->hybridIters1; i++)
		{
			if (params->hybridFormula1 == none) break;
			if (number < params->N)
			{
				params->formulaSequence[number] = params->hybridFormula1;
				params->hybridPowerSequence[number] = params->doubles.hybridPower1;
			}
			number++;
		}
		for (int i = 0; i < params->hybridIters2; i++)
		{
			if (params->hybridFormula2 == none) break;
			if (number < params->N)
			{
				params->formulaSequence[number] = params->hybridFormula2;
				params->hybridPowerSequence[number] = params->doubles.hybridPower2;
			}
			number++;
		}
		for (int i = 0; i < params->hybridIters3; i++)
		{
			if (params->hybridFormula3 == none) break;
			if (number < params->N)
			{
				params->formulaSequence[number] = params->hybridFormula3;
				params->hybridPowerSequence[number] = params->doubles.hybridPower3;
			}
			number++;
		}
		for (int i = 0; i < params->hybridIters4; i++)
		{
			if (params->hybridFormula4 == none) break;
			if (number < params->N)
			{
				params->formulaSequence[number] = params->hybridFormula4;
				params->hybridPowerSequence[number] = params->doubles.hybridPower4;
			}
			number++;
		}

		int temp_end = params->N;
		if (params->hybridCyclic) temp_end = params->hybridIters5;
		for (int i = 0; i < temp_end; i++)
		{
			if (params->hybridFormula5 == none) break;
			if (number < params->N)
			{
				params->formulaSequence[number] = params->hybridFormula5;
				params->hybridPowerSequence[number] = params->doubles.hybridPower5;
			}
			number++;
		}

		if (!params->hybridCyclic) break;
	}
}

void PressedIFSNormalizeOffset(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	params.doubles.IFSOffset.Normalize();
	WriteInterface(&params);
	ParamsReleaseMem(&params);
}

void PressedIFSNormalizeVectors(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	for (int i = 0; i < IFS_number_of_vectors; i++)
	{
		params.IFSDirection[i].Normalize();
	}

	WriteInterface(&params);
	ParamsReleaseMem(&params);
}

void PressedRecordKeyframe(GtkWidget *widget, gpointer data)
{
	char filename2[1000];
	int index = atoi(gtk_entry_get_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber)));
	IndexFilename(filename2, Interface_data.file_keyframes, (char*) "fract", index);

	sParamRender fractParamToSave;
	ParamsAllocMem(&fractParamToSave);
	ReadInterface(&fractParamToSave);
	SaveSettings(filename2, fractParamToSave);
	last_keyframe_position = fractParamToSave.doubles.vp;

	timeline->RecordKeyframe(index,filename2, false);

	index++;
	gtk_entry_set_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber), IntToString(index));

	//loading next keyframe if exists
	IndexFilename(filename2, Interface_data.file_keyframes, (char*) "fract", index);
	if (FileIfExist(filename2))
	{
		sParamRender fractParamLoaded;
		ParamsAllocMem(&fractParamLoaded);
		LoadSettings(filename2, fractParamLoaded, NULL, true);
		WriteInterface(&fractParamLoaded);
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

	ParamsReleaseMem(&fractParamToSave);
}

void PressedInsertKeyframe(GtkWidget *widget, gpointer data)
{
	char filename1[1000];
	char filename2[1000];

		int index = atoi(gtk_entry_get_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber)))+1;
		int maxIndex = timeline->CheckNumberOfKeyframes(Interface_data.file_keyframes)-1;
		printf("Maxindex = %d\n",maxIndex);

		if(index > maxIndex) index = maxIndex+1;

		for(int i=maxIndex; i>=index; i--)
		{
			IndexFilename(filename1, Interface_data.file_keyframes, (char*) "fract", i);
			IndexFilename(filename2, Interface_data.file_keyframes, (char*) "fract", i+1);
			rename(filename1,filename2);
		}

		IndexFilename(filename2, Interface_data.file_keyframes, (char*) "fract", index);

		sParamRender fractParamToSave;
		ParamsAllocMem(&fractParamToSave);
		ReadInterface(&fractParamToSave);
		SaveSettings(filename2, fractParamToSave);
		last_keyframe_position = fractParamToSave.doubles.vp;

		timeline->RecordKeyframe(index,filename2, true);

		index++;
		gtk_entry_set_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber), IntToString(index));

		//loading next keyframe if exists
		IndexFilename(filename2, Interface_data.file_keyframes, (char*) "fract", index);
		if (FileIfExist(filename2))
		{
			sParamRender fractParamLoaded;
			ParamsAllocMem(&fractParamLoaded);
			LoadSettings(filename2, fractParamLoaded, NULL, true);
			WriteInterface(&fractParamLoaded);
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

		ParamsReleaseMem(&fractParamToSave);
}

void PressedKeyframeAnimationRender(GtkWidget *widget, gpointer data)
{
	if (!isRendering)
	{
		Interface_data .animMode = true;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = true;
		MainRender();
	}
}

void PressedNextKeyframe(GtkWidget *widget, gpointer data)
{
	int index = atoi(gtk_entry_get_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber)));
	index++;
	gtk_entry_set_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber), IntToString(index));

	char filename2[1000];

	IndexFilename(filename2, Interface_data.file_keyframes, (char*) "fract", index);
	if (FileIfExist(filename2))
	{
		sParamRender fractParamLoaded;
		ParamsAllocMem(&fractParamLoaded);
		LoadSettings(filename2, fractParamLoaded, NULL, true);
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
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL, "Error! Keyframe not exists: %s", filename2);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		index--;
		gtk_entry_set_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber), IntToString(index));
	}
}

void PressedPreviousKeyframe(GtkWidget *widget, gpointer data)
{
	int index = atoi(gtk_entry_get_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber)));
	index--;
	gtk_entry_set_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber), IntToString(index));

	char filename2[1000];
	IndexFilename(filename2, Interface_data.file_keyframes, (char*) "fract", index);

	if (FileIfExist(filename2))
	{
		sParamRender fractParamLoaded;
		ParamsAllocMem(&fractParamLoaded);
		LoadSettings(filename2, fractParamLoaded, NULL, true);
		last_keyframe_position = fractParamLoaded.doubles.vp;
		WriteInterface(&fractParamLoaded);
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
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL, "Error! Keyframe not exists: %s", filename2);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		index++;
		gtk_entry_set_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber), IntToString(index));
	}
}

void PressedUndo(GtkWidget *widget, gpointer data)
{
	sParamRender undoParams;
	ParamsAllocMem(&undoParams);
	if (undoBuffer.GetUndo(&undoParams))
	{
		WriteInterface(&undoParams);
	}
	ParamsReleaseMem(&undoParams);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;

	programClosed = true;
	isPostRendering = false;
	renderRequest = true;
}

void PressedRedo(GtkWidget *widget, gpointer data)
{
	sParamRender undoParams;
	ParamsAllocMem(&undoParams);
	if (undoBuffer.GetRedo(&undoParams))
	{
		WriteInterface(&undoParams);
	}
	ParamsReleaseMem(&undoParams);

	Interface_data.animMode = false;
	Interface_data.playMode = false;
	Interface_data.recordMode = false;
	Interface_data.continueRecord = false;
	Interface_data.keyframeMode = false;

	programClosed = true;
	isPostRendering = false;
	renderRequest = true;
}

void PressedBuddhabrot(GtkWidget *widget, gpointer data)
{
	/*
	 if (!isRendering)
	 {
	 CompileImage();
	 sParamRender params;
	 ParamsAllocMem(&params);
	 ReadInterface(&params);
	 undoBuffer.SaveUndo(&params);
	 RenderBuddhabrot(&params);
	 ParamsReleaseMem(&params);
	 }
	 */
}

void PressedSelectDestination(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Select destination for image sequence...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), "images");
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "image");

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		printf("filename: %s\n", filename);
		sDialogFiles *dialogFiles = (sDialogFiles*) data;
		gtk_entry_set_text(GTK_ENTRY(dialogFiles->edit_destination), filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedSelectBackground(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Select background image...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), Interface_data.file_background);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		printf("filename: %s\n", filename);
		sDialogFiles *dialogFiles = (sDialogFiles*) data;
		gtk_entry_set_text(GTK_ENTRY(dialogFiles->edit_background), filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedSelectEnvmap(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Select environment map image...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), Interface_data.file_envmap);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		printf("filename: %s\n", filename);
		sDialogFiles *dialogFiles = (sDialogFiles*) data;
		gtk_entry_set_text(GTK_ENTRY(dialogFiles->edit_envmap), filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedSelectLightmap(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Select image with light map for ambient occlusion...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), Interface_data.file_lightmap);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		printf("filename: %s\n", filename);
		sDialogFiles *dialogFiles = (sDialogFiles*) data;
		gtk_entry_set_text(GTK_ENTRY(dialogFiles->edit_lightmap), filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedSelectFlightPath(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Select file with flight path...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), "paths");
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "path.txt");

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		printf("filename: %s\n", filename);
		sDialogFiles *dialogFiles = (sDialogFiles*) data;
		gtk_entry_set_text(GTK_ENTRY(dialogFiles->edit_path), filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedSelectKeyframes(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Select first file with key-frame sequence...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), "keyframes");
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "keyframe");

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (FileIfExist(filename))
		{
			filename[strlen(filename) - 11] = 0;
		}

		printf("filename: %s\n", filename);
		sDialogFiles *dialogFiles = (sDialogFiles*) data;
		gtk_entry_set_text(GTK_ENTRY(dialogFiles->edit_keyframes), filename);
	}

	gtk_widget_destroy(dialog);
}

void PressedSelectSound(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Select sound file (*.wav)...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), Interface_data.file_sound);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		printf("filename: %s\n", filename);
		sDialogFiles *dialogFiles = (sDialogFiles*) data;
		gtk_entry_set_text(GTK_ENTRY(dialogFiles->edit_sound), filename);
	}

	gtk_widget_destroy(dialog);
}

void ChangedSliderPaletteOffset(GtkWidget *widget, gpointer data)
{
	if (!interfaceCreated) return;
	sImageAdjustments *adj = mainImage->GetImageAdjustments();
	adj->paletteOffset = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentPaletteOffset));
	//srand(Interface_data.coloring_seed);
	//NowaPaleta(paleta, 1.0);
	DrawPalette(mainImage->GetPalettePtr());
	if (!isRendering && !Interface_data.disableInitRefresh)
	{
		mainImage->CompileImage();
		mainImage->ConvertTo8bit();
		mainImage->UpdatePreview();
		mainImage->RedrawInWidget(darea);
		while (gtk_events_pending())
			gtk_main_iteration();
	}
}

void PressedRandomPalette(GtkWidget *widget, gpointer data)
{
	//srand(clock());
	srand((unsigned int) ((double) clock() * 1000.0 / CLOCKS_PER_SEC));
	int coloring_seed = Random(999999);
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_color_seed), IntToString(coloring_seed));

	srand(coloring_seed);
	NowaPaleta(mainImage->GetPalettePtr(), 1.0);
	DrawPalette(mainImage->GetPalettePtr());
	if (!isRendering && !Interface_data.disableInitRefresh)
	{
		mainImage->CompileImage();
		mainImage->ConvertTo8bit();
		mainImage->UpdatePreview();
		mainImage->RedrawInWidget(darea);
		while (gtk_events_pending())
			gtk_main_iteration();
	}
}

void PressedLoadSound(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ParamsAllocMem(&params);
	ReadInterface(&params);

	sound.Load(Interface_data.file_sound);
	sound.SetFPS(params.doubles.soundFPS);
	sound.CreateEnvelope();
	int bandMin[4] = { params.soundBand1Min, params.soundBand2Min, params.soundBand3Min, params.soundBand4Min };
	int bandMax[4] = { params.soundBand1Max, params.soundBand2Max, params.soundBand3Max, params.soundBand4Max };

	sound.DoFFT(bandMin, bandMax);
	gtk_widget_queue_draw(Interface.dareaSound0);
	gtk_widget_queue_draw(Interface.dareaSoundA);
	gtk_widget_queue_draw(Interface.dareaSoundB);
	gtk_widget_queue_draw(Interface.dareaSoundC);
	gtk_widget_queue_draw(Interface.dareaSoundD);

	ParamsReleaseMem(&params);
}

void PressedGetPaletteFromImage(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Please select image to grab colour palette...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilenamePalette);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		printf("filename: %s\n", filename);

		cTexture paletteImage(filename);

		sRGB palette[256];
		if (paletteImage.IsLoaded())
		{
			int width = paletteImage.Width();
			int height = paletteImage.Height();
			for (int i = 0; i < 256; i++)
			{
				double angle = i / 256.0 * M_PI * 2.0;
				double x = width / 2 + cos(angle) * width * 0.4;
				double y = height / 2 + sin(angle) * height * 0.4;
				sRGB8 pixel = paletteImage.Pixel(x, y);
				palette[i].R = pixel.R;
				palette[i].G = pixel.G;
				palette[i].B = pixel.B;
			}
			DrawPalette(palette);
			mainImage->SetPalette(palette);
		}
		strcpy(lastFilenamePalette, filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedTimeline(GtkWidget *widget, gpointer data)
{
	if (!timeLineWindow)
	{
		timeLineWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(timeLineWindow), "Timeline");
		gtk_widget_set_size_request(timeLineWindow, 4 * (128 + 2) + 4, 200);
		gtk_widget_show(timeLineWindow);
		g_signal_connect(G_OBJECT(timeLineWindow), "destroy", G_CALLBACK(DeleteTimelineWindow), &timeLineWindow);

		if (timeline->IsCreated())
		{
			timeline->RebulidTimelineWindow();
		}
		else
		{
			timeline->Initialize(Interface_data.file_keyframes);
		}
	}
	else
	{
		gtk_window_present(GTK_WINDOW(timeLineWindow));
	}
}

void DeleteTimelineWindow(GtkWidget *widget, gpointer widget_pointer)
{
	gtk_widget_destroyed(widget, (GtkWidget**)widget_pointer);
	timeline->isOpened = false;
}

void PressedDeleteKeyframe(GtkWidget *widget, gpointer widget_pointer)
{
	int index = atoi(gtk_entry_get_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber)));
	timeline->DeleteKeyframe(index,Interface_data.file_keyframes);
}
