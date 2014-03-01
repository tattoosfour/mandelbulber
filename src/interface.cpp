/*********************************************************
 /                   MANDELBULBER
 / definition of user interface and functions
 / for parameters handling
 /
 / author: Krzysztof Marczak
 / contact: buddhi1980@gmail.com
 / licence: GNU GPL v3.0
 /
 ********************************************************/


#include <string.h>
#include <string>
#include <cstdlib>

#include "interface.h"
#include "settings.h"

#define CONNECT_SIGNAL(object, callback, event) g_signal_connect(G_OBJECT(object), event, G_CALLBACK(callback), NULL)
#define CONNECT_SIGNAL_CLICKED(x, y) CONNECT_SIGNAL(x, y, "clicked")

sMainWindow renderWindow;
sInterface Interface;
std::map<std::string, GtkWidget*> mapInterface;
std::map<std::string, GtkWidget*> mapAppParams;
std::map<std::string, sGtkEditVector3> mapInterfaceEditVector;
std::map<std::string, sGtkEditVector3> mapAppParamsEditVector;
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

bool programClosed = false;
bool interfaceCreated = false;
bool paletteViewCreated = false;

char *sharedDir;

int x_mouse = 0;
int y_mouse = 0;
double last_z_mouse = 2.0;
double smooth_last_z_mouse = 2.0;

using namespace std;

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

	dialog->edit_destination = gtk_entry_new();
	dialog->edit_background = gtk_entry_new();
	dialog->edit_envmap = gtk_entry_new();
	dialog->edit_lightmap = gtk_entry_new();
	dialog->edit_path = gtk_entry_new();
	dialog->edit_keyframes = gtk_entry_new();

	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_destination), 100);
	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_background), 100);
	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_envmap), 100);
	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_lightmap), 100);
	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_path), 100);
	gtk_entry_set_width_chars(GTK_ENTRY(dialog->edit_keyframes), 100);

	gtk_entry_set_text(GTK_ENTRY(dialog->edit_destination), Interface_data.file_destination);
	gtk_entry_set_text(GTK_ENTRY(dialog->edit_background), Interface_data.file_background);
	gtk_entry_set_text(GTK_ENTRY(dialog->edit_envmap), Interface_data.file_envmap);
	gtk_entry_set_text(GTK_ENTRY(dialog->edit_lightmap), Interface_data.file_lightmap);
	gtk_entry_set_text(GTK_ENTRY(dialog->edit_path), Interface_data.file_path);
	gtk_entry_set_text(GTK_ENTRY(dialog->edit_keyframes), Interface_data.file_keyframes);

	dialog->bu_cancel = gtk_button_new_with_label("Cancel");
	dialog->bu_ok = gtk_button_new_with_label("OK");
	dialog->bu_select_destination = gtk_button_new_with_label("Select");
	dialog->bu_select_background = gtk_button_new_with_label("Select");
	dialog->bu_select_envmap = gtk_button_new_with_label("Select");
	dialog->bu_select_lightmap = gtk_button_new_with_label("Select");
	dialog->bu_select_path = gtk_button_new_with_label("Select");
	dialog->bu_select_keyframes = gtk_button_new_with_label("Select");

	g_signal_connect(G_OBJECT(dialog->bu_ok), "clicked", G_CALLBACK(PressedOkDialogFiles), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_cancel), "clicked", G_CALLBACK(PressedCancelDialogFiles), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_destination), "clicked", G_CALLBACK(PressedSelectDestination), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_background), "clicked", G_CALLBACK(PressedSelectBackground), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_envmap), "clicked", G_CALLBACK(PressedSelectEnvmap), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_lightmap), "clicked", G_CALLBACK(PressedSelectLightmap), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_path), "clicked", G_CALLBACK(PressedSelectFlightPath), dialog);
	g_signal_connect(G_OBJECT(dialog->bu_select_keyframes), "clicked", G_CALLBACK(PressedSelectKeyframes), dialog);

	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_destination, 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_background, 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_envmap, 0, 1, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_lightmap, 0, 1, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_path, 0, 1, 4, 5);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->label_keyframes, 0, 1, 5, 6);

	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_destination, 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_background, 1, 2, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_envmap, 1, 2, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_lightmap, 1, 2, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_path, 1, 2, 4, 5);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->edit_keyframes, 1, 2, 5, 6);

	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_destination, 2, 3, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_background, 2, 3, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_envmap, 2, 3, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_lightmap, 2, 3, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_path, 2, 3, 4, 5);
	gtk_table_attach_defaults(GTK_TABLE(dialog->table_edits), dialog->bu_select_keyframes, 2, 3, 5, 6);

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

fractal::enumFractalFormula HybridFormulaNumberGUI2Data(int formula)
{
	fractal::enumFractalFormula formula2 = fractal::trig_optim;
	if (formula == 0) formula2 = fractal::none;
	if (formula == 1) formula2 = fractal::trig_optim;
	if (formula == 2) formula2 = fractal::fast_trig;
	if (formula == 3) formula2 = fractal::minus_fast_trig;
	if (formula == 4) formula2 = fractal::xenodreambuie;
	if (formula == 5) formula2 = fractal::hypercomplex;
	if (formula == 6) formula2 = fractal::quaternion;
	if (formula == 7) formula2 = fractal::menger_sponge;
	if (formula == 8) formula2 = fractal::tglad;
	if (formula == 9) formula2 = fractal::kaleidoscopic;
	if (formula == 10) formula2 = fractal::mandelbulb2;
	if (formula == 11) formula2 = fractal::mandelbulb3;
	if (formula == 12) formula2 = fractal::mandelbulb4;
	if (formula == 13) formula2 = fractal::foldingIntPow2;
	if (formula == 14) formula2 = fractal::smoothMandelbox;
	if (formula == 15) formula2 = fractal::mandelboxVaryScale4D;
	if (formula == 16) formula2 = fractal::aexion;
	if (formula == 17) formula2 = fractal::benesi;
	if (formula == 18) formula2 = fractal::bristorbrot;
	if (formula == 19) formula2 = fractal::generalizedFoldBox;
	if (formula == 20) formula2 = fractal::invertX;
	if (formula == 21) formula2 = fractal::invertY;
	if (formula == 22) formula2 = fractal::invertZ;
	if (formula == 23) formula2 = fractal::invertR;
	if (formula == 24) formula2 = fractal::sphericalFold;
	if (formula == 25) formula2 = fractal::powXYZ;
	if (formula == 26) formula2 = fractal::scaleX;
	if (formula == 27) formula2 = fractal::scaleY;
	if (formula == 28) formula2 = fractal::scaleZ;
	if (formula == 29) formula2 = fractal::offsetX;
	if (formula == 30) formula2 = fractal::offsetY;
	if (formula == 31) formula2 = fractal::offsetZ;
	if (formula == 32) formula2 = fractal::angleMultiplyX;
	if (formula == 33) formula2 = fractal::angleMultiplyY;
	if (formula == 34) formula2 = fractal::angleMultiplyZ;
	return formula2;
}

fractal::enumFractalFormula MainFormulaNumberGUI2Data(int formula)
{
	fractal::enumFractalFormula formula2 = fractal::trig_optim;
	if (formula == 0) formula2 = fractal::trig_optim;
	if (formula == 1) formula2 = fractal::trig_DE;
	if (formula == 2) formula2 = fractal::fast_trig;
	if (formula == 3) formula2 = fractal::minus_fast_trig;
	if (formula == 4) formula2 = fractal::xenodreambuie;
	if (formula == 5) formula2 = fractal::hypercomplex;
	if (formula == 6) formula2 = fractal::quaternion;
	if (formula == 7) formula2 = fractal::menger_sponge;
	if (formula == 8) formula2 = fractal::tglad;
	if (formula == 9) formula2 = fractal::kaleidoscopic;
	if (formula == 10) formula2 = fractal::mandelbulb2;
	if (formula == 11) formula2 = fractal::mandelbulb3;
	if (formula == 12) formula2 = fractal::mandelbulb4;
	if (formula == 13) formula2 = fractal::foldingIntPow2;
	if (formula == 14) formula2 = fractal::smoothMandelbox;
	if (formula == 15) formula2 = fractal::mandelboxVaryScale4D;
	if (formula == 16) formula2 = fractal::aexion;
	if (formula == 17) formula2 = fractal::benesi;
	if (formula == 18) formula2 = fractal::bristorbrot;
	if (formula == 19) formula2 = fractal::hybrid;
	if (formula == 20) formula2 = fractal::generalizedFoldBox;
	return formula2;
}

int HybridFormulaNumberData2GUI(fractal::enumFractalFormula formula)
{
	int formula2 = 0;
	if (formula == fractal::none) formula2 = 0;
	if (formula == fractal::trig_optim) formula2 = 1;
	if (formula == fractal::fast_trig) formula2 = 2;
	if (formula == fractal::minus_fast_trig) formula2 = 3;
	if (formula == fractal::xenodreambuie) formula2 = 4;
	if (formula == fractal::hypercomplex) formula2 = 5;
	if (formula == fractal::quaternion) formula2 = 6;
	if (formula == fractal::menger_sponge) formula2 = 7;
	if (formula == fractal::tglad) formula2 = 8;
	if (formula == fractal::kaleidoscopic) formula2 = 9;
	if (formula == fractal::mandelbulb2) formula2 = 10;
	if (formula == fractal::mandelbulb3) formula2 = 11;
	if (formula == fractal::mandelbulb4) formula2 = 12;
	if (formula == fractal::foldingIntPow2) formula2 = 13;
	if (formula == fractal::smoothMandelbox) formula2 = 14;
	if (formula == fractal::mandelboxVaryScale4D) formula2 = 15;
	if (formula == fractal::aexion) formula2 = 16;
	if (formula == fractal::benesi) formula2 = 17;
	if (formula == fractal::bristorbrot) formula2 = 18;
	if (formula == fractal::generalizedFoldBox) formula2 = 19;
	if (formula == fractal::invertX) formula2 = 20;
	if (formula == fractal::invertY) formula2 = 21;
	if (formula == fractal::invertZ) formula2 = 22;
	if (formula == fractal::invertR) formula2 = 23;
	if (formula == fractal::sphericalFold) formula2 = 24;
	if (formula == fractal::powXYZ) formula2 = 25;
	if (formula == fractal::scaleX) formula2 = 26;
	if (formula == fractal::scaleY) formula2 = 27;
	if (formula == fractal::scaleZ) formula2 = 28;
	if (formula == fractal::offsetX) formula2 = 29;
	if (formula == fractal::offsetY) formula2 = 30;
	if (formula == fractal::offsetZ) formula2 = 31;
	if (formula == fractal::angleMultiplyX) formula2 = 32;
	if (formula == fractal::angleMultiplyY) formula2 = 33;
	if (formula == fractal::angleMultiplyZ) formula2 = 34;
	return formula2;
}

int MainFormulaNumberData2GUI(fractal::enumFractalFormula formula)
{
	int formula2 = 0;
	if (formula == fractal::trig_optim) formula2 = 0;
	if (formula == fractal::trig_DE) formula2 = 1;
	if (formula == fractal::fast_trig) formula2 = 2;
	if (formula == fractal::minus_fast_trig) formula2 = 3;
	if (formula == fractal::xenodreambuie) formula2 = 4;
	if (formula == fractal::hypercomplex) formula2 = 5;
	if (formula == fractal::quaternion) formula2 = 6;
	if (formula == fractal::menger_sponge) formula2 = 7;
	if (formula == fractal::tglad) formula2 = 8;
	if (formula == fractal::kaleidoscopic) formula2 = 9;
	if (formula == fractal::mandelbulb2) formula2 = 10;
	if (formula == fractal::mandelbulb3) formula2 = 11;
	if (formula == fractal::mandelbulb4) formula2 = 12;
	if (formula == fractal::foldingIntPow2) formula2 = 13;
	if (formula == fractal::smoothMandelbox) formula2 = 14;
	if (formula == fractal::mandelboxVaryScale4D) formula2 = 15;
	if (formula == fractal::aexion) formula2 = 16;
	if (formula == fractal::benesi) formula2 = 17;
	if (formula == fractal::bristorbrot) formula2 = 18;
	if (formula == fractal::hybrid) formula2 = 19;
	if (formula == fractal::generalizedFoldBox) formula2 = 20;
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
		params->doubles.alpha = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_alfa))) / 180.0 * M_PI;
		params->doubles.beta = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_beta))) / 180.0 * M_PI;
		params->doubles.gamma = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_gammaAngle))) / 180.0 * M_PI;
		params->doubles.zoom = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_zoom)));
		params->doubles.persp = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_persp)));
		params->doubles.DE_factor = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_DE_stepFactor)));
		params->doubles.quality = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_DE_thresh)));
		params->doubles.smoothness = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_roughness)));
		params->fractal.doubles.N = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_maxN)));
		params->fractal.minN = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_minN)));
		params->fractal.doubles.power = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_power)));
		params->image_width = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_imageWidth)));
		params->image_height = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_imageHeight)));
		params->noOfTiles = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_tiles)));
		params->doubles.imageAdjustments.brightness = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_brightness)));
		params->doubles.imageAdjustments.imageGamma = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_gamma)));
		params->doubles.imageAdjustments.contrast = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_contrast)));
		params->imageSwitches.hdrEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkHDR));
		params->doubles.imageAdjustments.ambient = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_ambient)));
		params->doubles.imageAdjustments.globalIlum = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_ambient_occlusion)));
		params->doubles.imageAdjustments.glow_intensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_glow)));
		params->doubles.imageAdjustments.reflect = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_reflect)));
		params->doubles.imageAdjustments.shading = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_shading)));
		params->doubles.imageAdjustments.directLight = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_shadows)));
		params->doubles.imageAdjustments.specular = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_specular)));
		params->global_ilumination = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAmbientOcclusion));
		params->fastGlobalIllumination = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkFastAmbientOcclusion));
		params->doubles.fastAoTune = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_fastAoTune)));
		params->globalIlumQuality = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_AmbientOcclusionQuality)));
		params->shadow = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkShadow));
		params->fractal.iterThresh = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIterThresh));
		params->fractal.juliaMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkJulia));
		params->slowShading = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkSlowShading));
		params->texturedBackground = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkBitmapBackground));
		params->background_as_fulldome = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkBitmapBackgroundFulldome));
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
		params->doubles.colourSaturation = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_colour_saturation)));
		params->fractal.tgladFoldingMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkTgladMode));
		params->fractal.sphericalFoldingMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkSphericalFoldingMode));
		params->fractal.IFS.foldingMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSFoldingMode));
		params->fractal.doubles.foldingLimit = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_tglad_folding_1)));
		params->fractal.doubles.foldingValue = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_tglad_folding_2)));
		params->fractal.doubles.foldingSphericalFixed = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_spherical_folding_1)));
		params->fractal.doubles.foldingSphericalMin = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_spherical_folding_2)));
		params->imageSwitches.fogEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkFogEnabled));
		params->doubles.imageAdjustments.fogVisibility = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepth));
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
		params->doubles.auxLightPre[0].x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre1x)));
		params->doubles.auxLightPre[0].y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre1y)));
		params->doubles.auxLightPre[0].z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre1z)));
		params->doubles.auxLightPreIntensity[0] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre1intensity)));
		params->doubles.auxLightPre[1].x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre2x)));
		params->doubles.auxLightPre[1].y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre2y)));
		params->doubles.auxLightPre[1].z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre2z)));
		params->doubles.auxLightPreIntensity[1] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre2intensity)));
		params->doubles.auxLightPre[2].x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre3x)));
		params->doubles.auxLightPre[2].y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre3y)));
		params->doubles.auxLightPre[2].z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre3z)));
		params->doubles.auxLightPreIntensity[2] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre3intensity)));
		params->doubles.auxLightPre[3].x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre4x)));
		params->doubles.auxLightPre[3].y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre4y)));
		params->doubles.auxLightPre[3].z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre4z)));
		params->doubles.auxLightPreIntensity[3] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightPre4intensity)));
		params->auxLightPreEnabled[0] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre1Enabled));
		params->auxLightPreEnabled[1] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre2Enabled));
		params->auxLightPreEnabled[2] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre3Enabled));
		params->auxLightPreEnabled[3] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre4Enabled));
		params->doubles.auxLightVisibility = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightVisibility)));
		params->doubles.mainLightAlpha = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mainLightAlfa))) / 180.0 * M_PI;
		params->doubles.mainLightBeta = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mainLightBeta))) / 180.0 * M_PI;
		params->doubles.auxLightRandomCenter.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightRandomCentreX)));
		params->doubles.auxLightRandomCenter.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightRandomCentreY)));
		params->doubles.auxLightRandomCenter.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_auxLightRandomCentreZ)));
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
		params->fractal.IFS.doubles.edge.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_IFSEdgeX)));
		params->fractal.IFS.doubles.edge.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_IFSEdgeY)));
		params->fractal.IFS.doubles.edge.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_IFSEdgeZ)));
		params->fractal.IFS.absX = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsX));
		params->fractal.IFS.absY = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsY));
		params->fractal.IFS.absZ = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsZ));
		params->fractal.IFS.mengerSpongeMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIFSMengerSponge));

		params->startFrame = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_animationStartFrame)));
		params->endFrame = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_animationEndFrame)));
		params->framesPerKeyframe = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_animationFramesPerKey)));
		params->doubles.imageAdjustments.paletteOffset = gtk_adjustment_get_value(GTK_ADJUSTMENT(Interface.adjustmentPaletteOffset));
		for (int i = 0; i < HYBRID_COUNT; ++i) {
			params->fractal.hybridIters[i] = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_hybridIter[i])));
			params->fractal.doubles.hybridPower[i] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_hybridPower[i])));
		}
		params->fractal.hybridCyclic = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkHybridCyclic));
		params->perspectiveType = (enumPerspectiveType)gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboPerspectiveType));
		if(params->perspectiveType == fishEyeCut){
			params->fishEyeCut = true;
			params->perspectiveType = fishEye;
		}
		else
		{
			params->fishEyeCut = false;
		}
		params->doubles.stereoEyeDistance = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_stereoDistance)));
		params->stereoEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkStereoEnabled));
		params->doubles.viewDistanceMin = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_viewMinDistance)));
		params->doubles.viewDistanceMax = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_viewMaxDistance)));
		params->fractal.interiorMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkInteriorMode));
		params->fractal.doubles.constantFactor = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_FractalConstantFactor)));
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
		params->fractal.mandelbox.doubles.solid = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxSolid)));
		params->fractal.mandelbox.doubles.melt = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mandelboxMelt)));

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

		params->doubles.fogColour1Distance = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricFogColorDistance)));
		params->doubles.fogColour2Distance = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricFogColorDistance2)));
		params->doubles.fogDensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricFogDensity)));
		params->doubles.fogDistanceFactor = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_volumetricFogDistanceFact)));

		params->fractal.doubles.primitives.planeCentre.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitivePlaneCentreX)));
		params->fractal.doubles.primitives.planeCentre.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitivePlaneCentreY)));
		params->fractal.doubles.primitives.planeCentre.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitivePlaneCentreZ)));
		params->fractal.doubles.primitives.planeNormal.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitivePlaneNormalX)));
		params->fractal.doubles.primitives.planeNormal.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitivePlaneNormalY)));
		params->fractal.doubles.primitives.planeNormal.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitivePlaneNormalZ)));
		params->doubles.primitivePlaneReflect = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitivePlaneReflect)));
		params->fractal.primitives.planeEnable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitivePlaneEnabled));
		params->fractal.primitives.onlyPlane = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveOnlyPlane));
		params->fractal.doubles.primitives.boxCentre.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveBoxCentreX)));
		params->fractal.doubles.primitives.boxCentre.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveBoxCentreY)));
		params->fractal.doubles.primitives.boxCentre.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveBoxCentreZ)));
		params->fractal.doubles.primitives.boxSize.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveBoxSizeX)));
		params->fractal.doubles.primitives.boxSize.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveBoxSizeY)));
		params->fractal.doubles.primitives.boxSize.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveBoxSizeZ)));
		params->doubles.primitiveBoxReflect = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveBoxReflect)));
		params->fractal.primitives.boxEnable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveBoxEnabled));
		params->fractal.doubles.primitives.invertedBoxCentre.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxCentreX)));
		params->fractal.doubles.primitives.invertedBoxCentre.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxCentreY)));
		params->fractal.doubles.primitives.invertedBoxCentre.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxCentreZ)));
		params->fractal.doubles.primitives.invertedBoxSize.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxSizeX)));
		params->fractal.doubles.primitives.invertedBoxSize.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxSizeY)));
		params->fractal.doubles.primitives.invertedBoxSize.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxSizeZ)));
		params->doubles.primitiveInvertedBoxReflect = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxReflect)));
		params->fractal.primitives.invertedBoxEnable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveInvertedBoxEnabled));
		params->fractal.doubles.primitives.sphereCentre.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveSphereCentreX)));
		params->fractal.doubles.primitives.sphereCentre.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveSphereCentreY)));
		params->fractal.doubles.primitives.sphereCentre.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveSphereCentreZ)));
		params->fractal.doubles.primitives.sphereRadius = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveSphereRadius)));
		params->doubles.primitiveSphereReflect = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveSphereReflect)));
		params->fractal.primitives.sphereEnable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveSphereEnabled));
		params->fractal.doubles.primitives.invertedSphereCentre.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedSphereCentreX)));
		params->fractal.doubles.primitives.invertedSphereCentre.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedSphereCentreY)));
		params->fractal.doubles.primitives.invertedSphereCentre.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedSphereCentreZ)));
		params->fractal.doubles.primitives.invertedSphereRadius = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedSphereRadius)));
		params->doubles.primitiveInvertedSphereReflect = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveInvertedSphereReflect)));
		params->fractal.primitives.invertedSphereEnable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveInvertedSphereEnabled));
		params->fractal.doubles.primitives.waterHeight = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveWaterHeight)));
		params->fractal.doubles.primitives.waterAmplitude = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveWaterAmplitude)));
		params->fractal.doubles.primitives.waterLength = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveWaterLength)));
		params->fractal.doubles.primitives.waterRotation = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveWaterRotation)));
		params->fractal.doubles.primitives.waterAnimSpeed = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveWaterAnimSpeed)));
		params->doubles.primitiveWaterReflect = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveWaterReflect)));
		params->fractal.primitives.waterIterations = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_primitiveWaterIterations)));
		params->fractal.primitives.waterEnable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveWaterEnabled));

		params->fractal.genFoldBox.type = (fractal::enumGeneralizedFoldBoxType)gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboGeneralizedFoldBoxType));

		params->OpenCLEngine = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboOpenCLEngine));
		params->OpenCLPixelsPerJob = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_OpenCLPixelsPerJob)));
		params->doubles.iterFogOpacityTrim = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_iterFogOpacityTrim)));
		params->doubles.iterFogOpacity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_iterFogOpacity)));
		params->imageSwitches.iterFogEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkIterFogEnable));

		params->fakeLightsEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkFakeLightsEnabled));
		params->fractal.doubles.fakeLightsOrbitTrap.x = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_fakeLightsOrbitTrapX)));
		params->fractal.doubles.fakeLightsOrbitTrap.y = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_fakeLightsOrbitTrapY)));
		params->fractal.doubles.fakeLightsOrbitTrap.z = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_fakeLightsOrbitTrapZ)));
		params->doubles.fakeLightsIntensity = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_fakeLightsIntensity)));
		params->doubles.fakeLightsVisibility = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_fakeLightsVisibility)));
		params->doubles.fakeLightsVisibilitySize = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_fakeLightsVisibilitySize)));
		params->fractal.fakeLightsMinIter = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_fakeLightsMinIter)));
		params->fractal.fakeLightsMaxIter = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_fakeLightsMaxIter)));

		params->doubles.shadowConeAngle = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_shadowConeAngle)));

#ifdef CLSUPPORT
		params->fractal.useCustomOCLFormula = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkOpenClCustomEnable));
		params->fractal.customOCLFormulaDEMode = (fractal::enumOCLDEMode)gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboOpenCLDEMode));
		if(clSupport->customFormulas)
		{
			std::string actualName, actualFormula, actualIni;
			clSupport->customFormulas->GetActual(&actualName, &actualFormula, &actualIni);
			strcpy(params->fractal.customOCLFormulaName, actualName.c_str());

			for(int i=0; i<15; i++)
				params->fractal.doubles.customParameters[i] = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_customParameters[i])));
		}
		params->fractal.doubles.deltaDEStep = atofData(gtk_entry_get_text(GTK_ENTRY(Interface.edit_OpenCLDeltaDEStep)));
		params->OpenCLDOFMethod = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboOpenCLDOFMode));
#endif

		GdkColor color;
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorGlow1), &color);
		params->effectColours.glow_color1 = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorGlow2), &color);
		params->effectColours.glow_color2 = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorBackgroud1), &color);
		params->background_color1 = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorBackgroud2), &color);
		params->background_color2 = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorBackgroud3), &color);
		params->background_color3 = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorFog1), &color);
		params->fogColour1 = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorFog2), &color);
		params->fogColour2 = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorFog3), &color);
		params->fogColour3 = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorFog), &color);
		params->effectColours.fogColor = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre1), &color);
		params->auxLightPreColour[0] = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre2), &color);
		params->auxLightPreColour[1] = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre3), &color);
		params->auxLightPreColour[2] = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre4), &color);
		params->auxLightPreColour[3] = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorMainLight), &color);
		params->effectColours.mainLightColour = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorPrimitivePlane), &color);
		params->primitivePlaneColour = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorPrimitiveBox), &color);
		params->primitiveBoxColour = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorPrimitiveInvertedBox), &color);
		params->primitiveInvertedBoxColour = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorPrimitiveSphere), &color);
		params->primitiveSphereColour = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorPrimitiveInvertedSphere), &color);
		params->primitiveInvertedSphereColour = GdkColor2sRGB(color);
		gtk_color_button_get_color(GTK_COLOR_BUTTON(Interface.buColorPrimitiveWater), &color);
		params->primitiveWaterColour = GdkColor2sRGB(color);

		int formula = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboFractType));
		if (formula == 0) params->fractal.formula = fractal::trig_optim;
		if (formula == 1) params->fractal.formula = fractal::trig_DE;
		if (formula == 2) params->fractal.formula = fractal::fast_trig;
		if (formula == 3) params->fractal.formula = fractal::minus_fast_trig;
		if (formula == 4) params->fractal.formula = fractal::xenodreambuie;
		if (formula == 5) params->fractal.formula = fractal::hypercomplex;
		if (formula == 6) params->fractal.formula = fractal::quaternion;
		if (formula == 7) params->fractal.formula = fractal::menger_sponge;
		if (formula == 8) params->fractal.formula = fractal::tglad;
		if (formula == 9) params->fractal.formula = fractal::kaleidoscopic;
		if (formula == 10) params->fractal.formula = fractal::mandelbulb2;
		if (formula == 11) params->fractal.formula = fractal::mandelbulb3;
		if (formula == 12) params->fractal.formula = fractal::mandelbulb4;
		if (formula == 13) params->fractal.formula = fractal::foldingIntPow2;
		if (formula == 14) params->fractal.formula = fractal::smoothMandelbox;
		if (formula == 15) params->fractal.formula = fractal::mandelboxVaryScale4D;
		if (formula == 16) params->fractal.formula = fractal::aexion;
		if (formula == 17) params->fractal.formula = fractal::benesi;
		if (formula == 18) params->fractal.formula = fractal::bristorbrot;
		if (formula == 19) params->fractal.formula = fractal::hybrid;
		if (formula == 20) params->fractal.formula = fractal::generalizedFoldBox;

#ifdef CLSUPPORT
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkOpenClCustomEnable))) params->fractal.formula = fractal::ocl_custom;
#endif

		CheckPrameters(params);

		for (int i = 0; i < HYBRID_COUNT; ++i)
			params->fractal.hybridFormula[i] = HybridFormulaNumberGUI2Data(gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboHybridFormula[i])));

		double imageScale = 0;
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

		if ((params->SSAOEnabled || params->DOFEnabled) && params->noOfTiles > 1)
		{
			GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
					"Warning! When tile rendering is enabled, DOF and SSAO have to be disabled");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			params->SSAOEnabled = false;
			params->DOFEnabled = false;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkSSAOEnabled), params->SSAOEnabled);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkDOFEnabled), params->DOFEnabled);
		}
		params->fractal.frameNo = 0;
	}

	if (params->fractal.formula == fractal::trig_DE || params->fractal.formula == fractal::trig_optim || params->fractal.formula == fractal::menger_sponge || params->fractal.formula == fractal::kaleidoscopic
			|| params->fractal.formula == fractal::tglad || params->fractal.formula == fractal::smoothMandelbox || params->fractal.formula == fractal::mandelboxVaryScale4D
			|| params->fractal.formula == fractal::generalizedFoldBox) params->fractal.analitycDE = true;
	else params->fractal.analitycDE = false;

	params->doubles.resolution = 1.0 / params->image_width / params->noOfTiles;

	bool volLightEnabled = false;
	for (int i = 0; i < 5; i++)
	{
		if (params->volumetricLightEnabled[i]) volLightEnabled = true;
	}
	params->imageSwitches.volumetricLightEnabled = volLightEnabled;

	params->quiet = false;

	InterfaceData2Params(params);

	Interface_data.imageFormat = (enumImageFormat) params->imageFormat;

	mainImage.SetImageParameters(params->doubles.imageAdjustments, params->imageSwitches);

	//srand(Interface_data.coloring_seed);
	//NowaPaleta(paleta, 1.0);

	sRGB *palette2 = Interface_data.palette;
	for (int i = 0; i < 256; i++)
	{
		params->palette[i] = palette2[i];
	}

	if(params->fractal.primitives.planeEnable) params->palette[253] = sRGBDiv256(params->primitivePlaneColour);
	if(params->fractal.primitives.boxEnable) params->palette[252] = sRGBDiv256(params->primitiveBoxColour);
	if(params->fractal.primitives.invertedBoxEnable) params->palette[251] = sRGBDiv256(params->primitiveInvertedBoxColour);
	if(params->fractal.primitives.sphereEnable) params->palette[250] = sRGBDiv256(params->primitiveSphereColour);
	if(params->fractal.primitives.invertedSphereEnable) params->palette[249] = sRGBDiv256(params->primitiveInvertedSphereColour);
	if(params->fractal.primitives.waterEnable) params->palette[248] = sRGBDiv256(params->primitiveWaterColour);

	RecalculateIFSParams(params->fractal);
	CreateFormulaSequence(params->fractal);
}

std::string gtk_entry_get_vector3(sGtkEditVector3 entry3)
{
	std::string out;
	out += gtk_entry_get_text(GTK_ENTRY(entry3.x));
	out += " ";
	out += gtk_entry_get_text(GTK_ENTRY(entry3.y));
	out	+= " ";
	out += gtk_entry_get_text(GTK_ENTRY(entry3.z));
	return out;
}

void gtk_entry_set_vector3(sGtkEditVector3 entry3, std::string text)
{
	if (text.length() < 256)
	{
		char str1[256];
		char str2[256];
		char str3[256];
		sscanf(text.c_str(), "%s %s %s", str1, str2, str3);
		gtk_entry_set_text(GTK_ENTRY(entry3.x), str1);
		gtk_entry_set_text(GTK_ENTRY(entry3.y), str2);
		gtk_entry_set_text(GTK_ENTRY(entry3.z), str3);
	}
	else
	{
		std::cerr << "gtk_entry_set_vector3(): text too long" << std::endl;
	}
}

void ReadInterfaceNew(parameters::container *par)
{
	{
		std::map<std::string, GtkWidget*>::iterator it;
		for (it = mapInterface.begin(); it != mapInterface.end(); ++it)
		{
			std::string name = it->first;
			GtkWidget *widget = it->second;
			if(GTK_IS_ENTRY(widget))
			{
				par->Set(name, std::string(gtk_entry_get_text(GTK_ENTRY(widget))));
			}
			else if(GTK_IS_TOGGLE_BUTTON(widget))
			{
				par->Set(name, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
			}
			else if(GTK_IS_COMBO_BOX(widget))
			{
				par->Set(name, gtk_combo_box_get_active(GTK_COMBO_BOX(widget)));
			}
			else if(GTK_IS_COLOR_BUTTON(widget))
			{
				GdkColor color;
				gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &color);
				par->Set(name, GdkColor2sRGB(color));
			}
			else
			{
				std::cerr << "ReadInterfaceNew(): this type of widget is not proper here. Name: '" << name << "'" << std::endl;
			}
		}
	}
	{
		std::map<std::string, sGtkEditVector3>::iterator it;
		for (it = mapInterfaceEditVector.begin(); it != mapInterfaceEditVector.end(); ++it)
		{
			std::string name = it->first;
			sGtkEditVector3 widget3 = it->second;
			par->Set(name, gtk_entry_get_vector3(widget3));
		}
	}

	int formulaCombo = par->Get<int>("formula_combo");
	fractal::enumFractalFormula formulaNo = MainFormulaNumberGUI2Data(formulaCombo);
	par->Set("formula", (int)formulaNo);

	for (int i = 1; i <= HYBRID_COUNT; ++i)
		par->Set("hybrid_formula", i, (int)HybridFormulaNumberGUI2Data(par->Get<int>("hybrid_formula_combo", i)));
}

void WriteInterfaceNew(parameters::container *par)
{
	fractal::enumFractalFormula formulaNo = (fractal::enumFractalFormula) par->Get<int>("formula");
	int formulaCombo = MainFormulaNumberData2GUI(formulaNo);
	par->Set("formula_combo", formulaCombo);

	for (int i = 1; i <= HYBRID_COUNT; ++i)
		par->Set("hybrid_formula_combo", i, HybridFormulaNumberData2GUI((fractal::enumFractalFormula) par->Get<int>("hybrid_formula", i)));

	{
		std::map<std::string, GtkWidget*>::iterator it;
		for (it = mapInterface.begin(); it != mapInterface.end(); ++it)
		{
			std::string name = it->first;
			GtkWidget *widget = it->second;
			if (GTK_IS_ENTRY(widget))
			{
				gtk_entry_set_text(GTK_ENTRY(widget), par->Get<std::string>(name).c_str());
			}
			else if (GTK_IS_TOGGLE_BUTTON(widget))
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), par->Get<bool>(name));
			}
			else if (GTK_IS_COMBO_BOX(widget))
			{
				gtk_combo_box_set_active(GTK_COMBO_BOX(widget), par->Get<int>(name));
			}
			else if(GTK_IS_COLOR_BUTTON(widget))
			{
				GdkColor color = sRGB2GdkColor(par->Get<sRGB>(name));
				gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);
			}
			else
			{
				std::cerr << "WriteInterfaceNew(): this type of widget is not proper here. Name: '" << name << "'" << std::endl;
			}
		}
	}
	{
		std::map<std::string, sGtkEditVector3>::iterator it;
		for (it = mapInterfaceEditVector.begin(); it != mapInterfaceEditVector.end(); ++it)
		{
			std::string name = it->first;
			sGtkEditVector3 widget3 = it->second;
			std::string value = par->Get<std::string>(name);
			gtk_entry_set_vector3(widget3, value);
		}
	}
}

void ReadInterfaceAppSettings(sAppSettings *appParams)
{
#ifdef CLSUPPORT
	appParams->oclUseCPU = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboOpenCLGPUCPU));
	appParams->oclDeviceIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboOpenCLDeviceIndex));
	appParams->oclPlatformIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboOpenCLPlatformIndex));
	appParams->oclEngineSelection = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboOpenCLEngine));
	appParams->oclCycleTime = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_OpenCLProcessingCycleTime)));
	appParams->oclMemoryLimit = atoi(gtk_entry_get_text(GTK_ENTRY(Interface.edit_OpenCLMaxMem)));
	appParams->oclTextEditor = gtk_entry_get_text(GTK_ENTRY(Interface.edit_OpenCLTextEditor));
#endif
	appParams->absoluteMovementModeEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorAbsoluteDistance));
	appParams->zoomByMouseClickEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkZoomClickEnable));
	appParams->goCloseToSurfaceEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorGoToSurface));
	appParams->cameraMoveStepRelative = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_forward)));
	appParams->cameraMoveStepAbsolute = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_NavigatorAbsoluteDistance)));
	appParams->rotationStep = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_step_rotation)));
	appParams->mouseCloseUpRatio = atof(gtk_entry_get_text(GTK_ENTRY(Interface.edit_mouse_click_distance)));
	appParams->netRenderClientPort = gtk_entry_get_text(GTK_ENTRY(Interface.edit_netRenderClientPort));
	appParams->netRenderClientIP = gtk_entry_get_text(GTK_ENTRY(Interface.edit_netRenderClientName));
	appParams->netRenderServerPort = gtk_entry_get_text(GTK_ENTRY(Interface.edit_netRenderServerPort));
}

void WriteInterfaceAppSettings(sAppSettings *appParams)
{
#ifdef CLSUPPORT
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboOpenCLGPUCPU), appParams->oclUseCPU);
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboOpenCLDeviceIndex), appParams->oclDeviceIndex);
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboOpenCLPlatformIndex), appParams->oclPlatformIndex);
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboOpenCLEngine), appParams->oclEngineSelection);
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_OpenCLProcessingCycleTime), DoubleToString(appParams->oclCycleTime));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_OpenCLMaxMem), IntToString(appParams->oclMemoryLimit));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_OpenCLTextEditor), appParams->oclTextEditor.c_str());
#endif
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorAbsoluteDistance), appParams->absoluteMovementModeEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkZoomClickEnable), appParams->zoomByMouseClickEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkNavigatorGoToSurface), appParams->goCloseToSurfaceEnabled);
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_step_forward), DoubleToString(appParams->cameraMoveStepRelative));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_NavigatorAbsoluteDistance), DoubleToString(appParams->cameraMoveStepAbsolute));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_step_rotation), DoubleToString(appParams->rotationStep));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mouse_click_distance), DoubleToString(appParams->mouseCloseUpRatio));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_netRenderClientPort), appParams->netRenderClientPort.c_str());
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_netRenderClientName), appParams->netRenderClientIP.c_str());
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_netRenderServerPort), appParams->netRenderServerPort.c_str());
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
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_alfa), DoubleToString(params->doubles.alpha * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_beta), DoubleToString(params->doubles.beta * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_gammaAngle), DoubleToString(params->doubles.gamma * 180.0 / M_PI));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_zoom), DoubleToString(params->doubles.zoom));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_persp), DoubleToString(params->doubles.persp));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_DE_stepFactor), DoubleToString(params->doubles.DE_factor));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_DE_thresh), DoubleToString(params->doubles.quality));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_roughness), DoubleToString(params->doubles.smoothness));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_maxN), IntToString(params->fractal.doubles.N));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_minN), IntToString(params->fractal.minN));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_power), DoubleToString(params->fractal.doubles.power));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_imageWidth), IntToString(params->image_width));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_imageHeight), IntToString(params->image_height));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_tiles), IntToString(params->noOfTiles));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_brightness), DoubleToString(params->doubles.imageAdjustments.brightness));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_gamma), DoubleToString(params->doubles.imageAdjustments.imageGamma));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_contrast), DoubleToString(params->doubles.imageAdjustments.contrast));
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
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_colour_saturation), DoubleToString(params->doubles.colourSaturation));
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
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre1x), DoubleToString(params->doubles.auxLightPre[0].x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre1y), DoubleToString(params->doubles.auxLightPre[0].y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre1z), DoubleToString(params->doubles.auxLightPre[0].z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre1intensity), DoubleToString(params->doubles.auxLightPreIntensity[0]));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre2x), DoubleToString(params->doubles.auxLightPre[1].x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre2y), DoubleToString(params->doubles.auxLightPre[1].y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre2z), DoubleToString(params->doubles.auxLightPre[1].z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre2intensity), DoubleToString(params->doubles.auxLightPreIntensity[1]));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre3x), DoubleToString(params->doubles.auxLightPre[2].x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre3y), DoubleToString(params->doubles.auxLightPre[2].y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre3z), DoubleToString(params->doubles.auxLightPre[2].z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre3intensity), DoubleToString(params->doubles.auxLightPreIntensity[2]));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre4x), DoubleToString(params->doubles.auxLightPre[3].x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre4y), DoubleToString(params->doubles.auxLightPre[3].y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre4z), DoubleToString(params->doubles.auxLightPre[3].z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_auxLightPre4intensity), DoubleToString(params->doubles.auxLightPreIntensity[3]));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mainLightAlfa), DoubleToString(params->doubles.mainLightAlpha * 180.0 / M_PI));
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
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_IFSEdgeX), DoubleToString(params->fractal.IFS.doubles.edge.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_IFSEdgeY), DoubleToString(params->fractal.IFS.doubles.edge.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_IFSEdgeZ), DoubleToString(params->fractal.IFS.doubles.edge.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_animationStartFrame), IntToString(params->startFrame));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_animationEndFrame), IntToString(params->endFrame));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_animationFramesPerKey), IntToString(params->framesPerKeyframe));
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
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxSolid), DoubleToString(params->fractal.mandelbox.doubles.solid));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxMelt), DoubleToString(params->fractal.mandelbox.doubles.melt));

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

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_reflectionsMax), IntToString(params->reflectionsMax));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxVaryFold), DoubleToString(params->fractal.mandelbox.doubles.vary4D.fold));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxVaryMinR), DoubleToString(params->fractal.mandelbox.doubles.vary4D.minR));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxVaryRPower), DoubleToString(params->fractal.mandelbox.doubles.vary4D.rPower));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxVaryScale), DoubleToString(params->fractal.mandelbox.doubles.vary4D.scaleVary));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_mandelboxVaryWAdd), DoubleToString(params->fractal.mandelbox.doubles.vary4D.wadd));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_cadd), DoubleToString(params->fractal.doubles.cadd));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricFogColorDistance), DoubleToString(params->doubles.fogColour1Distance));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricFogColorDistance2), DoubleToString(params->doubles.fogColour2Distance));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricFogDensity), DoubleToString(params->doubles.fogDensity));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_volumetricFogDistanceFact), DoubleToString(params->doubles.fogDistanceFactor));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_fastAoTune), DoubleToString(params->doubles.fastAoTune));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitivePlaneCentreX), DoubleToString(params->fractal.doubles.primitives.planeCentre.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitivePlaneCentreY), DoubleToString(params->fractal.doubles.primitives.planeCentre.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitivePlaneCentreZ), DoubleToString(params->fractal.doubles.primitives.planeCentre.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitivePlaneNormalX), DoubleToString(params->fractal.doubles.primitives.planeNormal.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitivePlaneNormalY), DoubleToString(params->fractal.doubles.primitives.planeNormal.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitivePlaneNormalZ), DoubleToString(params->fractal.doubles.primitives.planeNormal.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitivePlaneReflect), DoubleToString(params->doubles.primitivePlaneReflect));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveBoxCentreX), DoubleToString(params->fractal.doubles.primitives.boxCentre.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveBoxCentreY), DoubleToString(params->fractal.doubles.primitives.boxCentre.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveBoxCentreZ), DoubleToString(params->fractal.doubles.primitives.boxCentre.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveBoxSizeX), DoubleToString(params->fractal.doubles.primitives.boxSize.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveBoxSizeY), DoubleToString(params->fractal.doubles.primitives.boxSize.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveBoxSizeZ), DoubleToString(params->fractal.doubles.primitives.boxSize.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveBoxReflect), DoubleToString(params->doubles.primitiveBoxReflect));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxCentreX), DoubleToString(params->fractal.doubles.primitives.invertedBoxCentre.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxCentreY), DoubleToString(params->fractal.doubles.primitives.invertedBoxCentre.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxCentreZ), DoubleToString(params->fractal.doubles.primitives.invertedBoxCentre.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxSizeX), DoubleToString(params->fractal.doubles.primitives.invertedBoxSize.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxSizeY), DoubleToString(params->fractal.doubles.primitives.invertedBoxSize.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxSizeZ), DoubleToString(params->fractal.doubles.primitives.invertedBoxSize.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedBoxReflect), DoubleToString(params->doubles.primitiveInvertedBoxReflect));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveSphereCentreX), DoubleToString(params->fractal.doubles.primitives.sphereCentre.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveSphereCentreY), DoubleToString(params->fractal.doubles.primitives.sphereCentre.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveSphereCentreZ), DoubleToString(params->fractal.doubles.primitives.sphereCentre.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveSphereRadius), DoubleToString(params->fractal.doubles.primitives.sphereRadius));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveSphereReflect), DoubleToString(params->doubles.primitiveSphereReflect));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedSphereCentreX), DoubleToString(params->fractal.doubles.primitives.invertedSphereCentre.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedSphereCentreY), DoubleToString(params->fractal.doubles.primitives.invertedSphereCentre.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedSphereCentreZ), DoubleToString(params->fractal.doubles.primitives.invertedSphereCentre.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedSphereRadius), DoubleToString(params->fractal.doubles.primitives.invertedSphereRadius));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveInvertedSphereReflect), DoubleToString(params->doubles.primitiveInvertedSphereReflect));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveWaterHeight), DoubleToString(params->fractal.doubles.primitives.waterHeight));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveWaterLength), DoubleToString(params->fractal.doubles.primitives.waterLength));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveWaterAmplitude), DoubleToString(params->fractal.doubles.primitives.waterAmplitude));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveWaterRotation), DoubleToString(params->fractal.doubles.primitives.waterRotation));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveWaterIterations), IntToString(params->fractal.primitives.waterIterations));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveWaterReflect), DoubleToString(params->doubles.primitiveWaterReflect));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_primitiveWaterAnimSpeed), DoubleToString(params->fractal.doubles.primitives.waterAnimSpeed));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_iterFogOpacity), DoubleToString(params->doubles.iterFogOpacity));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_iterFogOpacityTrim), DoubleToString(params->doubles.iterFogOpacityTrim));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_fakeLightsIntensity), DoubleToString(params->doubles.fakeLightsIntensity));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_fakeLightsVisibility), DoubleToString(params->doubles.fakeLightsVisibility));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_fakeLightsVisibilitySize), DoubleToString(params->doubles.fakeLightsVisibilitySize));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_fakeLightsOrbitTrapX), DoubleToString(params->fractal.doubles.fakeLightsOrbitTrap.x));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_fakeLightsOrbitTrapY), DoubleToString(params->fractal.doubles.fakeLightsOrbitTrap.y));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_fakeLightsOrbitTrapZ), DoubleToString(params->fractal.doubles.fakeLightsOrbitTrap.z));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_fakeLightsMinIter), DoubleToString(params->fractal.fakeLightsMinIter));
	gtk_entry_set_text(GTK_ENTRY(Interface.edit_fakeLightsMaxIter), DoubleToString(params->fractal.fakeLightsMaxIter));

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_shadowConeAngle), DoubleToString(params->doubles.shadowConeAngle));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkAmbientOcclusion), params->global_ilumination);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkFastAmbientOcclusion), params->fastGlobalIllumination);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkShadow), params->shadow);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIterThresh), params->fractal.iterThresh);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkJulia), params->fractal.juliaMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkSlowShading), params->slowShading);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkLimits), params->fractal.limits_enabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkBitmapBackground), params->texturedBackground);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkBitmapBackgroundFulldome), params->background_as_fulldome);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkColoring), params->imageSwitches.coloringEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkTgladMode), params->fractal.tgladFoldingMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkSphericalFoldingMode), params->fractal.sphericalFoldingMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIFSFoldingMode), params->fractal.IFS.foldingMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkFogEnabled), params->imageSwitches.fogEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkSSAOEnabled), params->SSAOEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkDOFEnabled), params->DOFEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre1Enabled), params->auxLightPreEnabled[0]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre2Enabled), params->auxLightPreEnabled[1]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre3Enabled), params->auxLightPreEnabled[2]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkAuxLightPre4Enabled), params->auxLightPreEnabled[3]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsX), params->fractal.IFS.absX);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsY), params->fractal.IFS.absY);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIFSAbsZ), params->fractal.IFS.absZ);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIFSMengerSponge), params->fractal.IFS.mengerSpongeMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkHybridCyclic), params->fractal.hybridCyclic);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkStereoEnabled), params->stereoEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkMandelboxRotationsEnable), params->fractal.mandelbox.rotationsEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkInteriorMode), params->fractal.interiorMode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkDELinearMode), params->fractal.linearDEmode);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkConstantDEThreshold), params->fractal.constantDEThreshold);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightMainEnabled), params->volumetricLightEnabled[0]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux1Enabled), params->volumetricLightEnabled[1]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux2Enabled), params->volumetricLightEnabled[2]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux3Enabled), params->volumetricLightEnabled[3]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkVolumetricLightAux4Enabled), params->volumetricLightEnabled[4]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkPenetratingLights), params->penetratingLights);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkRaytracedReflections), params->imageSwitches.raytracedReflections);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitivePlaneEnabled), params->fractal.primitives.planeEnable);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveOnlyPlane), params->fractal.primitives.onlyPlane);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveBoxEnabled), params->fractal.primitives.boxEnable);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveInvertedBoxEnabled), params->fractal.primitives.invertedBoxEnable);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveSphereEnabled), params->fractal.primitives.sphereEnable);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveInvertedSphereEnabled), params->fractal.primitives.invertedSphereEnable);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkPrimitiveWaterEnabled), params->fractal.primitives.waterEnable);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkIterFogEnable), params->imageSwitches.iterFogEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkFakeLightsEnabled), params->fakeLightsEnabled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkHDR), params->imageSwitches.hdrEnabled);

	gtk_adjustment_set_value(GTK_ADJUSTMENT(Interface.adjustmentFogDepth), params->doubles.imageAdjustments.fogVisibility);
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

#ifdef CLSUPPORT

	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboOpenCLDEMode), params->fractal.customOCLFormulaDEMode);
	params->fractal.useCustomOCLFormula = false;
	if(params->fractal.formula == fractal::ocl_custom) params->fractal.useCustomOCLFormula = true;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkOpenClCustomEnable), params->fractal.useCustomOCLFormula);

	if (params->fractal.useCustomOCLFormula)
	{
		if (!clSupport->IsEnabled())
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Interface.checkOpenClEnable), true);
		}
		if (clSupport->customFormulas)
		{
			bool result = clSupport->customFormulas->SetActualByName(params->fractal.customOCLFormulaName);
			if(!result)
			{
				printf("Formula %s doesn't exists in your library\n", params->fractal.customOCLFormulaName);
				if (!noGUI)
				{
					GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL,
							"Formula %s doesn't exists in your library\n", params->fractal.customOCLFormulaName);
					gtk_dialog_run(GTK_DIALOG(dialog));
					gtk_widget_destroy(dialog);
				}
			}
			for (int i = 0; i < 15; i++)
				gtk_entry_set_text(GTK_ENTRY(Interface.edit_customParameters[i]), DoubleToString(params->fractal.doubles.customParameters[i]));
		}
	}

	gtk_entry_set_text(GTK_ENTRY(Interface.edit_OpenCLDeltaDEStep), DoubleToString(params->fractal.doubles.deltaDEStep));
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboOpenCLDOFMode), params->OpenCLDOFMethod);
#endif

	enumPerspectiveType perspTypeTemp = params->perspectiveType;
	if(params->fishEyeCut && perspTypeTemp == fishEye) perspTypeTemp = fishEyeCut;
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboPerspectiveType), perspTypeTemp);
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboGeneralizedFoldBoxType), params->fractal.genFoldBox.type);

	int formula = gtk_combo_box_get_active(GTK_COMBO_BOX(Interface.comboFractType));
	if (params->fractal.formula == fractal::trig_optim) formula = 0;
	if (params->fractal.formula == fractal::trig_DE) formula = 1;
	if (params->fractal.formula == fractal::fast_trig) formula = 2;
	if (params->fractal.formula == fractal::minus_fast_trig) formula = 3;
	if (params->fractal.formula == fractal::xenodreambuie) formula = 4;
	if (params->fractal.formula == fractal::hypercomplex) formula = 5;
	if (params->fractal.formula == fractal::quaternion) formula = 6;
	if (params->fractal.formula == fractal::menger_sponge) formula = 7;
	if (params->fractal.formula == fractal::tglad) formula = 8;
	if (params->fractal.formula == fractal::kaleidoscopic) formula = 9;
	if (params->fractal.formula == fractal::mandelbulb2) formula = 10;
	if (params->fractal.formula == fractal::mandelbulb3) formula = 11;
	if (params->fractal.formula == fractal::mandelbulb4) formula = 12;
	if (params->fractal.formula == fractal::foldingIntPow2) formula = 13;
	if (params->fractal.formula == fractal::smoothMandelbox) formula = 14;
	if (params->fractal.formula == fractal::mandelboxVaryScale4D) formula = 15;
	if (params->fractal.formula == fractal::aexion) formula = 16;
	if (params->fractal.formula == fractal::benesi) formula = 17;
	if (params->fractal.formula == fractal::bristorbrot) formula = 18;
	if (params->fractal.formula == fractal::hybrid) formula = 19;
	if (params->fractal.formula == fractal::generalizedFoldBox) formula = 20;
	gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboFractType), formula);

	for (int i = 0; i < HYBRID_COUNT; ++i)
		gtk_combo_box_set_active(GTK_COMBO_BOX(Interface.comboHybridFormula[i]), HybridFormulaNumberData2GUI(params->fractal.hybridFormula[i]));

	GdkColor color;
	color = sRGB2GdkColor(params->effectColours.glow_color1);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorGlow1), &color);
	color = sRGB2GdkColor(params->effectColours.glow_color2);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorGlow2), &color);
	color = sRGB2GdkColor(params->background_color1);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorBackgroud1), &color);
	color = sRGB2GdkColor(params->background_color2);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorBackgroud2), &color);
	color = sRGB2GdkColor(params->background_color3);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorBackgroud3), &color);
	color = sRGB2GdkColor(params->fogColour1);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorFog1), &color);
	color = sRGB2GdkColor(params->fogColour2);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorFog2), &color);
	color = sRGB2GdkColor(params->fogColour3);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorFog3), &color);
	color = sRGB2GdkColor(params->effectColours.fogColor);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorFog), &color);
	color = sRGB2GdkColor(params->auxLightPreColour[0]);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre1), &color);
	color = sRGB2GdkColor(params->auxLightPreColour[1]);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre2), &color);
	color = sRGB2GdkColor(params->auxLightPreColour[2]);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre3), &color);
	color = sRGB2GdkColor(params->auxLightPreColour[3]);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorAuxLightPre4), &color);
	color = sRGB2GdkColor(params->effectColours.mainLightColour);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorMainLight), &color);
	color = sRGB2GdkColor(params->primitivePlaneColour);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorPrimitivePlane), &color);
	color = sRGB2GdkColor(params->primitiveBoxColour);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorPrimitiveBox), &color);
	color = sRGB2GdkColor(params->primitiveInvertedBoxColour);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorPrimitiveInvertedBox), &color);
	color = sRGB2GdkColor(params->primitiveSphereColour);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorPrimitiveSphere), &color);
	color = sRGB2GdkColor(params->primitiveInvertedSphereColour);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorPrimitiveInvertedSphere), &color);
	color = sRGB2GdkColor(params->primitiveWaterColour);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(Interface.buColorPrimitiveWater), &color);

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
	gtk_combo_box_append_text(combo, "Bristorbrot");
	gtk_combo_box_append_text(combo, "Generalized Mandelbox Fold");
	gtk_combo_box_append_text(combo, "x^2/(x + p)");
	gtk_combo_box_append_text(combo, "y^2/(y + p)");
	gtk_combo_box_append_text(combo, "z^2/(z + p)");
	gtk_combo_box_append_text(combo, "r^2/(r + p)");
	gtk_combo_box_append_text(combo, "spherical fold (radius = p)");
	gtk_combo_box_append_text(combo, "x^p, y^p, z^p");
	gtk_combo_box_append_text(combo, "x * p");
	gtk_combo_box_append_text(combo, "y * p");
	gtk_combo_box_append_text(combo, "z * p");
	gtk_combo_box_append_text(combo, "x + p");
	gtk_combo_box_append_text(combo, "y + p");
	gtk_combo_box_append_text(combo, "z + p");
	gtk_combo_box_append_text(combo, "axis X angle multiply by p");
	gtk_combo_box_append_text(combo, "axis Y angle multiply by p");
	gtk_combo_box_append_text(combo, "axis Z angle multiply by p");
}

GtkWidget* CreateEditWithMap(std::string name, std::map<std::string, GtkWidget*> *map)
{
	GtkWidget *widget = gtk_entry_new();
	std::pair<std::map<std::string, GtkWidget*>::iterator, bool> ret;
	ret = map->insert(pair<std::string, GtkWidget*>(name, widget));
	if (ret.second == false)
	{
		std::cerr << "CreateEditWithMap(): element '" << name << "' already existed" << std::endl;
	}
	return widget;
}

GtkWidget* CreateCheckBoxWithMap(std::string name, std::string label, std::map<std::string, GtkWidget*> *map)
{
	GtkWidget *widget = gtk_check_button_new_with_label(label.c_str());
	std::pair<std::map<std::string, GtkWidget*>::iterator, bool> ret;
	ret = map->insert(pair<std::string, GtkWidget*>(name, widget));
	if (ret.second == false)
	{
		std::cerr << "CreateCheckBoxWithMap(): element '" << name << "' already existed" << std::endl;
	}
	return widget;
}

GtkWidget* CreateCheckBoxWithMapIndexed(std::string name, int index, std::string label, std::map<std::string, GtkWidget*> *map)
{
	char cname[256];
	sprintf(cname, "%s_%d", name.c_str(), index);
	std::string name2(cname);
	GtkWidget *widget = gtk_check_button_new_with_label(label.c_str());
	std::pair<std::map<std::string, GtkWidget*>::iterator, bool> ret;
	ret = map->insert(pair<std::string, GtkWidget*>(name2, widget));
	if (ret.second == false)
	{
		std::cerr << "CreateCheckBoxWithMapIndexed(): element '" << name2 << "' already existed" << std::endl;
	}
	return widget;
}

GtkWidget* CreateEditWithMapIndexed(std::string name, int index, std::map<std::string, GtkWidget*> *map)
{
	char cname[256];
	sprintf(cname, "%s_%d", name.c_str(), index);
	std::string name2(cname);
	GtkWidget *widget = gtk_entry_new();
	std::pair<std::map<std::string, GtkWidget*>::iterator, bool> ret;
	ret = map->insert(pair<std::string, GtkWidget*>(name2, widget));
	if (ret.second == false)
	{
		std::cerr << "CreateEditWithMapIndexed(): element '" << name2 << "' already existed" << std::endl;
	}
	return widget;
}

void AddComboBoxToMap(std::string name, GtkWidget* combo, std::map<std::string, GtkWidget*> *map)
{
	std::pair<std::map<std::string, GtkWidget*>::iterator, bool> ret;
	ret = map->insert(pair<std::string, GtkWidget*>(name, combo));
	if (ret.second == false)
	{
		std::cerr << "AddComboBoxToMap(): element '" << name << "' already existed" << std::endl;
	}
}

void AddComboBoxToMapIndexed(std::string name, int index, GtkWidget* combo, std::map<std::string, GtkWidget*> *map)
{
	char cname[256];
	sprintf(cname, "%s_%d", name.c_str(), index);
	std::string name2(cname);
	std::pair<std::map<std::string, GtkWidget*>::iterator, bool> ret;
	ret = map->insert(pair<std::string, GtkWidget*>(name2, combo));
	if (ret.second == false)
	{
		std::cerr << "AddComboBoxToMapIndexed(): element '" << name2 << "' already existed" << std::endl;
	}
}

sGtkEditVector3 CreateEditVector3WithMap(std::string name, std::map<std::string, sGtkEditVector3> *map)
{
	sGtkEditVector3 widget3;
	widget3.x = gtk_entry_new();
	widget3.y = gtk_entry_new();
	widget3.z = gtk_entry_new();

	std::pair<std::map<std::string, sGtkEditVector3>::iterator, bool> ret;
	ret = map->insert(pair<std::string, sGtkEditVector3>(name, widget3));
	if (ret.second == false)
	{
		std::cerr << "CreateEditVector3WithMap(): element '" << name << "' already existed" << std::endl;
	}
	return widget3;
}

sGtkEditVector3 CreateEditVector3WithMapIndexed(std::string name, int index, std::map<std::string, sGtkEditVector3> *map)
{
	char cname[256];
	sprintf(cname, "%s_%d", name.c_str(), index);
	std::string name2(cname);
	sGtkEditVector3 widget3;
	widget3.x = gtk_entry_new();
	widget3.y = gtk_entry_new();
	widget3.z = gtk_entry_new();

	std::pair<std::map<std::string, sGtkEditVector3>::iterator, bool> ret;
	ret = map->insert(pair<std::string, sGtkEditVector3>(name2, widget3));
	if (ret.second == false)
	{
		std::cerr << "CreateEditVector3WithMapIndexed(): element '" << name2 << "' already existed" << std::endl;
	}
	return widget3;
}

GtkWidget* CreateColorButtonWithMap(std::string name, std::string label, std::map<std::string, GtkWidget*> *map)
{
	GtkWidget *widget = gtk_color_button_new();
	gtk_color_button_set_title(GTK_COLOR_BUTTON(widget), label.c_str());

	std::pair<std::map<std::string, GtkWidget*>::iterator, bool> ret;
	ret = map->insert(pair<std::string, GtkWidget*>(name, widget));
	if (ret.second == false)
	{
		std::cerr << "CreateColorButtonWithMap(): element '" << name << "' already existed" << std::endl;
	}
	return widget;
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
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Set Julia constant");
	gtk_combo_box_append_text(GTK_COMBO_BOX(renderWindow.comboMouseClickMode), "Measure");
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
	Interface.tab_label_engine = gtk_label_new("Engine");
	Interface.tab_label_fractal = gtk_label_new("Fractal");
	Interface.tab_label_hybrid = gtk_label_new("Hybrid");
	Interface.tab_label_mandelbox = gtk_label_new("Mandelbox");
	Interface.tab_label_shaders = gtk_label_new("Shaders 1");
	Interface.tab_label_image = gtk_label_new("Image");
	Interface.tab_label_animation = gtk_label_new("Animation");
	Interface.tab_label_shaders2 = gtk_label_new("Shaders 2");
	Interface.tab_label_lights = gtk_label_new("Lights");
	Interface.tab_label_IFS = gtk_label_new("IFS");
	Interface.tab_label_openCL = gtk_label_new("OpenCL");
	Interface.tab_label_about = gtk_label_new("About...");

	Interface.tab_box_view = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_engine = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_fractal = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_shaders = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_image = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_animation = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_shaders2 = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_about = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_lights = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_IFS = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_hybrid = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_mandelbox = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_openCL = gtk_vbox_new(FALSE, 1);

	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_view), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_engine), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_fractal), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_shaders), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_image), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_animation), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_shaders2), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_lights), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_IFS), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_about), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_hybrid), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_openCL), 5);

	Interface.tabsPrimitives = gtk_notebook_new();
	Interface.tab_label_primitivePlane = gtk_label_new("Plane");
	Interface.tab_label_primitiveWater = gtk_label_new("Water");
	Interface.tab_label_primitiveBox = gtk_label_new("Box");
	Interface.tab_label_primitiveBoxInv = gtk_label_new("Inverted Box");
	Interface.tab_label_primitiveSphere = gtk_label_new("Sphere");
	Interface.tab_label_primitiveSphereInv = gtk_label_new("Inverted sphere");

	Interface.tab_box_primitivePlane = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_primitiveWater = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_primitiveBox = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_primitiveBoxInv = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_primitiveSphere = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_primitiveSphereInv = gtk_vbox_new(FALSE, 1);

	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_primitivePlane), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_primitiveWater), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_primitiveBox), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_primitiveBoxInv), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_primitiveSphere), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_primitiveSphereInv), 5);

	Interface.tabsNetRender = gtk_notebook_new();
	Interface.tab_label_server = gtk_label_new("Server");
	Interface.tab_label_client = gtk_label_new("Client");

	Interface.tab_box_server = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_client = gtk_vbox_new(FALSE, 1);

	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_server), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_client), 5);

	Interface.tabsOpenCL = gtk_notebook_new();

	Interface.tab_label_openclEngine = gtk_label_new("Engine");
	Interface.tab_label_openclCustom = gtk_label_new("Custom formulas");

	Interface.tab_box_openclEngine = gtk_vbox_new(FALSE, 1);
	Interface.tab_box_openclCustom = gtk_vbox_new(FALSE, 1);

	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_openclEngine), 5);
	gtk_container_set_border_width(GTK_CONTAINER(Interface.tab_box_openclCustom), 5);

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
	Interface.boxIFSEdge = gtk_hbox_new(FALSE, 1);
	Interface.boxKeyframeAnimation = gtk_vbox_new(FALSE, 1);
	Interface.boxKeyframeAnimationButtons = gtk_hbox_new(FALSE, 1);
	Interface.boxKeyframeAnimationButtons2 = gtk_hbox_new(FALSE, 1);
	Interface.boxKeyframeAnimationEdits = gtk_hbox_new(FALSE, 1);
	Interface.boxBottomKeyframeAnimation = gtk_hbox_new(FALSE, 1);
	Interface.boxPalette = gtk_vbox_new(FALSE, 1);
	Interface.boxPaletteOffset = gtk_hbox_new(FALSE, 1);
	Interface.boxImageSaving = gtk_vbox_new(FALSE, 1);
	Interface.boxImageAutoSave = gtk_hbox_new(FALSE, 1);
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
	Interface.boxVolumetricFog = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitives = gtk_vbox_new(FALSE, 1);
	Interface.boxPrimitivePlane = gtk_vbox_new(FALSE, 1);
	Interface.boxPrimitivePlane1 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitivePlane2 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitiveBox = gtk_vbox_new(FALSE, 1);
	Interface.boxPrimitiveBox1 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitiveBox2 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitiveInvertedBox = gtk_vbox_new(FALSE, 1);
	Interface.boxPrimitiveInvertedBox1 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitiveInvertedBox2 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitiveSphere = gtk_vbox_new(FALSE, 1);
	Interface.boxPrimitiveSphere1 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitiveSphere2 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitiveInvertedSphere = gtk_vbox_new(FALSE, 1);
	Interface.boxPrimitiveInvertedSphere1 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitiveInvertedSphere2 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitiveWater = gtk_vbox_new(FALSE, 1);
	Interface.boxPrimitiveWater1 = gtk_hbox_new(FALSE, 1);
	Interface.boxPrimitiveWater2 = gtk_hbox_new(FALSE, 1);
	Interface.boxMeasure = gtk_vbox_new(FALSE, 1);
	Interface.boxMeasure1 = gtk_hbox_new(FALSE, 1);
	Interface.boxOpenClSettings = gtk_vbox_new(FALSE, 1);
	Interface.boxOpenClSwitches1 = gtk_hbox_new(FALSE, 1);
	Interface.boxOpenClInformation = gtk_vbox_new(FALSE, 1);
	Interface.boxOpenClEngineSettingsV = gtk_vbox_new(FALSE, 1);
	Interface.boxOpenClEngineSettingsH1 = gtk_hbox_new(FALSE, 1);
	Interface.boxOpenClEngineSettingsH2 = gtk_hbox_new(FALSE, 1);
	Interface.boxOpenClEngineSettingsH3 = gtk_hbox_new(FALSE, 1);
	Interface.boxOpenClEngineSettingsH4 = gtk_hbox_new(FALSE, 1);
	Interface.boxOpenClCustomV1 = gtk_vbox_new(FALSE, 1);
	Interface.boxOpenClCustomH11 = gtk_hbox_new(FALSE, 1);
	Interface.boxOpenClCustomH12 = gtk_hbox_new(FALSE, 1);
	Interface.boxOpenClCustomH13 = gtk_hbox_new(FALSE, 1);
	Interface.boxOpenClCustomV2 = gtk_vbox_new(FALSE, 1);

	Interface.boxNetRenderClientV = gtk_vbox_new(FALSE, 1);
	Interface.boxNetRenderClientH1 = gtk_hbox_new(FALSE, 1);
	Interface.boxNetRenderServerV = gtk_vbox_new(FALSE, 1);
	Interface.boxNetRenderServerH1 = gtk_hbox_new(FALSE, 1);
	Interface.boxFakeLightsV = gtk_vbox_new(FALSE, 1);
	Interface.boxFakeLightsH1 = gtk_hbox_new(FALSE, 1);
	Interface.boxFakeLightsH2 = gtk_hbox_new(FALSE, 1);
	Interface.boxImageAdjustmentsV = gtk_vbox_new(FALSE, 1);
	Interface.boxImageAdjustmentsH1 = gtk_hbox_new(FALSE, 1);
	Interface.boxShadersSurfaceV = gtk_vbox_new(FALSE, 1);
	Interface.boxShadersSurfaceH1 = gtk_hbox_new(FALSE, 1);
	Interface.boxShadersSurfaceH2 = gtk_hbox_new(FALSE, 1);
	Interface.boxShadersSurfaceH3 = gtk_hbox_new(FALSE, 1);
	Interface.boxShadersSurfaceH4 = gtk_hbox_new(FALSE, 1);
	Interface.boxShadersVolumetricV = gtk_vbox_new(FALSE, 1);
	Interface.boxShadersVolumetricH1 = gtk_hbox_new(FALSE, 1);
	Interface.boxShadersVolumetricH2 = gtk_hbox_new(FALSE, 1);
	Interface.boxShadersVolumetricH3 = gtk_hbox_new(FALSE, 1);
	Interface.boxShadersVolumetricH4 = gtk_hbox_new(FALSE, 1);

	//tables
	Interface.tableLimits = gtk_table_new(2, 3, false);
	Interface.tableArrows = gtk_table_new(3, 3, false);
	Interface.tableArrows2 = gtk_table_new(3, 3, false);
	Interface.tableIFSParams = gtk_table_new(9, 9, false);
	Interface.tableHybridParams = gtk_table_new(5, 3, false);
	Interface.tableMandelboxRotations = gtk_table_new(5, 7, false);
	Interface.tableOpenCLCustom = gtk_table_new(5, 3, false);

	//frames
	Interface.frCoordinates = gtk_frame_new("Viewpoint coordinates");
	Interface.fr3Dnavigator = gtk_frame_new("3D Navigator");
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
	Interface.frPostFog = gtk_frame_new("Simple fog");
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
	Interface.frPalette = gtk_frame_new("Colour palette (click on colour palette to edit)");
	Interface.frImageSaving = gtk_frame_new("Image saving");
	Interface.frHybrid = gtk_frame_new("Hybrid formula");
	Interface.frStereo = gtk_frame_new("Stereoscopic rendering");
	Interface.frMandelboxMainParams = gtk_frame_new("Main Mandelbox parameters");
	Interface.frMandelboxRotations = gtk_frame_new("Rotation of Mandelbox folding planes");
	Interface.frMandelboxColoring = gtk_frame_new("Mandelbox colouring parameters");
	Interface.frVolumetricLight = gtk_frame_new("Volumetric light");
	Interface.frMandelboxVary = gtk_frame_new("Mandelbox vary scale 4D");
	Interface.frPrimitives = gtk_frame_new("Primitive shapes");
	Interface.frPrimitivePlane = gtk_frame_new("Plane");
	Interface.frPrimitiveBox = gtk_frame_new("Box");
	Interface.frPrimitiveInvertedBox = gtk_frame_new("Inverted box");
	Interface.frPrimitiveSphere = gtk_frame_new("Sphere");
	Interface.frPrimitiveInvertedSphere = gtk_frame_new("Inverted sphere");
	Interface.frPrimitiveWater = gtk_frame_new("Water");
	Interface.frMeasure = gtk_frame_new("Coordinate measurement");
	Interface.frOpenClSettings = gtk_frame_new("OpenCL kernel");
	Interface.frOpenClInformation = gtk_frame_new("OpenCL information");
	Interface.frOpenClEngineSettings = gtk_frame_new("OpenCL engine settings");
	Interface.frNetRender = gtk_frame_new("Rendering via network");
	Interface.frFakeLights = gtk_frame_new("Fake lights based on orbit traps");
	Interface.frImageAdjustments = gtk_frame_new("Image adjustments");
	Interface.frShadersSurface = gtk_frame_new("Surface");
	Interface.frShadersVolumetric = gtk_frame_new("Volumetric");
	Interface.frOpenClCustomSelection = gtk_frame_new("Custom formula management");
	Interface.frOpenClCustomParams = gtk_frame_new("Custom parameters for custom formula");

	//separators
	Interface.hSeparator1 = gtk_hseparator_new();
	Interface.vSeparator1 = gtk_vseparator_new();


	//edits
	Interface.edit_measure.x = gtk_entry_new();
	Interface.edit_measure.y = gtk_entry_new();
	Interface.edit_measure.z = gtk_entry_new();

	//buttons
	Interface.buRender = gtk_button_new_with_label("RENDER");
	Interface.buStop = gtk_button_new_with_label("STOP");
	Interface.buApplyImageAdjustments = gtk_button_new_with_label("Apply changes");
	Interface.buSaveImage = gtk_button_new_with_label("Save JPG");
	Interface.buSavePNG = gtk_button_new_with_label("Save PNG");
	Interface.buSavePNG16 = gtk_button_new_with_label("Save PNG 16-bit");
	Interface.buSavePNG16Alpha = gtk_button_new_with_label("Save PNG 16-bit + Alpha");
	Interface.buFiles = gtk_button_new_with_label("Select file paths (output images, textures)");
	Interface.buLoadSettings = gtk_button_new_with_label("Load Settings");
	Interface.buSaveSettings = gtk_button_new_with_label("Save Settings");
	Interface.buUp = gtk_button_new();
	Interface.buDown = gtk_button_new();
	Interface.buLeft = gtk_button_new();
	Interface.buRight = gtk_button_new();
	Interface.buRotateLeft = gtk_button_new();
	Interface.buRotateRight = gtk_button_new();
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
	Interface.buUpdateSSAO = gtk_button_new_with_label("Update image");
	Interface.buUpdateDOF = gtk_button_new_with_label("Update DOF");
	Interface.buDistributeLights = gtk_button_new_with_label("Distribute / update lights");
	Interface.buIFSNormalizeOffset = gtk_button_new_with_label("Normalize offset vector");
	Interface.buIFSNormalizeVectors = gtk_button_new_with_label("Normalize symmetry vectors");
	Interface.buAnimationRecordKey = gtk_button_new_with_label("Record key-frame");
	Interface.buAnimationRenderFromKeys = gtk_button_new_with_label("Render from key-frames");
	Interface.buUndo = gtk_button_new_with_label("Undo");
	Interface.buRedo = gtk_button_new_with_label("Redo");
	Interface.buBuddhabrot = gtk_button_new_with_label("Render Buddhabrot");
	Interface.buRandomPalette = gtk_button_new_with_label("Random");
	Interface.buGetPaletteFromImage = gtk_button_new_with_label("Get palette from JPG");
	Interface.buTimeline = gtk_button_new_with_label("Timeline");
	Interface.buIFSDefaultDodeca = gtk_button_new_with_label("Dodecahedron");
	Interface.buIFSDefaultIcosa = gtk_button_new_with_label("Icosahedron");
	Interface.buIFSDefaultOcta = gtk_button_new_with_label("Octahedron");
	Interface.buIFSDefaultMengerSponge = gtk_button_new_with_label("Menger sponge");
	Interface.buIFSReset = gtk_button_new_with_label("Reset vectors");
	Interface.buAutoDEStep = gtk_button_new_with_label("LQ");
	Interface.buAutoDEStepHQ = gtk_button_new_with_label("HQ");
	Interface.buCopyToClipboard = gtk_button_new_with_label("Copy to clipboard");
	Interface.buGetFromClipboard = gtk_button_new_with_label("Paste from clipboard");
	Interface.buLoadExample = gtk_button_new_with_label("Load example");
	Interface.buAutoFog = gtk_button_new_with_label("Auto fog");
	Interface.buMeasureActivation = gtk_button_new_with_label("Activate measurement");
	Interface.buSaveAllImageLayers = gtk_button_new_with_label("Save all layers");
	Interface.buOpenCLNewFormula = gtk_button_new_with_label("New");
	Interface.buOpenCLEditFormula = gtk_button_new_with_label("Edit formula");
	Interface.buOpenCLEditFormulaInit = gtk_button_new_with_label("Edit formula Init");
	Interface.buOpenCLDeleteFormula = gtk_button_new_with_label("Delete");
	Interface.buOpenCLRecompile = gtk_button_new_with_label("Recompile");
	Interface.buConvertPathToKeyframes = gtk_button_new_with_label("Convert flight path to keyframes");

	//combo
	//		fract type
	GtkWidget *comboFractType = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Mandelbulb");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Mandelbulb - Daniel White's");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Polynomic power 2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Polynomic power 2 - minus z");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Xenodreambuie's formula");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Hypercomplex");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Quaternion");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Menger sponge");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Tglad's formula (Mandelbox)");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Kaleidoscopic IFS");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Modified Mandelbulb 1");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Modified Mandelbulb 2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Modified Mandelbulb 3");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "FoldingIntPower2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Smooth Mandelbox");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Mandelbox vary scale 4D");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Aexion");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Benesi");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Bristorbrot");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Hybrid");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboFractType), "Generalized Mandelbox Fold");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboFractType), 1);
	AddComboBoxToMap("formula_combo", comboFractType, &mapInterface);

	//image format
	GtkWidget *comboImageFormat = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboImageFormat), "JPEG");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboImageFormat), "PNG 8-bit");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboImageFormat), "PNG 16-bit");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboImageFormat), "PNG 16-bit with alpha channel");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboImageFormat), 0);
	AddComboBoxToMap("save_image_format", comboImageFormat, &mapAppParams);

	GtkWidget *comboPerspectiveType = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboPerspectiveType), "Three-point perspective");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboPerspectiveType), "Fish eye");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboPerspectiveType), "Equirectangular projection");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboPerspectiveType), "Fish eye 180 degree cut");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboPerspectiveType), 0);
	AddComboBoxToMap("perspective_type", comboPerspectiveType, &mapInterface);

	GtkWidget *comboImageProportion = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboImageProportion), "Free");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboImageProportion), "1:1");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboImageProportion), "5:4");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboImageProportion), "4:3");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboImageProportion), "16:10");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboImageProportion), "16:9");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboImageProportion), 0);
	AddComboBoxToMap("image_proportion", comboImageProportion, &mapInterface);

	GtkWidget *comboGeneralizedFoldBoxType = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboGeneralizedFoldBoxType), "Tetrahedron");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboGeneralizedFoldBoxType), "Cube");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboGeneralizedFoldBoxType), "Octahedron");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboGeneralizedFoldBoxType), "Dodecahedron");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboGeneralizedFoldBoxType), "Octahedron/Cube");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboGeneralizedFoldBoxType), "Icosahedron");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboGeneralizedFoldBoxType), "Box 6");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboGeneralizedFoldBoxType), "Box 5");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboGeneralizedFoldBoxType), 0);
	AddComboBoxToMap("mandelbox_fold_mode", comboGeneralizedFoldBoxType, &mapInterface);

#ifdef CLSUPPORT
	GtkWidget *comboOpenCLEngine = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLEngine), "Fast: no effects");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLEngine), "Normal: shadows, glow, fast AO");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLEngine), "Full: all shaders");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLEngine), "No DE");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboOpenCLEngine), 0);
	AddComboBoxToMap("openCL_engine", comboOpenCLEngine, &mapAppParams);

	GtkWidget *comboOpenCLDeviceIndex = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLDeviceIndex), "0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLDeviceIndex), "1");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLDeviceIndex), "2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLDeviceIndex), "3");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboOpenCLDeviceIndex), 0);
	AddComboBoxToMap("openCL_device_index", comboOpenCLDeviceIndex, &mapAppParams);

	GtkWidget *comboOpenCLPlatformIndex = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLPlatformIndex), "0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLPlatformIndex), "1");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLPlatformIndex), "2");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLPlatformIndex), "3");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboOpenCLPlatformIndex), 0);
	AddComboBoxToMap("openCL_platform_index", comboOpenCLPlatformIndex, &mapAppParams);

	GtkWidget *comboOpenCLGPUCPU = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLGPUCPU), "GPU");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLGPUCPU), "CPU");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboOpenCLGPUCPU), 0);
	AddComboBoxToMap("openCL_use_CPU", comboOpenCLGPUCPU, &mapAppParams);

	GtkWidget *comboOpenCLCustomFormulas = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLCustomFormulas), "Enable OpenCL to get list");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboOpenCLCustomFormulas), 0);
	AddComboBoxToMap("ocl_custom_formula_name", comboOpenCLCustomFormulas, &mapInterface);

	GtkWidget *comboOpenCLDEMode = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLDEMode), "Calculated DE in formula");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLDEMode), "Delta DE");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboOpenCLDEMode), 0);
	AddComboBoxToMap("ocl_custom_DE_mode", comboOpenCLDEMode, &mapInterface);

	GtkWidget *comboOpenCLDOFMode = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLDOFMode), "Post effect");
	gtk_combo_box_append_text(GTK_COMBO_BOX(comboOpenCLDOFMode), "Monte Carlo");
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboOpenCLDEMode), 0);
	AddComboBoxToMap("ocl_DOF_method", comboOpenCLDOFMode, &mapInterface);

#endif
	//progress bar
	Interface.progressBar = gtk_progress_bar_new();

	//checkboxes

	Interface.checkOpenClEnable = gtk_check_button_new_with_label("OpenCL Enable");
	Interface.checkNetRenderClientEnable = gtk_check_button_new_with_label("Client enable");
	Interface.checkNetRenderServerEnable = gtk_check_button_new_with_label("Server enable");
	Interface.checkNetRenderServerScan = gtk_check_button_new_with_label("Scan for clients");

	//pixamps
	Interface.pixmap_up = gtk_image_new_from_file((string(sharedDir)+"icons/go-up.png").c_str());
	Interface.pixmap_down = gtk_image_new_from_file((string(sharedDir)+"icons/go-down.png").c_str());
	Interface.pixmap_left = gtk_image_new_from_file((string(sharedDir)+"icons/go-previous.png").c_str());
	Interface.pixmap_right = gtk_image_new_from_file((string(sharedDir)+"icons/go-next.png").c_str());
	Interface.pixmap_rotate_left = gtk_image_new_from_file((string(sharedDir)+"icons/object-rotate-left.png").c_str());
	Interface.pixmap_rotate_right = gtk_image_new_from_file((string(sharedDir)+"icons/object-rotate-right.png").c_str());
	Interface.pixmap_move_up = gtk_image_new_from_file((string(sharedDir)+"icons/go-up.png").c_str());
	Interface.pixmap_move_down = gtk_image_new_from_file((string(sharedDir)+"icons/go-down.png").c_str());
	Interface.pixmap_move_left = gtk_image_new_from_file((string(sharedDir)+"icons/go-previous.png").c_str());
	Interface.pixmap_move_right = gtk_image_new_from_file((string(sharedDir)+"icons/go-next.png").c_str());

	//labels
	Interface.label_animationFrame = gtk_label_new("Frame:");
	Interface.label_animationDistance = gtk_label_new("Estimated distance to fractal:");
	Interface.label_animationSpeed = gtk_label_new("Flight speed:");
	Interface.label_keyframeInfo = gtk_label_new("Frame: ,keyframe: ");
	Interface.label_fog_visibility = gtk_label_new("Visibility:");
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
	Interface.label_DE_threshold = gtk_label_new("Detail level:");
	Interface.label_measureDistance = gtk_label_new("Distance from last point:");
	Interface.label_OpenClComputingUnits = gtk_label_new("");
	Interface.label_OpenClMaxClock = gtk_label_new("");
	Interface.label_OpenClMaxWorkgroup = gtk_label_new("");
	Interface.label_OpenClMemorySize = gtk_label_new("");
	Interface.label_OpenClPlatformBy = gtk_label_new("");
	Interface.label_OpenClStatus = gtk_label_new("");
	Interface.label_OpenClWorkgroupSize = gtk_label_new("");
	Interface.label_serverStatus = gtk_label_new("status: not enabled");
	Interface.label_clientStatus = gtk_label_new("status: not enabled");
	Interface.label_sliderFog = gtk_label_new("visibility = ");
	Interface.label_sliderDOF = gtk_label_new("fog focus distance = ");

	for (int i = 1; i <= HYBRID_COUNT; ++i)
		Interface.label_HybridFormula[i-1] = gtk_label_new(g_strdup_printf("Formula #%d:", i));

	Interface.label_NavigatorEstimatedDistance = gtk_label_new("Estimated distance to the surface:");

	Interface.label_about = gtk_label_new("Mandelbulber "MANDELBULBER_VERSION_STR"\n"
		"author: Krzysztof Marczak\n"
		"Licence: GNU GPL v3\n"
		"www: www.mandelbulber.com\n"
		"thanks to: Knighty, Makemeunsee, Marius Schilder, Ryan Hitchman, Jeff Epler, Martin Reinecke, Quazgaa"
	);

	//sliders
	Interface.adjustmentFogDepth = gtk_adjustment_new(30.0, 0.1, 200.0, 0.1, 10.0, 0.1);
	Interface.sliderFogDepth = gtk_hscale_new(GTK_ADJUSTMENT(Interface.adjustmentFogDepth));
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
	sGtkEditVector3 edit_v = CreateEditVector3WithMap("view_point", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxCoordinates), CreateEdit("0,0", "x:", 20, edit_v.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxCoordinates), CreateEdit("0,0", "y:", 20, edit_v.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxCoordinates), CreateEdit("0,0", "z:", 20, edit_v.z), false, false, 1);

	//		box angle
	gtk_box_pack_start(GTK_BOX(Interface.boxView), Interface.boxAngle, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAngle), CreateEdit("0,0", "alpha (yaw):", 15, CreateEditWithMap("angle_alpha", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAngle), CreateEdit("0,0", "beta (pitch):", 15, CreateEditWithMap("angle_beta", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAngle), CreateEdit("0,0", "gamma (roll):", 15, CreateEditWithMap("angle_gamma", &mapInterface)), false, false, 1);

	//		box zoom
	gtk_box_pack_start(GTK_BOX(Interface.boxView), Interface.boxZoom, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxZoom), CreateEdit("2,5", "Close up (zoom):", 20, CreateEditWithMap("zoom", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxZoom), CreateEdit("0,5", "perspective (FOV):", 5, CreateEditWithMap("fov", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxZoom), CreateWidgetWithLabel("Perspective projection:", comboPerspectiveType), false, false, 1);

	//buttons arrows
	gtk_container_add(GTK_CONTAINER(Interface.buUp), Interface.pixmap_up);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows), Interface.buUp, 1, 2, 0, 1);
	gtk_container_add(GTK_CONTAINER(Interface.buDown), Interface.pixmap_down);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows), Interface.buDown, 1, 2, 2, 3);
	gtk_container_add(GTK_CONTAINER(Interface.buLeft), Interface.pixmap_left);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows), Interface.buLeft, 0, 1, 1, 2);
	gtk_container_add(GTK_CONTAINER(Interface.buRight), Interface.pixmap_right);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows), Interface.buRight, 2, 3, 1, 2);
	gtk_container_add(GTK_CONTAINER(Interface.buRotateLeft), Interface.pixmap_rotate_left);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows), Interface.buRotateLeft, 0, 1, 2, 3);
	gtk_container_add(GTK_CONTAINER(Interface.buRotateRight), Interface.pixmap_rotate_right);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableArrows), Interface.buRotateRight, 2, 3, 2, 3);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_view), Interface.fr3Dnavigator, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.fr3Dnavigator), Interface.boxArrows);
	gtk_box_pack_start(GTK_BOX(Interface.boxArrows), Interface.boxArrows2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxArrows2), Interface.tableArrows, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxArrows2), CreateCheckBoxWithMap("camera_straight_rotation", "Rotation without\nusing gamma\nangle", &mapAppParams), false, false, 1);

	//navigation
	gtk_box_pack_start(GTK_BOX(Interface.boxArrows), Interface.boxNavigation, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), Interface.buInitNavigator, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), Interface.boxNavigationButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigationButtons), Interface.buForward, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigationButtons), Interface.buBackward, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), CreateEdit(DoubleToString(0.5), "Step for camera moving multiplied by DE:", 5, CreateEditWithMap("camera_movenent_step_de", &mapAppParams)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), CreateCheckBoxWithMap("camera_absolute_distance_mode", "Absolute distance mode", &mapAppParams), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), CreateEdit(DoubleToString(0.1), "Absolute movement distance:", 10, CreateEditWithMap("camera_movenent_step_absolute", &mapAppParams)), false, false,
			1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), CreateEdit(DoubleToString(10.0), "Rotation step in degrees", 5, CreateEditWithMap("camera_rotation_step", &mapAppParams)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), CreateEdit(DoubleToString(3.0), "Mouse click close-up ratio", 5, CreateEditWithMap("camera_mouse_click_step", &mapAppParams)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigation), Interface.boxNavigationZooming, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigationZooming), CreateCheckBoxWithMap("camera_zoom_by_click_mode", "Enable zoom by mouse click", &mapAppParams), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNavigationZooming), CreateCheckBoxWithMap("camera_go_to_surface_mode", "Go close to indicated surface", &mapAppParams), false, false, 1);
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

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_view), Interface.frMeasure, true, true, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frMeasure), Interface.boxMeasure);
	gtk_box_pack_start(GTK_BOX(Interface.boxMeasure), Interface.boxMeasure1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMeasure1), Interface.buMeasureActivation, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMeasure1), CreateEdit("0", "X:", 20, Interface.edit_measure.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMeasure1), CreateEdit("0", "Y:", 20, Interface.edit_measure.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMeasure1), CreateEdit("0", "Z:", 20, Interface.edit_measure.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMeasure), Interface.label_measureDistance, false, false, 1);

	//	frame fractal
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_fractal), Interface.frFractalFormula, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frFractalFormula), Interface.boxFractalFormula);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFormula), CreateWidgetWithLabel("Fractal formula type:", comboFractType), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFormula), Interface.boxJulia, false, false, 1);
	sGtkEditVector3 edit_julia = CreateEditVector3WithMap("julia_c", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxJulia), CreateEdit("0,0", "Julia x:", 20, edit_julia.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxJulia), CreateEdit("0,0", "Julia y:", 20, edit_julia.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxJulia), CreateEdit("0,0", "Julia z:", 20, edit_julia.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxJulia), CreateCheckBoxWithMap("julia_mode", "Julia mode", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFormula), Interface.boxFractalPower, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalPower), CreateEdit("8,0", "power:", 5, CreateEditWithMap("power", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalPower), CreateEdit("1,0", "Fractal constant factor:", 5, CreateEditWithMap("fractal_constant_factor", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalPower), CreateEdit("0,0", "c add:", 5, CreateEditWithMap("c_add", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_fractal), Interface.frFractalFoldingIntPow, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frFractalFoldingIntPow), Interface.boxFractalFoldingIntPow);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFoldingIntPow), CreateEdit("2,0", "Cubic folding factor:", 5, CreateEditWithMap("foldingIntPow_folding_factor", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFoldingIntPow), CreateEdit("10,0", "Z factor:", 5, CreateEditWithMap("foldingIntPow_z_factor", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_fractal), Interface.frFractalFolding, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frFractalFolding), Interface.boxFractalFolding);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFolding), Interface.boxTgladFolding, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxTgladFolding), CreateCheckBoxWithMap("tglad_folding_mode", "Tglad's folding mode      ", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxTgladFolding), CreateEdit("1,0", "Folding limit:", 5, CreateEditWithMap("folding_limit", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxTgladFolding), CreateEdit("2,0", "Folding value:", 5, CreateEditWithMap("folding_value", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFolding), Interface.boxSphericalFolding, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSphericalFolding), CreateCheckBoxWithMap("spherical_folding_mode", "Spherical folding mode      ", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSphericalFolding), CreateEdit("1,0", "Fixed radius:", 5, CreateEditWithMap("spherical_folding_fixed", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSphericalFolding), CreateEdit("0,5", "Min. radius:", 5, CreateEditWithMap("spherical_folding_min", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalFolding), CreateCheckBoxWithMap("IFS_folding_mode", "Kaleidoscopic IFS folding mode (parameters on Kaleidoscopic IFS tab)", &mapInterface), false, false, 1);

	//primitives
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_fractal), Interface.frPrimitives, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPrimitives), Interface.boxPrimitives);

	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitives), Interface.tabsPrimitives, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_primitivePlane), Interface.frPrimitivePlane, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPrimitivePlane), Interface.boxPrimitivePlane);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane), Interface.boxPrimitivePlane1, false, false, 1);
	sGtkEditVector3 edit_primitivePlaneCentre = CreateEditVector3WithMap("primitive_plane_centre", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane1), CreateEdit("0.0", "Centre x:", 10, edit_primitivePlaneCentre.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane1), CreateEdit("0.0", "y:", 10, edit_primitivePlaneCentre.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane1), CreateEdit("0.0", "z:", 10, edit_primitivePlaneCentre.z), false, false, 1);
	sGtkEditVector3 edit_primitivePlaneNormal = CreateEditVector3WithMap("primitive_plane_normal", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane1), CreateEdit("0.0", "Normal x:", 5, edit_primitivePlaneNormal.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane1), CreateEdit("0.0", "y:", 5, edit_primitivePlaneNormal.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane1), CreateEdit("-1,0", "z:", 5, edit_primitivePlaneNormal.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane), Interface.boxPrimitivePlane2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane2), CreateColorButtonWithMap("primitive_plane_colour", "Plane colour", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane2), CreateEdit("0,0", "Reflect:", 5, CreateEditWithMap("primitive_plane_reflect", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane2), CreateCheckBoxWithMap("primitive_plane_enabled", "Enabled", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitivePlane2), CreateCheckBoxWithMap("primitive_only_plane", "Only plane (2D mode)", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_primitiveWater), Interface.frPrimitiveWater, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPrimitiveWater), Interface.boxPrimitiveWater);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater), Interface.boxPrimitiveWater1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater1), CreateEdit("0.0", "Level:", 10, CreateEditWithMap("primitive_water_level", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater1), CreateEdit("0.1", "Wave amplitude:", 10, CreateEditWithMap("primitive_water_amplitude", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater1), CreateEdit("1.0", "Wave length:", 10, CreateEditWithMap("primitive_water_length", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater1), CreateEdit("0", "Rotation:", 10, CreateEditWithMap("primitive_water_rotation", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater1), CreateEdit("5", "Iterations:", 3, CreateEditWithMap("primitive_water_iterations", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater), Interface.boxPrimitiveWater2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater2), CreateColorButtonWithMap("primitive_water_colour", "Water colour", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater2), CreateEdit("0,7", "Reflect:", 5, CreateEditWithMap("primitive_water_reflect", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater2), CreateEdit("0,1", "Waves anim speed:", 5, CreateEditWithMap("primitive_water_anim_speed", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveWater2), CreateCheckBoxWithMap("primitive_water_enabled", "Enabled", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_primitiveBox), Interface.frPrimitiveBox, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPrimitiveBox), Interface.boxPrimitiveBox);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox), Interface.boxPrimitiveBox1, false, false, 1);
	sGtkEditVector3 edit_primitiveBoxCentre = CreateEditVector3WithMap("primitive_box_centre", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox1), CreateEdit("0.0", "Centre x:", 10, edit_primitiveBoxCentre.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox1), CreateEdit("0.0", "y:", 10, edit_primitiveBoxCentre.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox1), CreateEdit("0.0", "z:", 10, edit_primitiveBoxCentre.z), false, false, 1);
	sGtkEditVector3 edit_primitiveBoxSize = CreateEditVector3WithMap("primitive_box_size", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox1), CreateEdit("1.0", "Size x:", 10, edit_primitiveBoxSize.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox1), CreateEdit("1.0", "y:", 10, edit_primitiveBoxSize.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox1), CreateEdit("1.0", "z:", 10, edit_primitiveBoxSize.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox), Interface.boxPrimitiveBox2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox2), CreateColorButtonWithMap("primitive_box_colour", "Box colour", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox2), CreateEdit("0,0", "Reflect:", 5, CreateEditWithMap("primitive_box_reflect", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveBox2), CreateCheckBoxWithMap("primitive_box_enabled", "Enabled", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_primitiveBoxInv), Interface.frPrimitiveInvertedBox, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPrimitiveInvertedBox), Interface.boxPrimitiveInvertedBox);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox), Interface.boxPrimitiveInvertedBox1, false, false, 1);
	sGtkEditVector3 edit_primitiveInvertedBoxCentre = CreateEditVector3WithMap("primitive_invertedBox_centre", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox1), CreateEdit("0.0", "Centre x:", 10, edit_primitiveInvertedBoxCentre.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox1), CreateEdit("0.0", "y:", 10, edit_primitiveInvertedBoxCentre.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox1), CreateEdit("0.0", "z:", 10, edit_primitiveInvertedBoxCentre.z), false, false, 1);
	sGtkEditVector3 edit_primitiveInvertedBoxSize = CreateEditVector3WithMap("primitive_invertedBox_size", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox1), CreateEdit("10.0", "Size x:", 10, edit_primitiveInvertedBoxSize.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox1), CreateEdit("10.0", "y:", 10, edit_primitiveInvertedBoxSize.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox1), CreateEdit("10.0", "z:", 10, edit_primitiveInvertedBoxSize.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox), Interface.boxPrimitiveInvertedBox2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox2), CreateColorButtonWithMap("primitive_invertedBox_colour", "Inverted box colour", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox2), CreateEdit("0,0", "Reflect:", 5, CreateEditWithMap("primitive_invertedBox_reflect", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedBox2), CreateCheckBoxWithMap("primitive_invertedBox_enabled", "Enabled", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_primitiveSphere), Interface.frPrimitiveSphere, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPrimitiveSphere), Interface.boxPrimitiveSphere);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveSphere), Interface.boxPrimitiveSphere1, false, false, 1);
	sGtkEditVector3 edit_primitiveSphereCentre = CreateEditVector3WithMap("primitive_sphere_centre", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveSphere1), CreateEdit("0.0", "Centre x:", 10, edit_primitiveSphereCentre.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveSphere1), CreateEdit("0.0", "y:", 10, edit_primitiveSphereCentre.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveSphere1), CreateEdit("0.0", "z:", 10, edit_primitiveSphereCentre.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveSphere1), CreateEdit("2.0", "z:", 10, CreateEditWithMap("primitive_sphere_radius", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveSphere), Interface.boxPrimitiveSphere2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveSphere2), CreateColorButtonWithMap("primitive_sphere_colour", "Sphere colour", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveSphere2), CreateEdit("0,0", "Reflect:", 5, CreateEditWithMap("primitive_sphere_reflect", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveSphere2), CreateCheckBoxWithMap("primitive_sphere_enabled", "Enabled", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_primitiveSphereInv), Interface.frPrimitiveInvertedSphere, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPrimitiveInvertedSphere), Interface.boxPrimitiveInvertedSphere);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedSphere), Interface.boxPrimitiveInvertedSphere1, false, false, 1);
	sGtkEditVector3 edit_primitiveInvertedSphereCentre = CreateEditVector3WithMap("primitive_invertedSphere_centre", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedSphere1), CreateEdit("0.0", "Centre x:", 10, edit_primitiveInvertedSphereCentre.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedSphere1), CreateEdit("0.0", "y:", 10, edit_primitiveInvertedSphereCentre.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedSphere1), CreateEdit("0.0", "z:", 10, edit_primitiveInvertedSphereCentre.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedSphere1), CreateEdit("2.0", "z:", 10, CreateEditWithMap("primitive_invertedSphere_radius", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedSphere), Interface.boxPrimitiveInvertedSphere2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedSphere2), CreateColorButtonWithMap("primitive_invertedSphere_colour", "Inverted sphere colour", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedSphere2), CreateEdit("0,0", "Reflect:", 5, CreateEditWithMap("primitive_invertedSphere_reflect", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPrimitiveInvertedSphere2), CreateCheckBoxWithMap("primitive_invertedSphere_enabled", "Enabled", &mapInterface), false, false, 1);

	//tab --- engine
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_engine), Interface.frFractalRayMarching, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frFractalRayMarching), Interface.boxFractalRayMarching);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalRayMarching), Interface.boxQuality, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), CreateEdit("250", "Max. iterations:", 5, CreateEditWithMap("N", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), CreateEdit("1", "Min. iterations:", 5, CreateEditWithMap("minN", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), Interface.label_DE_threshold, false, false, 1);
	GtkWidget *edit_DE_thresh = CreateEditWithMap("DE_thresh", &mapInterface);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), edit_DE_thresh, false, false, 1);
	gtk_entry_set_text(GTK_ENTRY(edit_DE_thresh), "1,0");
	gtk_entry_set_width_chars(GTK_ENTRY(edit_DE_thresh), 5);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), CreateEdit("1,0", "DE step factor:", 5, CreateEditWithMap("DE_factor", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), Interface.buAutoDEStep, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), Interface.buAutoDEStepHQ, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxQuality), CreateEdit("1,0", "Smoothness:", 5, CreateEditWithMap("smoothness", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalRayMarching), Interface.boxFractalSwitches, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalSwitches), CreateCheckBoxWithMap("iteration_threshold_mode", "MaxIter mode", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalSwitches), CreateCheckBoxWithMap("interior_mode", "Interior mode", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalSwitches), CreateCheckBoxWithMap("linear_DE_mode", "Linear DE mode", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalSwitches), CreateCheckBoxWithMap("constant_DE_threshold", "Const. DE threshold", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFractalSwitches), CreateCheckBoxWithMap("slow_shading", "Not DE Shading mode (slow)", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFractalRayMarching), Interface.boxViewDistance, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxViewDistance), CreateEdit("1e-15", "Minimum render distance:", 10, CreateEditWithMap("view_distance_min", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxViewDistance), CreateEdit("20", "Maximum render distance:", 10, CreateEditWithMap("view_distance_max", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_engine), Interface.frLimits, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frLimits), Interface.boxLimits);
	gtk_box_pack_start(GTK_BOX(Interface.boxLimits), Interface.tableLimits, false, false, 1);
	sGtkEditVector3 edit_limit_min = CreateEditVector3WithMap("limit_min", &mapInterfaceEditVector);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("-0,5", "x min:", 20, edit_limit_min.x), 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("-0,5", "y min:", 20, edit_limit_min.y), 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("-0,5", "z min:", 20, edit_limit_min.z), 2, 3, 0, 1);
	sGtkEditVector3 edit_limit_max = CreateEditVector3WithMap("limit_max", &mapInterfaceEditVector);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("0,5", "x max:", 20, edit_limit_max.x), 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("0,5", "y max:", 20, edit_limit_max.y), 1, 2, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(Interface.tableLimits), CreateEdit("0,5", "z max:", 20, edit_limit_max.z), 2, 3, 1, 2);
	gtk_box_pack_start(GTK_BOX(Interface.boxLimits), CreateCheckBoxWithMap("limits_enabled", "Enable limits", &mapInterface), false, false, 1);

	//frame netRender
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_engine), Interface.frNetRender, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frNetRender), Interface.tabsNetRender);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_server), Interface.boxNetRenderServerV, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNetRenderServerV), Interface.boxNetRenderServerH1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNetRenderServerH1), Interface.checkNetRenderServerEnable, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNetRenderServerH1), Interface.checkNetRenderServerScan, false, false, 1);
	gtk_widget_set_sensitive(Interface.checkNetRenderServerScan, false);
	gtk_box_pack_start(GTK_BOX(Interface.boxNetRenderServerH1), CreateEdit("5555", "   network port:", 20,  CreateEditWithMap("net_render_server_port", &mapAppParams)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNetRenderServerV), Interface.label_serverStatus, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_client), Interface.boxNetRenderClientV, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNetRenderClientV), Interface.boxNetRenderClientH1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNetRenderClientH1), Interface.checkNetRenderClientEnable, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNetRenderClientH1), CreateEdit("11.0.0.4", "   IP or domain of server:", 20, CreateEditWithMap("net_render_client_IP", &mapAppParams)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNetRenderClientH1), CreateEdit("5555", "port:", 10, CreateEditWithMap("net_render_client_port", &mapAppParams)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxNetRenderClientV), Interface.label_clientStatus, false, false, 1);

	//frame image
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_image), Interface.frImage, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frImage), Interface.boxImage);

	gtk_box_pack_start(GTK_BOX(Interface.boxImage), Interface.boxImageRes, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageRes), CreateEdit("800", "Image width:", 5, CreateEditWithMap("image_width", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageRes), CreateEdit("600", "Image height:", 5, CreateEditWithMap("image_height", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageRes),  CreateWidgetWithLabel("Image proportion:", comboImageProportion), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageRes),  CreateEdit("1", "Tiles (rows and columns):", 5, CreateEditWithMap("tiles", &mapInterface)), false, false, 1);

	//frame image adjustments
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_image), Interface.frImageAdjustments, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frImageAdjustments), Interface.boxImageAdjustmentsV);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageAdjustmentsV), Interface.boxImageAdjustmentsH1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageAdjustmentsH1), CreateEdit("1,0", "brightness:", 5, CreateEditWithMap("brightness", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageAdjustmentsH1), CreateEdit("1,0", "contrast:", 5, CreateEditWithMap("contrast", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageAdjustmentsH1), CreateEdit("1,0", "gamma:", 5, CreateEditWithMap("gamma", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageAdjustmentsH1), CreateCheckBoxWithMap("hdr", "HDR", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageAdjustmentsH1), Interface.buApplyImageAdjustments, false, false, 1);

	//frame Stereoscopic
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_image), Interface.frStereo, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frStereo), Interface.boxStereoscopic);

	gtk_box_pack_start(GTK_BOX(Interface.boxStereoscopic), Interface.boxStereoParams, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxStereoParams), CreateEdit("0,1", "Distance between eyes:", 20, CreateEditWithMap("stereo_eye_distance", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxStereoParams), CreateCheckBoxWithMap("stereo_enabled", "Enable stereoscopic rendering", &mapInterface), false, false, 1);

	//frame Image saving
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_image), Interface.frImageSaving, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frImageSaving), Interface.boxImageSaving);

	gtk_box_pack_start(GTK_BOX(Interface.boxImageSaving), Interface.boxSaveImage, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSaveImage), Interface.buSaveImage, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSaveImage), Interface.buSavePNG, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSaveImage), Interface.buSavePNG16, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSaveImage), Interface.buSavePNG16Alpha, true, true, 1);
	//gtk_box_pack_start(GTK_BOX(Interface.boxSaveImage), Interface.buSaveAllImageLayers, true, true, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxImageSaving), Interface.boxImageAutoSave, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageAutoSave), CreateWidgetWithLabel("Auto-save / animation image format:", comboImageFormat), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxImageAutoSave), CreateCheckBoxWithMap("auto_save_images", "Auto-save", &mapAppParams), false, false, 1);

	//frame effects
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders), Interface.frShadersSurface, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frShadersSurface), Interface.boxShadersSurfaceV);

	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceV), Interface.boxShadersSurfaceH1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH1), CreateEdit("0,5", "shading:", 5, CreateEditWithMap("shading", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH1), CreateEdit("1,0", "direct light:", 5, CreateEditWithMap("shadows_intensity", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH1), CreateEdit("1,0", "specularity:", 5, CreateEditWithMap("specular", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH1), CreateEdit("0,0", "ambient:", 5, CreateEditWithMap("ambient", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH1), CreateEdit("10,0", "Soft shadow cone angle:", 5, CreateEditWithMap("shadows_cone_angle", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceV), Interface.boxShadersSurfaceH2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH2), CreateEdit("2,0", "ambient occlusion:", 5, CreateEditWithMap("ambient_occlusion", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH2), CreateEdit("4", "AO quality:", 5, CreateEditWithMap("ambient_occlusion_quality", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH2), CreateEdit("1,0", "Fast AO tune:", 5, CreateEditWithMap("ambient_occlusion_fast_tune", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH2), CreateEdit("0,0", "reflection:", 5, CreateEditWithMap("reflect", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH2), CreateEdit("5", "reflections depth:", 5, CreateEditWithMap("reflections_max", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceV), Interface.boxShadersSurfaceH3, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH3), CreateCheckBoxWithMap("shadows_enabled", "Shadows", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH3), CreateCheckBoxWithMap("ambient_occlusion_enabled", "Ambient occlusion", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH3), CreateCheckBoxWithMap("fast_ambient_occlusion_mode", "AO fast mode", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersSurfaceH3), CreateCheckBoxWithMap("raytraced_reflections", "Ray-traced reflections", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders), Interface.frShadersVolumetric, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frShadersVolumetric), Interface.boxShadersVolumetricV);

	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricV), Interface.boxShadersVolumetricH1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH1), CreateEdit("1,0", "glow:", 5, CreateEditWithMap("glow_intensity", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH1), CreateEdit("100", "iter. fog opacity", 6, CreateEditWithMap("iteration_fog_opacity", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH1), CreateEdit("3", "fog opacity trim (iterations)", 6, CreateEditWithMap("iteration_fog_opacity_trim", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricV), Interface.boxShadersVolumetricH2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH2), CreateEdit("1,0", "Fog density:", 5, CreateEditWithMap("volumetric_fog_density", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH2), CreateEdit("1,0", "Fog colour 1 distance:", 5, CreateEditWithMap("volumetric_fog_colour_1_distance", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH2), CreateEdit("1,0", "Fog colour 2 distance:", 5, CreateEditWithMap("volumetric_fog_colour_2_distance", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH2), CreateEdit("1,0", "Fog distance factor:", 5, CreateEditWithMap("volumetric_fog_distance_factor", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH2), Interface.buAutoFog, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricV), Interface.boxShadersVolumetricH3, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH3), CreateCheckBoxWithMap("textured_background", "Textured background", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH3), CreateCheckBoxWithMap("background_as_fuldome", "Fulldome background", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxShadersVolumetricH3), CreateCheckBoxWithMap("iteration_fog_enable", "Iteration fog", &mapInterface), false, false, 1);

	//frame fake lights
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders), Interface.frFakeLights, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frFakeLights), Interface.boxFakeLightsV);

	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsV), Interface.boxFakeLightsH1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsH1), CreateEdit("2", "Min iter.:", 5, CreateEditWithMap("fake_lights_min_iter", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsH1), CreateEdit("4", "Max iter.:", 5, CreateEditWithMap("fake_lights_max_iter", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsH1), CreateEdit("1,0", "Intensity:", 10, CreateEditWithMap("fake_lights_intensity", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsH1), CreateEdit("1,0", "Visibility:", 10, CreateEditWithMap("fake_lights_visibility", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsH1), CreateEdit("1,0", "Size:", 10, CreateEditWithMap("fake_lights_visibility_size", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsH1), CreateCheckBoxWithMap("fake_lights_enabled", "Enable fake lights", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsV), Interface.boxFakeLightsH2, false, false, 1);
	sGtkEditVector3 edit_fakeLightsOrbitTrap = CreateEditVector3WithMap("fake_lights_orbit_trap", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsH2), CreateEdit("0,0", "Orbit trap X:", 5, edit_fakeLightsOrbitTrap.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsH2), CreateEdit("0,0", "Y:", 5, edit_fakeLightsOrbitTrap.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFakeLightsH2), CreateEdit("0,0", "Z:", 5, edit_fakeLightsOrbitTrap.z), false, false, 1);

	//frame fog
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders), Interface.frPostFog, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPostFog), Interface.boxPostFog);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostFog), Interface.boxFogButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogButtons), CreateCheckBoxWithMap("post_fog_enabled", "Enable fog         ", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogButtons), CreateWidgetWithLabel("Fog colour:", CreateColorButtonWithMap("post_fog_color", "Fog colour", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogButtons), Interface.label_sliderFog, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostFog), Interface.boxFogSlider, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogSlider), Interface.label_fog_visibility, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxFogSlider), Interface.sliderFogDepth, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostFog), Interface.boxFogSlider2, false, false, 1);

	//frame animation
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_animation), Interface.frAnimation, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frAnimation), Interface.boxAnimation);

	gtk_box_pack_start(GTK_BOX(Interface.boxAnimation), Interface.boxAnimationButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationButtons), Interface.buAnimationRecordTrack, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationButtons), Interface.buAnimationContinueRecord, true, true, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationButtons), Interface.buAnimationRenderTrack, true, true, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxAnimation), Interface.boxAnimationEdits, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationEdits), CreateEdit(DoubleToString(0.01), "Flight speed (DE multiplier):", 5, CreateEditWithMap("flight_speed", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxAnimation), Interface.label_animationFrame, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimation), Interface.label_animationDistance, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimation), Interface.label_animationSpeed, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_animation), Interface.frKeyframeAnimation, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frKeyframeAnimation), Interface.boxKeyframeAnimation);

	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimation), Interface.boxKeyframeAnimationButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimationButtons), Interface.buAnimationRenderFromKeys, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimation), Interface.boxKeyframeAnimationButtons2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimationButtons2), Interface.buConvertPathToKeyframes, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimation), Interface.boxKeyframeAnimationEdits, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimationEdits), CreateEdit("100", "Frames per key:", 5, CreateEditWithMap("frames_per_keyframe", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxKeyframeAnimation), Interface.label_keyframeInfo, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_animation), Interface.frAnimationFrames, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frAnimationFrames), Interface.boxAnimationEdits2);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationEdits2), CreateEdit("0", "Start frame:", 5, CreateEditWithMap("start_frame", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxAnimationEdits2), CreateEdit("1000", "End frame:", 5, CreateEditWithMap("end_frame", &mapInterface)), false, false, 1);

	//---- tab pot effects

	//frame palette
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders2), Interface.frPalette, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPalette), Interface.boxPalette);

	gtk_box_pack_start(GTK_BOX(Interface.boxPalette), Interface.boxEffectsColoring, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), CreateCheckBoxWithMap("fractal_color", "Coloured surface", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), Interface.vSeparator1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), CreateEdit("123456", "Random seed:", 6, CreateEditWithMap("coloring_random_seed", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), CreateEdit("1,0", "Saturation:", 6, CreateEditWithMap("coloring_saturation", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), CreateEdit("1,0", "Colour speed:", 6, CreateEditWithMap("coloring_speed", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), Interface.buRandomPalette, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxEffectsColoring), Interface.buGetPaletteFromImage, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxPalette), dareaPalette, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPalette), Interface.boxPaletteOffset, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPaletteOffset), Interface.label_paletteOffset, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPaletteOffset), Interface.sliderPaletteOffset, true, true, 1);

	//frame colors
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders2), Interface.frColors, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frColors), Interface.boxColors);

	gtk_box_pack_start(GTK_BOX(Interface.boxColors), Interface.boxGlowColor, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("Glow 1:", CreateColorButtonWithMap("glow_color_1", "Glow colour 1", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("2:", CreateColorButtonWithMap("glow_color_2", "Glow colour 2", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("Background 1:", CreateColorButtonWithMap("background_color_1", "Background colour 1", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("2:", CreateColorButtonWithMap("background_color_2", "Background colour 2", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("3:", CreateColorButtonWithMap("background_color_3", "Background colour 3", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("Fog 1:", CreateColorButtonWithMap("fog_color_1", "Fog colour 1", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("2:", CreateColorButtonWithMap("fog_color_2", "Fog colour 2", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxGlowColor), CreateWidgetWithLabel("3:", CreateColorButtonWithMap("fog_color_3", "Fog colour 3", &mapInterface)), false, false, 1);

	//frame SSAO
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders2), Interface.frPostSSAO, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPostSSAO), Interface.boxPostSSAO);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostSSAO), Interface.boxSSAOButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSSAOButtons), CreateCheckBoxWithMap("post_SSAO_enabled", "Screen space ambient occlusion enable       ", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSSAOButtons), Interface.buUpdateSSAO, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostSSAO), Interface.boxSSAOSlider, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSSAOSlider), Interface.label_SSAO_quality, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxSSAOSlider), Interface.sliderSSAOQuality, true, true, 1);

	//frame DOF
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_shaders2), Interface.frPostDOF, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPostDOF), Interface.boxPostDOF);
	gtk_box_pack_start(GTK_BOX(Interface.boxPostDOF), Interface.boxDOFButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxDOFButtons), CreateCheckBoxWithMap("post_DOF_enabled", "Depth of field enable       ", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxDOFButtons), Interface.buUpdateDOF, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxDOFButtons), Interface.label_sliderDOF, false, false, 1);
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
	gtk_box_pack_start(GTK_BOX(Interface.boxMainLightPosition), CreateEdit("-45", "Horizontal angle relative to camera:", 6, CreateEditWithMap("main_light_alpha", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMainLightPosition), CreateEdit("45", "Vertical angle relative to camera:", 6, CreateEditWithMap("main_light_beta", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMainLightPosition), CreateWidgetWithLabel("Colour:", CreateColorButtonWithMap("main_light_colour", "Main light source colour", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frLightsCommon, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frLightsCommon), Interface.boxLightCommon);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightCommon), Interface.buDistributeLights, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightCommon), CreateEdit("0", "Number of aux. lights:", 6, CreateEditWithMap("aux_light_number", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightCommon), CreateEdit(DoubleToString(0.01), "Manual placement distance:", 6, CreateEditWithMap("light_manual_placement_dist", &mapAppParams)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frLightBallance, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frLightBallance), Interface.boxLightBallance);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightBallance), Interface.boxLightBrightness, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightBrightness), CreateEdit("1,0", "Main light intensity:", 6, CreateEditWithMap("main_light_intensity", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightBrightness), CreateEdit("1,0", "Auxiliary lights intensity:", 6, CreateEditWithMap("aux_light_intensity", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightBrightness), CreateEdit("1,0", "Lights visibility:", 6, CreateEditWithMap("aux_light_visibility", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightBrightness), CreateCheckBoxWithMap("penetrating_lights", "Penetrating lights", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frLightsParameters, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frLightsParameters), Interface.boxLightsParameters);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightsParameters), Interface.boxLightDistribution, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution), CreateEdit("1234", "Random seed:", 6, CreateEditWithMap("aux_light_random_seed", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution), CreateEdit("0,1", "Maximum distance from fractal", 12, CreateEditWithMap("aux_light_max_dist", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightsParameters), Interface.boxLightDistribution2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution2), CreateEdit("3.0", "Distribution radius of lights:", 6, CreateEditWithMap("aux_light_distribution_radius", &mapInterface)), false, false, 1);
	sGtkEditVector3 edit_auxLightRandomCentre = CreateEditVector3WithMap("aux_light_random_center", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution2), CreateEdit("0", "Centre of distribution X:", 12, edit_auxLightRandomCentre.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution2), CreateEdit("0", "Y:", 12, edit_auxLightRandomCentre.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightDistribution2), CreateEdit("0", "Z:", 12, edit_auxLightRandomCentre.z), false, false, 1);

	//frame: predefined lights
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frPredefinedLights, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frPredefinedLights), Interface.boxPredefinedLights);

	gtk_box_pack_start(GTK_BOX(Interface.boxPredefinedLights), Interface.boxLightPre1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), Interface.label_auxLightPre1, false, false, 1);
	sGtkEditVector3 edit_auxLightPre1 = CreateEditVector3WithMap("aux_light_predefined_position_1", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), CreateEdit("3,0", "x:", 12, edit_auxLightPre1.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), CreateEdit("-3,0", "y:", 12, edit_auxLightPre1.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), CreateEdit("-3,0", "z:", 12, edit_auxLightPre1.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), CreateEdit("1,0", "intensity:", 12, CreateEditWithMap("aux_light_predefined_intensity_1", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), CreateColorButtonWithMap("aux_light_predefined_colour_1", "Colour of Auxiliary light #1", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre1), CreateCheckBoxWithMap("aux_light_predefined_enabled_1", "Enable", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxPredefinedLights), Interface.boxLightPre2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), Interface.label_auxLightPre2, false, false, 1);
	sGtkEditVector3 edit_auxLightPre2 = CreateEditVector3WithMap("aux_light_predefined_position_2", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), CreateEdit("-3,0", "x:", 12, edit_auxLightPre2.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), CreateEdit("-3,0", "y:", 12, edit_auxLightPre2.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), CreateEdit("0,0", "z:", 12, edit_auxLightPre2.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), CreateEdit("1,0", "intensity:", 12, CreateEditWithMap("aux_light_predefined_intensity_2", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), CreateColorButtonWithMap("aux_light_predefined_colour_2", "Colour of Auxiliary light #2", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre2), CreateCheckBoxWithMap("aux_light_predefined_enabled_2", "Enable", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxPredefinedLights), Interface.boxLightPre3, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), Interface.label_auxLightPre3, false, false, 1);
	sGtkEditVector3 edit_auxLightPre3 = CreateEditVector3WithMap("aux_light_predefined_position_3", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), CreateEdit("1,0", "x:", 12, edit_auxLightPre3.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), CreateEdit("3,0", "y:", 12, edit_auxLightPre3.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), CreateEdit("-1,0", "z:", 12, edit_auxLightPre3.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), CreateEdit("1,0", "intensity:", 12, CreateEditWithMap("aux_light_predefined_intensity_3", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), CreateColorButtonWithMap("aux_light_predefined_colour_3", "Colour of Auxiliary light #3", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre3), CreateCheckBoxWithMap("aux_light_predefined_enabled_3", "Enable", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxPredefinedLights), Interface.boxLightPre4, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), Interface.label_auxLightPre4, false, false, 1);
	sGtkEditVector3 edit_auxLightPre4 = CreateEditVector3WithMap("aux_light_predefined_position_4", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), CreateEdit("1,0", "x:", 12, edit_auxLightPre4.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), CreateEdit("-1,0", "y:", 12, edit_auxLightPre4.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), CreateEdit("-3,0", "z:", 12, edit_auxLightPre4.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), CreateEdit("1,0", "intensity:", 12, CreateEditWithMap("aux_light_predefined_intensity_4", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), CreateColorButtonWithMap("aux_light_predefined_colour_4", "Colour of Auxiliary light #4", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxLightPre4), CreateCheckBoxWithMap("aux_light_predefined_enabled_4", "Enable", &mapInterface), false, false, 1);

	//frame: volumetric lights
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_lights), Interface.frVolumetricLight, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frVolumetricLight), Interface.boxVolumetricLight);

	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLight), Interface.boxVolumetricLightGeneral, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLight), Interface.boxVolumetricLightMain, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightMain), CreateEdit("1,0", "Intensity for main light:", 12, CreateEditWithMap("volumetric_light_intensity_0", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightMain), CreateCheckBoxWithMap("volumetric_light_enabled_0", "", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLight), Interface.boxVolumetricLightAux, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateEdit("1,0", "L #1:", 12, CreateEditWithMap("volumetric_light_intensity_1", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateCheckBoxWithMap("volumetric_light_enabled_1", "", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateEdit("1,0", "L #2:", 12, CreateEditWithMap("volumetric_light_intensity_2", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateCheckBoxWithMap("volumetric_light_enabled_2", "", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateEdit("1,0", "L #3:", 12, CreateEditWithMap("volumetric_light_intensity_3", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateCheckBoxWithMap("volumetric_light_enabled_3", "", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateEdit("1,0", "L #4:", 12, CreateEditWithMap("volumetric_light_intensity_4", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxVolumetricLightAux), CreateCheckBoxWithMap("volumetric_light_enabled_4", "", &mapInterface), false, false, 1);

	//tab IFS
	//frame: main IFS
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_IFS), Interface.frIFSMain, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frIFSMain), Interface.boxIFSMain);

	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMain), Interface.boxIFSMainEdit, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit), CreateEdit("2,0", "scale:", 6, CreateEditWithMap("IFS_scale", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit), CreateEdit("0", "rotation alpha:", 6, CreateEditWithMap("IFS_rot_alpha", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit), CreateEdit("0", "rotation beta:", 6, CreateEditWithMap("IFS_rot_beta", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit), CreateEdit("0", "rotation gamma:", 6, CreateEditWithMap("IFS_rot_gamma", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMain), Interface.boxIFSMainEdit2, false, false, 1);
	sGtkEditVector3 edit_IFSOffset = CreateEditVector3WithMap("IFS_offset", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), CreateEdit("1", "offset x:", 6, edit_IFSOffset.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), CreateEdit("0", "offset y:", 6, edit_IFSOffset.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), CreateEdit("0", "offset z:", 6, edit_IFSOffset.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), CreateCheckBoxWithMap("IFS_abs_X", "abs(x)", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), CreateCheckBoxWithMap("IFS_abs_Y", "abs(y)", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), CreateCheckBoxWithMap("IFS_abs_Z", "abs(z)", &mapInterface), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMainEdit2), CreateCheckBoxWithMap("IFS_menger_sponge_mode", "Menger Sponge", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxIFSMain), Interface.boxIFSEdge, false, false, 1);
	sGtkEditVector3 edit_IFSEdge = CreateEditVector3WithMap("IFS_edge", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSEdge), CreateEdit("0", "Edge x:", 6, edit_IFSEdge.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSEdge), CreateEdit("0", "Edge y:", 6, edit_IFSEdge.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSEdge), CreateEdit("0", "Edge z:", 6, edit_IFSEdge.z), false, false, 1);

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
		GtkWidget *checkIFSenabled = CreateCheckBoxWithMapIndexed("IFS_enabled", i, "", &mapInterface);
		sGtkEditVector3 edit_IFS_direction = CreateEditVector3WithMapIndexed("IFS_direction", i, &mapInterfaceEditVector);
		GtkWidget *editIFSalpha = CreateEditWithMapIndexed("IFS_alpha", i, &mapInterface);
		GtkWidget *editIFSbeta = CreateEditWithMapIndexed("IFS_beta", i, &mapInterface);
		GtkWidget *editIFSgamma = CreateEditWithMapIndexed("IFS_gamma", i, &mapInterface);
		GtkWidget *editIFSdistance = CreateEditWithMapIndexed("IFS_distance", i, &mapInterface);
		GtkWidget *editIFSintensity = CreateEditWithMapIndexed("IFS_intensity", i, &mapInterface);
		gtk_entry_set_width_chars(GTK_ENTRY(edit_IFS_direction.x), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(edit_IFS_direction.y), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(edit_IFS_direction.z), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(editIFSalpha), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(editIFSbeta), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(editIFSgamma), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(editIFSdistance), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(editIFSintensity), 6);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), edit_IFS_direction.x, 0, 1, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), edit_IFS_direction.y, 1, 2, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), edit_IFS_direction.z, 2, 3, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), editIFSalpha, 3, 4, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), editIFSbeta, 4, 5, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), editIFSgamma, 5, 6, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), editIFSdistance, 6, 7, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), editIFSintensity, 7, 8, i + 1, i + 2);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableIFSParams), checkIFSenabled, 8, 9, i + 1, i + 2);
	}

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_IFS), Interface.boxIFSButtons, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSButtons), Interface.buIFSNormalizeOffset, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSButtons), Interface.buIFSNormalizeVectors, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSButtons), Interface.buIFSReset, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_IFS), Interface.frIFSDefaults, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frIFSDefaults), Interface.boxIFSDefaults);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSDefaults), Interface.buIFSDefaultDodeca, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSDefaults), Interface.buIFSDefaultIcosa, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSDefaults), Interface.buIFSDefaultOcta, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxIFSDefaults), Interface.buIFSDefaultMengerSponge, false, false, 1);

	//tab hybrid formula
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_hybrid), Interface.frHybrid, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frHybrid), Interface.boxHybrid);

	gtk_box_pack_start(GTK_BOX(Interface.boxHybrid), Interface.tableHybridParams, false, false, 1);

	for (int i = 0; i < HYBRID_COUNT; ++i)
	{
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableHybridParams), Interface.label_HybridFormula[i], 0, 1, i, i + 1);
		GtkWidget *comboHybridFormula = gtk_combo_box_new_text();
		AddComboTextsFractalFormula(GTK_COMBO_BOX(comboHybridFormula));
		AddComboBoxToMapIndexed("hybrid_formula_combo", i + 1, comboHybridFormula, &mapInterface);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableHybridParams), comboHybridFormula, 1, 2, i, i + 1);
		GtkWidget *edit_hybridIter = CreateEditWithMapIndexed("hybrid_iterations", i + 1, &mapInterface);
		GtkWidget *edit_hybridPower = CreateEditWithMapIndexed("hybrid_power", i + 1, &mapInterface);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableHybridParams), CreateEdit("3", "  iterations:", 6, edit_hybridIter), 2, 3, i, i + 1);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableHybridParams), CreateEdit("2", "power / scale / p:", 6, edit_hybridPower), 3, 4, i, i + 1);
	}

	gtk_box_pack_start(GTK_BOX(Interface.boxHybrid), CreateCheckBoxWithMap("hybrid_cyclic", "Cyclic loop", &mapInterface), false, false, 1);

	//tab Mandelbox
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_mandelbox), Interface.frMandelboxMainParams, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frMandelboxMainParams), Interface.boxMandelboxMainParams);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams), Interface.boxMandelboxMainParams1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams1), CreateEdit("2", "Scale", 6, CreateEditWithMap("mandelbox_scale", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams1), CreateEdit("3,0", "Sharpness", 6, CreateEditWithMap("mandelbox_sharpness", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams1), CreateEdit("1,0", "Solid", 6, CreateEditWithMap("mandelbox_solid", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams1), CreateEdit("0,0", "Melt", 6, CreateEditWithMap("mandelbox_melt", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams), Interface.boxMandelboxMainParams2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams2), CreateEdit("1", "Folding limit", 6, CreateEditWithMap("mandelbox_folding_limit", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams2), CreateEdit("2", "Folding value", 6, CreateEditWithMap("mandelbox_folding_value", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams2), CreateEdit("1", "Fixed radius", 6, CreateEditWithMap("mandelbox_folding_fixed_radius", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams2), CreateEdit("0,5", "Min radius", 6, CreateEditWithMap("mandelbox_folding_min_radius", &mapInterface)), false, false, 1);


	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxMainParams), Interface.boxMandelboxOffset, false, false, 1);
	sGtkEditVector3 edit_mandelboxOffset = CreateEditVector3WithMap("mandelbox_offset", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxOffset), CreateEdit("1", "Spherical folding offset X", 6, edit_mandelboxOffset.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxOffset), CreateEdit("1", "Y", 6, edit_mandelboxOffset.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxOffset), CreateEdit("1", "Z", 6, edit_mandelboxOffset.z), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxOffset), CreateWidgetWithLabel("GeneralizedFoldBox type", comboGeneralizedFoldBoxType), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_mandelbox), Interface.frMandelboxRotations, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frMandelboxRotations), Interface.boxMandelboxRotations);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotations), Interface.boxMandelboxRotationMain, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotations), CreateCheckBoxWithMap("mandelbox_rotation_enabled", "Enable rotation of each folding plane", &mapInterface), false, false, 1);

	sGtkEditVector3 edit_mandelboxRotationMain = CreateEditVector3WithMap("mandelbox_rotation_main", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotationMain), CreateEdit("0", "Main rotation: alpha", 6, edit_mandelboxRotationMain.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotationMain), CreateEdit("0", "beta", 6, edit_mandelboxRotationMain.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotationMain), CreateEdit("0", "gamma", 6, edit_mandelboxRotationMain.z), false, false, 1);

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

	for (int axis = 0; axis < 3; ++axis)
	{
		sGtkEditVector3 edit_mandelbox_rotation_neg = CreateEditVector3WithMapIndexed("mandelbox_rotation_neg", axis + 1, &mapInterfaceEditVector);
		sGtkEditVector3 edit_mandelbox_rotation_pos = CreateEditVector3WithMapIndexed("mandelbox_rotation_pos", axis + 1, &mapInterfaceEditVector);
		gtk_entry_set_width_chars(GTK_ENTRY(edit_mandelbox_rotation_neg.x), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(edit_mandelbox_rotation_neg.y), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(edit_mandelbox_rotation_neg.z), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(edit_mandelbox_rotation_pos.x), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(edit_mandelbox_rotation_pos.y), 6);
		gtk_entry_set_width_chars(GTK_ENTRY(edit_mandelbox_rotation_pos.z), 6);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), edit_mandelbox_rotation_neg.x, 1, 2, axis + 2, axis + 3);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), edit_mandelbox_rotation_neg.y, 2, 3, axis + 2, axis + 3);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), edit_mandelbox_rotation_neg.z, 3, 4, axis + 2, axis + 3);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), edit_mandelbox_rotation_pos.x, 4, 5, axis + 2, axis + 3);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), edit_mandelbox_rotation_pos.y, 5, 6, axis + 2, axis + 3);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableMandelboxRotations), edit_mandelbox_rotation_pos.z, 6, 7, axis + 2, axis + 3);
	}

	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxRotations), Interface.tableMandelboxRotations, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_mandelbox), Interface.frMandelboxVary, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frMandelboxVary), Interface.boxMandelboxVary);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxVary), CreateEdit("0,1", "Vary scale", 6, CreateEditWithMap("mandelbox_vary_scale_vary", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxVary), CreateEdit("1", "Fold", 6, CreateEditWithMap("mandelbox_vary_fold", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxVary), CreateEdit("0,5", "min R", 6, CreateEditWithMap("mandelbox_vary_minr", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxVary), CreateEdit("1", "R power", 6, CreateEditWithMap("mandelbox_vary_rpower", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxVary), CreateEdit("0", "w add", 6, CreateEditWithMap("mandelbox_vary_wadd", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_mandelbox), Interface.frMandelboxColoring, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frMandelboxColoring), Interface.boxMandelboxColoring);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColoring), Interface.boxMandelboxColor1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor1), CreateEdit("0", "Resultant absolute value component", 6, CreateEditWithMap("mandelbox_color_R", &mapInterface)), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColoring), Interface.boxMandelboxColor2, false, false, 1);
	sGtkEditVector3 edit_mandelboxColorFactor = CreateEditVector3WithMap("mandelbox_color", &mapInterfaceEditVector);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor2), CreateEdit("0,1", "X plane component", 6, edit_mandelboxColorFactor.x), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor2), CreateEdit("0,2", "Y plane component", 6, edit_mandelboxColorFactor.y), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor2), CreateEdit("0,3", "Z plane component", 6, edit_mandelboxColorFactor.z), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColoring), Interface.boxMandelboxColor3, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor3), CreateEdit("5,0", "Min radius component", 6, CreateEditWithMap("mandelbox_color_Sp1", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxMandelboxColor3), CreateEdit("1,0", "Fixed radius component", 6, CreateEditWithMap("mandelbox_color_Sp2", &mapInterface)), false, false, 1);

	//tab OpenCL
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_openCL), Interface.tabsOpenCL, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_openclEngine), Interface.frOpenClSettings, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frOpenClSettings), Interface.boxOpenClSettings);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClSettings), Interface.boxOpenClSwitches1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClSwitches1), Interface.checkOpenClEnable, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClSwitches1), comboOpenCLEngine, false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_openclEngine), Interface.frOpenClInformation, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frOpenClInformation), Interface.boxOpenClInformation);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClInformation), Interface.label_OpenClPlatformBy, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClInformation), Interface.label_OpenClMaxClock, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClInformation), Interface.label_OpenClMemorySize, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClInformation), Interface.label_OpenClComputingUnits, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClInformation), Interface.label_OpenClMaxWorkgroup, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClInformation), Interface.label_OpenClWorkgroupSize, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClInformation), Interface.label_OpenClStatus, false, false, 1);

#ifdef CLSUPPORT
	gtk_box_pack_start(GTK_BOX(Interface.tab_box_openclEngine), Interface.frOpenClEngineSettings, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frOpenClEngineSettings), Interface.boxOpenClEngineSettingsV);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsV), Interface.boxOpenClEngineSettingsH1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsH1),  CreateWidgetWithLabel("Device type:", comboOpenCLGPUCPU), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsH1),  CreateWidgetWithLabel("Platform index:", comboOpenCLPlatformIndex), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsH1),  CreateWidgetWithLabel("Device index:", comboOpenCLDeviceIndex), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsH1),  CreateEdit("256", "Max GPU mem to use [MB]:", 6, CreateEditWithMap("openCL_memory_limit", &mapAppParams)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsV), Interface.boxOpenClEngineSettingsH2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsH2), CreateEdit("1", "Processing cycle time [s] (higher gives better performace)", 6, CreateEditWithMap("openCL_cycle_time", &mapAppParams)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsV), Interface.boxOpenClEngineSettingsH3, false, false, 1);
#ifdef WIN32
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsH3), CreateEdit("notepad.exe", "Text editor:", 40, CreateEditWithMap("openCL_text_editor", &mapAppParams)), false, false, 1);
#else	
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsH3), CreateEdit("/usr/bin/kate", "Text editor:", 40, CreateEditWithMap("openCL_text_editor", &mapAppParams)), false, false, 1);
#endif
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsV), Interface.boxOpenClEngineSettingsH4, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsH4), CreateEdit("1e-5", "Delta for DeltaDE distance estimation:", 6, CreateEditWithMap("ocl_delta_DE_step", &mapInterface)), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClEngineSettingsH4),  CreateWidgetWithLabel("DOF effect method:", comboOpenCLDOFMode), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_openclCustom), Interface.frOpenClCustomSelection, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frOpenClCustomSelection), Interface.boxOpenClCustomV1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomV1), Interface.boxOpenClCustomH11, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomH11), comboOpenCLCustomFormulas, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomH11), Interface.buOpenCLNewFormula, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomH11), Interface.buOpenCLEditFormula, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomH11), Interface.buOpenCLEditFormulaInit, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomH11), Interface.buOpenCLDeleteFormula, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomH11), Interface.buOpenCLRecompile, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomV1), Interface.boxOpenClCustomH12, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomH12), CreateWidgetWithLabel("Distance estimation method:", comboOpenCLDEMode), false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomV1), Interface.boxOpenClCustomH13, false, false, 1);
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomH13), CreateCheckBoxWithMap("ocl_use_custom_formula", "Use custom formula", &mapInterface), false, false, 1);

	gtk_box_pack_start(GTK_BOX(Interface.tab_box_openclCustom), Interface.frOpenClCustomParams, false, false, 1);
	gtk_container_add(GTK_CONTAINER(Interface.frOpenClCustomParams), Interface.boxOpenClCustomV2);
	for(int i=0; i<15; i++)
	{
		char text[100];
		int row = i / 3;
		int column = i % 3;
		sprintf(text, "consts->fractal.custom[%2d]:", i);
		GtkWidget *edit_customParameters = CreateEditWithMapIndexed("ocl_custom_par", i, &mapInterface);
		gtk_table_attach_defaults(GTK_TABLE(Interface.tableOpenCLCustom), CreateEdit("0", text, 6, edit_customParameters), column, column+1, row, row+1);
	}
	gtk_box_pack_start(GTK_BOX(Interface.boxOpenClCustomV2), Interface.tableOpenCLCustom, false, false, 1);
#endif //CLSUPPORT

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
	gtk_box_pack_start(GTK_BOX(Interface.boxLoadSave), Interface.buLoadExample, true, true, 1);
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
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_view, Interface.tab_label_view);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_engine, Interface.tab_label_engine);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_image, Interface.tab_label_image);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_fractal, Interface.tab_label_fractal);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_IFS, Interface.tab_label_IFS);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_hybrid, Interface.tab_label_hybrid);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_mandelbox, Interface.tab_label_mandelbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_shaders, Interface.tab_label_shaders);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_shaders2, Interface.tab_label_shaders2);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_lights, Interface.tab_label_lights);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_animation, Interface.tab_label_animation);
#ifdef CLSUPPORT
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_openCL, Interface.tab_label_openCL);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabsOpenCL), Interface.tab_box_openclEngine, Interface.tab_label_openclEngine);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabsOpenCL), Interface.tab_box_openclCustom, Interface.tab_label_openclCustom);
#endif
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabs), Interface.tab_box_about, Interface.tab_label_about);

	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabsPrimitives), Interface.tab_box_primitivePlane, Interface.tab_label_primitivePlane);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabsPrimitives), Interface.tab_box_primitiveWater, Interface.tab_label_primitiveWater);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabsPrimitives), Interface.tab_box_primitiveBox, Interface.tab_label_primitiveBox);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabsPrimitives), Interface.tab_box_primitiveBoxInv, Interface.tab_label_primitiveBoxInv);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabsPrimitives), Interface.tab_box_primitiveSphere, Interface.tab_label_primitiveSphere);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabsPrimitives), Interface.tab_box_primitiveSphereInv, Interface.tab_label_primitiveSphereInv);

	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabsNetRender), Interface.tab_box_server, Interface.tab_label_server);
	gtk_notebook_append_page(GTK_NOTEBOOK(Interface.tabsNetRender), Interface.tab_box_client, Interface.tab_label_client);

	//main window pack
	gtk_container_add(GTK_CONTAINER(window_interface), Interface.boxMain);

	//CreateTooltips();

	gtk_widget_show_all(window_interface);

	//set default sensivity settings
	//ChangedComboFormula(NULL, NULL);
	//ChangedTgladFoldingMode(NULL, NULL);
	//ChangedJulia(NULL, NULL);
	//ChangedSphericalFoldingMode(NULL, NULL);
	//ChangedLimits(NULL, NULL);
	//ChangedMandelboxRotations(NULL, NULL);
	//ChangedAmbientOcclusion(NULL, NULL);
	//ChangedFastAmbientOcclusion(NULL, NULL);
	//ChangedIterFogEnable(NULL, NULL);

	//Writing default settings
	//WriteInterface(default_settings);
	interfaceCreated = true;
	renderRequest = false;

	//load app settings
	sAppSettings appParams;
#ifdef CLSUPPORT
	if(LoadAppSettings("mandelbulber_ocl_settings", appParams))
#else
	if(LoadAppSettings("mandelbulber_settings", appParams))
#endif
	{
		//WriteInterfaceAppSettings(&appParams);
	}
	else
	{
		printf("Will be used default settings\n");
	}

  g_timeout_add (100,(GSourceFunc)CallerTimerLoop,NULL);
  g_timeout_add (100,(GSourceFunc)CallerTimerLoopWindowRefresh,NULL);

  clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

	//connected signals
	CONNECT_SIGNAL_CLICKED(Interface.buRender, StartRendering);
	CONNECT_SIGNAL_CLICKED(Interface.buStop, StopRendering);
	CONNECT_SIGNAL_CLICKED(Interface.buApplyImageAdjustments, PressedApplyBrigtness);
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
	CONNECT_SIGNAL_CLICKED(Interface.buRotateLeft, PressedNavigatorRotateLeft);
	CONNECT_SIGNAL_CLICKED(Interface.buRotateRight, PressedNavigatorRotateRight);
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
	CONNECT_SIGNAL_CLICKED(Interface.buGetPaletteFromImage, PressedGetPaletteFromImage);
	CONNECT_SIGNAL_CLICKED(Interface.buTimeline, PressedTimeline);
	CONNECT_SIGNAL_CLICKED(Interface.buIFSDefaultDodeca, PressedIFSDefaultDodeca);
	CONNECT_SIGNAL_CLICKED(Interface.buIFSDefaultIcosa, PressedIFSDefaultIcosa);
	CONNECT_SIGNAL_CLICKED(Interface.buIFSDefaultOcta, PressedIFSDefaultOcta);
	CONNECT_SIGNAL_CLICKED(Interface.buIFSDefaultMengerSponge, PressedIFSDefaultMengerSponge);
	CONNECT_SIGNAL_CLICKED(Interface.buIFSReset, PressedIFSReset);

	CONNECT_SIGNAL(renderWindow.comboImageScale, ChangedComboScale, "changed");
	CONNECT_SIGNAL(comboFractType, ChangedComboFormula, "changed");
	CONNECT_SIGNAL_CLICKED(mapInterface["tglad_folding_mode"], ChangedTgladFoldingMode);
	CONNECT_SIGNAL_CLICKED(mapInterface["julia_mode"], ChangedJulia);
	CONNECT_SIGNAL_CLICKED(mapInterface["spherical_folding_mode"], ChangedSphericalFoldingMode);
	CONNECT_SIGNAL_CLICKED(mapInterface["IFS_folding_mode"], ChangedIFSFoldingMode);
	CONNECT_SIGNAL_CLICKED(mapInterface["limits_enabled"], ChangedLimits);
	CONNECT_SIGNAL_CLICKED(mapInterface["ambient_occlusion_enabled"], ChangedAmbientOcclusion);
	CONNECT_SIGNAL_CLICKED(mapInterface["fast_ambient_occlusion_mode"], ChangedFastAmbientOcclusion);
	CONNECT_SIGNAL_CLICKED(mapInterface["mandelbox_rotation_enabled"], ChangedMandelboxRotations);
	CONNECT_SIGNAL_CLICKED(Interface.buAutoDEStep, PressedAutoDEStep);
	CONNECT_SIGNAL_CLICKED(Interface.buAutoDEStepHQ, PressedAutoDEStepHQ);
	CONNECT_SIGNAL_CLICKED(mapInterface["iteration_threshold_mode"], ChangedConstantDEThreshold);
	CONNECT_SIGNAL(comboImageProportion, ChangedImageProportion, "changed");
	CONNECT_SIGNAL(mapInterface["image_height"], ChangedImageProportion, "activate");
	CONNECT_SIGNAL_CLICKED(Interface.buCopyToClipboard, PressedCopyToClipboard);
	CONNECT_SIGNAL_CLICKED(Interface.buGetFromClipboard, PressedPasteFromClipboard);
	CONNECT_SIGNAL_CLICKED(Interface.buLoadExample, PressedLoadExample);
	CONNECT_SIGNAL_CLICKED(Interface.buAutoFog, PressedAutoFog);
	CONNECT_SIGNAL_CLICKED(Interface.buMeasureActivation, PressedMeasureActivation);
	CONNECT_SIGNAL_CLICKED(Interface.checkOpenClEnable, ChangedOpenClEnabled);
	CONNECT_SIGNAL_CLICKED(mapInterface["ocl_use_custom_formula"], ChangedOpenClCustomEnable);
	CONNECT_SIGNAL(comboOpenCLCustomFormulas, ChangedComboOpenCLCustomFormulas, "changed");
	CONNECT_SIGNAL_CLICKED(mapInterface["iteration_fog_enable"], ChangedIterFogEnable);
	CONNECT_SIGNAL_CLICKED(Interface.buSaveAllImageLayers, PressedSaveAllImageLayers);
	CONNECT_SIGNAL_CLICKED(Interface.checkNetRenderServerEnable, PressedServerEnable);
	CONNECT_SIGNAL_CLICKED(Interface.checkNetRenderServerScan, PressedServerScan);
	CONNECT_SIGNAL_CLICKED(Interface.checkNetRenderClientEnable, PressedClientEnable);
	CONNECT_SIGNAL_CLICKED(Interface.buOpenCLEditFormula, PressedOpenCLEditFormula);
	CONNECT_SIGNAL_CLICKED(Interface.buOpenCLEditFormulaInit, PressedOpenCLEditFormulaInit);
	CONNECT_SIGNAL_CLICKED(Interface.buOpenCLNewFormula, PressedOpenCLNewFormula);
	CONNECT_SIGNAL_CLICKED(Interface.buOpenCLDeleteFormula, PressedOpenCLDeleteFormula);
	CONNECT_SIGNAL_CLICKED(Interface.buOpenCLRecompile, PressedRecompile);
	CONNECT_SIGNAL_CLICKED(Interface.buConvertPathToKeyframes, PressedPath2Keyframes);

	gtk_signal_connect(GTK_OBJECT(dareaPalette), "button_press_event", GTK_SIGNAL_FUNC(pressed_button_on_palette), NULL);
	gtk_widget_set_events(GTK_WIDGET(dareaPalette), GDK_BUTTON_PRESS_MASK);

	gtk_signal_connect(GTK_OBJECT(dareaPalette), "expose-event", GTK_SIGNAL_FUNC(on_dareaPalette_expose), NULL);

}

void CreateTooltips(void)
{
#ifndef __sgi
	gtk_widget_set_tooltip_text(Interface.boxCoordinates, "Coordinates of camera view point in fractal units");
	gtk_widget_set_tooltip_text(Interface.comboFractType, "Type of fractal formula");
	gtk_widget_set_tooltip_text(Interface.checkJulia, "Fractal will be rendered as Julia fractal\naccording to coordinates of Julia constant");
	gtk_widget_set_tooltip_text(Interface.boxJulia, "Coordinates of camera view point in fractal units");
	gtk_widget_set_tooltip_text(Interface.boxCoordinates, "Coordinates of camera view point in fractal units");
	gtk_widget_set_tooltip_text(Interface.edit_power, "Fractal formula power\nOnly works with trigonometric formulas\nWhen using Tglad's formula, it is a scale factor");
	gtk_widget_set_tooltip_text(Interface.edit_maxN, "Maximum number of iterations");
	gtk_widget_set_tooltip_text(Interface.edit_minN, "Minimum number of iterations");
	gtk_widget_set_tooltip_text(Interface.edit_DE_thresh, "Dynamic DE threshold factor.\n1 = DE threshold equals to 1 screen pixel\nHigher value gives more details");
	gtk_widget_set_tooltip_text(Interface.edit_DE_stepFactor, "DE steps fractor.\n1 -> step = Estimated Distance\nLower value gives higher accuracy of finding fractal surface");
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
	gtk_widget_set_tooltip_text(Interface.edit_contrast, "Image contrast");
	gtk_widget_set_tooltip_text(Interface.buApplyImageAdjustments, "Applying all image adjustment changes during or after rendering");
	gtk_widget_set_tooltip_text(Interface.edit_shading, "angle of incidence of light effect intensity");
	gtk_widget_set_tooltip_text(Interface.edit_shadows, "shadow intensity");
	gtk_widget_set_tooltip_text(Interface.edit_specular, "intensity of specularity effect");
	gtk_widget_set_tooltip_text(Interface.edit_ambient, "intensity of global ambient light");
	gtk_widget_set_tooltip_text(Interface.edit_ambient_occlusion, "intensity of ambient occlusion effect");
	gtk_widget_set_tooltip_text(Interface.edit_glow, "intensity of glow effect");
	gtk_widget_set_tooltip_text(Interface.edit_reflect, "intensity of texture reflection (environment mapping effect)\nor intersity of raytraced reflections\n");
	gtk_widget_set_tooltip_text(Interface.edit_AmbientOcclusionQuality,
			"ambient occlusion quality\n1 -> 8 rays\n3 -> 64 rays\n5 -> 165 rays\n10 -> 645 rays\n30 -> 5702 rays (hardcore!!!)");
	gtk_widget_set_tooltip_text(Interface.checkShadow, "Enable shadows");
	gtk_widget_set_tooltip_text(Interface.checkAmbientOcclusion, "Enable ambient occlusion effect");
	gtk_widget_set_tooltip_text(Interface.checkFastAmbientOcclusion,
			"Switch to ambient occlusion based on distance estimation");
	gtk_widget_set_tooltip_text(Interface.checkSlowShading,
			"Enable calculation of light's angle of incidence based on fake gradient estimation\nVery slow but works with all fractal formulas");
	gtk_widget_set_tooltip_text(Interface.checkBitmapBackground, "Enable spherical wrapped textured background");
	gtk_widget_set_tooltip_text(Interface.checkBitmapBackgroundFulldome, "Beckground texture in FullDome format");
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

	gtk_widget_set_tooltip_text(Interface.checkNetRenderServerEnable, "Enables server for render farm. Specified port must be not firewalled");
	gtk_widget_set_tooltip_text(Interface.checkNetRenderServerScan, "Enables scanning for client-applications. It will wait for clients as long as it is needed\n"
			"During scanning time all clients have to enabled\n"
			"When the client will be found there will be displayed his IP addres\n"
			"When looking for clients will be done, uncheck scanning icon");

	gtk_widget_set_tooltip_text(Interface.checkNetRenderClientEnable, "Connects the client to specified server\n"
			"Enabling the client have to be done when scanning function on server side is enabled");
#endif
}

bool ReadComandlineParams(int argc, char *argv[])
{
	noGUIdata.animMode = false;
	noGUIdata.keyframeMode = false;
	noGUIdata.playMode = false;
	noGUIdata.startFrame = 0;
	noGUIdata.endFrame = 99999;
	noGUIdata.imageFormat = imgFormatJPG;
	noGUIdata.netrenderMode = false;
	strcpy(noGUIdata.netRenderPortString, "5555");
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
			if (strcmp(argv[i], "-ip") == 0)
			{
				i++;
				if(i < argc)
				{
					if(strlen(argv[i]) < 20)
					{
						strcpy(noGUIdata.netRenderIPString, argv[i]);
						noGUIdata.netrenderMode = true;
						continue;
					}
					else
					{
						printf("commandline: argument must be e.g. -ip 192.168.0.1\n");
						goto bad_arg;
					}
				}
			}
			if (strcmp(argv[i], "-port") == 0)
			{
				i++;
				if(i < argc)
				{
					if(strlen(argv[i]) < 6)
					{
						strcpy(noGUIdata.netRenderPortString, argv[i]);
						continue;
					}
					else
					{
						printf("commandline: argument must be -port NUMBER\n");
						goto bad_arg;
					}
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
				printf("  -flight           - render flight animation\n");
				printf("  -keyframe         - render keyframe animation\n");
				printf("  -start N          - start renderig from frame number N\n");
				printf("  -end N            - rendering will end on frame number N\n");
				printf("  -o key=value      - override item 'key' from settings file with new value 'value'\n");
				printf("  -res WIDTHxHEIGHT - override image resolution\n");
				printf("  -fpk N            - override frames per key parameter\n");
				printf("  -ip N.N.N.N       - set application as a client connected to server of given IP address\n");
				printf("  -port      				- set network port number for Netrender (default 5555)\n");
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
	Interface_data.franeNo = source->fractal.frameNo;
	Interface_data.tileCount = source->tileCount;
}

void InterfaceData2Params(sParamRender *dest)
{
	strcpy(dest->file_destination, Interface_data.file_destination);
	strcpy(dest->file_background, Interface_data.file_background);
	strcpy(dest->file_envmap, Interface_data.file_envmap);
	strcpy(dest->file_lightmap, Interface_data.file_lightmap);
	strcpy(dest->file_path, Interface_data.file_path);
	strcpy(dest->file_keyframes, Interface_data.file_keyframes);
	dest->fractal.frameNo = Interface_data.franeNo;
	dest->tileCount = Interface_data.tileCount;
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
	if(params->reflectionsMax > 100) params->reflectionsMax = 100;
	if(params->noOfTiles < 1) params->noOfTiles = 1;
	if(params->noOfTiles > 100) params->noOfTiles = 100;
	if(params->doubles.fogDensity < 0.0) params->doubles.fogDensity = 0.0;
	if(params->doubles.imageAdjustments.reflect < 0.0) params->doubles.imageAdjustments.reflect = 0.0;
}

sRGB GdkColor2sRGB(GdkColor color)
{
	sRGB color2;
	color2.R = color.red;
	color2.G = color.green;
	color2.B = color.blue;
	return color2;
}

GdkColor sRGB2GdkColor(sRGB color)
{
	GdkColor color2;
	color2.red = color.R;
	color2.green = color.G;
	color2.blue = color.B;
	color2.pixel = 0;
	return color2;
}

sRGB sRGBDiv256(sRGB color)
{
	sRGB color2;
	color2.R = color.R/256;
	color2.G = color.G/256;
	color2.B = color.B/256;
	return color2;
}

//#define CLSUPPORT
#ifdef CLSUPPORT
void Params2Cl(const sParamRender *params, sClInBuff *clInBuff, sClInConstants *clConstantsBuff)
{

	sClParams *clParams = &clConstantsBuff->params;
	sClFractal *clFractal = &clConstantsBuff->fractal;
	clParams->alpha = params->doubles.alpha;
	clParams->beta = params->doubles.beta;
	clParams->gamma = params->doubles.gamma;
	clParams->height = clSupport->GetHeight();
	clParams->width = clSupport->GetWidth();
	clParams->persp = params->doubles.persp;
	clParams->vp = CVector2float3(params->doubles.vp);
	clParams->zoom = params->doubles.zoom;
	clParams->DEfactor = params->doubles.DE_factor;
	clParams->quality = params->doubles.quality;
	clParams->slowShading = params->slowShading;

	clParams->mainLightAlfa = params->doubles.mainLightAlpha;
	clParams->mainLightBeta = params->doubles.mainLightBeta;

	clFractal->N = params->fractal.doubles.N;
	clFractal->power = params->fractal.doubles.power;
	clFractal->juliaMode = params->fractal.juliaMode;
	clFractal->mandelbox.scale = params->fractal.mandelbox.doubles.scale;
	clFractal->mandelbox.foldingLimit = params->fractal.mandelbox.doubles.foldingLimit;
	clFractal->mandelbox.foldingValue = params->fractal.mandelbox.doubles.foldingValue;
	clFractal->mandelbox.minRadius = params->fractal.mandelbox.doubles.foldingSphericalMin;
	clFractal->mandelbox.fixedRadius = params->fractal.mandelbox.doubles.foldingSphericalFixed;
	clFractal->mandelbox.mainRot = RotMatrix2matrix33(params->fractal.mandelbox.mainRot);
	for (int i = 0; i < 6; i++)
	{
		clFractal->mandelbox.rot[i] = RotMatrix2matrix33(params->fractal.mandelbox.rot[i % 2][i / 2]);
		clFractal->mandelbox.rotinv[i] = RotMatrix2matrix33(params->fractal.mandelbox.rotinv[i % 2][i / 2]);
	}
	clFractal->mandelbox.rotEnabled = params->fractal.mandelbox.rotationsEnabled;
	clFractal->mandelbox.colorFactorX = params->fractal.mandelbox.doubles.colorFactorX;
	clFractal->mandelbox.colorFactorX = params->fractal.mandelbox.doubles.colorFactorY;
	clFractal->mandelbox.colorFactorX = params->fractal.mandelbox.doubles.colorFactorZ;
	clFractal->formula = params->fractal.formula;
	clFractal->julia = CVector2float3(params->fractal.doubles.julia);
	clFractal->constantDEThreshold = params->fractal.constantDEThreshold;
	clFractal->fractalConstantFactor = params->fractal.doubles.constantFactor;


	clFractal->ifs.absX = params->fractal.IFS.absX;
	clFractal->ifs.absY = params->fractal.IFS.absY;
	clFractal->ifs.absZ = params->fractal.IFS.absZ;
	clFractal->ifs.mengerSpongeMode = params->fractal.IFS.mengerSpongeMode;
	clFractal->ifs.rotationAlpha = params->fractal.IFS.doubles.rotationAlfa;
	clFractal->ifs.rotationBeta = params->fractal.IFS.doubles.rotationBeta;
	clFractal->ifs.rotationGamma = params->fractal.IFS.doubles.rotationGamma;
	clFractal->ifs.scale = params->fractal.IFS.doubles.scale;
	clFractal->ifs.offset = CVector2float3(params->fractal.IFS.doubles.offset);
	clFractal->ifs.edge = CVector2float3(params->fractal.IFS.doubles.edge);
	clFractal->ifs.mainRot = RotMatrix2matrix33(params->fractal.IFS.mainRot);
	for(int i=0; i<9; i++)
	{
		clFractal->ifs.enabled[i] = params->fractal.IFS.enabled[i];
		clFractal->ifs.rot[i] = RotMatrix2matrix33(params->fractal.IFS.rot[i]);
		clFractal->ifs.distance[i] = params->fractal.IFS.doubles.distance[i];
		clFractal->ifs.alpha[i] = params->fractal.IFS.doubles.alfa[i];
		clFractal->ifs.beta[i] = params->fractal.IFS.doubles.beta[i];
		clFractal->ifs.gamma[i] = params->fractal.IFS.doubles.gamma[i];
		clFractal->ifs.intensity[i] = params->fractal.IFS.doubles.intensity[i];
		clFractal->ifs.direction[i] = CVector2float3(params->fractal.IFS.doubles.direction[i]);
	}

	clFractal->opacity = params->doubles.iterFogOpacity;
	clFractal->opacityTrim = params->doubles.iterFogOpacityTrim;

	clFractal->fakeLightsMinIter = params->fractal.fakeLightsMinIter;
	clFractal->fakeLightsMaxIter = params->fractal.fakeLightsMaxIter;

	clFractal->customOCLFormulaDEMode = params->fractal.customOCLFormulaDEMode;
	clFractal->linearDEmode = params->fractal.linearDEmode;

	for(int i=0; i<15; i++)
		clFractal->custom[i] = params->fractal.doubles.customParameters[i];

	clFractal->amin = params->fractal.doubles.amin;
	clFractal->amax = params->fractal.doubles.amax;
	clFractal->bmin = params->fractal.doubles.bmin;
	clFractal->bmax = params->fractal.doubles.bmax;
	clFractal->cmin = params->fractal.doubles.cmin;
	clFractal->cmax = params->fractal.doubles.cmax;
	clFractal->limitsEnabled = params->fractal.limits_enabled;
	clFractal->interiorMode = params->fractal.interiorMode;

	clFractal->primitives.planeEnable = params->fractal.primitives.planeEnable;
	clFractal->primitives.boxEnable = params->fractal.primitives.boxEnable;
	clFractal->primitives.invertedBoxEnable = params->fractal.primitives.invertedBoxEnable;
	clFractal->primitives.sphereEnable = params->fractal.primitives.sphereEnable;
	clFractal->primitives.invertedSphereEnable = params->fractal.primitives.invertedSphereEnable;
	clFractal->primitives.waterEnable = params->fractal.primitives.waterEnable;

	clFractal->primitives.planeCentre = CVector2float3(params->fractal.doubles.primitives.planeCentre);
	clFractal->primitives.planeNormal = CVector2float3(params->fractal.doubles.primitives.planeNormal);
	clFractal->primitives.boxCentre = CVector2float3(params->fractal.doubles.primitives.boxCentre);
	clFractal->primitives.boxSize = CVector2float3(params->fractal.doubles.primitives.boxSize);
	clFractal->primitives.invertedBoxCentre = CVector2float3(params->fractal.doubles.primitives.invertedBoxCentre);
	clFractal->primitives.invertedBoxSize = CVector2float3(params->fractal.doubles.primitives.invertedBoxSize);
	clFractal->primitives.sphereCentre = CVector2float3(params->fractal.doubles.primitives.sphereCentre);
	clFractal->primitives.invertedSphereCentre = CVector2float3(params->fractal.doubles.primitives.invertedSphereCentre);
	clFractal->primitives.sphereRadius = params->fractal.doubles.primitives.sphereRadius;
	clFractal->primitives.invertedSphereRadius = params->fractal.doubles.primitives.invertedSphereRadius;
	clFractal->primitives.waterHeight = params->fractal.doubles.primitives.waterHeight;
	clFractal->primitives.waterAmplitude = params->fractal.doubles.primitives.waterAmplitude;
	clFractal->primitives.waterLength = params->fractal.doubles.primitives.waterLength;
	clFractal->primitives.waterRotation = params->fractal.doubles.primitives.waterRotation;
	clFractal->primitives.waterAnimSpeed = params->fractal.doubles.primitives.waterAnimSpeed;
	clFractal->primitives.waterIterations = params->fractal.primitives.waterIterations;

	clFractal->primitives.primitivePlaneReflect = params->doubles.primitivePlaneReflect;
	clFractal->primitives.primitiveBoxReflect = params->doubles.primitiveBoxReflect;
	clFractal->primitives.primitiveInvertedBoxReflect = params->doubles.primitiveInvertedBoxReflect;
	clFractal->primitives.primitiveSphereReflect = params->doubles.primitiveSphereReflect;
	clFractal->primitives.primitiveInvertedSphereReflect = params->doubles.primitiveInvertedSphereReflect;
	clFractal->primitives.primitiveWaterReflect = params->doubles.primitiveWaterReflect;

	clFractal->primitives.primitivePlaneColour = sRGB2float3(params->primitivePlaneColour, 65536.0);
	clFractal->primitives.primitiveBoxColour = sRGB2float3(params->primitiveBoxColour, 65536.0);
	clFractal->primitives.primitiveInvertedBoxColour = sRGB2float3(params->primitiveInvertedBoxColour, 65536.0);
	clFractal->primitives.primitiveSphereColour = sRGB2float3(params->primitiveSphereColour, 65536.0);
	clFractal->primitives.primitiveInvertedSphereColour = sRGB2float3(params->primitiveInvertedSphereColour, 65536.0);
	clFractal->primitives.primitiveWaterColour = sRGB2float3(params->primitiveWaterColour, 65536.0);

	clFractal->frameNo = params->fractal.frameNo;

	clFractal->deltaDEStep = params->fractal.doubles.deltaDEStep;

	for(int i=0; i<256; i++)
	{
		clInBuff->palette[i].x = params->palette[i].R/256.0;
		clInBuff->palette[i].y = params->palette[i].G/256.0;
		clInBuff->palette[i].z = params->palette[i].B/256.0;
		clInBuff->palette[i].w = 0.0;
	}

	clParams->colouringSpeed = params->doubles.imageAdjustments.coloring_speed;
	clParams->colouringOffset = params->doubles.imageAdjustments.paletteOffset;
	clParams->ambientOcclusionIntensity = params->doubles.imageAdjustments.globalIlum;
	clParams->specularIntensity = params->doubles.imageAdjustments.specular;
	clParams->mainLightIntensity = params->doubles.imageAdjustments.directLight * params->doubles.imageAdjustments.mainLightIntensity;
	clParams->shading = params->doubles.imageAdjustments.shading;
	clParams->glowIntensity = params->doubles.imageAdjustments.glow_intensity;
	clParams->colouringEnabled = params->imageSwitches.coloringEnabled;
	clParams->fastAmbientOcclusionEnabled = (params->fastGlobalIllumination);
	clParams->slowAmbientOcclusionEnabled = params->global_ilumination && !params->fastGlobalIllumination;
	clParams->shadowConeAngle = params->doubles.shadowConeAngle;
	clParams->penetratingLights = params->penetratingLights;
	clParams->viewDistanceMax = params->doubles.viewDistanceMax;
	clParams->shadow = params->shadow;
	clParams->reflectionsMax = params->reflectionsMax;
	if(!params->imageSwitches.raytracedReflections) clParams->reflectionsMax = 0;
	clParams->reflect = params->doubles.imageAdjustments.reflect;
	clParams->globalIlumQuality = params->globalIlumQuality;
	clParams->fastAoTune = params->doubles.fastAoTune;
	clParams->fogVisibility = pow(10.0, params->doubles.imageAdjustments.fogVisibility / 10 - 16.0);
	clParams->fogEnabled = params->imageSwitches.fogEnabled;
	clParams->auxLightIntensity = params->doubles.auxLightIntensity;
	clParams->SSAOquality = params->SSAOQuality;

	clParams->fogColour = sRGB2float3(params->effectColours.fogColor, 65536.0);
	clParams->glowColour1 = sRGB2float3(params->effectColours.glow_color1, 65536.0);
	clParams->glowColour2 = sRGB2float3(params->effectColours.glow_color2, 65536.0);
	clParams->backgroundColour1 = sRGB2float3(params->background_color1, 65536.0);
	clParams->backgroundColour2 = sRGB2float3(params->background_color2, 65536.0);
	clParams->backgroundColour3 = sRGB2float3(params->background_color3, 65536.0);
	clParams->mainLightColour = sRGB2float3(params->effectColours.mainLightColour, 65536.0);


	clParams->fogColour1Distance = params->doubles.fogColour1Distance;
	clParams->fogColour2Distance = params->doubles.fogColour2Distance;
	clParams->fogDensity = params->doubles.fogDensity;
	clParams->fogDistanceFactor = params->doubles.fogDistanceFactor;
	clParams->fogColour1 = sRGB2float3(params->fogColour1, 65536.0);
	clParams->fogColour2 = sRGB2float3(params->fogColour2, 65536.0);
	clParams->fogColour3 = sRGB2float3(params->fogColour3, 65536.0);

	clParams->DOFEnabled = params->DOFEnabled;
	clParams->DOFFocus = pow(10, params->doubles.DOFFocus / 10.0 - 16.0);
	clParams->DOFRadius = params->doubles.DOFRadius;
	clParams->DOFmethod = params->OpenCLDOFMethod;

  clParams->auxLightVisibility = params->doubles.auxLightVisibility;

  int lightCount = 0;
  for(int i = 0; i < 4; i++)
  {
  	 if (Lights[i].enabled) lightCount = i+1;
  }
  int numberOfLights = max(lightCount, params->auxLightNumber);
	clParams->auxLightNumber = numberOfLights;

	for(int i = 0; i < clParams->auxLightNumber; i++)
	{
		clInBuff->lights[i].enabled = Lights[i].enabled;
		clInBuff->lights[i].intensity = Lights[i].intensity;
		clInBuff->lights[i].position = CVector2float3(Lights[i].position);
		clInBuff->lights[i].colour = sRGB2float3(Lights[i].colour, 65536.0);
	}

	clParams->imageBrightness = params->doubles.imageAdjustments.brightness;
	clParams->imageContrast = params->doubles.imageAdjustments.contrast;
	clParams->imageGamma = params->doubles.imageAdjustments.imageGamma;
	clParams->hdrEnabled = params->imageSwitches.hdrEnabled;

	clParams->perspectiveType = params->perspectiveType;
	clParams->fishEyeCut = params->fishEyeCut;

	clParams->iterFogEnabled = params->imageSwitches.iterFogEnabled;
	clParams->iterFogOpacity = params->doubles.iterFogOpacity;
	clParams->iterFogOpacityTrim = params->doubles.iterFogOpacityTrim;

	for(int i=0; i < 5; i++)
	{
		clParams->volumetricLightEnabled[i] = params->volumetricLightEnabled[i];
		clParams->volumetricLightIntensity[i] = params->doubles.volumetricLightIntensity[i];
	}
	clParams->volumetricLightEnabledAny = params->imageSwitches.volumetricLightEnabled;

	clParams->fakeLightsEnabled = params->fakeLightsEnabled;
	clParams->fakeLightsIntensity = params->doubles.fakeLightsIntensity;
	clParams->fakeLightsVisibility = params->doubles.fakeLightsVisibility;
	clParams->fakeLightsVisibilitySize = params->doubles.fakeLightsVisibilitySize;
	clParams->fakeLightsOrbitTrap = CVector2float3(params->fractal.doubles.fakeLightsOrbitTrap);

	clParams->texturedBackground = params->texturedBackground;
}

matrix33 RotMatrix2matrix33(CRotationMatrix rot)
{
	matrix33 rot33;
	CMatrix33 matrix = rot.GetMatrix();
	rot33.m1 = (cl_float3){{matrix.m11, matrix.m12, matrix.m13}};
	rot33.m2 = (cl_float3){{matrix.m21, matrix.m22, matrix.m23}};
	rot33.m3 = (cl_float3){{matrix.m31, matrix.m32, matrix.m33}};
	return rot33;
}

cl_float3 CVector2float3(CVector3 vect)
{
	cl_float3 vect2 = (cl_float3){{vect.x, vect.y, vect.z}};
	return vect2;
}

cl_float3 sRGB2float3(sRGB colour, double factor)
{
	cl_float3 col = (cl_float3){{colour.R / factor, colour.G / factor, colour.B / factor}};
	return col;
}
#endif

