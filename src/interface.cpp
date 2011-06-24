/********************************************************
 /                   MANDELBULBER                        *
 /                                                       *
 / author: Krzysztof Marczak                             *
 / contact: buddhi1980@gmail.com                         *
 / licence: GNU GPL                                      *
 ********************************************************/

/*
 * interface.cpp
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof
 */

#include <cstdlib>
#include <string.h>
#include "interface.h"
#include "callbacks.h"

#define CONNECT_SIGNAL(object, callback, event) g_signal_connect(G_OBJECT(object), event, G_CALLBACK(callback), NULL)
#define CONNECT_SIGNAL_CLICKED(x, y) CONNECT_SIGNAL(x, y, "clicked")

sMainWindow renderWindow;
sInterface Interface;
sInterface_data Interface_data;
sNoGUIdata noGUIdata;
GtkWidget *window_histogram, *window_interface;
GtkWidget *darea2, *darea3;
GtkWidget *dareaPalette;
GtkClipboard *clipboard;

sTimelineInterface timelineInterface;
GtkWidget *timeLineWindow = 0;

char lastFilenameImage[1000];
char lastFilenameSettings[1000];
char lastFilenamePalette[1000];

int x_mouse = 0;
int y_mouse = 0;
bool programClosed = false;
bool interfaceCreated = false;
bool paletteViewCreated = false;

GtkWidget* CreateEdit(const gchar *text, const gchar *txt_label, int size, GtkWidget *edit)
{
	GtkWidget *box = gtk_hbox_new(FALSE, 1);
	gtk_entry_set_text(GTK_ENTRY(edit), text);
	gtk_entry_set_width_chars(GTK_ENTRY(edit), size);
	GtkWidget *label = gtk_label_new(txt_label);
	gtk_box_pack_start(GTK_BOX(box), label, false, false, 1);
	gtk_box_pack_start(GTK_BOX(box), edit, false, false, 1);
	return box;
}

GtkWidget* CreateWidgetWithLabel(const gchar *txt_label, GtkWidget *widget)
{
	GtkWidget *box = gtk_hbox_new(FALSE, 1);
	GtkWidget *label = gtk_label_new(txt_label);
	gtk_box_pack_start(GTK_BOX(box), label, false, false, 1);
	gtk_box_pack_start(GTK_BOX(box), widget, false, false, 1);
	return box;
}

void CreateFilesDialog(GtkWidget *widget, gpointer data)
{
	sDialogFiles *dialog = new sDialogFiles;

	dialog->window_files = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(dialog->window_files), "File paths");
	gtk_container_set_border_width(GTK_CONTAINER(dialog->window_files), 10);

	dialog->box_main = gtk_vbox_new(FALSE, 1);
	dialog->box_buttons = gtk_hbox_new(FALSE, 1);

	dialog->table_edits = gtk_table_new(7, 4, false);

	dialog->label_destination = gtk_label_new("Destination image sequence");
	dialog->label_background = gtk_label_new("Background");
	dialog->label_envmap = gtk_label_new("Environment map");
	dialog->label_lightmap = gtk_label_new("Ambient occlusion color map");
	dialog->label_path = gtk_label_new("Animation path");
	dialog->label_keyframes = gtk_label_new("Keyframe sequence");
	//dialog->label_fileSound = gtk_label_new("Sound file (*.wav)");

	dialog->edit_destination = gtk_entry_new();
	dialog->edit_background = gtk_entry_new();
	dialog->edit_envmap = gtk_entry_new();
	dialog->edit_lightmap = gtk_entry_new();
	dialog->edit_path = gtk_entry_new();
	dialog->edit_keyframes = gtk_entry_new();
	//dialog->edit_sound = gtk_entry_new();

	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_destination), 100);
	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_background), 100);
	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_envmap), 100);
	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_lightmap), 100);
	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_path), 100);
	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_keyframes), 100);
	//gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_sound), 100);

	gtk_entry_set_text(GTK_ENTRY(dialog->edit_destination), Interface_data.file_destination);
	gtk_entry_set_text(GTK_ENTRY(dialog->edit_background), Interface_data.file_background);
	gtk_entry_set_text(GTK_ENTRY(dialog->edit_envmap), Interface_data.file_envmap);
	gtk_entry_set_text(GTK_ENTRY(dialog->edit_lightmap), Interface_data.file_lightmap);
	gtk_entry_set_text(GTK_ENTRY(dialog->edit_path), Interface_data.file_path);
	gtk_entry_set_text(GTK_ENTRY(dialog->edit_keyframes), Interface_data.file_keyframes);
	//gtk_entry_set_text(GTK_ENTRY(dialog->edit_sound), Interface_data.file_sound);

	dialog->bu_cancel = gtk_button_new_with_label("Cancel");
	dialog->bu_ok = gtk_button_new_with_label("OK");
	dialog->bu_select_destination = gtk_button_new_with_label("Select");
	dialog->bu_select_background = gtk_button_new_with_label("Select");
	dialog->bu_select_envmap = gtk_button_new_with_label("Select");
	dialog->bu_select_lightmap = gtk_button_new_with_label("Select");
	dialog->bu_select_path = gtk_button_new_with_label("Select");
	dialog->bu_select_keyframes = gtk_button_new_with_label("Select");
	//dialog->bu_select_sound = gtk_button_new_with_label("Select");

	g_signal_connect(G_OBJECT(dialog->bu_ok), "clicked", G_CALLBACK(PressedOkDialogFiles), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_cancel), "clicked", G_CALLBACK(PressedCancelDialogFiles), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_destination), "clicked", G_CALLBACK(PressedSelectDestination), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_background), "clicked", G_CALLBACK(PressedSelectBackground), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_envmap), "clicked", G_CALLBACK(PressedSelectEnvmap), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_lightmap), "clicked", G_CALLBACK(PressedSelectLightmap), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_path), "clicked", G_CALLBACK(PressedSelectFlightPath), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_keyframes), "clicked", G_CALLBACK(PressedSelectKeyframes), dialog);
	//g_signal_connect(G_OBJECT(dialog->bu_select_sound), "clicked", G_CALLBACK(PressedSelectSound), dialog);

	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_destination, 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_background, 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_envmap, 0, 1, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_lightmap, 0, 1, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_path, 0, 1, 4, 5);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_keyframes, 0, 1, 5, 6);
	//gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_fileSound, 0, 1, 6, 7);

	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_destination, 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_background, 1, 2, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_envmap, 1, 2, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_lightmap, 1, 2, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_path, 1, 2, 4, 5);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_keyframes, 1, 2, 5, 6);
	//gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_sound, 1, 2, 6, 7);

	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_destination, 2, 3, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_background, 2, 3, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_envmap, 2, 3, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_lightmap, 2, 3, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_path, 2, 3, 4, 5);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_keyframes, 2, 3, 5, 6);
	//gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_sound, 2, 3, 6, 7);

	gtk_box_pack_start(GTK_BOX(dialog->box_main), dialog->table_edits, false, false, 1);

	gtk_box_pack_start(GTK_BOX(dialog->box_main), dialog->box_buttons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(dialog->box_buttons), dialog->bu_cancel, true, true, 1);
	gtk_box_pack_start(GTK_BOX(dialog->box_buttons), dialog->bu_ok, true, true, 1);

	gtk_container_add(GTK_CONTAINER(dialog->window_files), dialog->box_main);

	gtk_widget_show_all(dialog->window_files);
}

double atofData(const gchar *text)
{
	double retval = atof(text);
	return retval;
}

enumFractalFormula FormulaNumberGUI2Data(int formula)
{
	enumFractalFormula formula2 = trig_optim;
	if (formula == 0) formula2 = none;
	if (formula == 1) formula2 = trig_optim;
	if (formula == 2) formula2 = fast_trig;
	if (formula == 3) formula2 = minus_fast_trig;
	if (formula == 4) formula2 = xenodreambuie;
	if (formula == 5) formula2 = hypercomplex;
	if (formula == 6) formula2 = quaternion;
	if (formula == 7) formula2 = menger_sponge;
	if (formula == 8) formula2 = tglad;
	if (formula == 9) formula2 = kaleidoscopic;
	if (formula == 10) formula2 = mandelbulb2;
	if (formula == 11) formula2 = mandelbulb3;
	if (formula == 12) formula2 = mandelbulb4;
	if (formula == 13) formula2 = foldingIntPow2;
	if (formula == 14) formula2 = smoothMandelbox;
	if (formula == 15) formula2 = mandelboxVaryScale4D;
	if (formula == 16) formula2 = aexion;
	if (formula == 17) formula2 = benesi;
	return formula2;
}

int FormulaNumberData2GUI(enumFractalFormula formula)
{
	int formula2 = 0;
	if (formula == none) formula2 = 0;
	if (formula == trig_optim) formula2 = 1;
	if (formula == fast_trig) formula2 = 2;
	if (formula == minus_fast_trig) formula2 = 3;
	if (formula == xenodreambuie) formula2 = 4;
	if (formula == hypercomplex) formula2 = 5;
	if (formula == quaternion) formula2 = 6;
	if (formula == menger_sponge) formula2 = 7;
	if (formula == tglad) formula2 = 8;
	if (formula == kaleidoscopic) formula2 = 9;
	if (formula == mandelbulb2) formula2 = 10;
	if (formula == mandelbulb3) formula2 = 11;
	if (formula == mandelbulb4) formula2 = 12;
	if (formula == foldingIntPow2) formula2 = 13;
	if (formula == smoothMandelbox) formula2 = 14;
	if (formula == mandelboxVaryScale4D) formula2 = 15;
	if (formula == aexion) formula2 = 16;
	if (formula == benesi) formula2 = 17;

	return formula2;
}

//read data from interface
void ReadInterface(sParamRender *params)
{
	if (!noGUI)
	{
		params->doubles.vp.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_va)));
		params->doubles.vp.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_vb)));
		params->doubles.vp.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_vc)));
		params->doubles.alfa = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_alfa))) / 180.0 * M_PI;
		params->doubles.beta = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_beta))) / 180.0 * M_PI;
		params->doubles.gamma = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_gammaAngle))) / 180.0 * M_PI;
		params->doubles.zoom = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_zoom)));
		params->doubles.persp = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_persp)));
		params->doubles.DE_factor = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_DE_stepFactor)));
		params->doubles.quality = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_DE_thresh)));
		params->doubles.smoothness = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_roughness)));
		params->fractal.N = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_maxN)));
		params->fractal.minN = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_minN)));
		params->fractal.doubles.power = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_power)));
		params->image_width = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_imageWidth)));
		params->image_height = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_imageHeight)));
		params->doubles.imageAdjustments.brightness = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_brightness)));
		params->doubles.imageAdjustments.imageGamma = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_gamma)));
		params->doubles.imageAdjustments.ambient = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_ambient)));
		params->doubles.imageAdjustments.globalIlum = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_ambient_occlusion)));
		params->doubles.imageAdjustments.glow_intensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_glow)));
		params->doubles.imageAdjustments.reflect = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_reflect)));
		params->doubles.imageAdjustments.shading = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_shading)));
		params->doubles.imageAdjustments.directLight = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_shadows)));
		params->doubles.imageAdjustments.specular = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_specular)));
		params->global_ilumination = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAmbientOcclusion));
		params->fastGlobalIllumination = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkFastAmbientOcclusion));
		params->globalIlumQuality = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_AmbientOcclusionQuality)));
		params->shadow = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkShadow));
		params->fractal.iterThresh = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIterThresh));
		params->fractal.juliaMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkJulia));
		params->slowShading = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkSlowShading));
		params->textured_background = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkBitmapBackground));
		params->fractal.doubles.julia.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_julia_a)));
		params->fractal.doubles.julia.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_julia_b)));
		params->fractal.doubles.julia.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_julia_c)));
		params->fractal.doubles.amin = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_amin)));
		params->fractal.doubles.amax = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_amax)));
		params->fractal.doubles.bmin = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_bmin)));
		params->fractal.doubles.bmax = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_bmax)));
		params->fractal.doubles.cmin = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_cmin)));
		params->fractal.doubles.cmax = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_cmax)));
		params->fractal.limits_enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkLimits));
		params->imageSwitches.coloringEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkColoring));
		params->coloring_seed = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_color_seed)));
		params->doubles.imageAdjustments.coloring_speed = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_color_speed)));
		params->fractal.tgladFoldingMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkTgladMode));
		params->fractal.sphericalFoldingMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkSphericalFoldingMode));
		params->fractal.IFS.foldingMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSFoldingMode));
		params->fractal.doubles.foldingLimit = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_tglad_folding_1)));
		params->fractal.doubles.foldingValue = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_tglad_folding_2)));
		params->fractal.doubles.foldingSphericalFixed = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_spherical_folding_1)));
		params->fractal.doubles.foldingSphericalMin = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_spherical_folding_2)));
		params->imageSwitches.fogEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkFogEnabled));
		params->doubles.imageAdjustments.fogVisibility = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepth));
		params->doubles.imageAdjustments.fogVisibilityFront = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepthFront));
		params->SSAOQuality = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentSSAOQuality));
		params->SSAOEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkSSAOEnabled));
		params->DOFEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkDOFEnabled));
		params->doubles.DOFFocus = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentDOFFocus));
		params->doubles.DOFRadius = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentDOFRadius));
		params->doubles.imageAdjustments.mainLightIntensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mainLightIntensity)));
		params->doubles.auxLightIntensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightIntensity)));
		params->auxLightRandomSeed = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightRandomSeed)));
		params->auxLightNumber = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightNumber)));
		params->doubles.auxLightMaxDist = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightMaxDist)));
		params->doubles.auxLightDistributionRadius = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightDistributionRadius)));
		params->doubles.auxLightPre1.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre1x)));
		params->doubles.auxLightPre1.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre1y)));
		params->doubles.auxLightPre1.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre1z)));
		params->doubles.auxLightPre1intensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre1intensity)));
		params->doubles.auxLightPre2.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre2x)));
		params->doubles.auxLightPre2.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre2y)));
		params->doubles.auxLightPre2.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre2z)));
		params->doubles.auxLightPre2intensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre2intensity)));
		params->doubles.auxLightPre3.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre3x)));
		params->doubles.auxLightPre3.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre3y)));
		params->doubles.auxLightPre3.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre3z)));
		params->doubles.auxLightPre3intensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre3intensity)));
		params->doubles.auxLightPre4.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre4x)));
		params->doubles.auxLightPre4.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre4y)));
		params->doubles.auxLightPre4.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre4z)));
		params->doubles.auxLightPre4intensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre4intensity)));
		params->auxLightPre1Enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre1Enabled));
		params->auxLightPre2Enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre2Enabled));
		params->auxLightPre3Enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre3Enabled));
		params->auxLightPre4Enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre4Enabled));
		params->doubles.auxLightVisibility = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightVisibility)));
		params->doubles.mainLightAlfa = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mainLightAlfa))) / 180.0 * M_PI;
		params->doubles.mainLightBeta = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mainLightBeta))) / 180.0 * M_PI;
		params->doubles.auxLightRandomCenter.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightRandomCentreX)));
		params->doubles.auxLightRandomCenter.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightRandomCentreY)));
		params->doubles.auxLightRandomCenter.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightRandomCentreZ)));
		params->doubles.volumetricLightQuality = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricLightQuality)));
		params->doubles.imageAdjustments.volumetricLightIntensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricLightIntensity)));
		params->doubles.volumetricLightIntensity[0] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricLightMainIntensity)));
		params->doubles.volumetricLightIntensity[1] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricLightAux1Intensity)));
		params->doubles.volumetricLightIntensity[2] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricLightAux2Intensity)));
		params->doubles.volumetricLightIntensity[3] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricLightAux3Intensity)));
		params->doubles.volumetricLightIntensity[4] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricLightAux4Intensity)));
		params->volumetricLightEnabled[0] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightMainEnabled));
		params->volumetricLightEnabled[1] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux1Enabled));
		params->volumetricLightEnabled[2] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux2Enabled));
		params->volumetricLightEnabled[3] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux3Enabled));
		params->volumetricLightEnabled[4] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux4Enabled));
		params->penetratingLights = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkPenetratingLights));

		params->fractal.IFS.doubles.scale = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_IFSScale)));
		params->fractal.IFS.doubles.rotationAlfa = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_IFSAlfa))) / 180.0 * M_PI;
		params->fractal.IFS.doubles.rotationBeta = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_IFSBeta))) / 180.0 * M_PI;
		params->fractal.IFS.doubles.rotationGamma = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_IFSGamma))) / 180.0 * M_PI;
		params->fractal.IFS.doubles.offset.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_IFSOffsetX)));
		params->fractal.IFS.doubles.offset.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_IFSOffsetY)));
		params->fractal.IFS.doubles.offset.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_IFSOffsetZ)));
		params->fractal.IFS.absX = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsX));
		params->fractal.IFS.absY = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsY));
		params->fractal.IFS.absZ = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsZ));
		params->startFrame = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_animationStartFrame)));
		params->endFrame = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_animationEndFrame)));
		params->framesPerKeyframe = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_animationFramesPerKey)));
		params->doubles.imageAdjustments.paletteOffset = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentPaletteOffset));
		params->soundBand1Min = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_sound1FreqMin)));
		params->soundBand1Max = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_sound1FreqMax)));
		params->soundBand2Min = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_sound2FreqMin)));
		params->soundBand2Max = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_sound2FreqMax)));
		params->soundBand3Min = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_sound3FreqMin)));
		params->soundBand3Max = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_sound3FreqMax)));
		params->soundBand4Min = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_sound4FreqMin)));
		params->soundBand4Max = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_sound4FreqMax)));
		params->soundEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkSoundEnabled));
		params->doubles.soundFPS = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_soundFPS)));
		for (int i = 0; i < HYBRID_COUNT; ++i) {
			params->fractal.hybridIters[i] = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_hybridIter[i])));
			params->fractal.doubles.hybridPower[i] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_hybridPower[i])));
		}
		params->fractal.hybridCyclic = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkHybridCyclic));
		params->perspectiveType = (enumPerspectiveType)gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboPerspectiveType));
		params->doubles.stereoEyeDistance = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_stereoDistance)));
		params->stereoEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkStereoEnabled));
		params->doubles.viewDistanceMin = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_viewMinDistance)));
		params->doubles.viewDistanceMax = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_viewMaxDistance)));
		params->fractal.interiorMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkInteriorMode));
		params->fractal.doubles.constantFactor = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_FractalConstantFactor)));
		params->fractal.dynamicDEcorrection = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkDECorrectionMode));
		params->fractal.linearDEmode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkDELinearMode));
		params->fractal.constantDEThreshold = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkConstantDEThreshold));

		params->fractal.mandelbox.rotationsEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkMandelboxRotationsEnable));
		params->fractal.mandelbox.doubles.foldingLimit = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxFoldingLimit)));
		params->fractal.mandelbox.doubles.foldingValue = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxFoldingValue)));
		params->fractal.mandelbox.doubles.foldingSphericalFixed = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxSpFoldingFixedRadius)));
		params->fractal.mandelbox.doubles.foldingSphericalMin = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxSpFoldingMinRadius)));
		params->fractal.mandelbox.doubles.sharpness = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxSharpness)));

		for (int component = 0; component < 3; ++component)
			params->fractal.mandelbox.doubles.rotationMain[component] = atofData(gtk_entry_get_text(GTK_ENTRY(
							Interface.edit_mandelboxRotationMain[component]))) / 180.0 * M_PI;

		for (int fold = 0; fold < MANDELBOX_FOLDS; ++fold)
			for (int axis = 0; axis < 3; ++axis)
				for (int component = 0; component < 3; ++component)
					params->fractal.mandelbox.doubles.rotation[fold][axis][component] = atofData(
						gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxRotation[fold][axis][component]))) / 180.0 * M_PI;

		params->fractal.mandelbox.doubles.colorFactorR = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorR)));
		params->fractal.mandelbox.doubles.colorFactorX = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorX)));
		params->fractal.mandelbox.doubles.colorFactorY = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorY)));
		params->fractal.mandelbox.doubles.colorFactorZ = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorZ)));
		params->fractal.mandelbox.doubles.colorFactorSp1 = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorSp1)));
		params->fractal.mandelbox.doubles.colorFactorSp2 = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorSp2)));
		params->fractal.mandelbox.doubles.scale = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxScale)));
		params->fractal.mandelbox.doubles.offset.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxOffsetX)));
		params->fractal.mandelbox.doubles.offset.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxOffsetY)));
		params->fractal.mandelbox.doubles.offset.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxOffsetZ)));

		params->image_width = (params->image_width/8)*8;
		params->image_height = (params->image_height/8)*8;

		for (int i = 0; i < IFS_VECTOR_COUNT; i++)
		{
			params->fractal.IFS.doubles.direction[i].x = atof(gtk_entry_get_text(GTK_ENTRY(Interface.IFSParams[i].editIFSx)));
			params->fractal.IFS.doubles.direction[i].y = atof(gtk_entry_get_text(GTK_ENTRY(Interface.IFSParams[i].editIFSy)));
			params->fractal.IFS.doubles.direction[i].z = atof(gtk_entry_get_text(GTK_ENTRY(Interface.IFSParams[i].editIFSz)));
			params->fractal.IFS.doubles.alfa[i] = atof(gtk_entry_get_text(GTK_ENTRY(Interface.IFSParams[i].editIFSalfa))) / 180.0 * M_PI;
			params->fractal.IFS.doubles.beta[i] = atof(gtk_entry_get_text(GTK_ENTRY(Interface.IFSParams[i].editIFSbeta))) / 180.0 * M_PI;
			params->fractal.IFS.doubles.gamma[i] = atof(gtk_entry_get_text(GTK_ENTRY(Interface.IFSParams[i].editIFSgamma))) / 180.0 * M_PI;
			params->fractal.IFS.doubles.distance[i] = atof(gtk_entry_get_text(GTK_ENTRY(Interface.IFSParams[i].editIFSdistance)));
			params->fractal.IFS.doubles.intensity[i] = atof(gtk_entry_get_text(GTK_ENTRY(Interface.IFSParams[i].editIFSintensity)));
			params->fractal.IFS.enabled[i] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.IFSParams[i].checkIFSenabled));
		}

		params->fractal.doubles.FoldingIntPowFoldFactor = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_FoldingIntPowFoldingFactor)));
		params->fractal.doubles.FoldingIntPowZfactor = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_FoldingIntPowZFactor)));

		params->reflectionsMax = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_reflectionsMax)));
		params->imageSwitches.raytracedReflections = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkRaytracedReflections));

		params->fractal.mandelbox.doubles.vary4D.fold = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxVaryFold)));
		params->fractal.mandelbox.doubles.vary4D.minR = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxVaryMinR)));
		params->fractal.mandelbox.doubles.vary4D.rPower = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxVaryRPower)));
		params->fractal.mandelbox.doubles.vary4D.scaleVary = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxVaryScale)));
		params->fractal.mandelbox.doubles.vary4D.wadd = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxVaryWAdd)));

		params->fractal.doubles.cadd = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_cadd)));

		GdkColor color;
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorGlow1), &color);
		params->effectColours.glow_color1.R = color.red;
		params->effectColours.glow_color1.G = color.green;
		params->effectColours.glow_color1.B = color.blue;

		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorGlow2), &color);
		params->effectColours.glow_color2.R = color.red;
		params->effectColours.glow_color2.G = color.green;
		params->effectColours.glow_color2.B = color.blue;

		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorBackgroud1), &color);
		params->background_color1.R = color.red;
		params->background_color1.G = color.green;
		params->background_color1.B = color.blue;

		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorBackgroud2), &color);
		params->background_color2.R = color.red;
		params->background_color2.G = color.green;
		params->background_color2.B = color.blue;

		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorFog), &color);
		params->effectColours.fogColor.R = color.red;
		params->effectColours.fogColor.G = color.green;
		params->effectColours.fogColor.B = color.blue;

		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre1), &color);
		params->auxLightPre1Colour.R = color.red;
		params->auxLightPre1Colour.G = color.green;
		params->auxLightPre1Colour.B = color.blue;

		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre2), &color);
		params->auxLightPre2Colour.R = color.red;
		params->auxLightPre2Colour.G = color.green;
		params->auxLightPre2Colour.B = color.blue;

		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre3), &color);
		params->auxLightPre3Colour.R = color.red;
		params->auxLightPre3Colour.G = color.green;
		params->auxLightPre3Colour.B = color.blue;

		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre4), &color);
		params->auxLightPre4Colour.R = color.red;
		params->auxLightPre4Colour.G = color.green;
		params->auxLightPre4Colour.B = color.blue;

		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorMainLight), &color);
		params->effectColours.mainLightColour.R = color.red;
		params->effectColours.mainLightColour.G = color.green;
		params->effectColours.mainLightColour.B = color.blue;

		int formula = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboFractType));
		if (formula == 0) params->fractal.formula = trig_optim;
		if (formula == 1) params->fractal.formula = trig_DE;
		if (formula == 2) params->fractal.formula = fast_trig;
		if (formula == 3) params->fractal.formula = minus_fast_trig;
		if (formula == 4) params->fractal.formula = xenodreambuie;
		if (formula == 5) params->fractal.formula = hypercomplex;
		if (formula == 6) params->fractal.formula = quaternion;
		if (formula == 7) params->fractal.formula = menger_sponge;
		if (formula == 8) params->fractal.formula = tglad;
		if (formula == 9) params->fractal.formula = kaleidoscopic;
		if (formula == 10) params->fractal.formula = mandelbulb2;
		if (formula == 11) params->fractal.formula = mandelbulb3;
		if (formula == 12) params->fractal.formula = mandelbulb4;
		if (formula == 13) params->fractal.formula = foldingIntPow2;
		if (formula == 14) params->fractal.formula = smoothMandelbox;
		if (formula == 15) params->fractal.formula = mandelboxVaryScale4D;
		if (formula == 16) params->fractal.formula = aexion;
		if (formula == 17) params->fractal.formula = benesi;
		if (formula == 18) params->fractal.formula = hybrid;

		CheckPrameters(params);

		for (int i = 0; i < HYBRID_COUNT; ++i)
			params->fractal.hybridFormula[i] = FormulaNumberGUI2Data(gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboHybridFormula[i])));

		double imageScale;
		int scale = gtk_combo_box_get_active(GTK_COMBO_BOX(renderWindow.comboImageScale));
		if (scale == 0) imageScale = 1.0/10.0;
		if (scale == 1) imageScale = 1.0/8.0;
		if (scale == 2) imageScale = 1.0/6.0;
		if (scale == 3) imageScale = 1.0/4.0;
		if (scale == 4) imageScale = 1.0/3.0;
		if (scale == 5) imageScale = 1.0/2.0;
		if (scale == 6) imageScale = 1.0;
		if (scale == 7) imageScale = 2.0;
		if (scale == 8) imageScale = 4.0;
		if (scale == 9) imageScale = 6.0;
		if (scale == 10) imageScale = 8.0;
		if (scale == 11)
		{
			int winWidth = renderWindow.scrolled_window->allocation.width;
			int winHeight = renderWindow.scrolled_window->allocation.height;;
			//gtk_window_get_size(GTK_WINDOW(renderWindow.window),&winWidth,&winHeight);
			winWidth-=renderWindow.scrollbarSize;
			winHeight-=renderWindow.scrollbarSize;
			imageScale = (double)winWidth / params->image_width;
			if(params->image_height*imageScale > winHeight) imageScale = (double)winHeight / params->image_height;
		}
		Interface_data.imageScale = imageScale;

		params->imageFormat = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboImageFormat));

		if (params->SSAOEnabled && params->global_ilumination)
		{
			GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
					"Warning! Both Ambient occlusion based on simulated rays and Screen Space Ambient Occlusion (SSAO) are activated. SSAO will be deactivated");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			params->SSAOEnabled = false;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkSSAOEnabled), params->SSAOEnabled);
		}

		if (!params->SSAOEnabled && !params->global_ilumination && params->doubles.imageAdjustments.reflect > 0 && !params->imageSwitches.raytracedReflections)
		{
			GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
					"Warning! For reflection effect the Screen Space Ambient Occlusion (SSAO) effect has to be activated");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			params->SSAOEnabled = true;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkSSAOEnabled), params->SSAOEnabled);
		}

	}

	if (params->fractal.formula == trig_DE || params->fractal.formula == trig_optim || params->fractal.formula == menger_sponge || params->fractal.formula == kaleidoscopic
			|| params->fractal.formula == tglad || params->fractal.formula == smoothMandelbox || params->fractal.formula == mandelboxVaryScale4D) params->fractal.analitycDE = true;
	else params->fractal.analitycDE = false;

	params->doubles.resolution = 1.0 / params->image_width;

	params->quiet = false;

	InterfaceData2Params(params);

	Interface_data.imageFormat = (enumImageFormat) params->imageFormat;

	mainImage.SetImageParameters(params->doubles.imageAdjustments,params->effectColours,params->imageSwitches);

	//srand(Interface_data.coloring_seed);
	//NowaPaleta(paleta, 1.0);

	sRGB *palette2 = mainImage.GetPalettePtr();
	for (int i = 0; i < 256; i++)
	{
		params->palette[i] = palette2[i];
	}

	RecalculateIFSParams(params->fractal);
	CreateFormulaSequence(params->fractal);
}

char* DoubleToString(double value)
{
	static char text[100];
	sprintf(text, "%.16lg", value);
	return text;
}

char*
IntToString(int value)
{
	static char text[100];
	sprintf(text, "%d", value);
	return text;
}

void WriteInterface(sParamRender *params)
{
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_va), DoubleToString(params->doubles.vp.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_vb), DoubleToString(params->doubles.vp.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_vc), DoubleToString(params->doubles.vp.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_alfa), DoubleToString(params->doubles.alfa * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_beta), DoubleToString(params->doubles.beta * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_gammaAngle), DoubleToString(params->doubles.gamma * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_zoom), DoubleToString(params->doubles.zoom));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_persp), DoubleToString(params->doubles.persp));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_DE_stepFactor), DoubleToString(params->doubles.DE_factor));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_DE_thresh), DoubleToString(params->doubles.quality));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_roughness), DoubleToString(params->doubles.smoothness));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_maxN), IntToString(params->fractal.N));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_minN), IntToString(params->fractal.minN));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_power), DoubleToString(params->fractal.doubles.power));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_imageWidth), IntToString(params->image_width));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_imageHeight), IntToString(params->image_height));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_brightness), DoubleToString(params->doubles.imageAdjustments.brightness));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_gamma), DoubleToString(params->doubles.imageAdjustments.imageGamma));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_ambient), DoubleToString(params->doubles.imageAdjustments.ambient));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_ambient_occlusion), DoubleToString(params->doubles.imageAdjustments.globalIlum));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_glow), DoubleToString(params->doubles.imageAdjustments.glow_intensity));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_reflect), DoubleToString(params->doubles.imageAdjustments.reflect));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_shading), DoubleToString(params->doubles.imageAdjustments.shading));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_shadows), DoubleToString(params->doubles.imageAdjustments.directLight));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_specular), DoubleToString(params->doubles.imageAdjustments.specular));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_AmbientOcclusionQuality), IntToString(params->globalIlumQuality));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_julia_a), DoubleToString(params->fractal.doubles.julia.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_julia_b), DoubleToString(params->fractal.doubles.julia.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_julia_c), DoubleToString(params->fractal.doubles.julia.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_amin), DoubleToString(params->fractal.doubles.amin));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_amax), DoubleToString(params->fractal.doubles.amax));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_bmin), DoubleToString(params->fractal.doubles.bmin));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_bmax), DoubleToString(params->fractal.doubles.bmax));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_cmin), DoubleToString(params->fractal.doubles.cmin));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_cmax), DoubleToString(params->fractal.doubles.cmax));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_color_seed), IntToString(params->coloring_seed));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_color_speed), DoubleToString(params->doubles.imageAdjustments.coloring_speed));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_tglad_folding_1), DoubleToString(params->fractal.doubles.foldingLimit));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_tglad_folding_2), DoubleToString(params->fractal.doubles.foldingValue));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_spherical_folding_1), DoubleToString(params->fractal.doubles.foldingSphericalFixed));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_spherical_folding_2), DoubleToString(params->fractal.doubles.foldingSphericalMin));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mainLightIntensity), DoubleToString(params->doubles.imageAdjustments.mainLightIntensity));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightIntensity), DoubleToString(params->doubles.auxLightIntensity));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightRandomSeed), IntToString(params->auxLightRandomSeed));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightNumber), IntToString(params->auxLightNumber));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightMaxDist), DoubleToString(params->doubles.auxLightMaxDist));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightDistributionRadius), DoubleToString(params->doubles.auxLightDistributionRadius));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre1x), DoubleToString(params->doubles.auxLightPre1.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre1y), DoubleToString(params->doubles.auxLightPre1.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre1z), DoubleToString(params->doubles.auxLightPre1.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre1intensity), DoubleToString(params->doubles.auxLightPre1intensity));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre2x), DoubleToString(params->doubles.auxLightPre2.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre2y), DoubleToString(params->doubles.auxLightPre2.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre2z), DoubleToString(params->doubles.auxLightPre2.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre2intensity), DoubleToString(params->doubles.auxLightPre2intensity));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre3x), DoubleToString(params->doubles.auxLightPre3.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre3y), DoubleToString(params->doubles.auxLightPre3.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre3z), DoubleToString(params->doubles.auxLightPre3.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre3intensity), DoubleToString(params->doubles.auxLightPre3intensity));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre4x), DoubleToString(params->doubles.auxLightPre4.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre4y), DoubleToString(params->doubles.auxLightPre4.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre4z), DoubleToString(params->doubles.auxLightPre4.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre4intensity), DoubleToString(params->doubles.auxLightPre4intensity));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mainLightAlfa), DoubleToString(params->doubles.mainLightAlfa * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mainLightBeta), DoubleToString(params->doubles.mainLightBeta * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightVisibility), DoubleToString(params->doubles.auxLightVisibility));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightRandomCentreX), DoubleToString(params->doubles.auxLightRandomCenter.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightRandomCentreY), DoubleToString(params->doubles.auxLightRandomCenter.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightRandomCentreZ), DoubleToString(params->doubles.auxLightRandomCenter.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_IFSScale), DoubleToString(params->fractal.IFS.doubles.scale));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_IFSAlfa), DoubleToString(params->fractal.IFS.doubles.rotationAlfa * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_IFSBeta), DoubleToString(params->fractal.IFS.doubles.rotationBeta * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_IFSGamma), DoubleToString(params->fractal.IFS.doubles.rotationGamma * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_IFSOffsetX), DoubleToString(params->fractal.IFS.doubles.offset.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_IFSOffsetY), DoubleToString(params->fractal.IFS.doubles.offset.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_IFSOffsetZ), DoubleToString(params->fractal.IFS.doubles.offset.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_animationStartFrame), IntToString(params->startFrame));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_animationEndFrame), IntToString(params->endFrame));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_animationFramesPerKey), IntToString(params->framesPerKeyframe));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_soundFPS), DoubleToString(params->doubles.soundFPS));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_sound1FreqMin), IntToString(params->soundBand1Min));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_sound1FreqMax), IntToString(params->soundBand1Max));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_sound2FreqMin), IntToString(params->soundBand2Min));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_sound2FreqMax), IntToString(params->soundBand2Max));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_sound3FreqMin), IntToString(params->soundBand3Min));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_sound3FreqMax), IntToString(params->soundBand3Max));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_sound4FreqMin), IntToString(params->soundBand4Min));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_sound4FreqMax), IntToString(params->soundBand4Max));
	for (int i = 0; i < HYBRID_COUNT; ++i) {
		gtk_entry_set_text(GTK_ENTRY(Interface.edit_hybridIter[i]), IntToString(params->fractal.hybridIters[i]));
		gtk_entry_set_text(GTK_ENTRY(Interface.edit_hybridPower[i]), DoubleToString(params->fractal.doubles.hybridPower[i]));
	}
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_stereoDistance), DoubleToString(params->doubles.stereoEyeDistance));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorR), DoubleToString(params->fractal.mandelbox.doubles.colorFactorR));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorX), DoubleToString(params->fractal.mandelbox.doubles.colorFactorX));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorY), DoubleToString(params->fractal.mandelbox.doubles.colorFactorY));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorZ), DoubleToString(params->fractal.mandelbox.doubles.colorFactorZ));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorSp1), DoubleToString(params->fractal.mandelbox.doubles.colorFactorSp1));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxColorFactorSp2), DoubleToString(params->fractal.mandelbox.doubles.colorFactorSp2));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxFoldingLimit), DoubleToString(params->fractal.mandelbox.doubles.foldingLimit));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxFoldingValue), DoubleToString(params->fractal.mandelbox.doubles.foldingValue));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxSpFoldingFixedRadius), DoubleToString(params->fractal.mandelbox.doubles.foldingSphericalFixed));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxSpFoldingMinRadius), DoubleToString(params->fractal.mandelbox.doubles.foldingSphericalMin));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxSharpness), DoubleToString(params->fractal.mandelbox.doubles.sharpness));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxOffsetX), DoubleToString(params->fractal.mandelbox.doubles.offset.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxOffsetY), DoubleToString(params->fractal.mandelbox.doubles.offset.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxOffsetZ), DoubleToString(params->fractal.mandelbox.doubles.offset.z));

	for (int component = 0; component < 3; ++component)
		gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxRotationMain[component]), DoubleToString(
					params->fractal.mandelbox.doubles.rotationMain[component] * 180.0 / M_PI));

	for (int fold = 0; fold < MANDELBOX_FOLDS; ++fold)
		for (int axis = 0; axis < 3; ++axis)
			for (int component = 0; component < 3; ++component)
				gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxRotation[fold][axis][component]),
						DoubleToString(params->fractal.mandelbox.doubles.rotation[fold][axis][component] * 180.0 / M_PI));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxScale), DoubleToString(params->fractal.mandelbox.doubles.scale));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_viewMaxDistance), DoubleToString(params->doubles.viewDistanceMax));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_viewMinDistance), DoubleToString(params->doubles.viewDistanceMin));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_FractalConstantFactor), DoubleToString(params->fractal.doubles.constantFactor));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_FoldingIntPowFoldingFactor), DoubleToString(params->fractal.doubles.FoldingIntPowFoldFactor));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_FoldingIntPowZFactor), DoubleToString(params->fractal.doubles.FoldingIntPowZfactor));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricLightMainIntensity), DoubleToString(params->doubles.volumetricLightIntensity[0]));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricLightAux1Intensity), DoubleToString(params->doubles.volumetricLightIntensity[1]));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricLightAux2Intensity), DoubleToString(params->doubles.volumetricLightIntensity[2]));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricLightAux3Intensity), DoubleToString(params->doubles.volumetricLightIntensity[3]));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricLightAux4Intensity), DoubleToString(params->doubles.volumetricLightIntensity[4]));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricLightQuality), DoubleToString(params->doubles.volumetricLightQuality));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricLightIntensity), DoubleToString(params->doubles.imageAdjustments.volumetricLightIntensity));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_reflectionsMax), IntToString(params->reflectionsMax));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxVaryFold), DoubleToString(params->fractal.mandelbox.doubles.vary4D.fold));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxVaryMinR), DoubleToString(params->fractal.mandelbox.doubles.vary4D.minR));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxVaryRPower), DoubleToString(params->fractal.mandelbox.doubles.vary4D.rPower));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxVaryScale), DoubleToString(params->fractal.mandelbox.doubles.vary4D.scaleVary));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxVaryWAdd), DoubleToString(params->fractal.mandelbox.doubles.vary4D.wadd));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_cadd), DoubleToString(params->fractal.doubles.cadd));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkAmbientOcclusion), params->global_ilumination);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkFastAmbientOcclusion), params->fastGlobalIllumination);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkShadow), params->shadow);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIterThresh), params->fractal.iterThresh);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkJulia), params->fractal.juliaMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkSlowShading), params->slowShading);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkLimits), params->fractal.limits_enabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkBitmapBackground), params->textured_background);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkColoring), params->imageSwitches.coloringEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkTgladMode), params->fractal.tgladFoldingMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkSphericalFoldingMode), params->fractal.sphericalFoldingMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIFSFoldingMode), params->fractal.IFS.foldingMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkFogEnabled), params->imageSwitches.fogEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkSSAOEnabled), params->SSAOEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkDOFEnabled), params->DOFEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre1Enabled), params->auxLightPre1Enabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre2Enabled), params->auxLightPre2Enabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre3Enabled), params->auxLightPre3Enabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre4Enabled), params->auxLightPre4Enabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsX), params->fractal.IFS.absX);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsY), params->fractal.IFS.absY);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsZ), params->fractal.IFS.absZ);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkSoundEnabled), params->soundEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkHybridCyclic), params->fractal.hybridCyclic);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkStereoEnabled), params->stereoEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkMandelboxRotationsEnable), params->fractal.mandelbox.rotationsEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkInteriorMode), params->fractal.interiorMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkDECorrectionMode), params->fractal.dynamicDEcorrection);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkDELinearMode), params->fractal.linearDEmode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkConstantDEThreshold), params->fractal.constantDEThreshold);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightMainEnabled), params->volumetricLightEnabled[0]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux1Enabled), params->volumetricLightEnabled[1]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux2Enabled), params->volumetricLightEnabled[2]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux3Enabled), params->volumetricLightEnabled[3]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux4Enabled), params->volumetricLightEnabled[4]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkPenetratingLights), params->penetratingLights);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkRaytracedReflections), params->imageSwitches.raytracedReflections);

	gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepth), params->doubles.imageAdjustments.fogVisibility);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepthFront), params->doubles.imageAdjustments.fogVisibilityFront);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentSSAOQuality), params->SSAOQuality);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentDOFFocus), params->doubles.DOFFocus);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentDOFRadius), params->doubles.DOFRadius);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentPaletteOffset), params->doubles.imageAdjustments.paletteOffset);

	for (int i = 0; i < IFS_VECTOR_COUNT; i++)
	{
		gtk_entry_set_text(GTK_ENTRY(Interface.IFSParams[i].editIFSx), DoubleToString(params->fractal.IFS.doubles.direction[i].x));
		gtk_entry_set_text(GTK_ENTRY(Interface.IFSParams[i].editIFSy), DoubleToString(params->fractal.IFS.doubles.direction[i].y));
		gtk_entry_set_text(GTK_ENTRY(Interface.IFSParams[i].editIFSz), DoubleToString(params->fractal.IFS.doubles.direction[i].z));
		gtk_entry_set_text(GTK_ENTRY(Interface.IFSParams[i].editIFSalfa), DoubleToString(params->fractal.IFS.doubles.alfa[i] * 180.0 / M_PI));
		gtk_entry_set_text(GTK_ENTRY(Interface.IFSParams[i].editIFSbeta), DoubleToString(params->fractal.IFS.doubles.beta[i] * 180.0 / M_PI));
		gtk_entry_set_text(GTK_ENTRY(Interface.IFSParams[i].editIFSgamma), DoubleToString(params->fractal.IFS.doubles.gamma[i] * 180.0 / M_PI));
		gtk_entry_set_text(GTK_ENTRY(Interface.IFSParams[i].editIFSdistance), DoubleToString(params->fractal.IFS.doubles.distance[i]));
		gtk_entry_set_text(GTK_ENTRY(Interface.IFSParams[i].editIFSintensity), DoubleToString(params->fractal.IFS.doubles.intensity[i]));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.IFSParams[i].checkIFSenabled), params->fractal.IFS.enabled[i]);
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboPerspectiveType), params->perspectiveType);

	int formula = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboFractType));
	if (params->fractal.formula == trig_optim) formula = 0;
	if (params->fractal.formula == trig_DE) formula = 1;
	if (params->fractal.formula == fast_trig) formula = 2;
	if (params->fractal.formula == minus_fast_trig) formula = 3;
	if (params->fractal.formula == xenodreambuie) formula = 4;
	if (params->fractal.formula == hypercomplex) formula = 5;
	if (params->fractal.formula == quaternion) formula = 6;
	if (params->fractal.formula == menger_sponge) formula = 7;
	if (params->fractal.formula == tglad) formula = 8;
	if (params->fractal.formula == kaleidoscopic) formula = 9;
	if (params->fractal.formula == mandelbulb2) formula = 10;
	if (params->fractal.formula == mandelbulb3) formula = 11;
	if (params->fractal.formula == mandelbulb4) formula = 12;
	if (params->fractal.formula == foldingIntPow2) formula = 13;
	if (params->fractal.formula == smoothMandelbox) formula = 14;
	if (params->fractal.formula == mandelboxVaryScale4D) formula = 15;
	if (params->fractal.formula == aexion) formula = 16;
	if (params->fractal.formula == benesi) formula = 17;
	if (params->fractal.formula == hybrid) formula = 18;
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboFractType), formula);

	for (int i = 0; i < HYBRID_COUNT; ++i)
		gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboHybridFormula[i]), FormulaNumberData2GUI(params->fractal.hybridFormula[i]));

	GdkColor color;

	color.red = params->effectColours.glow_color1.R;
	color.green = params->effectColours.glow_color1.G;
	color.blue = params->effectColours.glow_color1.B;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorGlow1), &color);

	color.red = params->effectColours.glow_color2.R;
	color.green = params->effectColours.glow_color2.G;
	color.blue = params->effectColours.glow_color2.B;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorGlow2), &color);

	color.red = params->background_color1.R;
	color.green = params->background_color1.G;
	color.blue = params->background_color1.B;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorBackgroud1), &color);

	color.red = params->background_color2.R;
	color.green = params->background_color2.G;
	color.blue = params->background_color2.B;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorBackgroud2), &color);

	color.red = params->effectColours.fogColor.R;
	color.green = params->effectColours.fogColor.G;
	color.blue = params->effectColours.fogColor.B;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorFog), &color);

	color.red = params->auxLightPre1Colour.R;
	color.green = params->auxLightPre1Colour.G;
	color.blue = params->auxLightPre1Colour.B;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre1), &color);

	color.red = params->auxLightPre2Colour.R;
	color.green = params->auxLightPre2Colour.G;
	color.blue = params->auxLightPre2Colour.B;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre2), &color);

	color.red = params->auxLightPre3Colour.R;
	color.green = params->auxLightPre3Colour.G;
	color.blue = params->auxLightPre3Colour.B;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre3), &color);

	color.red = params->auxLightPre4Colour.R;
	color.green = params->auxLightPre4Colour.G;
	color.blue = params->auxLightPre4Colour.B;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre4), &color);

	color.red = params->effectColours.mainLightColour.R;
	color.green = params->effectColours.mainLightColour.G;
	color.blue = params->effectColours.mainLightColour.B;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorMainLight), &color);

	DrawPalette(params->palette);
}

void AddComboTextsFractalFormula(GtkComboBox *combo)
{
	gtk_combo_box_append_text(combo, "None");
	gtk_combo_box_append_text(combo, "Mandelbulb");
	gtk_combo_box_append_text(combo, "Polynomic power 2");
	gtk_combo_box_append_text(combo, "Polynomic power 2 - minus z");
	gtk_combo_box_append_text(combo, "Xenodreambuie's formula");
	gtk_combo_box_append_text(combo, "Hypercomplex");
	gtk_combo_box_append_text(combo, "Quaternion");
	gtk_combo_box_append_text(combo, "Menger sponge");
	gtk_combo_box_append_text(combo, "Mandelbox");
	gtk_combo_box_append_text(combo, "Kaleidoscopic IFS");
	gtk_combo_box_append_text(combo, "Modified Mandelbulb 1");
	gtk_combo_box_append_text(combo, "Modified Mandelbulb 2");
	gtk_combo_box_append_text(combo, "Modified Mandelbulb 3");
	gtk_combo_box_append_text(combo, "FoldingIntPow2");
	gtk_combo_box_append_text(combo, "Smooth Mandelbox");
	gtk_combo_box_append_text(combo, "Mandelbox vary scale 4D");
	gtk_combo_box_append_text(combo, "Aexion");
	gtk_combo_box_append_text(combo, "Benesi");
}

void CreateInterface(sParamRender *default_settings)
{
	//------------- glowne okno renderowania
	renderWindow.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(renderWindow.window), "Mandelbulber Render Window");
	CONNECT_SIGNAL(renderWindow.window, StopRenderingAndQuit, "delete_event");
	gtk_widget_add_events(GTK_WIDGET(renderWindow.window), GDK_CONFIGURE);
  CONNECT_SIGNAL(renderWindow.window, WindowReconfigured, "configure-event");

	//glowny box w oknie
	renderWindow.mainBox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(renderWindow.mainBox), 0);

	//obszar rysowania
	renderWindow.drawingArea = gtk_drawing_area_new();

	gtk_widget_set_size_request(renderWindow.drawingArea, mainImage.GetPreviewWidth(), mainImage.GetPreviewHeight()+50);
	gtk_window_set_default_size(GTK_WINDOW(renderWindow.window), mainImage.GetPreviewWidth() + 16, mainImage.GetPreviewHeight() + 50);
	renderWindow.lastWindowWidth = mainImage.GetPreviewWidth()+16;
	renderWindow.lastWindowHeight = mainImage.GetPreviewHeight()+50;

	gtk_signal_connect(GTK_OBJECT(renderWindow.drawingArea), "expose-event", GTK_SIGNAL_FUNC(on_darea_expose), NULL);
	gtk_signal_connect(GTK_OBJECT(renderWindow.drawingArea), "motion_notify_event", (GtkSignalFunc) motion_notify_event, NULL);
	gtk_signal_connect(GTK_OBJECT(renderWindow.drawingArea), "button_press_event", GTK_SIGNAL_FUNC(pressed_button_on_image), NULL);
	gtk_widget_set_events(renderWindow.drawingArea, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK);

	renderWindow.hadjustment = gtk_adjustment_new(0, 0, 1000, 1, 1, 100);
	renderWindow.vadjustment = gtk_adjustment_new(0, 0, 1000, 1, 1, 100);

	renderWindow.scrolled_window = gtk_scrolled_window_new(GTK_ADJUSTMENT(renderWindow.hadjustment), GTK_ADJUSTMENT(renderWindow.vadjustment));

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(renderWindow.scrolled_window), renderWindow.drawingArea);

	gtk_box_pack_start(GTK_BOX(renderWindow.mainBox), renderWindow.scrolled_window, true, true, 1);

	//----- buttons
	renderWindow.boxButtons = gtk_hbox_new(FALSE, 1);
	gtk_box_pack_start(GTK_BOX(renderWindow.mainBox), renderWindow.boxButtons, false, false, 1);

	//image scale combo
	renderWindow.labelImageScale = gtk_label_new("Image scale:");
	gtk_box_pack_start(GTK_BOX(renderWindow.boxButtons), renderWindow.labelImageScale, false, false, 1);
	renderWindow.comboImageScale = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "1/10");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "1/8");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "1/6");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "1/4");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "1/3");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "1/2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "1");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "4");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "6");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "8");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboImageScale), "Fit to window");
	gtk_combo_box_set_active(GTK_COMBO_BOX(renderWindow.comboImageScale), 11);
	gtk_box_pack_start(GTK_BOX(renderWindow.boxButtons), renderWindow.comboImageScale, false, false, 10);

	//image scale combo
	renderWindow.labelMouseClickMode = gtk_label_new("Mouse click function:");
	gtk_box_pack_start(GTK_BOX(renderWindow.boxButtons), renderWindow.labelMouseClickMode, false, false, 1);
	renderWindow.comboMouseClickMode = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "None");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Move the camera");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Set fog front distance");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Set fog visibility distance");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Set Depth of Field focus point");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Set position of aux. light #1");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Set position of aux. light #2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Set position of aux. light #3");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Set position of aux. light #4");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Set position of centre for random lights");
	gtk_combo_box_set_active(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), 1);
	gtk_box_pack_start(GTK_BOX(renderWindow.boxButtons), renderWindow.comboMouseClickMode, false, false, 1);

	//mainBox into window
	gtk_container_add(GTK_CONTAINER(renderWindow.window), renderWindow.mainBox);

	//wyswietlenie wszystkich widgetow
	gtk_widget_show_all(renderWindow.window);

	//get scrollbar size
	GtkWidget *hscrollbar = gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(renderWindow.scrolled_window));
	renderWindow.scrollbarSize = hscrollbar->allocation.height;

	//------------------- okno histogramu iteracji ------------

	window_histogram = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window_histogram), "left - number of iterations (max 64) / right - number of steps (max. 1000)");
	CONNECT_SIGNAL(window_histogram, StopRenderingAndQuit, "delete_event");

	//glowny box w oknie
	GtkWidget *box2 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(box2), 5);

	//obszar rysowania
	darea2 = gtk_drawing_area_new();
	gtk_widget_set_size_request(darea2, 512, 128);

	gtk_box_pack_start(GTK_BOX(box2), darea2, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window_histogram), box2);
	gtk_widget_show_all(window_histogram);

	//------------------- MAIN WINDOW ------------

	window_interface = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window_interface), "Mandelbulber (default.fract)");
	gtk_container_set_border_width(GTK_CONTAINER(window_interface), 10);
	CONNECT_SIGNAL(window_interface, StopRenderingAndQuit, "delete_event");

	//tabs
	Interface.tabs = gtk_notebook_new();

	Interface.tab_label_view = gtk_label_new("View");
	Interface.tab_label_fractal = gtk_label_new("Fractal");
	Interface.tab_label_hybrid = gtk_label_new("Hybrid");
	Interface.tab_label_mandelbox = gtk_label_new("Mandelbox");
	Interface.tab_label_shaders = gtk_label_new("Shaders");
	Interface.tab_label_image = gtk_label_new("Image");
	Interface.tab_label_animation = gtk_label_new("Animation");
	Interface.tab_label_posteffects = gtk_label_new("Post effects");
	Interface.tab_label_lights = gtk_label_new("Lights");
	Interface.tab_label_IFS = gtk_label_new("IFS");
	Interface.tab_label_sound = gtk_label_new("Sound");
	Interface.tab_label_about = gtk_label_new("About...");

	Interface.tab_box_view = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_fractal = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_shaders = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_image = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_animation = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_posteffects = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_about = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_lights = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_sound = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_IFS = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_hybrid = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_mandelbox = gtk_vbox_new(FALSE, 1);

	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_view), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_fractal), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_shaders), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_image), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_animation), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_posteffects), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_lights), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_IFS), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_sound), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_about), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_hybrid), 5);

	//boxes
	Interface.boxMain = gtk_vbox_new(FALSE, 1);
	Interface.boxButtons = gtk_hbox_new(FALSE, 1);
	Interface.boxView = gtk_vbox_new(FALSE, 1);
	Interface.boxCoordinates = gtk_hbox_new(FALSE, 1);
	Interface.boxAngle = gtk_hbox_new(FALSE, 1);
	Interface.boxNavigation = gtk_vbox_new(FALSE, 1);
	Interface.boxNavigationButtons = gtk_hbox_new(FALSE, 1);
	Interface.boxNavigationZooming = gtk_hbox_new(FALSE, 1);
	Interface.boxZoom = gtk_hbox_new(FALSE, 1);
	Interface.boxArrows = gtk_hbox_new(FALSE, 1);
	Interface.boxArrows2 = gtk_vbox_new(FALSE, 1);
	Interface.boxArrows3 = gtk_vbox_new(FALSE, 1);
	Interface.boxFractal = gtk_vbox_new(FALSE, 1);
	Interface.boxFractalFormula = gtk_vbox_new(FALSE, 1);
	Interface.boxFractalPower = gtk_hbox_new(FALSE, 1);
	Interface.boxFractalFoldingIntPow = gtk_hbox_new(FALSE, 1);
	Interface.boxFractalFolding = gtk_vbox_new(FALSE, 1);
	Interface.boxFractalRayMarching = gtk_vbox_new(FALSE, 1);
	Interface.boxFractalSwitches = gtk_hbox_new(FALSE, 1);
	Interface.boxLimits = gtk_hbox_new(FALSE, 1);
	Interface.boxJulia = gtk_hbox_new(FALSE, 1);
	Interface.boxQuality = gtk_hbox_new(FALSE, 1);
	Interface.boxImage = gtk_vbox_new(FALSE, 1);
	Interface.boxImageRes = gtk_hbox_new(FALSE, 1);
	Interface.boxEffects = gtk_vbox_new(FALSE, 1);
	Interface.boxBrightness = gtk_hbox_new(FALSE, 1);
	Interface.boxShading = gtk_hbox_new(FALSE, 1);
	Interface.boxShading2 = gtk_hbox_new(FALSE, 1);
	Interface.boxEffectsChecks = gtk_hbox_new(FALSE, 1);
	Interface.boxEffectsChecks2 = gtk_hbox_new(FALSE, 1);
	Interface.boxEffectsColoring = gtk_hbox_new(FALSE, 1);
	Interface.boxColors = gtk_vbox_new(FALSE, 1);
	Interface.boxGlowColor = gtk_hbox_new(FALSE, 1);
	Interface.boxLoadSave = gtk_hbox_new(FALSE, 1);
	Interface.boxAnimation = gtk_vbox_new(FALSE, 1);
	Interface.boxAnimationButtons = gtk_hbox_new(FALSE, 1);
	Interface.boxAnimationEdits = gtk_hbox_new(FALSE, 1);
	Interface.boxAnimationEdits2 = gtk_hbox_new(FALSE, 1);
	Interface.boxTgladFolding = gtk_hbox_new(FALSE, 1);
	Interface.boxSphericalFolding = gtk_hbox_new(FALSE, 1);
	Interface.boxSaveImage = gtk_hbox_new(FALSE, 1);
	Interface.boxPostFog = gtk_vbox_new(FALSE, 1);
	Interface.boxFogButtons = gtk_hbox_new(FALSE, 1);
	Interface.boxFogSlider = gtk_hbox_new(FALSE, 1);
	Interface.boxFogSlider2 = gtk_hbox_new(FALSE, 1);
	Interface.boxPostSSAO = gtk_vbox_new(FALSE, 1);
	Interface.boxSSAOButtons = gtk_hbox_new(FALSE, 1);
	Interface.boxSSAOSlider = gtk_hbox_new(FALSE, 1);
	Interface.boxPostDOF = gtk_vbox_new(FALSE, 1);
	Interface.boxDOFSlider1 = gtk_hbox_new(FALSE, 1);
	Interface.boxDOFSlider2 = gtk_hbox_new(FALSE, 1);
	Interface.boxDOFButtons = gtk_hbox_new(FALSE, 1);
	Interface.boxLightBallance = gtk_vbox_new(FALSE, 1);
	Interface.boxLightsParameters = gtk_vbox_new(FALSE, 1);
	Interface.boxPredefinedLights = gtk_vbox_new(FALSE, 1);
	Interface.boxLightBrightness = gtk_hbox_new(FALSE, 1);
	Interface.boxLightDistribution = gtk_hbox_new(FALSE, 1);
	Interface.boxLightDistribution2 = gtk_hbox_new(FALSE, 1);
	Interface.boxLightPre1 = gtk_hbox_new(FALSE, 1);
	Interface.boxLightPre2 = gtk_hbox_new(FALSE, 1);
	Interface.boxLightPre3 = gtk_hbox_new(FALSE, 1);
	Interface.boxLightPre4 = gtk_hbox_new(FALSE, 1);
	Interface.boxLightCommon= gtk_hbox_new(FALSE, 1);
	Interface.boxMainLight = gtk_vbox_new(FALSE, 1);
	Interface.boxMainLightPosition = gtk_hbox_new(FALSE, 1);
	Interface.boxIFSMain = gtk_vbox_new(FALSE, 1);
	Interface.boxIFSMainEdit = gtk_hbox_new(FALSE, 1);
	Interface.boxIFSMainEdit2 = gtk_hbox_new(FALSE, 1);
	Interface.boxIFSParams = gtk_vbox_new(FALSE, 1);
	Interface.boxIFSButtons = gtk_hbox_new(FALSE, 1);
	Interface.boxIFSDefaults = gtk_hbox_new(FALSE, 1);
	Interface.boxKeyframeAnimation = gtk_vbox_new(FALSE, 1);
	Interface.boxKeyframeAnimationButtons = gtk_hbox_new(FALSE, 1);
	Interface.boxKeyframeAnimationButtons2 = gtk_hbox_new(FALSE, 1);
	Interface.boxKeyframeAnimationEdits = gtk_hbox_new(FALSE, 1);
	Interface.boxBottomKeyframeAnimation = gtk_hbox_new(FALSE, 1);
	Interface.boxPalette = gtk_vbox_new(FALSE, 1);
	Interface.boxPaletteOffset = gtk_hbox_new(FALSE, 1);
	Interface.boxImageSaving = gtk_vbox_new(FALSE, 1);
	Interface.boxImageAutoSave = gtk_hbox_new(FALSE, 1);
	Interface.boxSound = gtk_vbox_new(FALSE, 1);
	Interface.boxSoundBand1 = gtk_hbox_new(FALSE, 1);
	Interface.boxSoundBand2 = gtk_hbox_new(FALSE, 1);
	Interface.boxSoundBand3 = gtk_hbox_new(FALSE, 1);
	Interface.boxSoundBand4 = gtk_hbox_new(FALSE, 1);
	Interface.boxSoundMisc = gtk_hbox_new(FALSE, 1);
	Interface.boxHybrid = gtk_vbox_new(FALSE, 1);
	Interface.boxStereoscopic = gtk_vbox_new(FALSE, 1);
	Interface.boxStereoParams = gtk_hbox_new(FALSE, 1);
	Interface.boxMandelboxMainParams = gtk_vbox_new(FALSE, 1);
	Interface.boxMandelboxRotations = gtk_vbox_new(FALSE, 1);
	Interface.boxMandelboxColoring = gtk_vbox_new(FALSE, 1);
	Interface.boxMandelboxMainParams1 = gtk_hbox_new(FALSE, 1);
	Interface.boxMandelboxMainParams2 = gtk_hbox_new(FALSE, 1);
	Interface.boxMandelboxRotationMain = gtk_hbox_new(FALSE, 1);
	Interface.boxMandelboxColor1 = gtk_hbox_new(FALSE, 1);
	Interface.boxMandelboxColor2 = gtk_hbox_new(FALSE, 1);
	Interface.boxMandelboxColor3 = gtk_hbox_new(FALSE, 1);
	Interface.boxMandelboxOffset = gtk_hbox_new(FALSE, 1);
	Interface.boxViewDistance = gtk_hbox_new(FALSE, 1);
	Interface.boxVolumetricLight = gtk_vbox_new(FALSE, 1);
	Interface.boxVolumetricLightGeneral = gtk_hbox_new(FALSE, 1);
	Interface.boxVolumetricLightMain = gtk_hbox_new(FALSE, 1);
	Interface.boxVolumetricLightAux = gtk_hbox_new(FALSE, 1);
	Interface.boxMandelboxVary = gtk_hbox_new(FALSE, 1);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.boxFractal), 5);

	//tables
	Interface.tableLimits = gtk_table_new(2, 3, false);
	Interface.tableArrows = gtk_table_new(3, 3, false);
	Interface.tableArrows2 = gtk_table_new(3, 3, false);
	Interface.tableIFSParams = gtk_table_new(9, 9, false);
	Interface.tableHybridParams = gtk_table_new(5, 3, false);
	Interface.tableMandelboxRotations = gtk_table_new(5, 7, false);

	//frames
	Interface.frCoordinates = gtk_frame_new("Viewpoint coordinates");
	Interface.fr3Dnavigator = gtk_frame_new("3D Navigator");
	Interface.frFractal = gtk_frame_new("Fractal Parameters");
	Interface.frFractalFormula = gtk_frame_new("Formula");
	Interface.frFractalFoldingIntPow = gtk_frame_new("Folding Int Pow 2 formula");
	Interface.frFractalFolding = gtk_frame_new("Folding");
	Interface.frFractalRayMarching = gtk_frame_new("Ray-tracing parameters");
	Interface.frLimits = gtk_frame_new("Limits");
	Interface.frImage = gtk_frame_new("Image parameters");
	Interface.frEffects = gtk_frame_new("Shading effects");
	Interface.frColors = gtk_frame_new("Colours");
	Interface.frLoadSave = gtk_frame_new("Settings");
	Interface.frAnimation = gtk_frame_new("Flight animation");
	Interface.frAnimationFrames = gtk_frame_new("Frames to render");
	Interface.frPostFog = gtk_frame_new("Fog");
	Interface.frPostSSAO = gtk_frame_new("Screen space ambient occlusion");
	Interface.frPostDOF = gtk_frame_new("Depth of field");
	Interface.frLightBallance = gtk_frame_new("Light brightness balance");
	Interface.frLightsParameters = gtk_frame_new("Random lights parameters");
	Interface.frLightsCommon = gtk_frame_new("Common parameters");
	Interface.frPredefinedLights = gtk_frame_new("Predefined lights");
	Interface.frMainLight = gtk_frame_new("Main light source (connected with camera)");
	Interface.frIFSMain = gtk_frame_new("General IFS parameters");
	Interface.frIFSParams = gtk_frame_new("Symmetry vectors");
	Interface.frIFSDefaults = gtk_frame_new("Vector presets");
	Interface.frKeyframeAnimation = gtk_frame_new("Keyframe animation");
	Interface.frKeyframeAnimation2 = gtk_frame_new("Key-frames");
	Interface.frPalette = gtk_frame_new("Colour palette");
	Interface.frImageSaving = gtk_frame_new("Image saving");
	Interface.frSound = gtk_frame_new("Animation by sound");
	Interface.frHybrid = gtk_frame_new("Hybrid formula");
	Interface.frStereo = gtk_frame_new("Stereoscopic rendering");
	Interface.frMandelboxMainParams = gtk_frame_new("Main Mandelbox parameters");
	Interface.frMandelboxRotations = gtk_frame_new("Rotation of Mandelbox folding planes");
	Interface.frMandelboxColoring = gtk_frame_new("Mandelbox colouring parameters");
	Interface.frVolumetricLight = gtk_frame_new("Volumetric light");
	Interface.frMandelboxVary = gtk_frame_new("Mandelbox vary scale 4D");

	//separators
	Interface.hSeparator1 = gtk_hseparator_new();
	Interface.vSeparator1 = gtk_vseparator_new();

	//buttons
	Interface.buRender = gtk_button_new_with_label("RENDER");
	Interface.buStop = gtk_button_new_with_label("STOP");
	Interface.buApplyBrighness = gtk_button_new_with_label("Apply changes");
	Interface.buSaveImage = gtk_button_new_with_label("Save JPG");
	Interface.buSavePNG = gtk_button_new_with_label("Save PNG");
	Interface.buSavePNG16 = gtk_button_new_with_label("Save PNG 16-bit");
	Interface.buSavePNG16Alpha = gtk_button_new_with_label("Save PNG 16-bit + Alpha");
	Interface.buFiles = gtk_button_new_with_label("Select file paths (output images, textures)");
	Interface.buColorGlow1 = gtk_color_button_new();
	Interface.buColorGlow2 = gtk_color_button_new();
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorGlow1), "Glow colour 1");
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorGlow2), "Glow colour 2");
	Interface.buColorBackgroud1 = gtk_color_button_new();
	Interface.buColorBackgroud2 = gtk_color_button_new();
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorBackgroud1), "Background colour 1");
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorBackgroud2), "Background colour 2");
	Interface.buLoadSettings = gtk_button_new_with_label("Load Settings");
	Interface.buSaveSettings = gtk_button_new_with_label("Save Settings");
	Interface.buUp = gtk_button_new();
	Interface.buDown = gtk_button_new();
	Interface.buLeft = gtk_button_new();
	Interface.buRight = gtk_button_new();
	Interface.buMoveUp = gtk_button_new();
	Interface.buMoveDown = gtk_button_new();
	Interface.buMoveLeft = gtk_button_new();
	Interface.buMoveRight = gtk_button_new();
	Interface.buForward = gtk_button_new_with_label("Forward");
	Interface.buBackward = gtk_button_new_with_label("backward");
	Interface.buInitNavigator = gtk_button_new_with_label("Reset view");
	Interface.buAnimationRecordTrack = gtk_button_new_with_label("Record path");
	Interface.buAnimationContinueRecord = gtk_button_new_with_label("Continue recording");
	Interface.buAnimationRenderTrack = gtk_button_new_with_label("Render animation");
	Interface.buColorFog = gtk_color_button_new();
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorFog), "Fog colour");
	Interface.buColorSSAO = gtk_color_button_new();
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorSSAO), "Screen space ambient occlusion color");
	Interface.buUpdateSSAO = gtk_button_new_with_label("Update image");
	Interface.buUpdateDOF = gtk_button_new_with_label("Update DOF");
	Interface.buColorAuxLightPre1 = gtk_color_button_new();
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre1), "Colour of Auxiliary light #1");
	Interface.buColorAuxLightPre2 = gtk_color_button_new();
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre2), "Colour of Auxiliary light #2");
	Interface.buColorAuxLightPre3 = gtk_color_button_new();
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre3), "Colour of Auxiliary light #3");
	Interface.buColorAuxLightPre4 = gtk_color_button_new();
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre4), "Colour of Auxiliary light #4");
	Interface.buColorMainLight = gtk_color_button_new();
	gtk_color_button_set_title(GTK_COLOR_BUTTON(Interface.buColorMainLight), "Main light source colour");
	Interface.buDistributeLights = gtk_button_new_with_label("Distribute / update lights");
	Interface.buIFSNormalizeOffset = gtk_button_new_with_label("Normalize offset vector");
	Interface.buIFSNormalizeVectors = gtk_button_new_with_label("Normalize symmetry vectors");
	Interface.buAnimationRecordKey = gtk_button_new_with_label("Record key-frame");
	Interface.buAnimationRenderFromKeys = gtk_button_new_with_label("Render from key-frames");
	Interface.buUndo = gtk_button_new_with_label("Undo");
	Interface.buRedo = gtk_button_new_with_label("Redo");
	Interface.buBuddhabrot = gtk_button_new_with_label("Render Buddhabrot");
	Interface.buRandomPalette = gtk_button_new_with_label("Random palette");
	Interface.buLoadSound = gtk_button_new_with_label("Load sound");
	Interface.buGetPaletteFromImage = gtk_button_new_with_label("Get palette from image");
	Interface.buTimeline = gtk_button_new_with_label("Timeline");
	Interface.buIFSDefaultDodeca = gtk_button_new_with_label("Dodecahedron");
	Interface.buIFSDefaultIcosa = gtk_button_new_with_label("Icosahedron");
	Interface.buIFSDefaultOcta = gtk_button_new_with_label("Octahedron");
	Interface.buAutoDEStep = gtk_button_new_with_label("LQ");
	Interface.buAutoDEStepHQ = gtk_button_new_with_label("HQ");
	Interface.buCopyToClipboard = gtk_button_new_with_label("Copy to clipboard");
	Interface.buGetFromClipboard = gtk_button_new_with_label("Paste from clipboard");

	//edit
	Interface.edit_va = gtk_entry_new();
	Interface.edit_vb = gtk_entry_new();
	Interface.edit_vc = gtk_entry_new();
	Interface.edit_julia_a = gtk_entry_new();
	Interface.edit_julia_b = gtk_entry_new();
	Interface.edit_julia_c = gtk_entry_new();
	Interface.edit_amin = gtk_entry_new();
	Interface.edit_amax = gtk_entry_new();
	Interface.edit_bmin = gtk_entry_new();
	Interface.edit_bmax = gtk_entry_new();
	Interface.edit_cmin = gtk_entry_new();
	Interface.edit_cmax = gtk_entry_new();
	Interface.edit_alfa = gtk_entry_new();
	Interface.edit_beta = gtk_entry_new();
	Interface.edit_gammaAngle = gtk_entry_new();
	Interface.edit_zoom = gtk_entry_new();
	Interface.edit_persp = gtk_entry_new();
	Interface.edit_maxN = gtk_entry_new();
	Interface.edit_minN = gtk_entry_new();
	Interface.edit_power = gtk_entry_new();
	Interface.edit_FoldingIntPowFoldingFactor = gtk_entry_new();
	Interface.edit_FoldingIntPowZFactor = gtk_entry_new();
	Interface.edit_DE_thresh = gtk_entry_new();
	Interface.edit_DE_stepFactor = gtk_entry_new();
	Interface.edit_roughness = gtk_entry_new();
	Interface.edit_imageWidth = gtk_entry_new();
	Interface.edit_imageHeight = gtk_entry_new();
	Interface.edit_ambient = gtk_entry_new();
	Interface.edit_ambient_occlusion = gtk_entry_new();
	Interface.edit_brightness = gtk_entry_new();
	Interface.edit_gamma = gtk_entry_new();
	Interface.edit_glow = gtk_entry_new();
	Interface.edit_reflect = gtk_entry_new();
	Interface.edit_shading = gtk_entry_new();
	Interface.edit_shadows = gtk_entry_new();
	Interface.edit_specular = gtk_entry_new();
	Interface.edit_AmbientOcclusionQuality = gtk_entry_new();
	Interface.edit_step_forward = gtk_entry_new();
	Interface.edit_step_rotation = gtk_entry_new();
	Interface.edit_mouse_click_distance = gtk_entry_new();
	Interface.edit_animationDESpeed = gtk_entry_new();
	Interface.edit_color_seed = gtk_entry_new();
	Interface.edit_color_speed = gtk_entry_new();
	Interface.edit_tglad_folding_1 = gtk_entry_new();
	Interface.edit_tglad_folding_2 = gtk_entry_new();
	Interface.edit_spherical_folding_1 = gtk_entry_new();
	Interface.edit_spherical_folding_2 = gtk_entry_new();
	Interface.edit_mainLightIntensity = gtk_entry_new();
	Interface.edit_auxLightIntensity = gtk_entry_new();
	Interface.edit_auxLightNumber = gtk_entry_new();
	Interface.edit_auxLightMaxDist = gtk_entry_new();
	Interface.edit_auxLightRandomSeed = gtk_entry_new();
	Interface.edit_auxLightDistributionRadius = gtk_entry_new();
	Interface.edit_auxLightPre1x = gtk_entry_new();
	Interface.edit_auxLightPre1y = gtk_entry_new();
	Interface.edit_auxLightPre1z = gtk_entry_new();
	Interface.edit_auxLightPre1intensity = gtk_entry_new();
	Interface.edit_auxLightPre2x = gtk_entry_new();
	Interface.edit_auxLightPre2y = gtk_entry_new();
	Interface.edit_auxLightPre2z = gtk_entry_new();
	Interface.edit_auxLightPre2intensity = gtk_entry_new();
	Interface.edit_auxLightPre3x = gtk_entry_new();
	Interface.edit_auxLightPre3y = gtk_entry_new();
	Interface.edit_auxLightPre3z = gtk_entry_new();
	Interface.edit_auxLightPre3intensity = gtk_entry_new();
	Interface.edit_auxLightPre4x = gtk_entry_new();
	Interface.edit_auxLightPre4y = gtk_entry_new();
	Interface.edit_auxLightPre4z = gtk_entry_new();
	Interface.edit_auxLightPre4intensity = gtk_entry_new();
	Interface.edit_mainLightAlfa = gtk_entry_new();
	Interface.edit_mainLightBeta = gtk_entry_new();
	Interface.edit_auxLightVisibility = gtk_entry_new();
	Interface.edit_auxLightPlacementDistance = gtk_entry_new();
	Interface.edit_auxLightRandomCentreX = gtk_entry_new();
	Interface.edit_auxLightRandomCentreY = gtk_entry_new();
	Interface.edit_auxLightRandomCentreZ = gtk_entry_new();
	Interface.edit_IFSScale = gtk_entry_new();
	Interface.edit_IFSAlfa = gtk_entry_new();
	Interface.edit_IFSBeta = gtk_entry_new();
	Interface.edit_IFSGamma = gtk_entry_new();
	Interface.edit_IFSOffsetX = gtk_entry_new();
	Interface.edit_IFSOffsetY = gtk_entry_new();
	Interface.edit_IFSOffsetZ = gtk_entry_new();
	Interface.edit_animationFramesPerKey = gtk_entry_new();
	Interface.edit_animationStartFrame = gtk_entry_new();
	Interface.edit_animationEndFrame = gtk_entry_new();
	Interface.edit_sound1FreqMin = gtk_entry_new();
	Interface.edit_sound1FreqMax = gtk_entry_new();
	Interface.edit_sound2FreqMin = gtk_entry_new();
	Interface.edit_sound2FreqMax = gtk_entry_new();
	Interface.edit_sound3FreqMin = gtk_entry_new();
	Interface.edit_sound3FreqMax = gtk_entry_new();
	Interface.edit_sound4FreqMin = gtk_entry_new();
	Interface.edit_sound4FreqMax = gtk_entry_new();
	Interface.edit_soundFPS = gtk_entry_new();

	for (int i = 0; i < HYBRID_COUNT; ++i) {
		Interface.edit_hybridIter[i] = gtk_entry_new();
		Interface.edit_hybridPower[i] = gtk_entry_new();
	}

	Interface.edit_NavigatorAbsoluteDistance = gtk_entry_new();
	Interface.edit_stereoDistance = gtk_entry_new();
	Interface.edit_mandelboxScale = gtk_entry_new();
	Interface.edit_mandelboxFoldingLimit = gtk_entry_new();
	Interface.edit_mandelboxFoldingValue = gtk_entry_new();
	Interface.edit_mandelboxSpFoldingFixedRadius = gtk_entry_new();
	Interface.edit_mandelboxSpFoldingMinRadius = gtk_entry_new();
	Interface.edit_mandelboxSharpness = gtk_entry_new();
	Interface.edit_mandelboxOffsetX= gtk_entry_new();
	Interface.edit_mandelboxOffsetY= gtk_entry_new();
	Interface.edit_mandelboxOffsetZ= gtk_entry_new();

	for (int component = 0; component < 3; ++component)
		Interface.edit_mandelboxRotationMain[component] = gtk_entry_new();

	for (int fold = 0; fold < MANDELBOX_FOLDS; ++fold)
		for (int axis = 0; axis < 3; ++axis)
			for (int component = 0; component < 3; ++component) {
				Interface.edit_mandelboxRotation[fold][axis][component] = gtk_entry_new();
				gtk_entry_set_width_chars(GTK_ENTRY(Interface.edit_mandelboxRotation[fold][axis][component]), 5);
			}

	Interface.edit_mandelboxColorFactorR = gtk_entry_new();
	Interface.edit_mandelboxColorFactorSp1 = gtk_entry_new();
	Interface.edit_mandelboxColorFactorSp2 = gtk_entry_new();
	Interface.edit_mandelboxColorFactorX = gtk_entry_new();
	Interface.edit_mandelboxColorFactorY = gtk_entry_new();
	Interface.edit_mandelboxColorFactorZ = gtk_entry_new();
	Interface.edit_viewMinDistance = gtk_entry_new();
	Interface.edit_viewMaxDistance = gtk_entry_new();
	Interface.edit_FractalConstantFactor = gtk_entry_new();

	Interface.edit_volumetricLightQuality = gtk_entry_new();
	Interface.edit_volumetricLightIntensity = gtk_entry_new();
	Interface.edit_volumetricLightMainIntensity = gtk_entry_new();
	Interface.edit_volumetricLightAux1Intensity = gtk_entry_new();
	Interface.edit_volumetricLightAux2Intensity = gtk_entry_new();
	Interface.edit_volumetricLightAux3Intensity = gtk_entry_new();
	Interface.edit_volumetricLightAux4Intensity = gtk_entry_new();

	Interface.edit_reflectionsMax = gtk_entry_new();

	Interface.edit_mandelboxVaryFold = gtk_entry_new();
	Interface.edit_mandelboxVaryMinR = gtk_entry_new();
	Interface.edit_mandelboxVaryRPower = gtk_entry_new();
	Interface.edit_mandelboxVaryScale = gtk_entry_new();
	Interface.edit_mandelboxVaryWAdd = gtk_entry_new();

	Interface.edit_cadd = gtk_entry_new();

	//combo
	//		fract type
	Interface.comboFractType = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Mandelbulb");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Mandelbulb - Daniel White's");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Polynomic power 2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Polynomic power 2 - minus z");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Xenodreambuie's formula");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Hypercomplex");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Quaternion");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Menger sponge");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Tglad's formula (Mandelbox)");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Kaleidoscopic IFS");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Modified Mandelbulb 1");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Modified Mandelbulb 2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Modified Mandelbulb 3");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "FoldingIntPower2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Smooth Mandelbox");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Mandelbox vary scale 4D");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Aexion");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Benesi");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboFractType), "Hybrid");
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboFractType), 1);

	//image format
	Interface.comboImageFormat = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboImageFormat), "JPEG");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboImageFormat), "PNG 8-bit");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboImageFormat), "PNG 16-bit");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboImageFormat), "PNG 16-bit with alpha channel");
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboImageFormat), 0);

	for (int i = 0; i < HYBRID_COUNT; ++i) {
		Interface.comboHybridFormula[i] = gtk_combo_box_new_text();
		AddComboTextsFractalFormula(GTK_COMBO_BOX(Interface.comboHybridFormula[i]));
	}

	Interface.comboPerspectiveType = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboPerspectiveType), "Three-point perspective");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboPerspectiveType), "Fish eye");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboPerspectiveType), "Equirectangular projection");
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboPerspectiveType), 0);

	Interface.comboImageProportion = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboImageProportion), "Free");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboImageProportion), "1:1");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboImageProportion), "5:4");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboImageProportion), "4:3");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboImageProportion), "16:10");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Interface.comboImageProportion), "16:9");
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboImageProportion), 0);


	//progress bar
	Interface.progressBar = gtk_progress_bar_new();

	//checkboxes
	Interface.checkShadow = gtk_check_button_new_with_label("Shadows");
	Interface.checkAmbientOcclusion = gtk_check_button_new_with_label("Ambient occlusion");
	Interface.checkFastAmbientOcclusion = gtk_check_button_new_with_label("Ambient occlusion fast mode");
	Interface.checkIterThresh = gtk_check_button_new_with_label("Maxiter threshold mode");
	Interface.checkJulia = gtk_check_button_new_with_label("Julia mode");
	Interface.checkSlowShading = gtk_check_button_new_with_label("Not DE Shading mode (slow)");
	Interface.checkLimits = gtk_check_button_new_with_label("Enable limits");
	Interface.checkBitmapBackground = gtk_check_button_new_with_label("Textured background");
	Interface.checkColoring = gtk_check_button_new_with_label("Coloured surface");
	Interface.checkTgladMode = gtk_check_button_new_with_label("Tglad's folding mode      ");
	Interface.checkSphericalFoldingMode = gtk_check_button_new_with_label("Spherical folding mode      ");
	Interface.checkIFSFoldingMode = gtk_check_button_new_with_label("Kaleidoscopic IFS folding mode (parameters on Kaleidoscopic IFS tab)");
	Interface.checkFogEnabled = gtk_check_button_new_with_label("Enable fog         ");
	Interface.checkSSAOEnabled = gtk_check_button_new_with_label("Screen space ambient occlusion enable       ");
	Interface.checkDOFEnabled = gtk_check_button_new_with_label("Depth of field enable       ");
	Interface.checkZoomClickEnable = gtk_check_button_new_with_label("Enable zoom by mouse click");
	Interface.checkAuxLightPre1Enabled = gtk_check_button_new_with_label("Enable");
	Interface.checkAuxLightPre2Enabled = gtk_check_button_new_with_label("Enable");
	Interface.checkAuxLightPre3Enabled = gtk_check_button_new_with_label("Enable");
	Interface.checkAuxLightPre4Enabled = gtk_check_button_new_with_label("Enable");
	Interface.checkIFSAbsX = gtk_check_button_new_with_label("abs(x)");
	Interface.checkIFSAbsY = gtk_check_button_new_with_label("abs(y)");
	Interface.checkIFSAbsZ = gtk_check_button_new_with_label("abs(z)");
	Interface.checkAutoSaveImage = gtk_check_button_new_with_label("Auto-save");
	Interface.checkSoundEnabled = gtk_check_button_new_with_label("Enable animation by sound");
	Interface.checkHybridCyclic = gtk_check_button_new_with_label("Cyclic loop");
	Interface.checkNavigatorAbsoluteDistance = gtk_check_button_new_with_label("Absolute distance mode");
	Interface.checkNavigatorGoToSurface = gtk_check_button_new_with_label("Go close to indicated surface");
	Interface.checkStraightRotation = gtk_check_button_new_with_label("Rotation without\nusing gamma\nangle");
	Interface.checkStereoEnabled = gtk_check_button_new_with_label("Enable stereoscopic rendering");
	Interface.checkMandelboxRotationsEnable = gtk_check_button_new_with_label("Enable rotation of each folding plane");
	Interface.checkInteriorMode = gtk_check_button_new_with_label("Interior mode");
	Interface.checkDECorrectionMode = gtk_check_button_new_with_label("Dynamic DE correction");
	Interface.checkDELinearMode = gtk_check_button_new_with_label("Linear DE mode");
	Interface.checkConstantDEThreshold = gtk_check_button_new_with_label("Const. DE threshold");
	Interface.checkVolumetricLightMainEnabled = gtk_check_button_new_with_label("Enable");
	Interface.checkVolumetricLightAux1Enabled = gtk_check_button_new_with_label("");
	Interface.checkVolumetricLightAux2Enabled = gtk_check_button_new_with_label("");
	Interface.checkVolumetricLightAux3Enabled = gtk_check_button_new_with_label("");
	Interface.checkVolumetricLightAux4Enabled = gtk_check_button_new_with_label("");
	Interface.checkPenetratingLights = gtk_check_button_new_with_label("Penetrating lights");
	Interface.checkRaytracedReflections = gtk_check_button_new_with_label("Ray-traced reflections");

	//pixamps
	Interface.pixmap_up = gtk_image_new_from_file("icons/go-up.png");
	Interface.pixmap_down = gtk_image_new_from_file("icons/go-down.png");
	Interface.pixmap_left = gtk_image_new_from_file("icons/go-previous.png");
	Interface.pixmap_right = gtk_image_new_from_file("icons/go-next.png");
	Interface.pixmap_move_up = gtk_image_new_from_file("icons/go-up.png");
	Interface.pixmap_move_down = gtk_image_new_from_file("icons/go-down.png");
	Interface.pixmap_move_left = gtk_image_new_from_file("icons/go-previous.png");
	Interface.pixmap_move_right = gtk_image_new_from_file("icons/go-next.png");

	//labels
	Interface.label_animationFrame = gtk_label_new("Frame:");
	Interface.label_animationDistance = gtk_label_new("Estimated distance to fractal:");
	Interface.label_animationSpeed = gtk_label_new("Flight speed:");
	Interface.label_keyframeInfo = gtk_label_new("Frame: ,keyframe: ");
	Interface.label_fog_visibility = gtk_label_new("Visibility:");
	Interface.label_fog_visibility_front = gtk_label_new("Front:");
	Interface.label_SSAO_quality = gtk_label_new("Quality:");
	Interface.label_DOF_focus = gtk_label_new("Focus:");
	Interface.label_DOF_radius = gtk_label_new("Radius:");
	Interface.label_auxLightPre1 = gtk_label_new("Light #1:");
	Interface.label_auxLightPre2 = gtk_label_new("Light #2:");
	Interface.label_auxLightPre3 = gtk_label_new("Light #3:");
	Interface.label_auxLightPre4 = gtk_label_new("Light #4:");
	Interface.label_IFSx = gtk_label_new("symmetry x");
	Interface.label_IFSy = gtk_label_new("symmetry y");
	Interface.label_IFSz = gtk_label_new("symmetry z");
	Interface.label_IFSalfa = gtk_label_new("alpha");
	Interface.label_IFSbeta = gtk_label_new("beta");
	Interface.label_IFSgamma = gtk_label_new("gamma");
	Interface.label_IFSdistance = gtk_label_new("distance");
	Interface.label_IFSintensity = gtk_label_new("intensity");
	Interface.label_IFSenabled = gtk_label_new("enabled");
	Interface.label_paletteOffset = gtk_label_new("offset:");
	Interface.label_soundEnvelope = gtk_label_new("sound envelope");
	Interface.label_DE_threshold = gtk_label_new("Detail level:");
	gtk_misc_set_alignment(GTK_MISC(Interface.label_soundEnvelope), 0, 0);

	for (int i = 1; i <= HYBRID_COUNT; ++i)
		Interface.label_HybridFormula[i-1] = gtk_label_new(g_strdup_printf("Formula #%d:", i));

	Interface.label_NavigatorEstimatedDistance = gtk_label_new("Estimated distance to the surface:");

	Interface.label_about = gtk_label_new("Mandelbulber "MANDELBULBER_VERSION_STR"\n"
		"author: Krzysztof Marczak\n"
		"Licence: GNU GPL v3\n"
		"www: http://sites.google.com/site/mandelbulber/home\n"
		"thanks to: Knighty, Makemeunsee, Marius Schilder, Ryan Hitchman, Jeff Epler, Martin Reinecke"
	);

	//sliders
	Interface.adjustmentFogDepth = gtk_adjustment_new(30.0, 0.1, 200.0, 0.1, 10.0, 0.1);
	Interface.sliderFogDepth = gtk_hscale_new(GTK_ADJUSTMENT(Interface.adjustmentFogDepth));
	Interface.adjustmentFogDepthFront = gtk_adjustment_new(20.0, 0.1, 200.0, 0.1, 10.0, 0.1);
	Interface.sliderFogDepthFront = gtk_hscale_new(GTK_ADJUSTMENT(Interface.adjustmentFogDepthFront));
	Interface.adjustmentSSAOQuality = gtk_adjustment_new(20, 1, 100, 1, 10, 1);
	Interface.sliderSSAOQuality = gtk_hscale_new(GTK_ADJUSTMENT(Interface.adjustmentSSAOQuality));
	Interface.adjustmentDOFFocus = gtk_adjustment_new(20.0, 0.1, 200.0, 0.1, 10.0, 0.1);
	Interface.sliderDOFFocus = gtk_hscale_new(GTK_ADJUSTMENT(Interface.adjustmentDOFFocus));
	Interface.adjustmentDOFRadius = gtk_adjustment_new(10.0, 0.1, 100.0, 0.1, 10.0, 0.1);
	Interface.sliderDOFRadius = gtk_hscale_new(GTK_ADJUSTMENT(Interface.adjustmentDOFRadius));
	Interface.adjustmentPaletteOffset = gtk_adjustment_new(0, 0, 256.0, 0.1, 10.0, 0.1);
	Interface.sliderPaletteOffset = gtk_hscale_new(GTK_ADJUSTMENT(Interface.adjustmentPaletteOffset));

	//colour palette
	dareaPalette = gtk_drawing_area_new();
	gtk_widget_set_size_request(dareaPalette, 640, 30);

	//sound
	Interface.dareaSound0 = gtk_drawing_area_new();
	gtk_widget_set_size_request(Interface.dareaSound0, 640, 40);
	Interface.dareaSoundA = gtk_drawing_area_new();
	gtk_widget_set_size_request(Interface.dareaSoundA, 640, 40);
	Interface.dareaSoundB = gtk_drawing_area_new();
	gtk_widget_set_size_request(Interface.dareaSoundB, 640, 40);
	Interface.dareaSoundC = gtk_drawing_area_new();
	gtk_widget_set_size_request(Interface.dareaSoundC, 640, 40);
	Interface.dareaSoundD = gtk_drawing_area_new();
	gtk_widget_set_size_request(Interface.dareaSoundD, 640, 40);

	//connected signals
	CONNECT_SIGNAL_CLICKED(Interface.buRender, StartRendering);
	CONNECT_SIGNAL_CLICKED(Interface.buRender, StartRendering);
	CONNECT_SIGNAL_CLICKED(Interface.buStop, StopRendering);
	CONNECT_SIGNAL_CLICKED(Interface.buApplyBrighness, PressedApplyBrigtness);
	CONNECT_SIGNAL_CLICKED(Interface.buSaveImage, PressedSaveImage);
	CONNECT_SIGNAL_CLICKED(Interface.buSavePNG, PressedSaveImagePNG);
	CONNECT_SIGNAL_CLICKED(Interface.buSavePNG16, PressedSaveImagePNG16);
	CONNECT_SIGNAL_CLICKED(Interface.buSavePNG16Alpha, PressedSaveImagePNG16Alpha);
	CONNECT_SIGNAL_CLICKED(Interface.buLoadSettings, PressedLoadSettings);
	CONNECT_SIGNAL_CLICKED(Interface.buSaveSettings, PressedSaveSettings);
	CONNECT_SIGNAL_CLICKED(Interface.buFiles, CreateFilesDialog);
	CONNECT_SIGNAL_CLICKED(Interface.buUp, PressedNavigatorUp);
	CONNECT_SIGNAL_CLICKED(Interface.buDown, PressedNavigatorDown);
	CONNECT_SIGNAL_CLICKED(Interface.buLeft, PressedNavigatorLeft);
	CONNECT_SIGNAL_CLICKED(Interface.buRight, PressedNavigatorRight);
	CONNECT_SIGNAL_CLICKED(Interface.buMoveUp, PressedNavigatorMoveUp);
	CONNECT_SIGNAL_CLICKED(Interface.buMoveDown, PressedNavigatorMoveDown);
	CONNECT_SIGNAL_CLICKED(Interface.buMoveLeft, PressedNavigatorMoveLeft);
	CONNECT_SIGNAL_CLICKED(Interface.buMoveRight, PressedNavigatorMoveRight);
	CONNECT_SIGNAL_CLICKED(Interface.buForward, PressedNavigatorForward);
	CONNECT_SIGNAL_CLICKED(Interface.buBackward, PressedNavigatorBackward);
	CONNECT_SIGNAL_CLICKED(Interface.buInitNavigator, PressedNavigatorInit);
	CONNECT_SIGNAL_CLICKED(Interface.buAnimationRecordTrack, PressedAnimationRecord);
	CONNECT_SIGNAL_CLICKED(Interface.buAnimationContinueRecord, PressedAnimationContinueRecording);
	CONNECT_SIGNAL_CLICKED(Interface.buAnimationRenderTrack, PressedAnimationRender);
	CONNECT_SIGNAL(Interface.adjustmentFogDepth, ChangedSliderFog, "value-changed");
	CONNECT_SIGNAL(Interface.adjustmentFogDepthFront, ChangedSliderFog, "value-changed");
	CONNECT_SIGNAL_CLICKED(Interface.checkFogEnabled, ChangedSliderFog);
	CONNECT_SIGNAL_CLICKED(Interface.checkSSAOEnabled, PressedSSAOUpdate);
	CONNECT_SIGNAL_CLICKED(Interface.buUpdateSSAO, PressedSSAOUpdate);
	CONNECT_SIGNAL_CLICKED(Interface.buUpdateDOF, PressedDOFUpdate);
	CONNECT_SIGNAL_CLICKED(Interface.buDistributeLights, PressedDistributeLights);
	CONNECT_SIGNAL_CLICKED(Interface.buIFSNormalizeOffset, PressedIFSNormalizeOffset);
	CONNECT_SIGNAL_CLICKED(Interface.buIFSNormalizeVectors, PressedIFSNormalizeVectors);
	CONNECT_SIGNAL_CLICKED(Interface.buAnimationRecordKey, PressedRecordKeyframe);
	CONNECT_SIGNAL_CLICKED(Interface.buAnimationRenderFromKeys, PressedKeyframeAnimationRender);
	CONNECT_SIGNAL_CLICKED(Interface.buUndo, PressedUndo);
	CONNECT_SIGNAL_CLICKED(Interface.buRedo, PressedRedo);
	CONNECT_SIGNAL_CLICKED(Interface.buBuddhabrot, PressedBuddhabrot);
	CONNECT_SIGNAL(Interface.adjustmentPaletteOffset, ChangedSliderPaletteOffset, "value-changed");
	CONNECT_SIGNAL_CLICKED(Interface.buRandomPalette, PressedRandomPalette);
	//CONNECT_SIGNAL_CLICKED(Interface.buLoadSound, PressedLoadSound);
	CONNECT_SIGNAL_CLICKED(Interface.buGetPaletteFromImage, PressedGetPaletteFromImage);
	CONNECT_SIGNAL_CLICKED(Interface.buTimeline, PressedTimeline);
	CONNECT_SIGNAL_CLICKED(Interface.buIFSDefaultDodeca, PressedIFSDefaultDodeca);
	CONNECT_SIGNAL_CLICKED(Interface.buIFSDefaultIcosa, PressedIFSDefaultIcosa);
	CONNECT_SIGNAL_CLICKED(Interface.buIFSDefaultOcta, PressedIFSDefaultOcta);

	CONNECT_SIGNAL(renderWindow.comboImageScale, ChangedComboScale, "changed");
	CONNECT_SIGNAL(Interface.comboFractType, ChangedComboFormula, "changed");
	CONNECT_SIGNAL_CLICKED(Interface.checkTgladMode, ChangedTgladFoldingMode);
	CONNECT_SIGNAL_CLICKED(Interface.checkJulia, ChangedJulia);
	CONNECT_SIGNAL_CLICKED(Interface.checkSphericalFoldingMode, ChangedSphericalFoldingMode);
	CONNECT_SIGNAL_CLICKED(Interface.checkIFSFoldingMode, ChangedIFSFoldingMode);
	CONNECT_SIGNAL_CLICKED(Interface.checkLimits, ChangedLimits);
	CONNECT_SIGNAL_CLICKED(Interface.checkMandelboxRotationsEnable, ChangedMandelboxRotations);
	CONNECT_SIGNAL_CLICKED(Interface.buAutoDEStep, PressedAutoDEStep);
	CONNECT_SIGNAL_CLICKED(Interface.buAutoDEStepHQ, PressedAutoDEStepHQ);
	CONNECT_SIGNAL_CLICKED(Interface.checkConstantDEThreshold, ChangedConstantDEThreshold);
	CONNECT_SIGNAL(Interface.comboImageProportion, ChangedImageProportion, "changed");
	CONNECT_SIGNAL(Interface.edit_imageHeight, ChangedImageProportion, "activate");
	CONNECT_SIGNAL_CLICKED(Interface.buCopyToClipboard, PressedCopyToClipboard);
	CONNECT_SIGNAL_CLICKED(Interface.buGetFromClipboard, PressedPasteFromClipboard);

	gtk_signal_connect(GTK_OBJECT(dareaPalette), "expose-event", GTK_SIGNAL_FUNC(on_dareaPalette_expose), NULL);
	//gtk_signal_connect(GTK_OBJECT(Interface.dareaSound0), "expose-event", GTK_SIGNAL_FUNC(on_dareaSound_expose), (void*) "0");
	//gtk_signal_connect(GTK_OBJECT(Interface.dareaSoundA), "expose-event", GTK_SIGNAL_FUNC(on_dareaSound_expose), (void*) "A");
	//gtk_signal_connect(GTK_OBJECT(Interface.dareaSoundB), "expose-event", GTK_SIGNAL_FUNC(on_dareaSound_expose), (void*) "B");
	//gtk_signal_connect(GTK_OBJECT(Interface.dareaSoundC), "expose-event", GTK_SIGNAL_FUNC(on_dareaSound_expose), (void*) "C");
	//gtk_signal_connect(GTK_OBJECT(Interface.dareaSoundD), "expose-event", GTK_SIGNAL_FUNC(on_dareaSound_expose), (void*) "D");

	//----------------------- main box -----------------------

	//	box buttons
	gtk_box_pack_start(GTK_BOX(Interface.boxMain), Interface.boxButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxButtons), Interface.buRender, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxButtons), Interface.buStop, true, true, 1);

	//	frame view point
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_view), Interface.frCoordinates, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frCoordinates), Interface.boxView);

	//		box coordinates
	gtk_box_pack_start(GTK_BOX(Interface.boxView), Interface.boxCoordinates, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxCoordinates), CreateEdit("0,0", "x:", 20, Interface.edit_va), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxCoordinates), CreateEdit("0,0", "y:", 20, Interface.edit_vb), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxCoordinates), CreateEdit("0,0", "z:", 20, Interface.edit_vc), false, false, 1);

	//		box angle
	gtk_box_pack_start(GTK_BOX(Interface.boxView), Interface.boxAngle, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAngle), CreateEdit("0,0", "alpha (yaw):", 15, Interface.edit_alfa), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAngle), CreateEdit("0,0", "beta (pitch):", 15, Interface.edit_beta), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAngle), CreateEdit("0,0", "gamma (roll):", 15, Interface.edit_gammaAngle), false, false, 1);

	//		box zoom
	gtk_box_pack_start(GTK_BOX(Interface.boxView), Interface.boxZoom, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxZoom), CreateEdit("2,5", "Close up (zoom):", 20, Interface.edit_zoom), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxZoom), CreateEdit("0,5", "perspective (FOV):", 5, Interface.edit_persp), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxZoom), CreateWidgetWithLabel("Perspective projection:", Interface.comboPerspectiveType), false, false, 1);

	//buttons arrows
	gtk_container_add(GTK_CONTAINER(Interface.buUp), Interface.pixmap_up);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows), Interface.buUp, 1, 2, 0, 1);
	gtk_container_add(GTK_CONTAINER(Interface.buDown), Interface.pixmap_down);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows), Interface.buDown, 1, 2, 2, 3);
	gtk_container_add(GTK_CONTAINER(Interface.buLeft), Interface.pixmap_left);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows), Interface.buLeft, 0, 1, 1, 2);
	gtk_container_add(GTK_CONTAINER(Interface.buRight), Interface.pixmap_right);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows), Interface.buRight, 2, 3, 1, 2);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_view), Interface.fr3Dnavigator, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.fr3Dnavigator), Interface.boxArrows);
	gtk_box_pack_start(GTK_BOX(Interface.boxArrows), Interface.boxArrows2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxArrows2), Interface.tableArrows, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxArrows2), Interface.checkStraightRotation, false, false, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkStraightRotation), true);

	//navigation
	gtk_box_pack_start(GTK_BOX(Interface.boxArrows), Interface.boxNavigation, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), Interface.buInitNavigator, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), Interface.boxNavigationButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigationButtons), Interface.buForward, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigationButtons), Interface.buBackward, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), CreateEdit(DoubleToString(0.5), "Step for camera moving multiplied by DE:", 5, Interface.edit_step_forward), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), Interface.checkNavigatorAbsoluteDistance, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), CreateEdit(DoubleToString(0.1), "Absolute movement distance:", 10, Interface.edit_NavigatorAbsoluteDistance), false, false,
			1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), CreateEdit(DoubleToString(10.0), "Rotation step in degrees", 5, Interface.edit_step_rotation), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), CreateEdit(DoubleToString(5.0), "Mouse click close-up ratio", 5, Interface.edit_mouse_click_distance), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), Interface.boxNavigationZooming, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigationZooming), Interface.checkZoomClickEnable, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigationZooming), Interface.checkNavigatorGoToSurface, false, false, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkZoomClickEnable), true);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), Interface.label_NavigatorEstimatedDistance, false, false, 1);

	//buttons arrows 2
	gtk_container_add(GTK_CONTAINER(Interface.buMoveUp), Interface.pixmap_move_up);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows2), Interface.buMoveUp, 1, 2, 0, 1);
	gtk_container_add(GTK_CONTAINER(Interface.buMoveDown), Interface.pixmap_move_down);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows2), Interface.buMoveDown, 1, 2, 2, 3);
	gtk_container_add(GTK_CONTAINER(Interface.buMoveLeft), Interface.pixmap_move_left);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows2), Interface.buMoveLeft, 0, 1, 1, 2);
	gtk_container_add(GTK_CONTAINER(Interface.buMoveRight), Interface.pixmap_move_right);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows2), Interface.buMoveRight, 2, 3, 1, 2);

	gtk_box_pack_start(GTK_BOX(Interface.boxArrows), Interface.boxArrows3, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxArrows3), Interface.tableArrows2, false, false, 1);

	//	frame fractal
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_fractal), Interface.frFractal, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frFractal), Interface.boxFractal);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractal), Interface.frFractalFormula, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frFractalFormula), Interface.boxFractalFormula);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFormula), CreateWidgetWithLabel("Fractal formula type:", Interface.comboFractType), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFormula), Interface.boxJulia, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxJulia), CreateEdit("0,0", "Julia x:", 20, Interface.edit_julia_a), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxJulia), CreateEdit("0,0", "Julia y:", 20, Interface.edit_julia_b), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxJulia), CreateEdit("0,0", "Julia z:", 20, Interface.edit_julia_c), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxJulia), Interface.checkJulia, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFormula), Interface.boxFractalPower, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalPower), CreateEdit("8,0", "power:", 5, Interface.edit_power), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalPower), CreateEdit("1,0", "Fractal constant factor:", 5, Interface.edit_FractalConstantFactor), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalPower), CreateEdit("0,0", "c add:", 5, Interface.edit_cadd), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractal), Interface.frFractalFoldingIntPow, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frFractalFoldingIntPow), Interface.boxFractalFoldingIntPow);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFoldingIntPow), CreateEdit("2,0", "Cubic folding factor:", 5, Interface.edit_FoldingIntPowFoldingFactor), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFoldingIntPow), CreateEdit("10,0", "Z factor:", 5, Interface.edit_FoldingIntPowZFactor), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractal), Interface.frFractalFolding, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frFractalFolding), Interface.boxFractalFolding);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFolding), Interface.boxTgladFolding, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxTgladFolding), Interface.checkTgladMode, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxTgladFolding), CreateEdit("1,0", "Folding limit:", 5, Interface.edit_tglad_folding_1), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxTgladFolding), CreateEdit("2,0", "Folding value:", 5, Interface.edit_tglad_folding_2), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFolding), Interface.boxSphericalFolding, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSphericalFolding), Interface.checkSphericalFoldingMode, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSphericalFolding), CreateEdit("1,0", "Fixed radius:", 5, Interface.edit_spherical_folding_1), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSphericalFolding), CreateEdit("0,5", "Min. radius:", 5, Interface.edit_spherical_folding_2), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFolding), Interface.checkIFSFoldingMode, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_fractal), Interface.frFractalRayMarching, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frFractalRayMarching), Interface.boxFractalRayMarching);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalRayMarching), Interface.boxQuality, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), CreateEdit("250", "Max. iterations:", 5, Interface.edit_maxN), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), CreateEdit("1", "Min. iterations:", 5, Interface.edit_minN), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), Interface.label_DE_threshold, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), Interface.edit_DE_thresh, false, false, 1);
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_DE_thresh), "1,0");
	gtk_entry_set_width_chars(GTK_ENTRY(Interface.edit_DE_thresh), 5);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), CreateEdit("1,0", "DE step factor:", 5, Interface.edit_DE_stepFactor), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), Interface.buAutoDEStep, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), Interface.buAutoDEStepHQ, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), CreateEdit("1,0", "Smoothness:", 5, Interface.edit_roughness), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalRayMarching), Interface.boxFractalSwitches, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalSwitches), Interface.checkIterThresh, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalSwitches), Interface.checkInteriorMode, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalSwitches), Interface.checkDECorrectionMode, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalSwitches), Interface.checkDELinearMode, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalSwitches), Interface.checkConstantDEThreshold, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalRayMarching), Interface.boxViewDistance, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxViewDistance), CreateEdit("1e-15", "Minimum render distance:", 10, Interface.edit_viewMinDistance), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxViewDistance), CreateEdit("20", "Maximum render distance:", 10, Interface.edit_viewMaxDistance), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_fractal), Interface.frLimits, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frLimits), Interface.boxLimits);
	gtk_box_pack_start(GTK_BOX(Interface.boxLimits), Interface.tableLimits, false, false, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("-0,5", "x min:", 20, Interface.edit_amin), 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("-0,5", "y min:", 20, Interface.edit_bmin), 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("-0,5", "z min:", 20, Interface.edit_cmin), 2, 3, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("0,5", "x max:", 20, Interface.edit_amax), 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("0,5", "y max:", 20, Interface.edit_bmax), 1, 2, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("0,5", "z max:", 20, Interface.edit_cmax), 2, 3, 1, 2);
	gtk_box_pack_start(GTK_BOX(Interface.boxLimits), Interface.checkLimits, false, false, 1);

	//frame image
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_image), Interface.frImage, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frImage), Interface.boxImage);

	gtk_box_pack_start(GTK_BOX(Interface.boxImage), Interface.boxImageRes, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageRes), CreateEdit("800", "Image width:", 5, Interface.edit_imageWidth), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageRes), CreateEdit("600", "Image height:", 5, Interface.edit_imageHeight), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageRes),  CreateWidgetWithLabel("Image proportion:", Interface.comboImageProportion), false, false, 1);

	//frame Stereoscopic
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_image), Interface.frStereo, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frStereo), Interface.boxStereoscopic);

	gtk_box_pack_start(GTK_BOX(Interface.boxStereoscopic), Interface.boxStereoParams, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxStereoParams), CreateEdit("0,1", "Distance between eyes:", 20, Interface.edit_stereoDistance), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxStereoParams), Interface.checkStereoEnabled, false, false, 1);

	//frame Image saving
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_image), Interface.frImageSaving, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frImageSaving), Interface.boxImageSaving);

	gtk_box_pack_start(GTK_BOX(Interface.boxImageSaving), Interface.boxSaveImage, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSaveImage), Interface.buSaveImage, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSaveImage), Interface.buSavePNG, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSaveImage), Interface.buSavePNG16, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSaveImage), Interface.buSavePNG16Alpha, true, true, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxImageSaving), Interface.boxImageAutoSave, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageAutoSave), CreateWidgetWithLabel("Auto-save / animation image format:", Interface.comboImageFormat), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageAutoSave), Interface.checkAutoSaveImage, false, false, 1);

	//frame effects
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders), Interface.frEffects, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frEffects), Interface.boxEffects);

	gtk_box_pack_start(GTK_BOX(Interface.boxEffects), Interface.boxBrightness, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxBrightness), CreateEdit("1,0", "brightness:", 5, Interface.edit_brightness), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxBrightness), CreateEdit("1,0", "gamma:", 5, Interface.edit_gamma), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxEffects), Interface.boxShading, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShading), CreateEdit("0,5", "shading:", 5, Interface.edit_shading), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShading), CreateEdit("1,0", "direct light:", 5, Interface.edit_shadows), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShading), CreateEdit("1,0", "specularity:", 5, Interface.edit_specular), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShading), CreateEdit("0,0", "ambient:", 5, Interface.edit_ambient), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShading), CreateEdit("2,0", "ambient occlusion:", 5, Interface.edit_ambient_occlusion), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxEffects), Interface.boxShading2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShading2), CreateEdit("1,0", "glow:", 5, Interface.edit_glow), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShading2), CreateEdit("0,0", "reflection:", 5, Interface.edit_reflect), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShading2), Interface.checkRaytracedReflections, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShading2), CreateEdit("5", "reflections depth:", 5, Interface.edit_reflectionsMax), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShading2), CreateEdit("4", "ambient occlusion quality:", 5, Interface.edit_AmbientOcclusionQuality), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxEffects), Interface.boxEffectsChecks, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsChecks), Interface.checkShadow, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsChecks), Interface.checkAmbientOcclusion, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsChecks), Interface.checkFastAmbientOcclusion, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsChecks), Interface.checkSlowShading, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxEffects), Interface.boxEffectsChecks2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsChecks2), Interface.checkBitmapBackground, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders), Interface.frPalette, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPalette), Interface.boxPalette);

	gtk_box_pack_start(GTK_BOX(Interface.boxPalette), Interface.boxEffectsColoring, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), Interface.checkColoring, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), Interface.vSeparator1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), CreateEdit("123456", "Random seed:", 6, Interface.edit_color_seed), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), CreateEdit("1,0", "Colour speed:", 6, Interface.edit_color_speed), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), Interface.buRandomPalette, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), Interface.buGetPaletteFromImage, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxPalette), dareaPalette, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPalette), Interface.boxPaletteOffset, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPaletteOffset), Interface.label_paletteOffset, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPaletteOffset), Interface.sliderPaletteOffset, true, true, 1);

	//frame colors
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders), Interface.frColors, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frColors), Interface.boxColors);

	gtk_box_pack_start(GTK_BOX(Interface.boxColors), Interface.boxGlowColor, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("Glow colour 1:", Interface.buColorGlow1), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("Glow colour 2:", Interface.buColorGlow2), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("Background colour 1:", Interface.buColorBackgroud1), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("Background colour 2:", Interface.buColorBackgroud2), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders), Interface.buApplyBrighness, false, false, 1);

	//frame animation
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_animation), Interface.frAnimation, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frAnimation), Interface.boxAnimation);

	gtk_box_pack_start(GTK_BOX(Interface.boxAnimation), Interface.boxAnimationButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationButtons), Interface.buAnimationRecordTrack, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationButtons), Interface.buAnimationContinueRecord, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationButtons), Interface.buAnimationRenderTrack, true, true, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxAnimation), Interface.boxAnimationEdits, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationEdits), CreateEdit(DoubleToString(0.01), "Flight speed (DE multiplier):", 5, Interface.edit_animationDESpeed), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxAnimation), Interface.label_animationFrame, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimation), Interface.label_animationDistance, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimation), Interface.label_animationSpeed, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_animation), Interface.frKeyframeAnimation, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frKeyframeAnimation), Interface.boxKeyframeAnimation);

	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimation), Interface.boxKeyframeAnimationButtons, false, false, 1);
	//gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimationButtons), Interface.buAnimationRecordKey, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimationButtons), Interface.buAnimationRenderFromKeys, false, false, 1);

	//gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimation), Interface.boxKeyframeAnimationButtons2, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimation), Interface.boxKeyframeAnimationEdits, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimationEdits), CreateEdit("100", "Frames per key:", 5, Interface.edit_animationFramesPerKey), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimation), Interface.label_keyframeInfo, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_animation), Interface.frAnimationFrames, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frAnimationFrames), Interface.boxAnimationEdits2);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationEdits2), CreateEdit("0", "Start frame:", 5, Interface.edit_animationStartFrame), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationEdits2), CreateEdit("1000", "End frame:", 5, Interface.edit_animationEndFrame), false, false, 1);

	//---- tab pot effects
	//frame fog
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_posteffects), Interface.frPostFog, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPostFog), Interface.boxPostFog);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostFog), Interface.boxFogButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogButtons), Interface.checkFogEnabled, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogButtons), CreateWidgetWithLabel("Fog colour:", Interface.buColorFog), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostFog), Interface.boxFogSlider, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogSlider), Interface.label_fog_visibility, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogSlider), Interface.sliderFogDepth, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostFog), Interface.boxFogSlider2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogSlider2), Interface.label_fog_visibility_front, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogSlider2), Interface.sliderFogDepthFront, true, true, 1);


	//frame SSAO
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_posteffects), Interface.frPostSSAO, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPostSSAO), Interface.boxPostSSAO);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostSSAO), Interface.boxSSAOButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSSAOButtons), Interface.checkSSAOEnabled, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSSAOButtons), Interface.buUpdateSSAO, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostSSAO), Interface.boxSSAOSlider, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSSAOSlider), Interface.label_SSAO_quality, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSSAOSlider), Interface.sliderSSAOQuality, true, true, 1);

	//frame DOF
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_posteffects), Interface.frPostDOF, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPostDOF), Interface.boxPostDOF);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostDOF), Interface.boxDOFButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxDOFButtons), Interface.checkDOFEnabled, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxDOFButtons), Interface.buUpdateDOF, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostDOF), Interface.boxDOFSlider1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxDOFSlider1), Interface.label_DOF_focus, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxDOFSlider1), Interface.sliderDOFFocus, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostDOF), Interface.boxDOFSlider2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxDOFSlider2), Interface.label_DOF_radius, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxDOFSlider2), Interface.sliderDOFRadius, true, true, 1);

	//gtk_box_pack_start(GTK_BOX(Interface.tab_box_posteffects), Interface.buBuddhabrot, true, true, 1);

	//---- tab Lights
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frMainLight, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frMainLight), Interface.boxMainLight);
	gtk_box_pack_start(GTK_BOX(Interface.boxMainLight), Interface.boxMainLightPosition, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMainLightPosition), CreateEdit("-45", "Horizontal angle relative to camera:", 6, Interface.edit_mainLightAlfa), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMainLightPosition), CreateEdit("45", "Vertical angle relative to camera:", 6, Interface.edit_mainLightBeta), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMainLightPosition), CreateWidgetWithLabel("Colour:", Interface.buColorMainLight), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frLightsCommon, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frLightsCommon), Interface.boxLightCommon);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightCommon), Interface.buDistributeLights, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightCommon), CreateEdit("0", "Number of aux. lights:", 6, Interface.edit_auxLightNumber), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightCommon), CreateEdit(DoubleToString(0.01), "Manual placement distance:", 6, Interface.edit_auxLightPlacementDistance), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frLightBallance, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frLightBallance), Interface.boxLightBallance);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightBallance), Interface.boxLightBrightness, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightBrightness), CreateEdit("1,0", "Main light intensity:", 6, Interface.edit_mainLightIntensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightBrightness), CreateEdit("1,0", "Auxiliary lights intensity:", 6, Interface.edit_auxLightIntensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightBrightness), CreateEdit("1,0", "Lights visibility:", 6, Interface.edit_auxLightVisibility), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightBrightness), Interface.checkPenetratingLights, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frLightsParameters, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frLightsParameters), Interface.boxLightsParameters);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightsParameters), Interface.boxLightDistribution, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution), CreateEdit("1234", "Random seed:", 6, Interface.edit_auxLightRandomSeed), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution), CreateEdit("0,1", "Maximum distance from fractal", 12, Interface.edit_auxLightMaxDist), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightsParameters), Interface.boxLightDistribution2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution2), CreateEdit("3.0", "Distribution radius of lights:", 6, Interface.edit_auxLightDistributionRadius), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution2), CreateEdit("0", "Centre of distribution X:", 12, Interface.edit_auxLightRandomCentreX), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution2), CreateEdit("0", "Y:", 12, Interface.edit_auxLightRandomCentreY), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution2), CreateEdit("0", "Z:", 12, Interface.edit_auxLightRandomCentreZ), false, false, 1);

	//frame: predefined lights
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frPredefinedLights, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPredefinedLights), Interface.boxPredefinedLights);

	gtk_box_pack_start(GTK_BOX(Interface.boxPredefinedLights), Interface.boxLightPre1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), Interface.label_auxLightPre1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), CreateEdit("3,0", "x:", 12, Interface.edit_auxLightPre1x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), CreateEdit("-3,0", "y:", 12, Interface.edit_auxLightPre1y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), CreateEdit("-3,0", "z:", 12, Interface.edit_auxLightPre1z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), CreateEdit("1,0", "intensity:", 12, Interface.edit_auxLightPre1intensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), Interface.buColorAuxLightPre1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), Interface.checkAuxLightPre1Enabled, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxPredefinedLights), Interface.boxLightPre2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), Interface.label_auxLightPre2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), CreateEdit("-3,0", "x:", 12, Interface.edit_auxLightPre2x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), CreateEdit("-3,0", "y:", 12, Interface.edit_auxLightPre2y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), CreateEdit("0,0", "z:", 12, Interface.edit_auxLightPre2z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), CreateEdit("1,0", "intensity:", 12, Interface.edit_auxLightPre2intensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), Interface.buColorAuxLightPre2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), Interface.checkAuxLightPre2Enabled, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxPredefinedLights), Interface.boxLightPre3, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), Interface.label_auxLightPre3, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), CreateEdit("1,0", "x:", 12, Interface.edit_auxLightPre3x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), CreateEdit("3,0", "y:", 12, Interface.edit_auxLightPre3y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), CreateEdit("-1,0", "z:", 12, Interface.edit_auxLightPre3z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), CreateEdit("1,0", "intensity:", 12, Interface.edit_auxLightPre3intensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), Interface.buColorAuxLightPre3, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), Interface.checkAuxLightPre3Enabled, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxPredefinedLights), Interface.boxLightPre4, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), Interface.label_auxLightPre4, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), CreateEdit("1,0", "x:", 12, Interface.edit_auxLightPre4x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), CreateEdit("-1,0", "y:", 12, Interface.edit_auxLightPre4y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), CreateEdit("-3,0", "z:", 12, Interface.edit_auxLightPre4z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), CreateEdit("1,0", "intensity:", 12, Interface.edit_auxLightPre4intensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), Interface.buColorAuxLightPre4, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), Interface.checkAuxLightPre4Enabled, false, false, 1);

	//frame: volumetric lights
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frVolumetricLight, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frVolumetricLight), Interface.boxVolumetricLight);

	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLight), Interface.boxVolumetricLightGeneral, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightGeneral), CreateEdit("5,0", "Effect quality:", 12, Interface.edit_volumetricLightQuality), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightGeneral), CreateEdit("1,0", "Effect intensity (post tunning):", 12, Interface.edit_volumetricLightIntensity), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLight), Interface.boxVolumetricLightMain, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightMain), CreateEdit("1,0", "Intensity for main light:", 12, Interface.edit_volumetricLightMainIntensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightMain), Interface.checkVolumetricLightMainEnabled, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLight), Interface.boxVolumetricLightAux, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateEdit("1,0", "L #1:", 12, Interface.edit_volumetricLightAux1Intensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), Interface.checkVolumetricLightAux1Enabled, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateEdit("1,0", "L #2:", 12, Interface.edit_volumetricLightAux2Intensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), Interface.checkVolumetricLightAux2Enabled, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateEdit("1,0", "L #3:", 12, Interface.edit_volumetricLightAux3Intensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), Interface.checkVolumetricLightAux3Enabled, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateEdit("1,0", "L #4:", 12, Interface.edit_volumetricLightAux4Intensity), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), Interface.checkVolumetricLightAux4Enabled, false, false, 1);

	//tab IFS
	//frame: main IFS
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_IFS), Interface.frIFSMain, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frIFSMain), Interface.boxIFSMain);

	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMain), Interface.boxIFSMainEdit, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit), CreateEdit("2,0", "scale:", 6, Interface.edit_IFSScale), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit), CreateEdit("0", "rotation alpha:", 6, Interface.edit_IFSAlfa), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit), CreateEdit("0", "rotation beta:", 6, Interface.edit_IFSBeta), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit), CreateEdit("0", "rotation gamma:", 6, Interface.edit_IFSGamma), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMain), Interface.boxIFSMainEdit2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), CreateEdit("1", "offset x:", 6, Interface.edit_IFSOffsetX), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), CreateEdit("0", "offset y:", 6, Interface.edit_IFSOffsetY), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), CreateEdit("0", "offset z:", 6, Interface.edit_IFSOffsetZ), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), Interface.checkIFSAbsX, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), Interface.checkIFSAbsY, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), Interface.checkIFSAbsZ, false, false, 1);

	//frame: IFS params
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_IFS), Interface.frIFSParams, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frIFSParams), Interface.boxIFSParams);

	gtk_box_pack_start(GTK_BOX(Interface.boxIFSParams), Interface.tableIFSParams, false, false, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.label_IFSx, 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.label_IFSy, 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.label_IFSz, 2, 3, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.label_IFSalfa, 3, 4, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.label_IFSbeta, 4, 5, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.label_IFSgamma, 5, 6, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.label_IFSdistance, 6, 7, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.label_IFSintensity, 7, 8, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.label_IFSenabled, 8, 9, 0, 1);

	for (int i = 0; i < IFS_VECTOR_COUNT; i++)
	{
		Interface.IFSParams[i].editIFSx = gtk_entry_new();
		Interface.IFSParams[i].editIFSy = gtk_entry_new();
		Interface.IFSParams[i].editIFSz = gtk_entry_new();
		Interface.IFSParams[i].editIFSalfa = gtk_entry_new();
		Interface.IFSParams[i].editIFSbeta = gtk_entry_new();
		Interface.IFSParams[i].editIFSgamma = gtk_entry_new();
		Interface.IFSParams[i].editIFSdistance = gtk_entry_new();
		Interface.IFSParams[i].editIFSintensity = gtk_entry_new();
		Interface.IFSParams[i].checkIFSenabled = gtk_check_button_new();
		gtk_entry_set_width_chars(GTK_ENTRY(Interface.IFSParams[i].editIFSx), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(Interface.IFSParams[i].editIFSy), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(Interface.IFSParams[i].editIFSz), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(Interface.IFSParams[i].editIFSalfa), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(Interface.IFSParams[i].editIFSbeta), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(Interface.IFSParams[i].editIFSgamma), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(Interface.IFSParams[i].editIFSdistance), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(Interface.IFSParams[i].editIFSintensity), 6);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.IFSParams[i].editIFSx, 0, 1, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.IFSParams[i].editIFSy, 1, 2, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.IFSParams[i].editIFSz, 2, 3, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.IFSParams[i].editIFSalfa, 3, 4, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.IFSParams[i].editIFSbeta, 4, 5, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.IFSParams[i].editIFSgamma, 5, 6, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.IFSParams[i].editIFSdistance, 6, 7, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.IFSParams[i].editIFSintensity, 7, 8, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), Interface.IFSParams[i].checkIFSenabled, 8, 9, i + 1, i + 2);
	}

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_IFS), Interface.boxIFSButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSButtons), Interface.buIFSNormalizeOffset, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSButtons), Interface.buIFSNormalizeVectors, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_IFS), Interface.frIFSDefaults, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frIFSDefaults), Interface.boxIFSDefaults);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSDefaults), Interface.buIFSDefaultDodeca, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSDefaults), Interface.buIFSDefaultIcosa, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSDefaults), Interface.buIFSDefaultOcta, false, false, 1);

	//tab hybrid formula
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_hybrid), Interface.frHybrid, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frHybrid), Interface.boxHybrid);

	gtk_box_pack_start(GTK_BOX(Interface.boxHybrid), Interface.tableHybridParams, false, false, 1);

	for (int i = 0; i < HYBRID_COUNT; ++i)
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableHybridParams), Interface.label_HybridFormula[i], 0, 1, i, i + 1);
	for (int i = 0; i < HYBRID_COUNT; ++i)
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableHybridParams), Interface.comboHybridFormula[i], 1, 2, i, i + 1);

	for (int i = 0; i < HYBRID_COUNT; ++i)
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableHybridParams), CreateEdit("3", "  iterations:", 6, Interface.edit_hybridIter[i]), 2, 3, i, i + 1);
	for (int i = 0; i < HYBRID_COUNT; ++i)
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableHybridParams), CreateEdit("2", "power/scale:", 6, Interface.edit_hybridPower[i]), 3, 4, i, i + 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxHybrid), Interface.checkHybridCyclic, false, false, 1);

	//tab Mandelbox
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_mandelbox), Interface.frMandelboxMainParams, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frMandelboxMainParams), Interface.boxMandelboxMainParams);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams), Interface.boxMandelboxMainParams1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams1), CreateEdit("2", "Scale", 6, Interface.edit_mandelboxScale), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams), Interface.boxMandelboxMainParams2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams2), CreateEdit("1", "Folding limit", 6, Interface.edit_mandelboxFoldingLimit), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams2), CreateEdit("2", "Folding value", 6, Interface.edit_mandelboxFoldingValue), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams2), CreateEdit("1", "Fixed radius", 6, Interface.edit_mandelboxSpFoldingFixedRadius), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams2), CreateEdit("0,5", "Min radius", 6, Interface.edit_mandelboxSpFoldingMinRadius), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams2), CreateEdit("3,0", "Sharpness", 6, Interface.edit_mandelboxSharpness), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams), Interface.boxMandelboxOffset, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxOffset), CreateEdit("1", "Spherical folding offset X", 6, Interface.edit_mandelboxOffsetX), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxOffset), CreateEdit("1", "Y", 6, Interface.edit_mandelboxOffsetY), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxOffset), CreateEdit("1", "Z", 6, Interface.edit_mandelboxOffsetZ), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_mandelbox), Interface.frMandelboxRotations, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frMandelboxRotations), Interface.boxMandelboxRotations);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotations), Interface.boxMandelboxRotationMain, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotations), Interface.checkMandelboxRotationsEnable, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotationMain), CreateEdit("0", "Main rotation: alpha", 6, Interface.edit_mandelboxRotationMain[0]), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotationMain), CreateEdit("0", "beta", 6, Interface.edit_mandelboxRotationMain[1]), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotationMain), CreateEdit("0", "gamma", 6, Interface.edit_mandelboxRotationMain[2]), false, false, 1);

	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("Negative plane"), 1, 4, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("Positive plane"), 4, 7, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("Alpha"), 1, 2, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("Beta"), 2, 3, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("Gamma"), 3, 4, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("Alfa"), 4, 5, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("Beta"), 5, 6, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("Gamma"), 6, 7, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("X Axis"), 0, 1, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("Y Axis"), 0, 1, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), gtk_label_new("Z Axis"), 0, 1, 4, 5);

	for (int fold = 0; fold < MANDELBOX_FOLDS; ++fold)
		for (int axis = 0; axis < 3; ++axis)
			for (int component = 0; component < 3; ++component)
				gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations),
						Interface.edit_mandelboxRotation[fold][axis][component], component + fold * 3 + 1, component + fold * 3 + 2, axis + 2, axis + 3);

	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotations), Interface.tableMandelboxRotations, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_mandelbox), Interface.frMandelboxVary, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frMandelboxVary), Interface.boxMandelboxVary);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxVary), CreateEdit("0,1", "Vary scale", 6, Interface.edit_mandelboxVaryScale), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxVary), CreateEdit("1", "Fold", 6, Interface.edit_mandelboxVaryFold), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxVary), CreateEdit("0,5", "min R", 6, Interface.edit_mandelboxVaryMinR), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxVary), CreateEdit("1", "R power", 6, Interface.edit_mandelboxVaryRPower), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxVary), CreateEdit("0", "w add", 6, Interface.edit_mandelboxVaryWAdd), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_mandelbox), Interface.frMandelboxColoring, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frMandelboxColoring), Interface.boxMandelboxColoring);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColoring), Interface.boxMandelboxColor1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor1), CreateEdit("0", "Resultant absolute value component", 6, Interface.edit_mandelboxColorFactorR), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColoring), Interface.boxMandelboxColor2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor2), CreateEdit("0,1", "X plane component", 6, Interface.edit_mandelboxColorFactorX), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor2), CreateEdit("0,2", "Y plane component", 6, Interface.edit_mandelboxColorFactorY), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor2), CreateEdit("0,3", "Z plane component", 6, Interface.edit_mandelboxColorFactorZ), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColoring), Interface.boxMandelboxColor3, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor3), CreateEdit("5,0", "Min radius component", 6, Interface.edit_mandelboxColorFactorSp1), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor3), CreateEdit("1,0", "Fixed radius component", 6, Interface.edit_mandelboxColorFactorSp2), false, false, 1);

	//tab sound
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_sound), Interface.frSound, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frSound), Interface.boxSound);

	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.boxSoundMisc, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundMisc), Interface.buLoadSound, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundMisc), CreateEdit("25", "Animation FPS", 6, Interface.edit_soundFPS), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundMisc), Interface.checkSoundEnabled, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.label_soundEnvelope, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.dareaSound0, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.boxSoundBand1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundBand1), CreateEdit("1", "Band 1: min freq [Hz]", 6, Interface.edit_sound1FreqMin), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundBand1), CreateEdit("100", "max freq [Hz]", 6, Interface.edit_sound1FreqMax), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.dareaSoundA, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.boxSoundBand2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundBand2), CreateEdit("100", "Band 2: min freq [Hz]", 6, Interface.edit_sound2FreqMin), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundBand2), CreateEdit("500", "max freq [Hz]", 6, Interface.edit_sound2FreqMax), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.dareaSoundB, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.boxSoundBand3, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundBand3), CreateEdit("500", "Band 3: min freq [Hz]", 6, Interface.edit_sound3FreqMin), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundBand3), CreateEdit("2000", "max freq [Hz]", 6, Interface.edit_sound3FreqMax), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.dareaSoundC, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.boxSoundBand4, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundBand4), CreateEdit("2000", "Band 4: min freq [Hz]", 6, Interface.edit_sound4FreqMin), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSoundBand4), CreateEdit("20000", "max freq [Hz]", 6, Interface.edit_sound4FreqMax), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSound), Interface.dareaSoundD, false, false, 1);

	//tab About...
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_about), Interface.label_about, false, false, 1);

	//tabs
	gtk_box_pack_start(GTK_BOX(Interface.boxMain), Interface.tabs, false, false, 1);

	//setup files button
	gtk_box_pack_start(GTK_BOX(Interface.boxMain), Interface.buFiles, false, false, 1);

	//Load / Save settings
	gtk_box_pack_start(GTK_BOX(Interface.boxMain), Interface.frLoadSave, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frLoadSave), Interface.boxLoadSave);

	gtk_box_pack_start(GTK_BOX(Interface.boxLoadSave), Interface.buLoadSettings, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLoadSave), Interface.buSaveSettings, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLoadSave), Interface.buCopyToClipboard, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLoadSave), Interface.buGetFromClipboard, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLoadSave), Interface.buUndo, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLoadSave), Interface.buRedo, false, false, 1);

	//Keyframe aniation
	gtk_box_pack_start(GTK_BOX(Interface.boxMain), Interface.frKeyframeAnimation2, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frKeyframeAnimation2), Interface.boxBottomKeyframeAnimation);
	gtk_box_pack_start(GTK_BOX(Interface.boxBottomKeyframeAnimation), Interface.buTimeline, true, true, 1);

	//progressbar
	gtk_box_pack_start(GTK_BOX(Interface.boxMain), Interface.progressBar, false, false, 1);

	//tabs
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_fractal, Interface.tab_label_fractal);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_IFS, Interface.tab_label_IFS);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_hybrid, Interface.tab_label_hybrid);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_mandelbox, Interface.tab_label_mandelbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_view, Interface.tab_label_view);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_shaders, Interface.tab_label_shaders);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_lights, Interface.tab_label_lights);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_image, Interface.tab_label_image);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_posteffects, Interface.tab_label_posteffects);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_animation, Interface.tab_label_animation);
	//gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_sound, Interface.tab_label_sound);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_about, Interface.tab_label_about);

	//main window pack
	gtk_container_add(GTK_CONTAINER(window_interface), Interface.boxMain);

	CreateTooltips();

	gtk_widget_show_all(window_interface);

	ChangedComboFormula(NULL, NULL);
	ChangedTgladFoldingMode(NULL, NULL);
	ChangedJulia(NULL, NULL);
	ChangedSphericalFoldingMode(NULL, NULL);
	ChangedLimits(NULL, NULL);
	ChangedMandelboxRotations(NULL, NULL);

	//Writing default settings
	WriteInterface(default_settings);
	interfaceCreated = true;
	renderRequest = false;

  g_timeout_add (100,(GSourceFunc)CallerTimerLoop,NULL);
  g_timeout_add (100,(GSourceFunc)CallerTimerLoopWindowRefresh,NULL);

  clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
}

void CreateTooltips(void)
{
	gtk_widget_set_tooltip_text(Interface.boxCoordinates, "Coordinates of camera view point in fractal units");
	gtk_widget_set_tooltip_text(Interface.comboFractType, "Type of fractal formula");
	gtk_widget_set_tooltip_text(Interface.checkJulia, "Fractal will be rendered as Julia fractal\naccording to coordinates of Julia constant");
	gtk_widget_set_tooltip_text(Interface.boxJulia, "Coordinates of camera view point in fractal units");
	gtk_widget_set_tooltip_text(Interface.boxCoordinates, "Coordinates of camera view point in fractal units");
	gtk_widget_set_tooltip_text(Interface.edit_power, "Fractal formula power\nOnly works with trigonometric formulas\nWhen using Tglad's formula, it is a scale factor");
	gtk_widget_set_tooltip_text(Interface.edit_maxN, "Maximum number of iterations");
	gtk_widget_set_tooltip_text(Interface.edit_minN, "Minimum number of iterations");
	gtk_widget_set_tooltip_text(Interface.edit_DE_thresh, "Dynamic DE threshold factor.\n1 = DE threshold equals to 1 screen pixel\nHigher value gives more details");
	gtk_widget_set_tooltip_text(Interface.edit_DE_stepFactor, "DE steps fractor.\n1 -> step = Estimated Distance\nHigher value gives higher accuracy of finding fractal surface");
	gtk_widget_set_tooltip_text(Interface.checkIterThresh, "Iteration count threshold mode");
	gtk_widget_set_tooltip_text(Interface.checkLimits, "Enables cross-sections of fractal");
	gtk_widget_set_tooltip_text(Interface.boxLimits, "Coordinates of cross-sections (limit box)");
	gtk_widget_set_tooltip_text(Interface.edit_alfa, "Camera rotation around vertical axis");
	gtk_widget_set_tooltip_text(Interface.edit_beta, "Camera rotation around horizontal axis");
	gtk_widget_set_tooltip_text(Interface.edit_zoom, "Camera size. Smaller value gives higher zoom (distance between camera and viewpoint)");
	gtk_widget_set_tooltip_text(Interface.edit_persp, "Perspective ratio\n0 = Axonometric projection\nHigher value gives deeper perspective effect");
	gtk_widget_set_tooltip_text(Interface.tableArrows, "Camera rotation");
	gtk_widget_set_tooltip_text(Interface.tableArrows2, "Camera movement");
	gtk_widget_set_tooltip_text(Interface.buInitNavigator, "Initialise camera.\nCamera viewpoint will be moved far and camera very close to viewpoint");
	gtk_widget_set_tooltip_text(Interface.buForward, "Move camera forward");
	gtk_widget_set_tooltip_text(Interface.buBackward, "Move camera backward");
	gtk_widget_set_tooltip_text(Interface.edit_step_forward, "Step length factor\nstep = factor * estimated_distance_to_fractal");
	gtk_widget_set_tooltip_text(Interface.edit_step_rotation, "Rotation step in degrees");
	gtk_widget_set_tooltip_text(Interface.edit_imageWidth, "Final image width");
	gtk_widget_set_tooltip_text(Interface.edit_imageHeight, "Final image height");
	gtk_widget_set_tooltip_text(renderWindow.comboImageScale, "Image scale in render window\nSmaller value <-> smaller render window");
	gtk_widget_set_tooltip_text(Interface.edit_brightness, "Image brightness");
	gtk_widget_set_tooltip_text(Interface.edit_gamma, "Image gamma");
	gtk_widget_set_tooltip_text(Interface.buApplyBrighness, "Applying all shader changes during rendering and after rendering");
	gtk_widget_set_tooltip_text(Interface.edit_shading, "angle of incidence of light effect intensity");
	gtk_widget_set_tooltip_text(Interface.edit_shadows, "shadow intensity");
	gtk_widget_set_tooltip_text(Interface.edit_specular, "intensity of specularity effect");
	gtk_widget_set_tooltip_text(Interface.edit_ambient, "intensity of global ambient light");
	gtk_widget_set_tooltip_text(Interface.edit_ambient_occlusion, "intensity of ambient occlusion effect");
	gtk_widget_set_tooltip_text(Interface.edit_glow, "intensity of glow effect");
	gtk_widget_set_tooltip_text(Interface.edit_reflect, "intensity of texture reflection (environment mapping effect)");
	gtk_widget_set_tooltip_text(Interface.edit_AmbientOcclusionQuality,
			"ambient occlusion quality\n1 -> 8 rays\n3 -> 64 rays\n5 -> 165 rays\n10 -> 645 rays\n30 -> 5702 rays (hardcore!!!)");
	gtk_widget_set_tooltip_text(Interface.checkShadow, "Enable shadow");
	gtk_widget_set_tooltip_text(Interface.checkAmbientOcclusion, "Enable ambient occlusion effect");
	gtk_widget_set_tooltip_text(Interface.checkFastAmbientOcclusion,
			"Switch to ambient occlusion based on orbit traps. Very fast but works properly only with trigonometric Mandelbulbs");
	gtk_widget_set_tooltip_text(Interface.checkSlowShading,
			"Enable calculation of light's angle of incidence based on fake gradient estimation\nVery slow but works with all fractal formulas");
	gtk_widget_set_tooltip_text(Interface.checkBitmapBackground, "Enable spherical wrapped textured background");
	gtk_widget_set_tooltip_text(Interface.checkColoring, "Enable fractal coloring algorithm");
	gtk_widget_set_tooltip_text(Interface.edit_color_seed, "Seed for random fractal colors");
	gtk_widget_set_tooltip_text(Interface.buColorGlow1, "Glow color - low intensity area");
	gtk_widget_set_tooltip_text(Interface.buColorGlow2, "Glow color - high intesity area");
	gtk_widget_set_tooltip_text(Interface.buColorBackgroud1, "Color of background top");
	gtk_widget_set_tooltip_text(Interface.buColorBackgroud2, "Color of background bottom");
	gtk_widget_set_tooltip_text(
			Interface.buAnimationRecordTrack,
			"Recording of flight path. First you should set the camera starting position using the 3D Navigator tool and choose a low image resolution. For change of flight direction please use the mouse pointer");
	gtk_widget_set_tooltip_text(Interface.buAnimationRenderTrack, "Render animation according to recorded flight path");
	gtk_widget_set_tooltip_text(Interface.edit_animationDESpeed, "Flight speed - depends on estimated distance to fractal surface");
	gtk_widget_set_tooltip_text(Interface.buRender, "Render and save image");
	gtk_widget_set_tooltip_text(Interface.buStop, "Terminate rendering");
	gtk_widget_set_tooltip_text(Interface.buFiles, "Configure file paths");
	gtk_widget_set_tooltip_text(Interface.buLoadSettings, "Load fractal settings");
	gtk_widget_set_tooltip_text(Interface.buSaveSettings, "Save fractal settings");
	gtk_widget_set_tooltip_text(Interface.buColorBackgroud1, "Color of background top");
	gtk_widget_set_tooltip_text(Interface.edit_mouse_click_distance, "Close-up ratio in mouse clicking mode");
	gtk_widget_set_tooltip_text(Interface.frKeyframeAnimation, "Function for rendering animation with keyframes.\n"
		"All keyframes are stored in keyframes/keyframeXXXXX.fract files\n"
		"Keyframe files are settings files and can be also loaded and saved using Load Settings and Save Settings\n"
		"Most of the floating point parameters are interpolated using Catmull-Rom Splines");
	gtk_widget_set_tooltip_text(Interface.frSound, "A sound file can be used for control of some fractal or shader parameters.\n"
		"After loading the sound file, the sound is divided into 5 channels:\n"
		"s0 - sound envelope (absolute value of sound amplitude)\n"
		"sa, sb, sc, sd - selected frequency bands of sound\n"
		"To animate a given parameter you have to use the following string instead of a parameter value:\n"
		"sx amp - where sx is channel name, amp is a gain for channel\n"
		"for example, in the IFS parameters you can put: rotation alfa = s0 1.0, rotation beta = sc -0.3\n"
		"Animation must be rendered in key-frame mode");
	gtk_widget_set_tooltip_text(Interface.checkTgladMode, "Additional formula modifier. Cubic folding from Mandelbox formula");
	gtk_widget_set_tooltip_text(Interface.checkSphericalFoldingMode, "Additional formula modifier. Spherical folding from Mandelbox formula");

	for (int i = 1; i < HYBRID_COUNT; ++i)
		gtk_widget_set_tooltip_text(Interface.comboHybridFormula[i-1], g_strdup_printf("Formula #%d in sequence", i));

	gtk_widget_set_tooltip_text(Interface.comboHybridFormula[HYBRID_COUNT-1],
		g_strdup_printf("Formula #%d in sequence. When cyclic loop is disabled, then this formula will be used for the rest of iterations and cannot be set as NONE",
			HYBRID_COUNT - 1));

	for (int i = 1; i < HYBRID_COUNT; ++i)
		gtk_widget_set_tooltip_text(Interface.edit_hybridIter[i-1], g_strdup_printf("Number of iterations for formula #%d", i));

	gtk_widget_set_tooltip_text(Interface.edit_hybridIter[HYBRID_COUNT-1],
			g_strdup_printf("Number of iterations for the %dth formula (not used when cyclic loop is enabled)", HYBRID_COUNT-1));

	for (int i = 1; i < HYBRID_COUNT; ++i)
		gtk_widget_set_tooltip_text(Interface.edit_hybridPower[i-1], g_strdup_printf("Power / scale parameter for formula #%d", i));
	gtk_widget_set_tooltip_text(Interface.checkHybridCyclic, "When enabled, the formula sequence is looped");
	gtk_widget_set_tooltip_text(Interface.checkNavigatorAbsoluteDistance, "Movement step distance will be constant (absolute)");
	gtk_widget_set_tooltip_text(Interface.edit_NavigatorAbsoluteDistance, "Absolute movement step distance");
	gtk_widget_set_tooltip_text(Interface.edit_color_speed, "Colour variation speed");
	gtk_widget_set_tooltip_text(Interface.buRandomPalette, "Generates random palette according to entered random seed");
	gtk_widget_set_tooltip_text(Interface.buGetPaletteFromImage, "Grab colours from selected image");
	gtk_widget_set_tooltip_text(Interface.sliderFogDepth, "Visibility distance measured from Front value");
	gtk_widget_set_tooltip_text(Interface.sliderFogDepth, "Front distance of fog");
	gtk_widget_set_tooltip_text(Interface.buAutoDEStep, "Scan for optimal DE factor for low image quality");
	gtk_widget_set_tooltip_text(Interface.buAutoDEStepHQ, "Scan for optimal DE factor for high image quality");
	gtk_widget_set_tooltip_text(Interface.checkConstantDEThreshold, "Switches to constant DE threshold mode\nDetail size not depends on image resolution and perspective depthness");
}

bool ReadComandlineParams(int argc, char *argv[])
{
	noGUIdata.lowMemMode = false;
	noGUIdata.animMode = false;
	noGUIdata.keyframeMode = false;
	noGUIdata.playMode = false;
	noGUIdata.startFrame = 0;
	noGUIdata.endFrame = 99999;
	noGUIdata.imageFormat = imgFormatJPG;
	strcpy(noGUIdata.settingsFile, "default.fract");
	bool result = true;

	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			//printf("%s*\n", argv[i]);
			if (strcmp(argv[i], "-nogui") == 0)
			{
				noGUI = true;
				printf("commandline: no GUI mode\n");
				continue;
			}
			if (strcmp(argv[i], "-lowmem") == 0)
			{
				noGUIdata.lowMemMode = true;
				printf("commandline: Low memory mode\n");
				continue;
			}
			if (strcmp(argv[i], "-flight") == 0)
			{
				noGUIdata.animMode = true;
				noGUIdata.playMode = true;
				printf("commandline: flight mode\n");
				continue;
			}
			if (strcmp(argv[i], "-keyframe") == 0)
			{
				noGUIdata.animMode = true;
				noGUIdata.keyframeMode = true;
				printf("commandline: keyframe mode\n");
				continue;
			}
			if (strcmp(argv[i], "-start") == 0)
			{
				i++;
				if (i < argc)
				{
					noGUIdata.startFrame = atoi(argv[i]);
					printf("commandline: start frame = %d\n", noGUIdata.startFrame);
					continue;
				}
			}
			if (strcmp(argv[i], "-end") == 0)
			{
				i++;
				if (i < argc)
				{
					noGUIdata.endFrame = atoi(argv[i]);
					printf("commandline: end frame = %d\n", noGUIdata.endFrame);
					continue;
				}
			}
			if (strcmp(argv[i], "-format") == 0)
			{
				i++;
				if (i < argc && (strcmp(argv[i], "jpg") == 0))
				{
					noGUIdata.imageFormat = imgFormatJPG;
					printf("commandline: JPG image format\n");
					continue;
				}
				else if (i < argc && (strcmp(argv[i], "png") == 0))
				{
					noGUIdata.imageFormat = imgFormatPNG;
					printf("commandline: PNG image format\n");
					continue;
				}
				else if (i < argc && (strcmp(argv[i], "png16") == 0))
				{
					noGUIdata.imageFormat = imgFormatPNG16;
					printf("commandline: PNG 16-bit image format\n");
					continue;
				}
				else if (i < argc && (strcmp(argv[i], "png16alpha") == 0))
				{
					noGUIdata.imageFormat = imgFormatPNG16Alpha;
					printf("commandline: PNG 16-bit with alpha channel image format\n");
					continue;
				}
				else
				{
					printf("commandline: wrong image format\n");
					goto bad_arg;
				}
			}
			if (strcmp(argv[i], "-res") == 0)
			{
				i++;
				if(i < argc)
				{
					int w, h;
					int res = sscanf(argv[i], "%dx%d", &w, &h);
					if(res != 2)
					{
						printf("commandline: Argument must be -res WWWWxHHHH\n");
						goto bad_arg;
					}
					else
					{
						static char width_override[100], height_override[100];
						snprintf(width_override, sizeof(width_override), "image_width=%d", w);
						snprintf(height_override, sizeof(height_override), "image_height=%d", h);
						noGUIdata.overrideStrings.push_back(width_override);
						noGUIdata.overrideStrings.push_back(height_override);
						continue;
					}
				}
				else
				{
					printf("commandline: Argument must be -res WWWWxHHHH\n");
					goto bad_arg;
				}
			}
			if (strcmp(argv[i], "-fpk") == 0)
			{
				i++;
				if(i < argc)
				{
					int fpk = atoi(argv[i]);
					if(fpk <= 0)
					{
						printf("commandline: argument must be -fpk NNN\n");
						goto bad_arg;
					}
					else
					{
						static char fpk_override[100];
						snprintf(fpk_override, sizeof(fpk_override), "frames_per_keyframe=%d", fpk);
						noGUIdata.overrideStrings.push_back(fpk_override);
						continue;
					}
				}
				else
				{
					printf("commandline: argument must be -fpk NNN\n");
					goto bad_arg;
				}
			}
			if (strcmp(argv[i], "-o") == 0)
			{
				i++;
				if(i < argc)
				{
					noGUIdata.overrideStrings.push_back(argv[i]);
					continue;
				}
				else
				{
					printf("commandline: need override argument after -o\n");
				}
			}
			if (i < argc - 1 || (strcmp(argv[i], "-help") == 0))
			{
bad_arg:
				printf("commandline: wrong parameters\n");
				printf("Syntax:\nmandelbulber [options...] [settings_file]\n");
				printf("options:\n");
				printf("  -nogui            - start program without GUI\n");
				printf("  -lowmem           - low memory usage mode\n");
				printf("  -flight           - render flight animation\n");
				printf("  -keyframe         - render keyframe animation\n");
				printf("  -start N          - start renderig from frame number N\n");
				printf("  -end N            - rendering will end on frame number N\n");
				printf("  -o key=value      - override item 'key' from settings file with new value 'value'\n");
				printf("  -res WIDTHxHEIGHT - override image resolution\n");
				printf("  -fpk N            - override frames per key parameter\n");
				printf("  -format FORMAT    - image output format\n");
				printf("     jpg - JPEG format\n");
				printf("     png - PNG format\n");
				printf("     png16 - 16-bit PNG format\n");
				printf("     png16alpha - 16-bit PNG with alpha channel format\n");
				printf("[settings_file]     - file with fractal settings (program also tries\nto find file in ./mandelbulber/settings directory)\n");
				printf("When settings_file is put as command argument then program will start in noGUI mode\n");
				result = false;
				break;
			}
			else
			{
				strcpy(noGUIdata.settingsFile, argv[i]);
				printf("commandline: settings file: %s\n", noGUIdata.settingsFile);
				noGUI = true;
			}
		}
	}
	return result;
}

void Params2InterfaceData(sParamRender *source)
{
	strcpy(Interface_data.file_destination, source->file_destination);
	strcpy(Interface_data.file_background, source->file_background);
	strcpy(Interface_data.file_envmap, source->file_envmap);
	strcpy(Interface_data.file_lightmap, source->file_lightmap);
	strcpy(Interface_data.file_path, source->file_path);
	strcpy(Interface_data.file_keyframes, source->file_keyframes);
	strcpy(Interface_data.file_sound, source->file_sound);
}

void InterfaceData2Params(sParamRender *dest)
{
	strcpy(dest->file_destination, Interface_data.file_destination);
	strcpy(dest->file_background, Interface_data.file_background);
	strcpy(dest->file_envmap, Interface_data.file_envmap);
	strcpy(dest->file_lightmap, Interface_data.file_lightmap);
	strcpy(dest->file_path, Interface_data.file_path);
	strcpy(dest->file_keyframes, Interface_data.file_keyframes);
	strcpy(dest->file_sound, Interface_data.file_sound);
}

void CheckPrameters(sParamRender *params)
{
	if(params->doubles.zoom <= 1e-15) params->doubles.zoom = 1e-15;
	if(params->doubles.DE_factor <= 0.0001) params->doubles.DE_factor = 0.0001;
	if(params->doubles.smoothness <= 0.00001) params->doubles.smoothness = 0.00001;
	if(params->doubles.persp <= 0.001) params->doubles.persp = 0.001;
	if(params->globalIlumQuality >=30) params->globalIlumQuality = 30;
	if(params->globalIlumQuality <=1) params->globalIlumQuality = 1;
	if(params->image_width < 32) params->image_width = 32;
	if(params->image_height < 32) params->image_height = 32;
	if(params->reflectionsMax > 10) params->reflectionsMax = 10;
}
