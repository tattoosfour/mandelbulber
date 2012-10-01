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

#include <cstdlib>
#include <string.h>

#include "callbacks.h"
#include "interface.h"
#include "settings.h"
#include "shaders.h"
#include "files.h"
#include "undo.hpp"
#include "timeline.hpp"
#include "cl_support.hpp"
#include "netrender.hpp"

using namespace std;

double last_navigator_step;
CVector3 last_keyframe_position;
bool renderRequest = false;
bool refreshNeeded = false;
bool imageCompileNeeded = false;
bool refreshInNextLoop = false;

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

gboolean CallerTimerLoopWindowRefresh(GtkWidget *widget)
{
	if(refreshInNextLoop && !refreshNeeded)
	{
		refreshInNextLoop = false;
		if(imageCompileNeeded)
		{
			imageCompileNeeded = false;
			mainImage.CompileImage();
		}
		gtk_widget_set_size_request(renderWindow.drawingArea, mainImage.GetPreviewWidth(), mainImage.GetPreviewHeight());
		gtk_widget_queue_resize (renderWindow.scrolled_window);
		mainImage.CreatePreview(Interface_data.imageScale);
		mainImage.ConvertTo8bit();
		mainImage.UpdatePreview();
		mainImage.RedrawInWidget(renderWindow.drawingArea);
		gtk_widget_set_size_request(renderWindow.drawingArea, mainImage.GetPreviewWidth(), mainImage.GetPreviewHeight());
	}

	if(refreshNeeded)
	{
		refreshInNextLoop = true;
		refreshNeeded = false;
	}
	return true;
}

gboolean pressed_button_on_image(GtkWidget *widget, GdkEventButton *event)
{
	int clickMode = gtk_combo_box_get_active(GTK_COMBO_BOX(renderWindow.comboMouseClickMode));

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
	else
	{
		if (clickMode == 0) //none
		{
			//nothing
		}
		else
		{
			int x = event->x;
			int z = event->y;
			x = x / mainImage.GetPreviewScale();
			z = z / mainImage.GetPreviewScale();
			double y = mainImage.GetPixelZBuffer(x, z);

			if(clickMode ==1 || clickMode >= 5) //camera move or light position setup
			{
				int width = mainImage.GetWidth();
				int height = mainImage.GetHeight();

				double closeUpRatio = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mouse_click_distance)));
				double lightPlacementDistance = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPlacementDistance)));
				if(clickMode == 9) lightPlacementDistance = 0; //random light center

				if (event->button == 3) //right mouse button
				{
					closeUpRatio = 1.0 / closeUpRatio;
				}

				sParamRender params;
				ReadInterface(&params);
				undoBuffer.SaveUndo(&params);

				double x2, z2;
				double aspectRatio = (double) width / height;

				enumPerspectiveType perspectiveType = (enumPerspectiveType)gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboPerspectiveType));
				if(perspectiveType == fishEyeCut) perspectiveType = fishEye;

				if (perspectiveType == fishEye || perspectiveType == equirectangular)
				{
					y /= 1e12;
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

				if (y < 1e19)
				{
					CVector3 vector, vector2;
					double y2 = 0;
					double perspFactor1 = y * params.doubles.persp + 1.0;

					if(perspectiveType == fishEye || perspectiveType == equirectangular)
					{
						if(clickMode == 1) y2 = y * (1.0 - 1.0 / closeUpRatio);
						else if(clickMode >= 5) y2 = y - lightPlacementDistance;
					}
					else
					{
						double delta_y;
						if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorGoToSurface))) delta_y = 0;
						else delta_y = (y + (1.0 / params.doubles.persp)) / closeUpRatio;

						if (clickMode == 1)
						{
							y2 = y - delta_y;
							double perspFactor2 = y2 * params.doubles.persp + 1.0;
							x2 *= perspFactor1/perspFactor2;
							z2 *= perspFactor1/perspFactor2;
						}
						else if (clickMode >= 5 && clickMode < 10) y2 = y - lightPlacementDistance/params.doubles.zoom; //lights placement
						else if (clickMode == 10 || clickMode == 11) y2 = y; //julia constant
					}

					CVector3 point = Projection3D(CVector3(x2, y2, z2), params.doubles.vp, mRot, perspectiveType, params.doubles.persp, params.doubles.zoom);

					if(clickMode == 1) //move the camera
					{
					  params.doubles.vp = point;
						params.doubles.zoom /= closeUpRatio;

						char distanceString[1000];
						double distance = CalculateDistance(params.doubles.vp, params.fractal);
						double key_distance = (params.doubles.vp - last_keyframe_position).Length();
						sprintf(distanceString, "Estimated viewpoint distance to the surface: %g\nDistance from last keyframe: %g", distance, key_distance);
						gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

						WriteInterface(&params);
						Interface_data.animMode = false;
						Interface_data.playMode = false;
						Interface_data.recordMode = false;
						Interface_data.continueRecord = false;

						programClosed = true;
						isPostRendering = false;
						renderRequest = true;
					}
					if(clickMode >= 5)
					{
						double distance = CalculateDistance(point, params.fractal);
						if (clickMode == 5)
						{
							params.doubles.auxLightPre1 = point;
							params.doubles.auxLightPre1intensity = distance * distance;
						}
						if (clickMode == 6)
						{
							params.doubles.auxLightPre2 = point;
							params.doubles.auxLightPre2intensity = distance * distance;
						}
						if (clickMode == 7)
						{
							params.doubles.auxLightPre3 = point;
							params.doubles.auxLightPre3intensity = distance * distance;
						}
						if (clickMode == 8)
						{
							params.doubles.auxLightPre4 = point;
							params.doubles.auxLightPre4intensity = distance * distance;
						}
						if (clickMode == 9)
						{
							params.doubles.auxLightRandomCenter = point;
						}
						if (clickMode == 10)
						{
							params.fractal.doubles.julia = point;
						}
						if (clickMode == 11)
						{
							CVector3 tempPoint;
							tempPoint.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_measureX)));
							tempPoint.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_measureY)));
							tempPoint.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_measureZ)));
							double distanceFromLast = (point - tempPoint).Length();
							char distanceString[1000];
							sprintf(distanceString, "Distance from last point: %g\n", distanceFromLast);
							gtk_label_set_text(GTK_LABEL(Interface.label_measureDistance), distanceString);
							gtk_entry_set_text(GTK_ENTRY(Interface.edit_measureX), DoubleToString(point.x));
							gtk_entry_set_text(GTK_ENTRY(Interface.edit_measureY), DoubleToString(point.y));
							gtk_entry_set_text(GTK_ENTRY(Interface.edit_measureZ), DoubleToString(point.z));
						}
						WriteInterface(&params);
						PlaceRandomLights(&params, false);
					}
				}
			}
			else if(clickMode == 2) //fog distance front
			{
				double fog = (log10(y + 10.0) + 2.0)*10.0;
				gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepthFront),fog);
			}
			else if(clickMode == 3) //fog visibility distance
			{
				double fogFront = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepthFront));
				double fogFront2 = pow(10, fogFront / 10 - 2.0) - 10.0;

				if(y > fogFront2)
				{
					double fog = (log10(y - fogFront2) + 2.0) * 10.0;
					gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepth), fog);
				}
				else
				{
					gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepth), 0.1);
				}
			}
			else if(clickMode == 4) //DOF point
			{
				double persp = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_persp)));

				//double DOF_focus = pow(10, DOFFocus / 10.0 - 2.0) - 1.0 / persp;

				double dof = (log10(y + 1.0/persp) + 2.0) * 10;
				gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentDOFFocus),dof);
			}
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

	if (width != renderWindow.lastWindowWidth || height != renderWindow.lastWindowHeight)
	{
		ChangedComboScale(renderWindow.comboImageScale, NULL);
		renderWindow.lastWindowWidth = width;
		renderWindow.lastWindowHeight = height;
	}
	return false;
}

gboolean on_darea_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	if (mainImage.IsPreview())
	{
		mainImage.RedrawInWidget(renderWindow.drawingArea);
		return TRUE;
	}
	else return false;
}

gboolean on_dareaPalette_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	paletteViewCreated = true;
	//srand(Interface_data.coloring_seed);
	//NowaPaleta(paleta, 1.0);
	DrawPalette(mainImage.GetPalettePtr());
	return TRUE;
}

void StartRendering(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	char distanceString[1000];
	double distance = CalculateDistance(params.doubles.vp, params.fractal);
	sprintf(distanceString, "Estimated viewpoint distance to the surface: %g\n", distance);
	gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

	if (clSupport->IsEnabled())
	{
		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;
		programClosed = false;
		gdk_threads_enter();
		MainRender();
		gdk_threads_leave();
	}
	else
	{
		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;

		programClosed = true;
		isPostRendering = false;
		renderRequest = true;
	}
}

void PressedAnimationRecord(GtkWidget *widget, gpointer data)
{
	if (!isRendering)
	{
		sParamRender params;
		ReadInterface(&params);
		undoBuffer.SaveUndo(&params);

		Interface_data.animMode = true;
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
		ReadInterface(&params);
		undoBuffer.SaveUndo(&params);

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
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

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
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	GdkColor color;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorGlow1), &color);

	gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorGlow2), &color);

	gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorMainLight), &color);

	//generating color palette
	//srand(Interface_data.coloring_seed);
	//NowaPaleta(paleta, 1.0);
	DrawPalette(mainImage.GetPalettePtr());

	//if (!isRendering)
	//{
		mainImage.CompileImage();
		PostRenderingLights(&mainImage, &params);
		mainImage.ConvertTo8bit();
		mainImage.UpdatePreview();
		mainImage.RedrawInWidget(renderWindow.drawingArea);
		while (gtk_events_pending())
			gtk_main_iteration();
	//}
}

void PressedLoadSettings(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Load fractal settings", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);

	GtkWidget *preview;
	GtkWidget *checkBox = gtk_check_button_new_with_label("Render preview of settings file");
	preview = gtk_drawing_area_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), checkBox);

	gtk_widget_set_size_request(preview, 128, 128);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(UpdatePreviewSettingsDialog), preview);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilenameSettings);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		sParamRender fractParamLoaded;
		LoadSettings(filename, fractParamLoaded);
		Params2InterfaceData(&fractParamLoaded);
		WriteInterface(&fractParamLoaded);

		strcpy(lastFilenameSettings, filename);
		string windowTitle= string("Mandelbulber (")+filename+")";
		gtk_window_set_title(GTK_WINDOW(window_interface), windowTitle.c_str());

		timeline->Reset();
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
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		sParamRender fractParamToSave;
		ReadInterface(&fractParamToSave);
		SaveSettings(filename, fractParamToSave, true);

		strcpy(lastFilenameSettings, filename);
		string windowTitle= string("Mandelbulber (")+filename+")";
		gtk_window_set_title(GTK_WINDOW(window_interface), windowTitle.c_str());
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
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		SaveJPEG(filename, 100, mainImage.GetWidth(), mainImage.GetHeight(), (JSAMPLE*) mainImage.ConvertTo8bit());
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
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		SavePNG(filename, 100, mainImage.GetWidth(), mainImage.GetHeight(), (JSAMPLE*) mainImage.ConvertTo8bit());
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
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		SavePNG16(filename, 100, mainImage.GetWidth(), mainImage.GetHeight(), mainImage.GetImage16Ptr());
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
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		SavePNG16Alpha(filename, 100, mainImage.GetWidth(), mainImage.GetHeight(), &mainImage);
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

	gtk_widget_destroy(dialog->window_files);
	delete dialog;

	timeline->Reset();
}

void PressedCancelDialogFiles(GtkWidget *widget, gpointer data)
{
	sDialogFiles *dialog = (sDialogFiles*) data;
	printf("Files cancel\n");
	gtk_widget_destroy(dialog->window_files);
}

static void Navigate(int x, int y, int r)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	double rotation_step = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_rotation))) / 180.0 * M_PI;

	params.doubles.gamma += r * rotation_step;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkStraightRotation)))
	{
		params.doubles.alfa += x * rotation_step;
		params.doubles.beta += y * rotation_step;
	}
	else
	{
		double alfa_old = params.doubles.alfa;
		double beta_old = params.doubles.beta;
		double gamma_old = params.doubles.gamma;

		CRotationMatrix mRot;
		mRot.RotateZ(params.doubles.alfa);
		mRot.RotateX(params.doubles.beta);
		mRot.RotateY(params.doubles.gamma);
		mRot.RotateX(y * rotation_step);
		mRot.RotateZ(x * rotation_step);

		params.doubles.alfa = -mRot.GetAlfa();
		params.doubles.beta = -mRot.GetBeta();
		params.doubles.gamma = -mRot.GetGamma();

		//Fix view angle jumps +180°/-180° due to asin and atan functions. These are nasty with keyframes. This test may not be accurate in every case. (Mintaka 20111005)

		//Fix by mintaka (fractalforums.com/mandelbulber/mandelbulber-1-06-patch/)
		if (rotation_step < M_PI) //does not work if we turn more than 180°
		{
			if ((alfa_old - params.doubles.alfa) > M_PI) //angle jump over +180° to negative value
			{
				params.doubles.alfa = params.doubles.alfa + 2 * M_PI;
			}
			if ((params.doubles.alfa - alfa_old) > M_PI) //angle jump over -180° to positive value
			{
				params.doubles.alfa = params.doubles.alfa - 2 * M_PI;
			}
			if ((beta_old - params.doubles.beta) > M_PI)
			{
				params.doubles.beta = params.doubles.beta + 2 * M_PI;
			}
			if ((params.doubles.beta - beta_old) > M_PI)
			{
				params.doubles.beta = params.doubles.beta - 2 * M_PI;
			}
			if ((gamma_old - params.doubles.gamma) > M_PI)
			{
				params.doubles.gamma = params.doubles.gamma + 2 * M_PI;
			}
			if ((params.doubles.gamma - gamma_old) > M_PI)
			{
				params.doubles.gamma = params.doubles.gamma - 2 * M_PI;
			}
		}
	}

	WriteInterface(&params);

	if (clSupport->IsEnabled())
	{
		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;
		programClosed = false;
		gdk_threads_enter();
		MainRender();
		gdk_threads_leave();
	}
	else
	{
		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;
		programClosed = true;
		isPostRendering = false;
		renderRequest = true;
	}
}

void PressedNavigatorUp(GtkWidget *widget, gpointer data)
{
	Navigate(0, -1, 0);
}

void PressedNavigatorDown(GtkWidget *widget, gpointer data)
{
	Navigate(0, 1, 0);
}

void PressedNavigatorLeft(GtkWidget *widget, gpointer data)
{
	Navigate(1, 0, 0);
}

void PressedNavigatorRight(GtkWidget *widget, gpointer data)
{
	Navigate(-1, 0, 0);
}

void PressedNavigatorRotateLeft(GtkWidget *widget, gpointer data)
{
	Navigate(0, 0, 1);
}

void PressedNavigatorRotateRight(GtkWidget *widget, gpointer data)
{
 	Navigate(0, 0, -1);
}

static void Move(int x, int y, int z)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	double distance = 0;
	double speed = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_forward)));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorAbsoluteDistance)))
	{
		distance = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_NavigatorAbsoluteDistance)));
	}
	else
	{
		distance = CalculateDistance(params.doubles.vp, params.fractal) * speed;
	}

	if (z > 0) last_navigator_step = distance;

	CVector3 vDelta;
	CRotationMatrix mRot;
	mRot.RotateZ(params.doubles.alfa);
	mRot.RotateX(params.doubles.beta);
	mRot.RotateY(params.doubles.gamma);
	CVector3 directionVector(x * distance, abs(z) * distance, y * distance);
	vDelta = mRot.RotateVector(directionVector);

	if (z >= 0) params.doubles.vp += vDelta;
	else params.doubles.vp -= vDelta;

	char distanceString[1000];
	distance = CalculateDistance(params.doubles.vp, params.fractal);
	double key_distance = (params.doubles.vp - last_keyframe_position).Length();
	sprintf(distanceString, "Estimated viewpoint distance to the surface: %g\nDistance from last keyframe: %g", distance, key_distance);
	gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

	WriteInterface(&params);

	if (clSupport->IsEnabled())
	{
		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;
		programClosed = false;
		gdk_threads_enter();
		MainRender();
		gdk_threads_leave();
	}
	else
	{
		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;
		programClosed = true;
		isPostRendering = false;
		renderRequest = true;
	}
}

void PressedNavigatorMoveUp(GtkWidget *widget, gpointer data)
{
	Move(0, -1, 0);
}

void PressedNavigatorMoveDown(GtkWidget *widget, gpointer data)
{
	Move(0, 1, 0);
}

void PressedNavigatorMoveLeft(GtkWidget *widget, gpointer data)
{
	Move(-1, 0, 0);
}

void PressedNavigatorMoveRight(GtkWidget *widget, gpointer data)
{
	Move(1, 0, 0);
}

void PressedNavigatorForward(GtkWidget *widget, gpointer data)
{
	Move(0, 0, 1);
}

void PressedNavigatorBackward(GtkWidget *widget, gpointer data)
{
	Move(0, 0, -1);
}

void PressedNavigatorInit(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	double objectSize = ScanSizeOfFractal(&params);
	double initCameraDistance = objectSize + objectSize / params.doubles.persp * 1.2 * params.image_width/params.image_height;

	CVector3 initVector(0,-initCameraDistance,0);
	CRotationMatrix mRot;
	mRot.RotateZ(params.doubles.alfa);
	mRot.RotateX(params.doubles.beta);
	initVector = mRot.RotateVector(initVector);

	params.doubles.zoom = 1e-7;
	params.doubles.vp = initVector;
	params.doubles.viewDistanceMax = initCameraDistance + objectSize;

	char distanceString[1000];
	double distance = CalculateDistance(params.doubles.vp, params.fractal);
	sprintf(distanceString, "Estimated viewpoint distance to the surface: %g", distance);
	gtk_label_set_text(GTK_LABEL(Interface.label_NavigatorEstimatedDistance), distanceString);

	WriteInterface(&params);

	if (clSupport->IsEnabled())
	{
		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;
		programClosed = false;
		gdk_threads_enter();
		MainRender();
		gdk_threads_leave();
	}
	else
	{
		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;
		programClosed = true;
		isPostRendering = false;
		renderRequest = true;
	}
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
		int winWidth = renderWindow.scrolled_window->allocation.width;
		int winHeight = renderWindow.scrolled_window->allocation.height;;
		//gtk_window_get_size(GTK_WINDOW(renderWindow.scrolled_window), &winWidth, &winHeight);
		winWidth -= renderWindow.scrollbarSize;
		winHeight -= renderWindow.scrollbarSize;
		imageScale = (double) winWidth / mainImage.GetWidth();
		if (mainImage.GetHeight() * imageScale > winHeight) imageScale = (double) winHeight / mainImage.GetHeight();
	}
	Interface_data.imageScale = imageScale;
	refreshNeeded = true;
}

void ChangedComboFormula(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ReadInterface(&params);
	enumFractalFormula formula = params.fractal.formula;

	/*
	if (formula == trig_DE || formula == trig_optim)
	{
		gtk_widget_set_sensitive(Interface.checkFastAmbientOcclusion, true);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkFastAmbientOcclusion), false);
		gtk_widget_set_sensitive(Interface.checkFastAmbientOcclusion, false);
	}
	*/

	gtk_widget_set_sensitive(Interface.tab_box_mandelbox, formula == tglad || formula == smoothMandelbox || formula == mandelboxVaryScale4D || formula == generalizedFoldBox || formula == hybrid);
	gtk_widget_set_sensitive(Interface.tab_box_IFS, formula == kaleidoscopic || params.fractal.IFS.foldingMode || formula == hybrid);
	gtk_widget_set_sensitive(Interface.edit_power, formula == trig_DE || formula == trig_optim || formula == xenodreambuie || formula == mandelbulb4);
	gtk_widget_set_sensitive(Interface.edit_cadd, formula == aexion || formula == hybrid);
	gtk_widget_set_sensitive(Interface.tab_box_hybrid, formula == hybrid);
	gtk_widget_set_sensitive(Interface.edit_mandelboxSharpness, formula == smoothMandelbox);
	gtk_widget_set_sensitive(Interface.comboGeneralizedFoldBoxType, formula == generalizedFoldBox);
	gtk_widget_set_sensitive(Interface.edit_mandelboxSolid, formula == generalizedFoldBox);
	gtk_widget_set_sensitive(Interface.edit_mandelboxMelt, formula == generalizedFoldBox);
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

void ChangedAmbientOcclusion(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAmbientOcclusion)))
	{
		gtk_widget_set_sensitive(Interface.edit_AmbientOcclusionQuality, true);
		gtk_widget_set_sensitive(Interface.checkFastAmbientOcclusion, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.edit_AmbientOcclusionQuality, false);
		gtk_widget_set_sensitive(Interface.checkFastAmbientOcclusion, false);
	}
}

void ChangedFastAmbientOcclusion(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkFastAmbientOcclusion)))
	{
		gtk_widget_set_sensitive(Interface.edit_fastAoTune, true);
	}
	else
	{
		gtk_widget_set_sensitive(Interface.edit_fastAoTune, false);
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

	sImageAdjustments *adj = mainImage.GetImageAdjustments();
	adj->fogVisibility = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepth));
	adj->fogVisibilityFront = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepthFront));
	sImageSwitches *sw = mainImage.GetImageSwitches();
	sw->fogEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkFogEnabled));
	sEffectColours *ecol = mainImage.GetEffectColours();
	ecol->fogColor = color2;

	imageCompileNeeded = true;
	refreshNeeded = true;

	/*
	if (!isRendering)
	{
		mainImage.CompileImage();
		mainImage.ConvertTo8bit();
		mainImage.UpdatePreview();
		mainImage.RedrawInWidget(renderWindow.drawingArea);
		while (gtk_events_pending())
			gtk_main_iteration();
	}
	*/
}

void PressedSSAOUpdate(GtkWidget *widget, gpointer data)
{
	if (Interface_data.disableInitRefresh) return;
	double SSAOQuality = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentSSAOQuality));
	bool SSAOEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkSSAOEnabled));
	double persp = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_persp)));
	enumPerspectiveType perspectiveType = (enumPerspectiveType)gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboPerspectiveType));
	if (!isRendering)
	{
		if (SSAOEnabled)
		{
			PostRendering_SSAO(&mainImage, persp, SSAOQuality, perspectiveType, false);
		}
		mainImage.CompileImage();
		mainImage.ConvertTo8bit();
		mainImage.UpdatePreview();
		mainImage.RedrawInWidget(renderWindow.drawingArea);
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
		mainImage.CompileImage();
		if (DOFEnabled)
		{
			double DOF_focus = pow(10, DOFFocus / 10.0 - 2.0) - 1.0 / persp;
			PostRendering_DOF(&mainImage, DOFRadius * mainImage.GetWidth() / 1000.0, DOF_focus, persp);
		}
		mainImage.ConvertTo8bit();
		mainImage.UpdatePreview();
		mainImage.RedrawInWidget(renderWindow.drawingArea);
		while (gtk_events_pending())
			gtk_main_iteration();
	}
}

void PressedDistributeLights(GtkWidget *widget, gpointer data)
{
	sParamRender fractParams;
	ReadInterface(&fractParams);
	undoBuffer.SaveUndo(&fractParams);
	PlaceRandomLights(&fractParams, false);
}

void RecalculateIFSParams(sFractal &fractal)
{
	fractal.IFS.mainRot.SetRotation(fractal.IFS.doubles.rotationAlfa, fractal.IFS.doubles.rotationBeta, fractal.IFS.doubles.rotationGamma);

	for (int i = 0; i < IFS_VECTOR_COUNT; i++)
	{
		fractal.IFS.rot[i].SetRotation(fractal.IFS.doubles.alfa[i], fractal.IFS.doubles.beta[i], fractal.IFS.doubles.gamma[i]);
		fractal.IFS.doubles.direction[i].Normalize();
	}

	fractal.mandelbox.mainRot.SetRotation(fractal.mandelbox.doubles.rotationMain);

	for (int fold = 0; fold < MANDELBOX_FOLDS; ++fold)
		for (int axis = 0; axis < 3; ++axis)
		{
			fractal.mandelbox.rot[fold][axis].SetRotation(fractal.mandelbox.doubles.rotation[fold][axis]);
			fractal.mandelbox.rotinv[fold][axis] = fractal.mandelbox.rot[fold][axis].Transpose();
		}

	//Generalized Fold Box precalculated vectors
	double sqrt_i3 = 1.0/sqrt(3.0);
	fractal.genFoldBox.Nv_tet[0] = CVector3( sqrt_i3, sqrt_i3,-sqrt_i3);
	fractal.genFoldBox.Nv_tet[1] = CVector3( sqrt_i3,-sqrt_i3, sqrt_i3);
	fractal.genFoldBox.Nv_tet[2] = CVector3(-sqrt_i3, sqrt_i3, sqrt_i3);
	fractal.genFoldBox.Nv_tet[3] = CVector3(-sqrt_i3,-sqrt_i3,-sqrt_i3);
	fractal.genFoldBox.sides_tet = 4;

	fractal.genFoldBox.Nv_cube[0] = CVector3( 1, 0, 0);
	fractal.genFoldBox.Nv_cube[1] = CVector3(-1, 0, 0);
	fractal.genFoldBox.Nv_cube[2] = CVector3( 0, 1, 0);
	fractal.genFoldBox.Nv_cube[3] = CVector3( 0,-1, 0);
	fractal.genFoldBox.Nv_cube[4] = CVector3( 0, 0, 1);
	fractal.genFoldBox.Nv_cube[5] = CVector3( 0, 0,-1);
	fractal.genFoldBox.sides_cube = 6;

  fractal.genFoldBox.Nv_oct[0] = CVector3( sqrt_i3, sqrt_i3,-sqrt_i3);
  fractal.genFoldBox.Nv_oct[1] = CVector3( sqrt_i3,-sqrt_i3, sqrt_i3);
  fractal.genFoldBox.Nv_oct[2] = CVector3(-sqrt_i3, sqrt_i3, sqrt_i3);
  fractal.genFoldBox.Nv_oct[3] = CVector3(-sqrt_i3,-sqrt_i3,-sqrt_i3);
  fractal.genFoldBox.Nv_oct[4] = CVector3( sqrt_i3, sqrt_i3, sqrt_i3);
  fractal.genFoldBox.Nv_oct[5] = CVector3(-sqrt_i3,-sqrt_i3, sqrt_i3);
  fractal.genFoldBox.Nv_oct[6] = CVector3(-sqrt_i3, sqrt_i3,-sqrt_i3);
  fractal.genFoldBox.Nv_oct[7] = CVector3( sqrt_i3,-sqrt_i3,-sqrt_i3);
	fractal.genFoldBox.sides_oct = 8;

  fractal.genFoldBox.Nv_oct_cube[0] = CVector3( sqrt_i3, sqrt_i3,-sqrt_i3);
  fractal.genFoldBox.Nv_oct_cube[1] = CVector3( sqrt_i3,-sqrt_i3, sqrt_i3);
  fractal.genFoldBox.Nv_oct_cube[2] = CVector3(-sqrt_i3, sqrt_i3, sqrt_i3);
  fractal.genFoldBox.Nv_oct_cube[3] = CVector3(-sqrt_i3,-sqrt_i3,-sqrt_i3);
  fractal.genFoldBox.Nv_oct_cube[4] = CVector3( sqrt_i3, sqrt_i3, sqrt_i3);
  fractal.genFoldBox.Nv_oct_cube[5] = CVector3(-sqrt_i3,-sqrt_i3, sqrt_i3);
  fractal.genFoldBox.Nv_oct_cube[6] = CVector3(-sqrt_i3, sqrt_i3,-sqrt_i3);
  fractal.genFoldBox.Nv_oct_cube[7] = CVector3( sqrt_i3,-sqrt_i3,-sqrt_i3);
  fractal.genFoldBox.Nv_oct_cube[8] = CVector3( 1, 0, 0);
  fractal.genFoldBox.Nv_oct_cube[9] = CVector3(-1, 0, 0);
  fractal.genFoldBox.Nv_oct_cube[10] = CVector3( 0, 1, 0);
  fractal.genFoldBox.Nv_oct_cube[11] = CVector3( 0,-1, 0);
  fractal.genFoldBox.Nv_oct_cube[12] = CVector3( 0, 0, 1);
  fractal.genFoldBox.Nv_oct_cube[13] = CVector3( 0, 0,-1);
	fractal.genFoldBox.sides_oct_cube = 14;

	double aa = ((1.0+sqrt(5.0))/2.0);
	double bb = (1.0/sqrt(aa*aa+1.0));
  fractal.genFoldBox.Nv_dodeca[0] = CVector3(0, bb, aa * bb);
	fractal.genFoldBox.Nv_dodeca[1] = CVector3(0, bb, -aa * bb);
	fractal.genFoldBox.Nv_dodeca[2] = CVector3(0, -bb, aa * bb);
	fractal.genFoldBox.Nv_dodeca[3] = CVector3(0, -bb, -aa * bb);
	fractal.genFoldBox.Nv_dodeca[4] = CVector3(bb, aa * bb, 0);
	fractal.genFoldBox.Nv_dodeca[5] = CVector3(bb, -aa * bb, 0);
	fractal.genFoldBox.Nv_dodeca[6] = CVector3(-bb, aa * bb, 0);
	fractal.genFoldBox.Nv_dodeca[7] = CVector3(-bb, -aa * bb, 0);
	fractal.genFoldBox.Nv_dodeca[8] = CVector3(aa * bb, 0, bb);
	fractal.genFoldBox.Nv_dodeca[9] = CVector3(-aa * bb, 0, bb);
	fractal.genFoldBox.Nv_dodeca[10] = CVector3(aa * bb, 0, -bb);
	fractal.genFoldBox.Nv_dodeca[11] = CVector3(-aa * bb, 0, -bb);
	fractal.genFoldBox.sides_dodeca = 12;

  double ff = sqrt(aa*aa+1.0/(aa*aa));
  double cc = aa/ff;
  double dd = 1.0/aa/ff;

  fractal.genFoldBox.Nv_icosa[0] = CVector3( sqrt_i3, sqrt_i3, -sqrt_i3);
	fractal.genFoldBox.Nv_icosa[1] = CVector3( sqrt_i3,-sqrt_i3, sqrt_i3);
	fractal.genFoldBox.Nv_icosa[2] = CVector3( -sqrt_i3, sqrt_i3, sqrt_i3);
	fractal.genFoldBox.Nv_icosa[3] = CVector3( -sqrt_i3,-sqrt_i3,-sqrt_i3);
	fractal.genFoldBox.Nv_icosa[4] = CVector3( sqrt_i3, sqrt_i3, sqrt_i3);
	fractal.genFoldBox.Nv_icosa[5] = CVector3( -sqrt_i3,-sqrt_i3, sqrt_i3);
	fractal.genFoldBox.Nv_icosa[6] = CVector3( -sqrt_i3, sqrt_i3,-sqrt_i3);
	fractal.genFoldBox.Nv_icosa[7] = CVector3( sqrt_i3,-sqrt_i3,-sqrt_i3);
	fractal.genFoldBox.Nv_icosa[8] = CVector3( 0, dd, cc);
	fractal.genFoldBox.Nv_icosa[9] = CVector3( 0, dd,-cc);
	fractal.genFoldBox.Nv_icosa[10] = CVector3( 0,-dd, cc);
	fractal.genFoldBox.Nv_icosa[11] = CVector3( 0,-dd,-cc);
	fractal.genFoldBox.Nv_icosa[12] = CVector3( cc, 0, dd);
	fractal.genFoldBox.Nv_icosa[13] = CVector3( cc, 0,-dd);
	fractal.genFoldBox.Nv_icosa[14] = CVector3( -cc, 0, dd);
	fractal.genFoldBox.Nv_icosa[15] = CVector3( -cc, 0,-dd);
	fractal.genFoldBox.Nv_icosa[16] = CVector3( dd, cc, 0);
	fractal.genFoldBox.Nv_icosa[17] = CVector3( dd,-cc, 0);
	fractal.genFoldBox.Nv_icosa[18] = CVector3( -dd, cc, 0);
	fractal.genFoldBox.Nv_icosa[19] = CVector3( -dd,-cc, 0);
	fractal.genFoldBox.sides_icosa = 20;

	double tha = 3.14159*2.0/6.0;

  fractal.genFoldBox.Nv_box6[0] = CVector3(0, 0, 1);
	fractal.genFoldBox.Nv_box6[1] = CVector3(0, 0, -1);
	fractal.genFoldBox.Nv_box6[2] = CVector3(cos(tha * 0), sin(tha * 0), 0);
	fractal.genFoldBox.Nv_box6[3] = CVector3(cos(tha * 1), sin(tha * 1), 0);
	fractal.genFoldBox.Nv_box6[4] = CVector3(cos(tha * 2), sin(tha * 2), 0);
	fractal.genFoldBox.Nv_box6[5] = CVector3(cos(tha * 3), sin(tha * 3), 0);
	fractal.genFoldBox.Nv_box6[6] = CVector3(cos(tha * 4), sin(tha * 4), 0);
	fractal.genFoldBox.Nv_box6[7] = CVector3(cos(tha * 5), sin(tha * 5), 0);
	fractal.genFoldBox.sides_box6 = 8;

	double tha5 = 3.14159*2.0/5.0;

  fractal.genFoldBox.Nv_box5[0] = CVector3(0, 0, 1);
	fractal.genFoldBox.Nv_box5[1] = CVector3(0, 0, -1);
	fractal.genFoldBox.Nv_box5[2] = CVector3(cos(tha5 * 0), sin(tha5 * 0), 0);
	fractal.genFoldBox.Nv_box5[3] = CVector3(cos(tha5 * 1), sin(tha5 * 1), 0);
	fractal.genFoldBox.Nv_box5[4] = CVector3(cos(tha5 * 2), sin(tha5 * 2), 0);
	fractal.genFoldBox.Nv_box5[5] = CVector3(cos(tha5 * 3), sin(tha5 * 3), 0);
	fractal.genFoldBox.Nv_box5[6] = CVector3(cos(tha5 * 4), sin(tha5 * 4), 0);
	fractal.genFoldBox.sides_box5 = 7;

}

void CreateFormulaSequence(sFractal &fractal)
{
	fractal.formulaSequence.resize(fractal.N);
	fractal.hybridPowerSequence.resize(fractal.N);

	int number = 0;
	while (number < fractal.N)
	{
		for (int hybrid_n = 0; hybrid_n < HYBRID_COUNT - 1; ++hybrid_n)
		{
			if (fractal.hybridFormula[hybrid_n] == none) continue;
			for (int i = 0; i < fractal.hybridIters[hybrid_n]; ++i)
			{
				if (number < fractal.N)
				{
					fractal.formulaSequence[number] = fractal.hybridFormula[hybrid_n];
					fractal.hybridPowerSequence[number] = fractal.doubles.hybridPower[hybrid_n];
				}
				number++;
			}
		}

		int temp_end = fractal.N;
		if (fractal.hybridCyclic) temp_end = fractal.hybridIters[HYBRID_COUNT - 1];

		if (fractal.hybridFormula[HYBRID_COUNT - 1] != none) for (int i = 0; i < temp_end; i++)
		{
			if (number < fractal.N)
			{
				fractal.formulaSequence[number] = fractal.hybridFormula[HYBRID_COUNT - 1];
				fractal.hybridPowerSequence[number] = fractal.doubles.hybridPower[HYBRID_COUNT - 1];
			}
			number++;
		}

		if (!fractal.hybridCyclic) break;
	}
}

void PressedIFSNormalizeOffset(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	params.fractal.IFS.doubles.offset.Normalize();
	WriteInterface(&params);
}

void PressedIFSNormalizeVectors(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);
	for (int i = 0; i < IFS_VECTOR_COUNT; i++)
	{
		params.fractal.IFS.doubles.direction[i].Normalize();
	}

	WriteInterface(&params);
}

void PressedRecordKeyframe(GtkWidget *widget, gpointer data)
{
	string filename2;
	int index = atoi(gtk_entry_get_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber)));
	filename2=IndexFilename(Interface_data.file_keyframes, "fract", index);

	sParamRender fractParamToSave;
	ReadInterface(&fractParamToSave);
	SaveSettings(filename2.c_str(), fractParamToSave, true);
	last_keyframe_position = fractParamToSave.doubles.vp;

	timeline->RecordKeyframe(index, filename2.c_str(), false);

	gtk_widget_queue_draw(timelineInterface.table);

	index++;
	gtk_entry_set_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber), IntToString(index));

	//loading next keyframe if exists
	filename2=IndexFilename(Interface_data.file_keyframes, "fract", index);
	if (FileIfExists(filename2.c_str()))
	{
		sParamRender fractParamLoaded;
		LoadSettings(filename2.c_str(), fractParamLoaded, true);
		KeepOtherSettings(&fractParamLoaded);
		WriteInterface(&fractParamLoaded);

		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;

		programClosed = true;
		isPostRendering = false;
		renderRequest = true;
	}

}

void PressedInsertKeyframe(GtkWidget *widget, gpointer data)
{
	string filename1,filename2;

	int index = atoi(gtk_entry_get_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber))) + 1;
	int maxIndex = timeline->CheckNumberOfKeyframes(Interface_data.file_keyframes) - 1;
	printf("Maxindex = %d\n", maxIndex);

	if (index > maxIndex) index = maxIndex + 1;

	for (int i = maxIndex; i >= index; i--)
	{
		filename1=IndexFilename(Interface_data.file_keyframes, "fract", i);
		filename2=IndexFilename(Interface_data.file_keyframes, "fract", i + 1);
		rename(filename1.c_str(), filename2.c_str());
	}

	filename2=IndexFilename(Interface_data.file_keyframes, "fract", index);

	sParamRender fractParamToSave;
	ReadInterface(&fractParamToSave);
	SaveSettings(filename2.c_str(), fractParamToSave, true);
	last_keyframe_position = fractParamToSave.doubles.vp;

	timeline->RecordKeyframe(index, filename2.c_str(), true);

	gtk_widget_queue_draw(timelineInterface.table);

	index++;
	gtk_entry_set_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber), IntToString(index));

	//loading next keyframe if exists
	filename2=IndexFilename(Interface_data.file_keyframes, "fract", index);
	if (FileIfExists(filename2.c_str()))
	{
		sParamRender fractParamLoaded;
		LoadSettings(filename2.c_str(), fractParamLoaded, true);
		KeepOtherSettings(&fractParamLoaded);
		WriteInterface(&fractParamLoaded);

		Interface_data.animMode = false;
		Interface_data.playMode = false;
		Interface_data.recordMode = false;
		Interface_data.continueRecord = false;
		Interface_data.keyframeMode = false;

		programClosed = true;
		isPostRendering = false;
		renderRequest = true;
	}

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

	string filename2;

	filename2=IndexFilename(Interface_data.file_keyframes, "fract", index);
	if (FileIfExists(filename2.c_str()))
	{
		sParamRender fractParamLoaded;
		LoadSettings(filename2.c_str(), fractParamLoaded, true);
		KeepOtherSettings(&fractParamLoaded);
		WriteInterface(&fractParamLoaded);
		last_keyframe_position = fractParamLoaded.doubles.vp;

		gtk_widget_queue_draw(timelineInterface.table);

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
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL, "Error! Keyframe does not exist: %s", filename2.c_str());
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

	string filename2=IndexFilename(Interface_data.file_keyframes, "fract", index);

	if (FileIfExists(filename2.c_str()))
	{
		sParamRender fractParamLoaded;
		LoadSettings(filename2.c_str(), fractParamLoaded, true);
		KeepOtherSettings(&fractParamLoaded);
		last_keyframe_position = fractParamLoaded.doubles.vp;
		WriteInterface(&fractParamLoaded);

		gtk_widget_queue_draw(timelineInterface.table);

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
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL, "Error! Keyframe does not exist: %s", filename2.c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		index++;
		gtk_entry_set_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber), IntToString(index));
	}
}

void PressedTimelineRefresh(GtkWidget *widget, gpointer data)
{
	timeline->Refresh();
}

void PressedUndo(GtkWidget *widget, gpointer data)
{
	sParamRender undoParams;
	if (undoBuffer.GetUndo(&undoParams))
	{
		WriteInterface(&undoParams);
	}

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
	if (undoBuffer.GetRedo(&undoParams))
	{
		WriteInterface(&undoParams);
	}

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
	 ReadInterface(&params);
	 undoBuffer.SaveUndo(&params);
	 RenderBuddhabrot(&params);
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
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
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

	GtkWidget *preview;
	preview = gtk_drawing_area_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);
	gtk_widget_set_size_request(preview, 256, 256);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(UpdatePreviewImageDialog), preview);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), Interface_data.file_background);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
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

	GtkWidget *preview;
	preview = gtk_drawing_area_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);
	gtk_widget_set_size_request(preview, 256, 256);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(UpdatePreviewImageDialog), preview);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), Interface_data.file_envmap);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
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

	GtkWidget *preview;
	preview = gtk_drawing_area_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);
	gtk_widget_set_size_request(preview, 256, 256);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(UpdatePreviewImageDialog), preview);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), Interface_data.file_lightmap);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
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
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
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
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (FileIfExists(filename))
		{
			filename[strlen(filename) - 11] = 0;
		}

		printf("filename: %s\n", filename);
		sDialogFiles *dialogFiles = (sDialogFiles*) data;
		gtk_entry_set_text(GTK_ENTRY(dialogFiles->edit_keyframes), filename);
	}

	gtk_widget_destroy(dialog);
}

void ChangedSliderPaletteOffset(GtkWidget *widget, gpointer data)
{
	if (!interfaceCreated) return;
	sImageAdjustments *adj = mainImage.GetImageAdjustments();
	adj->paletteOffset = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentPaletteOffset));
	//srand(Interface_data.coloring_seed);
	//NowaPaleta(paleta, 1.0);
	DrawPalette(mainImage.GetPalettePtr());

	imageCompileNeeded = true;
	refreshNeeded = true;
}

void PressedRandomPalette(GtkWidget *widget, gpointer data)
{
	//srand(clock());
	srand((unsigned int) ((double) clock() * 1000.0 / CLOCKS_PER_SEC));
	int coloring_seed = Random(999999);
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_color_seed), IntToString(coloring_seed));
	srand(coloring_seed);

	double saturation = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_colour_saturation)));

	NewPalette(mainImage.GetPalettePtr(), saturation);
	DrawPalette(mainImage.GetPalettePtr());
	if (!isRendering && !Interface_data.disableInitRefresh)
	{
		mainImage.CompileImage();
		mainImage.ConvertTo8bit();
		mainImage.UpdatePreview();
		mainImage.RedrawInWidget(renderWindow.drawingArea);
		while (gtk_events_pending())
			gtk_main_iteration();
	}
}

void PressedGetPaletteFromImage(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Please select image to grab colour palette...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	GtkWidget *preview;
	preview = gtk_drawing_area_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);
	gtk_widget_set_size_request(preview, 256, 256);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(UpdatePreviewImageDialog), preview);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilenamePalette);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
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
			mainImage.SetPalette(palette);
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
		gtk_widget_set_size_request(timeLineWindow, 4 * (128 + 2) + 4, 230);
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
	gtk_widget_destroyed(widget, (GtkWidget**) widget_pointer);
	timeline->isOpened = false;
}

void PressedDeleteKeyframe(GtkWidget *widget, gpointer widget_pointer)
{
	int index = atoi(gtk_entry_get_text(GTK_ENTRY(timelineInterface.editAnimationKeyNumber)));
	timeline->DeleteKeyframe(index, Interface_data.file_keyframes);
}

void UpdatePreviewSettingsDialog(GtkFileChooser *file_chooser, gpointer data)
{
	GtkWidget *preview;
	preview = GTK_WIDGET(data);

	GtkWidget *checkBox = gtk_file_chooser_get_extra_widget(file_chooser);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkBox)))
	{
		const char *filename = gtk_file_chooser_get_preview_filename(file_chooser);

		char string[13];

		FILE *fileSettings = fopen(filename, "r");
		if (fileSettings)
		{
			fgets(string, 13, fileSettings);
			//printf("%s*\n", string);
			if (!strcmp(string, "Mandelbulber") || !strcmp(string, "locale_test "))
			{
				smart_ptr<cImage> thumbnail;
				thumbnail.reset(new cImage(128, 128));
				ThumbnailRender(filename, thumbnail.ptr(), 0);
				thumbnail->CreatePreview(1.0);
				thumbnail->ConvertTo8bit();
				thumbnail->UpdatePreview();
				thumbnail->RedrawInWidget(preview);

				gtk_file_chooser_set_preview_widget_active(file_chooser, true);
			}
			fclose(fileSettings);
		}
	}
}

void UpdatePreviewImageDialog(GtkFileChooser *file_chooser, gpointer data)
{
	GtkWidget *preview;
	preview = GTK_WIDGET(data);

	int size = 256;

	const char *filename = gtk_file_chooser_get_preview_filename(file_chooser);

	if (FileIfExists(filename))
	{
		cTexture *image = new cTexture(filename);

		if (image->IsLoaded())
		{
			int iw = image->Width();
			int ih = image->Height();
			double scale;
			if (iw > ih)
			{
				scale = (double)iw / size;
			}
			else
			{
				scale = (double)ih / size;
			}

			sRGB8 *smallImage = new sRGB8[size * size];
			memset(smallImage, 0, sizeof(sRGB8) * size * size);

			for (int y = 0; y < size; y++)
			{
				for (int x = 0; x < size; x++)
				{
					double x2 = (x - size / 2) * scale + iw / 2.0;
					double y2 = (y - size / 2) * scale + ih / 2.0;
					sRGB8 pixel = image->Pixel(x2, y2);
					smallImage[x + y * size] = pixel;
				}
			}
			gdk_draw_rgb_image(preview->window, preview->style->fg_gc[GTK_STATE_NORMAL], 0, 0, size, size, GDK_RGB_DITHER_MAX, (guchar*) smallImage, size * 3);
			delete image;
			delete[] smallImage;
		}
	}
}

void PressedIFSDefaultDodeca(GtkWidget *widget, gpointer widget_pointer)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	double phi = (1 + sqrt(5.0))/2.0;
	params.fractal.IFS.doubles.direction[0].x = phi*phi;
	params.fractal.IFS.doubles.direction[0].y = 1.0;
	params.fractal.IFS.doubles.direction[0].z = -phi;

	params.fractal.IFS.doubles.direction[1].x = -phi;
	params.fractal.IFS.doubles.direction[1].y = phi*phi;
	params.fractal.IFS.doubles.direction[1].z = 1.0;

	params.fractal.IFS.doubles.direction[2].x = 1.0;
	params.fractal.IFS.doubles.direction[2].y = -phi;
	params.fractal.IFS.doubles.direction[2].z = phi*phi;

	params.fractal.IFS.enabled[0] = true;
	params.fractal.IFS.enabled[1] = true;
	params.fractal.IFS.enabled[2] = true;

	params.fractal.IFS.doubles.offset.x = 1.0;
	params.fractal.IFS.doubles.offset.y = 1.0;
	params.fractal.IFS.doubles.offset.z = 1.0;

	params.fractal.IFS.absX = true;
	params.fractal.IFS.absY = true;
	params.fractal.IFS.absZ = true;

	params.fractal.IFS.doubles.scale = phi*phi;

	params.fractal.IFS.mengerSpongeMode = false;

	WriteInterface(&params);
}

void PressedIFSDefaultIcosa(GtkWidget *widget, gpointer widget_pointer)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	double phi = (1 + sqrt(5.0))/2.0;
	params.fractal.IFS.doubles.direction[3].x = -phi*phi;
	params.fractal.IFS.doubles.direction[3].y = 1.0;
	params.fractal.IFS.doubles.direction[3].z = phi;

	params.fractal.IFS.doubles.direction[4].x = phi;
	params.fractal.IFS.doubles.direction[4].y = -phi*phi;
	params.fractal.IFS.doubles.direction[4].z = 1.0;

	params.fractal.IFS.enabled[3] = true;
	params.fractal.IFS.enabled[4] = true;

	params.fractal.IFS.doubles.offset.x = 1.0;
	params.fractal.IFS.doubles.offset.y = 0;
	params.fractal.IFS.doubles.offset.z = phi;

	params.fractal.IFS.absX = true;
	params.fractal.IFS.absY = true;
	params.fractal.IFS.absZ = true;

	params.fractal.IFS.doubles.scale = 2.0;

	params.fractal.IFS.mengerSpongeMode = false;

	WriteInterface(&params);
}

void PressedIFSDefaultOcta(GtkWidget *widget, gpointer widget_pointer)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	params.fractal.IFS.doubles.direction[5].x = 1.0;
	params.fractal.IFS.doubles.direction[5].y = -1.0;
	params.fractal.IFS.doubles.direction[5].z = 0;

	params.fractal.IFS.doubles.direction[6].x = 1.0;
	params.fractal.IFS.doubles.direction[6].y = 0;
	params.fractal.IFS.doubles.direction[6].z = -1.0;

	params.fractal.IFS.doubles.direction[7].x = 0;
	params.fractal.IFS.doubles.direction[7].y = 1.0;
	params.fractal.IFS.doubles.direction[7].z = -1.0;

	params.fractal.IFS.enabled[5] = true;
	params.fractal.IFS.enabled[6] = true;
	params.fractal.IFS.enabled[7] = true;

	params.fractal.IFS.doubles.offset.x = 1.0;
	params.fractal.IFS.doubles.offset.y = 0.0;
	params.fractal.IFS.doubles.offset.z = 0.0;

	params.fractal.IFS.absX = true;
	params.fractal.IFS.absY = true;
	params.fractal.IFS.absZ = true;

	params.fractal.IFS.doubles.scale = 2.0;

	params.fractal.IFS.mengerSpongeMode = false;

	WriteInterface(&params);
}

void PressedIFSDefaultMengerSponge(GtkWidget *widget, gpointer widget_pointer)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	params.fractal.IFS.doubles.direction[5].x = 1.0;
	params.fractal.IFS.doubles.direction[5].y = -1.0;
	params.fractal.IFS.doubles.direction[5].z = 0;

	params.fractal.IFS.doubles.direction[6].x = 1.0;
	params.fractal.IFS.doubles.direction[6].y = 0;
	params.fractal.IFS.doubles.direction[6].z = -1.0;

	params.fractal.IFS.doubles.direction[7].x = 0;
	params.fractal.IFS.doubles.direction[7].y = 1.0;
	params.fractal.IFS.doubles.direction[7].z = -1.0;

	params.fractal.IFS.enabled[5] = true;
	params.fractal.IFS.enabled[6] = true;
	params.fractal.IFS.enabled[7] = true;

	params.fractal.IFS.doubles.offset.x = 1.0;
	params.fractal.IFS.doubles.offset.y = 1.0;
	params.fractal.IFS.doubles.offset.z = 1.0;

	params.fractal.IFS.absX = true;
	params.fractal.IFS.absY = true;
	params.fractal.IFS.absZ = true;

	params.fractal.IFS.doubles.scale = 3.0;

	params.fractal.IFS.mengerSpongeMode = true;

	WriteInterface(&params);
}

void PressedIFSReset(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	for(int i=0; i<=8; i++)
	{
		params.fractal.IFS.doubles.direction[i].x = 1.0;
		params.fractal.IFS.doubles.direction[i].y = 0;
		params.fractal.IFS.doubles.direction[i].z = 0;
		params.fractal.IFS.enabled[i] = false;
		params.fractal.IFS.doubles.distance[i] = 0;
		params.fractal.IFS.doubles.alfa[i] = 0;
		params.fractal.IFS.doubles.beta[i] = 0;
		params.fractal.IFS.doubles.gamma[i] = 0;
		params.fractal.IFS.doubles.intensity[i] = 1;
	}

	params.fractal.IFS.absX = false;
	params.fractal.IFS.absY = false;
	params.fractal.IFS.absZ = false;
	params.fractal.IFS.mengerSpongeMode = false;

	WriteInterface(&params);
}

double ScanFractal(sParamRender *params, CVector3 direction)
{

	double result = 0;
	double distStep;
	sFractal calcParams = params->fractal;
	for (double scan = 100.0; scan > 0; scan -= distStep)
	{
		CVector3 point = direction * scan;
		double dist = CalculateDistance(point, calcParams);
		if (dist < params->doubles.resolution / params->doubles.quality)
		{
			result = scan;
			return result;
		}
		distStep = dist * 0.1;
		if(distStep>1) distStep = 1.0;
	}
	return 0.0;
}

double ScanSizeOfFractal(sParamRender *params)
{
	double maxDist = 0.0;
	double dist;

	for(int i=0; i<100; i++)
	{
		CVector3 direction(Random(1000)/500.0-1.0, Random(1000)/500.0-1.0, Random(1000)/500.0-1.0);
		direction.Normalize();
		dist = ScanFractal(params, direction);
		maxDist = (dist > maxDist) ? dist : maxDist;
	}
	return maxDist;
}

void PressedAutoDEStep(GtkWidget *widget, gpointer widget_pointer)
{
	AutoDEStep(false);
}

void PressedAutoDEStepHQ(GtkWidget *widget, gpointer widget_pointer)
{
	AutoDEStep(true);
}

void AutoDEStep(bool highQuality)
{
	char progressText[1000];

	sParamRender fractParams;
	ReadInterface(&fractParams);
	smart_ptr<cImage> testImage;
	testImage.reset(new cImage(64, 64));
	fractParams.doubles.quality = fractParams.doubles.quality * fractParams.image_width / 64.0;
	fractParams.shadow = false;
	fractParams.auxLightNumber = 0;
	fractParams.global_ilumination = false;
	fractParams.SSAOEnabled = false;
	fractParams.DOFEnabled = false;
	fractParams.auxLightPre1Enabled = false;
	fractParams.auxLightPre2Enabled = false;
	fractParams.auxLightPre3Enabled = false;
	fractParams.auxLightPre4Enabled = false;
	fractParams.fractal.iterThresh = false;
	for(int i=0; i<5; i++)
		fractParams.volumetricLightEnabled[i] = false;
	fractParams.imageSwitches.raytracedReflections = false;
	if (fractParams.fractal.N < 200) fractParams.fractal.N = 200;
	if (fractParams.fractal.limits_enabled) return;

	int scanCount = 0;
	double DEfactor = 1.0;
	double step = 1.0;

	double limit = 1.0;
	if(highQuality) limit = 0.01;

	programClosed = false;
	for (int i = 0; i < 100; i++)
	{
		scanCount++;
		fractParams.doubles.DE_factor = DEfactor;
		ThumbnailRender2(fractParams, testImage.ptr());
		double avgMissedDE = (double) Missed_DE_counter / Pixel_counter * 100.0;
		printf("DE factor = %f, Missed DE = %f%%\n", DEfactor, avgMissedDE);
		if (avgMissedDE < limit)
		{
			if (scanCount == 1)
			{
				if (step < 10000)
				{
					DEfactor = DEfactor * 10.0;
					step = step * 10.0;
					scanCount = 0;
					continue;
				}
				else
				{
					return;
				}
			}
			else
			{
				if (step < 0.05 * DEfactor) break;
				DEfactor += step;
			}
		}
		step /= 2.0;
		DEfactor -= step;

		DrawHistogram();
		DrawHistogram2();

		while (gtk_events_pending())
			gtk_main_iteration();

		sprintf(progressText, "Scanning for the best DE factor: DE factor = %f, Missed DE = %f%%", DEfactor, avgMissedDE);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);

		double progress;
		if(highQuality)
		{
		progress = 1.0 - (log10(avgMissedDE + (step / DEfactor)/10.0) + 3.0) / 5.0;
		}
		else
		{
			progress = 1.0 - (log10(avgMissedDE + (step / DEfactor)/1.0) + 2.0) / 4.0;
		}
		printf("progress = %f, log = %f\n", progress, log10(avgMissedDE + (step / DEfactor)));

		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), progress);
		if (programClosed) break;
	}

	sprintf(progressText, "Optimal DE factor found: DE factor = %f", DEfactor);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(Interface.progressBar), 1.0);

	ReadInterface(&fractParams);
	fractParams.doubles.DE_factor = DEfactor;
	fractParams.doubles.imageAdjustments.glow_intensity = 0.5 * DEfactor;
	if(fractParams.doubles.imageAdjustments.glow_intensity > 1.0) fractParams.doubles.imageAdjustments.glow_intensity = 0.5;
	WriteInterface(&fractParams);
}

void ChangedConstantDEThreshold(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkConstantDEThreshold)))
	{
		gtk_label_set_text(GTK_LABEL(Interface.label_DE_threshold),"DE threshold:");
	}
	else
	{
		gtk_label_set_text(GTK_LABEL(Interface.label_DE_threshold),"Detail level:");
	}
}

void ChangedImageProportion(GtkWidget *widget, gpointer data)
{
	int selection = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboImageProportion));

	if (selection > 0)
	{
		gtk_widget_set_sensitive(Interface.edit_imageWidth, false);
		int imageHeight = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_imageHeight)));
		if(selection == 1) gtk_entry_set_text(GTK_ENTRY(Interface.edit_imageWidth), IntToString(imageHeight));
		else if(selection == 2) gtk_entry_set_text(GTK_ENTRY(Interface.edit_imageWidth), IntToString(imageHeight*5/4));
		else if(selection == 3) gtk_entry_set_text(GTK_ENTRY(Interface.edit_imageWidth), IntToString(imageHeight*4/3));
		else if(selection == 4) gtk_entry_set_text(GTK_ENTRY(Interface.edit_imageWidth), IntToString(imageHeight*16/10));
		else if(selection == 5) gtk_entry_set_text(GTK_ENTRY(Interface.edit_imageWidth), IntToString(imageHeight*16/9));
	}
	else
	{
		gtk_widget_set_sensitive(Interface.edit_imageWidth, true);
	}
}

void PressedCopyToClipboard(GtkWidget *widget, gpointer data)
{
	sParamRender fractParamToSave;
	ReadInterface(&fractParamToSave);
	SaveSettings("settings/.clipboard", fractParamToSave, true);

	FILE * pFile;
	pFile = fopen("settings/.clipboard", "rb");
	if (pFile != NULL)
	{
		// obtain file size:
		fseek(pFile, 0, SEEK_END);
		unsigned int lSize = ftell(pFile);
		rewind(pFile);

		char *buffer = new char[lSize];

		// copy the file into the buffer:
		size_t result = fread(buffer, 1, lSize, pFile);
		if (result != lSize)
		{
			printf("Reading error of clipboard temporary file");
			fclose(pFile);
			delete [] buffer;
			return;
		}

		gtk_clipboard_set_text(clipboard, buffer, lSize);

		fclose(pFile);
		delete [] buffer;
		remove("settings/.clipboard");
	}
}

void PressedPasteFromClipboard(GtkWidget *widget, gpointer data)
{
	char *buffer = gtk_clipboard_wait_for_text(clipboard);
	unsigned int len = strlen(buffer);
	if (len < 15)
	{
		printf("text in clipboard is to short\n");
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
				"Error! Text in clipboard is too short");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}

	if(strncmp(buffer, "Mandelbulber", 12))
	{
		printf("text in clipboard doesn't contain settings for Mandelbulber\n");
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
				"Error! Text in clipboard doesn't contain settings for Mandelbulber\n");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}

	FILE * pFile;
	pFile = fopen("settings/.clipboard", "wb");
	if (pFile != NULL)
	{
		fwrite(buffer, 1, len, pFile);
		fclose(pFile);

		sParamRender fractParamLoaded;
		LoadSettings("settings/.clipboard", fractParamLoaded);
		Params2InterfaceData(&fractParamLoaded);
		WriteInterface(&fractParamLoaded);

		delete buffer;
		remove("settings/.clipboard");
	}
}

void PressedLoadExample(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Load fractal settings", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);

	GtkWidget *preview;
	GtkWidget *checkBox = gtk_check_button_new_with_label("Render preview of settings file");
	preview = gtk_drawing_area_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), checkBox);

	gtk_widget_set_size_request(preview, 128, 128);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(UpdatePreviewSettingsDialog), preview);

	string exampleFile = string(sharedDir) + "examples/default.fract";
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), exampleFile.c_str());

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		sParamRender fractParamLoaded;
		LoadSettings(filename, fractParamLoaded);
		Params2InterfaceData(&fractParamLoaded);
		WriteInterface(&fractParamLoaded);

		string file = string(filename);
		int length = file.length();

#ifdef WIN32
		int start = file.rfind("\\");
#else
		int start = file.rfind("/");
#endif
		string onlyFile(file, start+1, length);
		string fileWithPath = "settings/" + onlyFile;

		strcpy(lastFilenameSettings, fileWithPath.c_str());
		string windowTitle = string("Mandelbulber (") + fileWithPath.c_str() + ")";
		gtk_window_set_title(GTK_WINDOW(window_interface), windowTitle.c_str());

		timeline->Reset();
	}
	gtk_widget_destroy(dialog);

}

void PressedAutoFog(GtkWidget *widget, gpointer data)
{
	sParamRender params;
	ReadInterface(&params);
	undoBuffer.SaveUndo(&params);

	double distance = CalculateDistance(params.doubles.vp, params.fractal);
	double distance2 = distance + 1.0/params.doubles.persp*params.doubles.zoom;

	params.doubles.fogDensity = 0.5;
	params.doubles.fogDistanceFactor = distance2;
	params.doubles.fogColour1Distance = distance2*0.5;
	params.doubles.fogColour2Distance = distance2;

	WriteInterface(&params);
}

void PressedMeasureActivation(GtkWidget *widget, gpointer data)
{
	gtk_combo_box_set_active(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), 11);
}

void ChangedOpenClEnabled(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkOpenClEnable)))
	{
		clSupport->SetSize(atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_imageWidth))), atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_imageHeight))));
		clSupport->Init();
		clSupport->Enable();
	}
	else
	{
		clSupport->Disable();
	}
}

void ChangedIterFogEnable(GtkWidget *widget, gpointer data)
{
	bool active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIterFogEnable));
	gtk_widget_set_sensitive(Interface.edit_iterFogOpacity, active);
	gtk_widget_set_sensitive(Interface.edit_iterFogOpacityTrim, active);
}

void PressedSaveAllImageLayers(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Save all layers image as...", GTK_WINDOW(window_interface), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
			GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), lastFilenameImage);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		SaveAllImageLayers(filename, &mainImage);
		strcpy(lastFilenameImage, filename);
	}
	gtk_widget_destroy(dialog);
}

void PressedServerEnable(GtkWidget *widget, gpointer data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNetRenderServerEnable)))
	{
		char status[1000];
		bool result = netRender->SetServer((char*)gtk_entry_get_text(GTK_ENTRY(Interface.edit_netRenderServerPort)), status);
		gtk_label_set_text(GTK_LABEL(Interface.label_serverStatus),status);
		if(result)
		{
			gtk_widget_set_sensitive(Interface.checkNetRenderServerScan, true);
		}
	}
	else
	{
		netRender->DeleteServer();
		gtk_label_set_text(GTK_LABEL(Interface.label_serverStatus),"status: not enabled");
		gtk_widget_set_sensitive(Interface.checkNetRenderServerScan, false);
	}
}

void PressedClientEnable(GtkWidget *widget, gpointer data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNetRenderClientEnable)))
	{
		char status[1000];
		bool result = netRender->SetClient((char*)gtk_entry_get_text(GTK_ENTRY(Interface.edit_netRenderClientPort)), (char*)gtk_entry_get_text(GTK_ENTRY(Interface.edit_netRenderClientName)), status);
		gtk_label_set_text(GTK_LABEL(Interface.label_clientStatus), status);

		if(result)
		{
			if(netRender->CompareVersion(MANDELBULBER_VERSION * 1000, netRender->getNoOfClients()-1))
			{
				printf("Proper version of client\n");
			}
			else
			{
				printf("Wrong version of client\n");
			}
		}
	}
	else
	{
		netRender->DeleteClient();
		gtk_label_set_text(GTK_LABEL(Interface.label_clientStatus),"status: not enabled");
	}
}

void PressedServerScan(GtkWidget *widget, gpointer data)
{
	char status[1000];

	if (netRender->IsServer())
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNetRenderServerScan)))
		{
			gtk_label_set_text(GTK_LABEL(Interface.label_serverStatus), "status: waiting for clients");

			while (gtk_events_pending())
				gtk_main_iteration();

			while (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNetRenderServerScan)))
			{
				bool result = netRender->WaitForClient(status);
				if (result)
				{
					gtk_label_set_text(GTK_LABEL(Interface.label_serverStatus), status);

					if(netRender->CompareVersion(MANDELBULBER_VERSION * 1000, netRender->getNoOfClients()-1))
					{
						printf("Proper version of client\n");
					}
					else
					{
						printf("Wrong version of client\n");
					}
				}
				while (gtk_events_pending())
					gtk_main_iteration();
			}
		}
		else
		{
			sprintf(status, "status: %d clients connected", netRender->getNoOfClients());
			gtk_label_set_text(GTK_LABEL(Interface.label_serverStatus), status);
		}
	}
}
