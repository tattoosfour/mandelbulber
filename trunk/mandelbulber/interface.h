/*
 * interface.h
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof marczak
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "Render3D.h"

#define IFS_number_of_vectors 9

enum enumImageFormat
{
	imgFormatJPG = 0,
	imgFormatPNG = 1,
	imgFormatPNG16 = 2,
	imgFormatPNG16Alpha = 3
};

struct sInterfaceIFS
{
	GtkWidget *editIFSx;
	GtkWidget *editIFSy;
	GtkWidget *editIFSz;
	GtkWidget *editIFSalfa;
	GtkWidget *editIFSbeta;
	GtkWidget *editIFSgamma;
	GtkWidget *editIFSdistance;
	GtkWidget *editIFSintensity;
	GtkWidget *checkIFSenabled;
};

struct sInterface
{
	GtkWidget *tabs;
	GtkWidget *tab_label_view;
	GtkWidget *tab_label_fractal;
	GtkWidget *tab_label_image;
	GtkWidget *tab_label_animation;
	GtkWidget *tab_label_posteffects;
	GtkWidget *tab_label_lights;
	GtkWidget *tab_label_IFS;
	GtkWidget *tab_label_about;
	GtkWidget *tab_label_shaders;
	GtkWidget *tab_label_sound;
	GtkWidget *tab_label_hybrid;
	GtkWidget *tab_box_view;
	GtkWidget *tab_box_fractal;
	GtkWidget *tab_box_image;
	GtkWidget *tab_box_animation;
	GtkWidget *tab_box_posteffects;
	GtkWidget *tab_box_lights;
	GtkWidget *tab_box_IFS;
	GtkWidget *tab_box_about;
	GtkWidget *tab_box_shaders;
	GtkWidget *tab_box_sound;
	GtkWidget *tab_box_hybrid;


	GtkWidget *boxMain;
	GtkWidget *boxButtons;
	GtkWidget *boxView;
	GtkWidget *boxCoordinates;
	GtkWidget *boxAngle;
	GtkWidget *boxZoom;
	GtkWidget *boxArrows;
	GtkWidget *boxArrows2;
	GtkWidget *boxArrows3;
	GtkWidget *boxNavigation;
	GtkWidget *boxNavigationButtons;
	GtkWidget *boxNavigationZooming;
	GtkWidget *boxFractal;
	GtkWidget *boxLimits;
	GtkWidget *boxJulia;
	GtkWidget *boxQuality;
	GtkWidget *boxImage;
	GtkWidget *boxImageRes;
	GtkWidget *boxEffects;
	GtkWidget *boxBrightness;
	GtkWidget *boxShading;
	GtkWidget *boxShading2;
	GtkWidget *boxEffectsChecks;
	GtkWidget *boxEffectsChecks2;
	GtkWidget *boxEffectsColoring;
	GtkWidget *boxColors;
	GtkWidget *boxGlowColor;
	GtkWidget *boxFilenames;
	GtkWidget *boxLoadSave;
	GtkWidget *boxAnimation;
	GtkWidget *boxAnimationButtons;
	GtkWidget *boxAnimationEdits;
	GtkWidget *boxAnimationEdits2;
	GtkWidget *boxTgladFolding;
	GtkWidget *boxSphericalFolding;
	GtkWidget *boxSaveImage;
	GtkWidget *boxPostFog;
	GtkWidget *boxFogButtons;
	GtkWidget *boxFogSlider;
	GtkWidget *boxFogSlider2;
	GtkWidget *boxPostSSAO;
	GtkWidget *boxSSAOSlider;
	GtkWidget *boxSSAOButtons;
	GtkWidget *boxPostDOF;
	GtkWidget *boxDOFSlider1;
	GtkWidget *boxDOFSlider2;
	GtkWidget *boxDOFButtons;
	GtkWidget *boxLightBallance;
	GtkWidget *boxLightsParameters;
	GtkWidget *boxPredefinedLights;
	GtkWidget *boxLightBrightness;
	GtkWidget *boxLightDistribution;
	GtkWidget *boxLightDistribution2;
	GtkWidget *boxLightPre1;
	GtkWidget *boxLightPre2;
	GtkWidget *boxLightPre3;
	GtkWidget *boxLightPre4;
	GtkWidget *boxMainLight;
	GtkWidget *boxMainLightPosition;
	GtkWidget *boxIFSMain;
	GtkWidget *boxIFSMainEdit;
	GtkWidget *boxIFSMainEdit2;
	GtkWidget *boxIFSParams;
	GtkWidget *boxIFSButtons;
	GtkWidget *boxKeyframeAnimation;
	GtkWidget *boxKeyframeAnimationButtons;
	GtkWidget *boxKeyframeAnimationButtons2;
	GtkWidget *boxKeyframeAnimationEdits;
	GtkWidget *boxBottomKeyframeAnimation;
	GtkWidget *boxPalette;
	GtkWidget *boxPaletteOffset;
	GtkWidget *boxImageSaving;
	GtkWidget *boxImageAutoSave;
	GtkWidget *boxSound;
	GtkWidget *boxSoundMisc;
	GtkWidget *boxSoundBand1;
	GtkWidget *boxSoundBand2;
	GtkWidget *boxSoundBand3;
	GtkWidget *boxSoundBand4;
	GtkWidget *boxHybrid;
	GtkWidget *boxStereoscopic;
	GtkWidget *boxStereoParams;

	GtkWidget *tableLimits;
	GtkWidget *tableArrows;
	GtkWidget *tableArrows2;
	GtkWidget *tableIFSParams;
	GtkWidget *tableHybridParams;

	GtkWidget *frCoordinates;
	GtkWidget *fr3Dnavigator;
	GtkWidget *frFractal;
	GtkWidget *frLimits;
	GtkWidget *frImage;
	GtkWidget *frEffects;
	GtkWidget *frColors;
	GtkWidget *frFilenames;
	GtkWidget *frLoadSave;
	GtkWidget *frAnimation;
	GtkWidget *frAnimationFrames;
	GtkWidget *frPostFog;
	GtkWidget *frPostSSAO;
	GtkWidget *frPostDOF;
	GtkWidget *frLightBallance;
	GtkWidget *frLightsParameters;
	GtkWidget *frPredefinedLights;
	GtkWidget *frMainLight;
	GtkWidget *frIFSMain;
	GtkWidget *frIFSParams;
	GtkWidget *frKeyframeAnimation;
	GtkWidget *frKeyframeAnimation2;
	GtkWidget *frPalette;
	GtkWidget *frImageSaving;
	GtkWidget *frSound;
	GtkWidget *frHybrid;
	GtkWidget *frStereo;

	GtkWidget *hSeparator1;
	GtkWidget *hSeparator2;
	GtkWidget *hSeparator3;
	GtkWidget *hSeparator4;
	GtkWidget *vSeparator1;

	GtkWidget *buRender;
	GtkWidget *buStop;
	GtkWidget *buColorGlow1;
	GtkWidget *buColorGlow2;
	GtkWidget *buColorBackgroud1;
	GtkWidget *buColorBackgroud2;
	GtkWidget *buApplyBrighness;
	GtkWidget *buSaveImage;
	GtkWidget *buSavePNG;
	GtkWidget *buSavePNG16;
	GtkWidget *buSavePNG16Alpha;
	GtkWidget *buFiles;
	GtkWidget *buLoadSettings;
	GtkWidget *buSaveSettings;
	GtkWidget *buUp;
	GtkWidget *buDown;
	GtkWidget *buLeft;
	GtkWidget *buRight;
	GtkWidget *buMoveUp;
	GtkWidget *buMoveDown;
	GtkWidget *buMoveLeft;
	GtkWidget *buMoveRight;
	GtkWidget *buForward;
	GtkWidget *buBackward;
	GtkWidget *buInitNavigator;
	GtkWidget *buAnimationRecordTrack;
	GtkWidget *buAnimationContinueRecord;
	GtkWidget *buAnimationRenderTrack;
	GtkWidget *buColorFog;
	GtkWidget *buColorSSAO;
	GtkWidget *buUpdateSSAO;
	GtkWidget *buUpdateDOF;
	GtkWidget *buColorAuxLightPre1;
	GtkWidget *buColorAuxLightPre2;
	GtkWidget *buColorAuxLightPre3;
	GtkWidget *buColorAuxLightPre4;
	GtkWidget *buColorMainLight;
	GtkWidget *buDistributeLights;
	GtkWidget *buIFSNormalizeOffset;
	GtkWidget *buIFSNormalizeVectors;
	GtkWidget *buAnimationRecordKey;
	GtkWidget *buAnimationRenderFromKeys;
	GtkWidget *buUndo;
	GtkWidget *buRedo;
	GtkWidget *buBuddhabrot;
	GtkWidget *buRandomPalette;
	GtkWidget *buLoadSound;
	GtkWidget *buGetPaletteFromImage;
	GtkWidget *buTimeline;

	GtkWidget *edit_va;
	GtkWidget *edit_vb;
	GtkWidget *edit_vc;
	GtkWidget *edit_julia_a;
	GtkWidget *edit_julia_b;
	GtkWidget *edit_julia_c;
	GtkWidget *edit_amin;
	GtkWidget *edit_amax;
	GtkWidget *edit_bmin;
	GtkWidget *edit_bmax;
	GtkWidget *edit_cmin;
	GtkWidget *edit_cmax;
	GtkWidget *edit_alfa;
	GtkWidget *edit_beta;
	GtkWidget *edit_gammaAngle;
	GtkWidget *edit_zoom;
	GtkWidget *edit_persp;
	GtkWidget *edit_maxDepth;
	GtkWidget *edit_maxN;
	GtkWidget *edit_minN;
	GtkWidget *edit_power;
	GtkWidget *edit_DE_thresh;
	GtkWidget *edit_DE_stepFactor;
	GtkWidget *edit_imageWidth;
	GtkWidget *edit_imageHeight;
	GtkWidget *edit_shading;
	GtkWidget *edit_shadows;
	GtkWidget *edit_glow;
	GtkWidget *edit_ambient_occlusion;
	GtkWidget *edit_ambient;
	GtkWidget *edit_brightness;
	GtkWidget *edit_gamma;
	GtkWidget *edit_specular;
	GtkWidget *edit_reflect;
	GtkWidget *edit_AmbientOcclusionQuality;
	GtkWidget *edit_fileOutput;
	GtkWidget *edit_fileEnvMap;
	GtkWidget *edit_fileAmbient;
	GtkWidget *edit_fileBackground;
	GtkWidget *edit_fileKeyframes;
	GtkWidget *edit_step_forward;
	GtkWidget *edit_step_rotation;
	GtkWidget *edit_mouse_click_distance;
	GtkWidget *edit_animationDESpeed;
	GtkWidget *edit_color_seed;
	GtkWidget *edit_tglad_folding_1;
	GtkWidget *edit_tglad_folding_2;
	GtkWidget *edit_spherical_folding_1;
	GtkWidget *edit_spherical_folding_2;
	GtkWidget *edit_color_speed;
	GtkWidget *edit_mainLightIntensity;
	GtkWidget *edit_auxLightIntensity;
	GtkWidget *edit_auxLightRandomSeed;
	GtkWidget *edit_auxLightNumber;
	GtkWidget *edit_auxLightMaxDist;
	GtkWidget *edit_auxLightDistributionRadius;
	GtkWidget *edit_auxLightPre1x;
	GtkWidget *edit_auxLightPre1y;
	GtkWidget *edit_auxLightPre1z;
	GtkWidget *edit_auxLightPre1intensity;
	GtkWidget *edit_auxLightPre2x;
	GtkWidget *edit_auxLightPre2y;
	GtkWidget *edit_auxLightPre2z;
	GtkWidget *edit_auxLightPre2intensity;
	GtkWidget *edit_auxLightPre3x;
	GtkWidget *edit_auxLightPre3y;
	GtkWidget *edit_auxLightPre3z;
	GtkWidget *edit_auxLightPre3intensity;
	GtkWidget *edit_auxLightPre4x;
	GtkWidget *edit_auxLightPre4y;
	GtkWidget *edit_auxLightPre4z;
	GtkWidget *edit_auxLightPre4intensity;
	GtkWidget *edit_auxLightVisibility;
	GtkWidget *edit_mainLightAlfa;
	GtkWidget *edit_mainLightBeta;
	GtkWidget *edit_IFSAlfa;
	GtkWidget *edit_IFSBeta;
	GtkWidget *edit_IFSGamma;
	GtkWidget *edit_IFSScale;
	GtkWidget *edit_IFSOffsetX;
	GtkWidget *edit_IFSOffsetY;
	GtkWidget *edit_IFSOffsetZ;
	GtkWidget *edit_animationFramesPerKey;
	GtkWidget *edit_animationStartFrame;
	GtkWidget *edit_animationEndFrame;
	GtkWidget *edit_sound1FreqMin;
	GtkWidget *edit_sound1FreqMax;
	GtkWidget *edit_sound2FreqMin;
	GtkWidget *edit_sound2FreqMax;
	GtkWidget *edit_sound3FreqMin;
	GtkWidget *edit_sound3FreqMax;
	GtkWidget *edit_sound4FreqMin;
	GtkWidget *edit_sound4FreqMax;
	GtkWidget *edit_soundFPS;
	GtkWidget *edit_hybridIter1;
	GtkWidget *edit_hybridIter2;
	GtkWidget *edit_hybridIter3;
	GtkWidget *edit_hybridIter4;
	GtkWidget *edit_hybridIter5;
	GtkWidget *edit_hybridPower1;
	GtkWidget *edit_hybridPower2;
	GtkWidget *edit_hybridPower3;
	GtkWidget *edit_hybridPower4;
	GtkWidget *edit_hybridPower5;
	GtkWidget *edit_NavigatorAbsoluteDistance;
	GtkWidget *edit_stereoDistance;

	GtkWidget *label_animationFrame;
	GtkWidget *label_animationSpeed;
	GtkWidget *label_animationDistance;
	GtkWidget *label_fog_visibility;
	GtkWidget *label_fog_visibility_front;
	GtkWidget *label_SSAO_quality;
	GtkWidget *label_DOF_focus;
	GtkWidget *label_DOF_radius;
	GtkWidget *label_about;
	GtkWidget *label_auxLightPre1;
	GtkWidget *label_auxLightPre2;
	GtkWidget *label_auxLightPre3;
	GtkWidget *label_auxLightPre4;
	GtkWidget *label_IFSx;
	GtkWidget *label_IFSy;
	GtkWidget *label_IFSz;
	GtkWidget *label_IFSalfa;
	GtkWidget *label_IFSbeta;
	GtkWidget *label_IFSgamma;
	GtkWidget *label_IFSdistance;
	GtkWidget *label_IFSintensity;
	GtkWidget *label_IFSenabled;
	GtkWidget *label_keyframeInfo;
	GtkWidget *label_paletteOffset;
	GtkWidget *label_soundEnvelope;
	GtkWidget *label_HybridFormula1;
	GtkWidget *label_HybridFormula2;
	GtkWidget *label_HybridFormula3;
	GtkWidget *label_HybridFormula4;
	GtkWidget *label_HybridFormula5;
	GtkWidget *label_NavigatorEstimatedDistance;

	GtkWidget *comboFractType;
	GtkWidget *combo_imageScale;
	GtkWidget *comboImageFormat;
	GtkWidget *comboHybridFormula1;
	GtkWidget *comboHybridFormula2;
	GtkWidget *comboHybridFormula3;
	GtkWidget *comboHybridFormula4;
	GtkWidget *comboHybridFormula5;
	GtkWidget *comboHybridDEMethod;

	GtkWidget *progressBar;

	GtkWidget *checkAmbientOcclusion;
	GtkWidget *checkFastAmbientOcclusion;
	GtkWidget *checkShadow;
	GtkWidget *checkIterThresh;
	GtkWidget *checkJulia;
	GtkWidget *checkLimits;
	GtkWidget *checkSlowShading;
	GtkWidget *checkBitmapBackground;
	GtkWidget *checkAnimationSpeedDE;
	GtkWidget *checkColoring;
	GtkWidget *checkTgladMode;
	GtkWidget *checkSphericalFoldingMode;
	GtkWidget *checkIFSFoldingMode;
	GtkWidget *checkFogEnabled;
	GtkWidget *checkSSAOEnabled;
	GtkWidget *checkDOFEnabled;
	GtkWidget *checkZoomClickEnable;
	GtkWidget *checkAuxLightPre1Enabled;
	GtkWidget *checkAuxLightPre2Enabled;
	GtkWidget *checkAuxLightPre3Enabled;
	GtkWidget *checkAuxLightPre4Enabled;
	GtkWidget *checkIFSAbsX;
	GtkWidget *checkIFSAbsY;
	GtkWidget *checkIFSAbsZ;
	GtkWidget *checkAutoSaveImage;
	GtkWidget *checkSoundEnabled;
	GtkWidget *checkHybridCyclic;
	GtkWidget *checkNavigatorAbsoluteDistance;
	GtkWidget *checkNavigatorGoToSurface;
	GtkWidget *checkFishEye;
	GtkWidget *checkStraightRotation;
	GtkWidget *checkStereoEnabled;

	GtkWidget *colorSelectionGlow1;
	GtkWidget *colorSelectionGlow2;

	GtkWidget *pixmap_up;
	GtkWidget *pixmap_down;
	GtkWidget *pixmap_left;
	GtkWidget *pixmap_right;
	GtkWidget *pixmap_move_up;
	GtkWidget *pixmap_move_down;
	GtkWidget *pixmap_move_left;
	GtkWidget *pixmap_move_right;

	GtkWidget *sliderFogDepth;
	GtkWidget *sliderFogDepthFront;
	GtkWidget *sliderSSAOQuality;
	GtkWidget *sliderDOFFocus;
	GtkWidget *sliderDOFRadius;
	GtkWidget *sliderPaletteOffset;

	GtkObject *adjustmentFogDepth;
	GtkObject *adjustmentFogDepthFront;
	GtkObject *adjustmentSSAOQuality;
	GtkObject *adjustmentDOFFocus;
	GtkObject *adjustmentDOFRadius;
	GtkObject *adjustmentPaletteOffset;

	GtkWidget *dareaSound0;
	GtkWidget *dareaSoundA;
	GtkWidget *dareaSoundB;
	GtkWidget *dareaSoundC;
	GtkWidget *dareaSoundD;

	sInterfaceIFS IFSParams[IFS_number_of_vectors];
};

struct sDialogFiles
{
	GtkWidget *window_files;
	GtkWidget *box_main;
	GtkWidget *box_buttons;

	GtkWidget *boxFilePath;
	GtkWidget *boxFileBackground;
	GtkWidget *boxFileImage;
	GtkWidget *boxFileEnvMap;
	GtkWidget *boxFileAmbient;
	GtkWidget *boxFileKeyframes;
	GtkWidget *boxFileSound;

	GtkWidget *table_edits;
	GtkWidget *label_destination;
	GtkWidget *label_envmap;
	GtkWidget *label_lightmap;
	GtkWidget *label_background;
	GtkWidget *label_path;
	GtkWidget *label_keyframes;
	GtkWidget *label_fileSound;

	GtkWidget *edit_destination;
	GtkWidget *edit_envmap;
	GtkWidget *edit_lightmap;
	GtkWidget *edit_background;
	GtkWidget *edit_path;
	GtkWidget *edit_keyframes;
	GtkWidget *edit_sound;
	GtkWidget *bu_ok;
	GtkWidget *bu_cancel;
	GtkWidget *bu_select_destination;
	GtkWidget *bu_select_envmap;
	GtkWidget *bu_select_lightmap;
	GtkWidget *bu_select_background;
	GtkWidget *bu_select_path;
	GtkWidget *bu_select_keyframes;
	GtkWidget *bu_select_sound;
};

struct sTimelineInterface
{
	GtkWidget **darea;
	GtkWidget **label;
	GtkWidget *table;
	GtkAdjustment *windowHadjustment;
	GtkAdjustment *windowVadjustment;
	GtkWidget *scrolledWindow;
	GtkWidget *layoutContainer;
	GtkWidget *boxMain;
	GtkWidget *boxTable;
	GtkWidget *boxButtons;

	GtkWidget *buAnimationRecordKey2;
	GtkWidget *buNextKeyframe;
	GtkWidget *buPreviousKeyframe;

	GtkWidget *editAnimationKeyNumber;
};

struct sInterface_data
{
	bool disableInitRefresh;

	double imageScale;

	bool animMode;
	bool recordMode;
	bool continueRecord;
	bool playMode;
	bool keyframeMode;

	enumImageFormat imageFormat;

	char file_destination[1000];
	char file_envmap[1000];
	char file_background[1000];
	char file_lightmap[1000];
	char file_path[1000];
	char file_keyframes[1000];
	char file_sound[1000];
};

struct sNoGUIdata
{
	char settingsFile[1000];
	bool animMode;
	bool playMode;
	bool keyframeMode;
	bool lowMemMode;
	int startFrame;
	int endFrame;
	enumImageFormat imageFormat;
	sParamRender fractparams;
};
//Global variables

extern sInterface Interface;
extern sTimelineInterface timelineInterface;
extern sInterface_data Interface_data;
extern sNoGUIdata noGUIdata;
extern GtkWidget *window2, *window_histogram, *window_interface;
extern GtkWidget *darea, *darea2, *darea3;
extern GtkWidget *dareaPalette;
extern int x_mouse;
extern int y_mouse;
extern bool programClosed;
extern bool interfaceCreated;
extern bool paletteViewCreated;
extern int scrollbarSize;
extern int lastWindowWidth;
extern int lastWindowHeight;

extern char lastFilenameImage[1000];
extern char lastFilenameSettings[1000];
extern char lastFilenamePalette[1000];

extern GtkWidget *timeLineWindow;

//functions
char* DoubleToString(double value, sAddData *addData = 0);
char* IntToString(int value);
GtkWidget* CreateEdit(const gchar *text, const gchar *txt_label, int size, GtkWidget *edit);
GtkWidget* CreateWidgetWithLabel(const gchar *txt_label, GtkWidget *widget);
void CreateInterface(sParamRender *default_settings);
void ReadInterface(sParamRender *params, sParamSpecial *special = 0);
void WriteInterface(sParamRender *params, sParamSpecial *special = 0);
void CreateFilesDialog(GtkWidget *widget, gpointer data);
void CreateTooltips(void);
bool ReadComandlineParams(int argc, char *argv[]);
double atofData(const gchar* text, sAddData *addData = 0);
void AddComboTextsFractalFormula(GtkComboBox *combo);
enumFractalFormula FormulaNumberGUI2Data(int formula);
int FormulaNumberData2GUI(enumFractalFormula formula);
void Params2InterfaceData(sParamRender *source);
void InterfaceData2Params(sParamRender *dest);

#endif /* INTERFACE_H_ */
