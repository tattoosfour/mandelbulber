/********************************************************
 /                   MANDELBULBER                        *
 /                                                       *
 / author: Krzysztof Marczak                             *
 / contact: buddhi1980@gmail.com                         *
 / licence: GNU GPL                                      *
 ********************************************************/

/*
 * settings.cpp
 *
 *  Created on: 2010-01-24
 *      Author: krzysztof
 */

#include <cstdlib>

#include "files.h"
#include "interface.h"
#include "shaders.h"
#include "settings.h"
#include "smartptr.h"

const char* axis_names[] = {"X", "Y", "Z"};
const char* component_names[] = {"alfa", "beta", "gamma"};

bool paletteLoadedFromSettingsFile = false;

void fprintfDot(FILE *file, const char *string, double value)
{
	char str[100];

	int length = sprintf(str, "%s %.16lg;\n", string, value);

	for (int i = 0; i < length; i++)
	{
		if (str[i] == ',')
		{
			str[i] = '.';
		}
	}
	fprintf(file, "%s", str);
}

double atof2(char *str, bool locale_dot)
{
	double val;
	if (!locale_dot)
	{
		int length = strlen(str);
		for (int i = 0; i < length; i++)
		{
			if (str[i] == '.')
			{
				str[i] = ',';
			}
		}
	}

	val = atof(str);
	return val;
}

void MakePaletteString(const sRGB *palette, char *paletteString)
{
	int length;
	int pointer = 0;
	for (int i = 0; i < 256; i++)
	{
		int colour = palette[i].R * 65536 + palette[i].G * 256 + palette[i].B;
		length = sprintf(&paletteString[pointer], "%x ", colour);
		pointer += length;
	}
}

void GetPaletteFromString(sRGB *palette, char *paletteString)
{
	int index = 0;
	for (int i = 0; i < 2000; i++)
	{
		int colour = 0;
		sscanf(&paletteString[i], "%x", &colour);
		sRGB rgbColour;
		rgbColour.R = colour / 65536;
		rgbColour.G = (colour / 256) % 256;
		rgbColour.B = colour % 256;
		palette[index] = rgbColour;
		//printf("R = %d, G = %d, B = %d\n", rgbColour.R, rgbColour.G, rgbColour.B);

		while (paletteString[i] != ' ' && i < 2000)
		{
			i++;
		}
		index++;
		if (index == 256)
		{
			paletteLoadedFromSettingsFile = true;
			break;
		}
	}
}

void SaveSettings(char *filename, const sParamRender& params)
{
	char *paletteString = new char[257 * 7];
	memset(paletteString, 0, 257 * 7);
	MakePaletteString(params.palette, paletteString);

	FILE * fileSettings;
	fileSettings = fopen(filename, "w");
	fprintfDot(fileSettings, "locale_test", 0.5);
	fprintf(fileSettings, "image_width %d;\n", params.image_width);
	fprintf(fileSettings, "image_height %d;\n", params.image_height);
	fprintfDot(fileSettings, "x_min", params.fractal.doubles.amin);
	fprintfDot(fileSettings, "x_max", params.fractal.doubles.amax);
	fprintfDot(fileSettings, "y_min", params.fractal.doubles.bmin);
	fprintfDot(fileSettings, "y_max", params.fractal.doubles.bmax);
	fprintfDot(fileSettings, "z_min", params.fractal.doubles.cmin);
	fprintfDot(fileSettings, "z_max", params.fractal.doubles.cmax);
	fprintfDot(fileSettings, "view_point_x", params.doubles.vp.x);
	fprintfDot(fileSettings, "view_point_y", params.doubles.vp.y);
	fprintfDot(fileSettings, "view_point_z", params.doubles.vp.z);
	fprintfDot(fileSettings, "angle_alfa", params.doubles.alfa * 180.0 / M_PI);
	fprintfDot(fileSettings, "angle_beta", params.doubles.beta * 180.0 / M_PI);
	fprintfDot(fileSettings, "angle_gamma", params.doubles.gamma * 180.0 / M_PI);
	fprintfDot(fileSettings, "zoom", params.doubles.zoom);
	fprintfDot(fileSettings, "perspective", params.doubles.persp);
	fprintf(fileSettings, "formula %d;\n", params.fractal.formula);
	fprintfDot(fileSettings, "power", params.fractal.doubles.power);
	fprintf(fileSettings, "N %d;\n", params.fractal.N);
	fprintf(fileSettings, "minN %d;\n", params.fractal.minN);
	fprintfDot(fileSettings, "fractal_constant_factor", params.fractal.doubles.constantFactor);
	fprintfDot(fileSettings, "quality", params.doubles.quality);
	fprintfDot(fileSettings, "smoothness", params.doubles.smoothness);
	fprintf(fileSettings, "julia_mode %d;\n", params.fractal.juliaMode);
	fprintfDot(fileSettings, "julia_a", params.fractal.doubles.julia.x);
	fprintfDot(fileSettings, "julia_b", params.fractal.doubles.julia.y);
	fprintfDot(fileSettings, "julia_c", params.fractal.doubles.julia.z);
	fprintf(fileSettings, "tglad_folding_mode %d;\n", params.fractal.tgladFoldingMode);
	fprintfDot(fileSettings, "folding_limit", params.fractal.doubles.foldingLimit);
	fprintfDot(fileSettings, "folding_value", params.fractal.doubles.foldingValue);
	fprintf(fileSettings, "spherical_folding_mode %d;\n", params.fractal.sphericalFoldingMode);
	fprintfDot(fileSettings, "spherical_folding_fixed", params.fractal.doubles.foldingSphericalFixed);
	fprintfDot(fileSettings, "spherical_folding_min", params.fractal.doubles.foldingSphericalMin);
	fprintf(fileSettings, "IFS_folding_mode %d;\n", params.fractal.IFS.foldingMode);
	fprintf(fileSettings, "iteration_threshold_mode %d;\n", params.fractal.iterThresh);
	fprintf(fileSettings, "analityc_DE_mode %d;\n", params.fractal.analitycDE);
	fprintfDot(fileSettings, "DE_factor", params.doubles.DE_factor);
	fprintfDot(fileSettings, "brightness", params.doubles.imageAdjustments.brightness);
	fprintfDot(fileSettings, "gamma", params.doubles.imageAdjustments.imageGamma);
	fprintfDot(fileSettings, "ambient", params.doubles.imageAdjustments.ambient);
	fprintfDot(fileSettings, "reflect", params.doubles.imageAdjustments.reflect);
	fprintfDot(fileSettings, "shadows_intensity", params.doubles.imageAdjustments.directLight);
	fprintfDot(fileSettings, "ambient_occlusion", params.doubles.imageAdjustments.globalIlum);
	fprintf(fileSettings, "ambient_occlusion_quality %d;\n", params.globalIlumQuality);
	fprintfDot(fileSettings, "shading", params.doubles.imageAdjustments.shading);
	fprintfDot(fileSettings, "specular", params.doubles.imageAdjustments.specular);
	fprintfDot(fileSettings, "glow_intensity", params.doubles.imageAdjustments.glow_intensity);
	fprintf(fileSettings, "glow_color_1_R %d;\n", params.effectColours.glow_color1.R);
	fprintf(fileSettings, "glow_color_1_G %d;\n", params.effectColours.glow_color1.G);
	fprintf(fileSettings, "glow_color_1_B %d;\n", params.effectColours.glow_color1.B);
	fprintf(fileSettings, "glow_color_2_R %d;\n", params.effectColours.glow_color2.R);
	fprintf(fileSettings, "glow_color_2_G %d;\n", params.effectColours.glow_color2.G);
	fprintf(fileSettings, "glow_color_2_B %d;\n", params.effectColours.glow_color2.B);
	fprintf(fileSettings, "background_color_1_R %d;\n", params.background_color1.R);
	fprintf(fileSettings, "background_color_1_G %d;\n", params.background_color1.G);
	fprintf(fileSettings, "background_color_1_B %d;\n", params.background_color1.B);
	fprintf(fileSettings, "background_color_2_R %d;\n", params.background_color2.R);
	fprintf(fileSettings, "background_color_2_G %d;\n", params.background_color2.G);
	fprintf(fileSettings, "background_color_2_B %d;\n", params.background_color2.B);
	fprintf(fileSettings, "textured_background %d;\n", params.textured_background);
	fprintf(fileSettings, "shadows_enabled %d;\n", params.shadow);
	fprintf(fileSettings, "ambient_occlusion_enabled %d;\n", params.global_ilumination);
	fprintf(fileSettings, "fast_ambient_occlusion_mode %d;\n", params.fastGlobalIllumination);
	fprintf(fileSettings, "fractal_color %d;\n", params.imageSwitches.coloringEnabled);
	fprintf(fileSettings, "coloring_random_seed %d;\n", params.coloring_seed);
	fprintfDot(fileSettings, "coloring_speed", params.doubles.imageAdjustments.coloring_speed);
	fprintfDot(fileSettings, "coloring_palette_offset", params.doubles.imageAdjustments.paletteOffset);
	fprintf(fileSettings, "slow_shading %d;\n", params.slowShading);
	fprintf(fileSettings, "limits_enabled %d;\n", params.fractal.limits_enabled);
	fprintf(fileSettings, "post_fog_enabled %d;\n", params.imageSwitches.fogEnabled);
	fprintfDot(fileSettings, "post_fog_visibility", params.doubles.imageAdjustments.fogVisibility);
	fprintfDot(fileSettings, "post_fog_visibility_front", params.doubles.imageAdjustments.fogVisibilityFront);
	fprintf(fileSettings, "post_fog_color_R %d;\n", params.effectColours.fogColor.R);
	fprintf(fileSettings, "post_fog_color_G %d;\n", params.effectColours.fogColor.G);
	fprintf(fileSettings, "post_fog_color_B %d;\n", params.effectColours.fogColor.B);
	fprintf(fileSettings, "post_SSAO_enabled %d;\n", params.SSAOEnabled);
	fprintf(fileSettings, "post_SSAO_quality %d;\n", params.SSAOQuality);
	fprintf(fileSettings, "post_DOF_enabled %d;\n", params.DOFEnabled);
	fprintfDot(fileSettings, "post_DOF_focus", params.doubles.DOFFocus);
	fprintfDot(fileSettings, "post_DOF_radius", params.doubles.DOFRadius);
	fprintfDot(fileSettings, "main_light_intensity", params.doubles.imageAdjustments.mainLightIntensity);
	fprintfDot(fileSettings, "main_light_alfa", params.doubles.mainLightAlfa);
	fprintfDot(fileSettings, "main_light_beta", params.doubles.mainLightBeta);
	fprintf(fileSettings, "main_light_colour_R %d;\n", params.effectColours.mainLightColour.R);
	fprintf(fileSettings, "main_light_colour_G %d;\n", params.effectColours.mainLightColour.G);
	fprintf(fileSettings, "main_light_colour_B %d;\n", params.effectColours.mainLightColour.B);
	fprintfDot(fileSettings, "aux_light_intensity", params.doubles.auxLightIntensity);
	fprintf(fileSettings, "aux_light_random_seed %d;\n", params.auxLightRandomSeed);
	fprintf(fileSettings, "aux_light_number %d;\n", params.auxLightNumber);
	fprintfDot(fileSettings, "aux_light_max_dist", params.doubles.auxLightMaxDist);
	fprintfDot(fileSettings, "aux_light_distribution_radius", params.doubles.auxLightDistributionRadius);
	fprintfDot(fileSettings, "aux_light_predefined_1_x", params.doubles.auxLightPre1.x);
	fprintfDot(fileSettings, "aux_light_predefined_1_y", params.doubles.auxLightPre1.y);
	fprintfDot(fileSettings, "aux_light_predefined_1_z", params.doubles.auxLightPre1.z);
	fprintfDot(fileSettings, "aux_light_predefined_1_intensity", params.doubles.auxLightPre1intensity);
	fprintfDot(fileSettings, "aux_light_predefined_2_x", params.doubles.auxLightPre2.x);
	fprintfDot(fileSettings, "aux_light_predefined_2_y", params.doubles.auxLightPre2.y);
	fprintfDot(fileSettings, "aux_light_predefined_2_z", params.doubles.auxLightPre2.z);
	fprintfDot(fileSettings, "aux_light_predefined_2_intensity", params.doubles.auxLightPre2intensity);
	fprintfDot(fileSettings, "aux_light_predefined_3_x", params.doubles.auxLightPre3.x);
	fprintfDot(fileSettings, "aux_light_predefined_3_y", params.doubles.auxLightPre3.y);
	fprintfDot(fileSettings, "aux_light_predefined_3_z", params.doubles.auxLightPre3.z);
	fprintfDot(fileSettings, "aux_light_predefined_3_intensity", params.doubles.auxLightPre3intensity);
	fprintfDot(fileSettings, "aux_light_predefined_4_x", params.doubles.auxLightPre4.x);
	fprintfDot(fileSettings, "aux_light_predefined_4_y", params.doubles.auxLightPre4.y);
	fprintfDot(fileSettings, "aux_light_predefined_4_z", params.doubles.auxLightPre4.z);
	fprintfDot(fileSettings, "aux_light_predefined_4_intensity", params.doubles.auxLightPre4intensity);
	fprintf(fileSettings, "aux_light_predefined_1_enabled %d;\n", params.auxLightPre1Enabled);
	fprintf(fileSettings, "aux_light_predefined_2_enabled %d;\n", params.auxLightPre2Enabled);
	fprintf(fileSettings, "aux_light_predefined_3_enabled %d;\n", params.auxLightPre3Enabled);
	fprintf(fileSettings, "aux_light_predefined_4_enabled %d;\n", params.auxLightPre4Enabled);
	fprintf(fileSettings, "aux_light_predefined_1_colour_R %d;\n", params.auxLightPre1Colour.R);
	fprintf(fileSettings, "aux_light_predefined_1_colour_G %d;\n", params.auxLightPre1Colour.G);
	fprintf(fileSettings, "aux_light_predefined_1_colour_B %d;\n", params.auxLightPre1Colour.B);
	fprintf(fileSettings, "aux_light_predefined_2_colour_R %d;\n", params.auxLightPre2Colour.R);
	fprintf(fileSettings, "aux_light_predefined_2_colour_G %d;\n", params.auxLightPre2Colour.G);
	fprintf(fileSettings, "aux_light_predefined_2_colour_B %d;\n", params.auxLightPre2Colour.B);
	fprintf(fileSettings, "aux_light_predefined_3_colour_R %d;\n", params.auxLightPre3Colour.R);
	fprintf(fileSettings, "aux_light_predefined_3_colour_G %d;\n", params.auxLightPre3Colour.G);
	fprintf(fileSettings, "aux_light_predefined_3_colour_B %d;\n", params.auxLightPre3Colour.B);
	fprintf(fileSettings, "aux_light_predefined_4_colour_R %d;\n", params.auxLightPre4Colour.R);
	fprintf(fileSettings, "aux_light_predefined_4_colour_G %d;\n", params.auxLightPre4Colour.G);
	fprintf(fileSettings, "aux_light_predefined_4_colour_B %d;\n", params.auxLightPre4Colour.B);
	fprintfDot(fileSettings, "aux_light_visibility", params.doubles.auxLightVisibility);
	fprintfDot(fileSettings, "aux_light_random_center_X", params.doubles.auxLightRandomCenter.x);
	fprintfDot(fileSettings, "aux_light_random_center_Y", params.doubles.auxLightRandomCenter.y);
	fprintfDot(fileSettings, "aux_light_random_center_Z", params.doubles.auxLightRandomCenter.z);
	fprintfDot(fileSettings, "IFS_scale", params.fractal.IFS.doubles.scale);
	fprintfDot(fileSettings, "IFS_rot_alfa", params.fractal.IFS.doubles.rotationAlfa);
	fprintfDot(fileSettings, "IFS_rot_beta", params.fractal.IFS.doubles.rotationBeta);
	fprintfDot(fileSettings, "IFS_rot_gamma", params.fractal.IFS.doubles.rotationGamma);
	fprintfDot(fileSettings, "IFS_offsetX", params.fractal.IFS.doubles.offset.x);
	fprintfDot(fileSettings, "IFS_offsetY", params.fractal.IFS.doubles.offset.y);
	fprintfDot(fileSettings, "IFS_offsetZ", params.fractal.IFS.doubles.offset.z);
	fprintf(fileSettings, "IFS_absX %d;\n", params.fractal.IFS.absX);
	fprintf(fileSettings, "IFS_absY %d;\n", params.fractal.IFS.absY);
	fprintf(fileSettings, "IFS_absZ %d;\n", params.fractal.IFS.absZ);

	for (int i = 0; i < IFS_VECTOR_COUNT; i++)
	{
		fprintf(fileSettings, "IFS_%d", i);
		fprintfDot(fileSettings, "_x", params.fractal.IFS.doubles.direction[i].x);
		fprintf(fileSettings, "IFS_%d", i);
		fprintfDot(fileSettings, "_y", params.fractal.IFS.doubles.direction[i].y);
		fprintf(fileSettings, "IFS_%d", i);
		fprintfDot(fileSettings, "_z", params.fractal.IFS.doubles.direction[i].z);
		fprintf(fileSettings, "IFS_%d", i);
		fprintfDot(fileSettings, "_alfa", params.fractal.IFS.doubles.alfa[i]);
		fprintf(fileSettings, "IFS_%d", i);
		fprintfDot(fileSettings, "_beta", params.fractal.IFS.doubles.beta[i]);
		fprintf(fileSettings, "IFS_%d", i);
		fprintfDot(fileSettings, "_gamma", params.fractal.IFS.doubles.gamma[i]);
		fprintf(fileSettings, "IFS_%d", i);
		fprintfDot(fileSettings, "_distance", params.fractal.IFS.doubles.distance[i]);
		fprintf(fileSettings, "IFS_%d", i);
		fprintfDot(fileSettings, "_intensity", params.fractal.IFS.doubles.intensity[i]);
		fprintf(fileSettings, "IFS_%d", i);
		fprintf(fileSettings, "_enabled %d;\n", params.fractal.IFS.enabled[i]);
	}

	fprintf(fileSettings, "start_frame %d;\n", params.startFrame);
	fprintf(fileSettings, "end_frame %d;\n", params.endFrame);
	fprintf(fileSettings, "frames_per_keyframe %d;\n", params.framesPerKeyframe);
	fprintf(fileSettings, "sound_animation_enabled %d;\n", params.soundEnabled);
	fprintfDot(fileSettings, "sound_animation_FPS", params.doubles.soundFPS);
	fprintf(fileSettings, "sound_animation_band1_min %d;\n", params.soundBand1Min);
	fprintf(fileSettings, "sound_animation_band1_max %d;\n", params.soundBand1Max);
	fprintf(fileSettings, "sound_animation_band2_min %d;\n", params.soundBand2Min);
	fprintf(fileSettings, "sound_animation_band2_max %d;\n", params.soundBand2Max);
	fprintf(fileSettings, "sound_animation_band3_min %d;\n", params.soundBand3Min);
	fprintf(fileSettings, "sound_animation_band3_max %d;\n", params.soundBand3Max);
	fprintf(fileSettings, "sound_animation_band4_min %d;\n", params.soundBand4Min);
	fprintf(fileSettings, "sound_animation_band4_max %d;\n", params.soundBand4Max);

	for (int i = 1; i <= HYBRID_COUNT; ++i)
		fprintf(fileSettings, "hybrid_formula_%d %d;\n", i, params.fractal.hybridFormula[i-1]);

	for (int i = 1; i <= HYBRID_COUNT; ++i)
		fprintf(fileSettings, "hybrid_iterations_%d %d;\n", i, params.fractal.hybridIters[i-1]);

	for (int i = 1; i <= HYBRID_COUNT; ++i) {
		fprintf(fileSettings, "hybrid_power_%d", i);
		fprintfDot(fileSettings, "", params.fractal.doubles.hybridPower[i-1]);
	}

	fprintf(fileSettings, "hybrid_cyclic %d;\n", params.fractal.hybridCyclic);
	fprintf(fileSettings, "fish_eye %d;\n", params.perspectiveType);
	fprintfDot(fileSettings, "stereo_eye_distance", params.doubles.stereoEyeDistance);
	fprintf(fileSettings, "stereo_enabled %d;\n", params.stereoEnabled);

	fprintfDot(fileSettings, "mandelbox_scale", params.fractal.mandelbox.doubles.scale);
	fprintfDot(fileSettings, "mandelbox_folding_limit", params.fractal.mandelbox.doubles.foldingLimit);
	fprintfDot(fileSettings, "mandelbox_folding_value", params.fractal.mandelbox.doubles.foldingValue);
	fprintfDot(fileSettings, "mandelbox_folding_min_radius", params.fractal.mandelbox.doubles.foldingSphericalMin);
	fprintfDot(fileSettings, "mandelbox_folding_fixed_radius", params.fractal.mandelbox.doubles.foldingSphericalFixed);
	fprintfDot(fileSettings, "mandelbox_sharpness", params.fractal.mandelbox.doubles.sharpness);

	for (int component = 0; component < 3; ++component) {
		fprintf(fileSettings, "mandelbox_rotation_main_%s", component_names[component]);
		fprintfDot(fileSettings, "", params.fractal.mandelbox.doubles.rotationMain[component] * 180.0 / M_PI);
	}

	for (int fold = 0; fold < MANDELBOX_FOLDS; ++fold)
		for (int axis = 0; axis < 3; ++axis)
			for (int component = 0; component < 3; ++component) {
				fprintf(fileSettings, "mandelbox_rotation_%s%d_%s", axis_names[axis], fold + 1, component_names[component]);
				fprintfDot(fileSettings, "", params.fractal.mandelbox.doubles.rotation[fold][axis][component] * 180.0 / M_PI);
			}

	fprintfDot(fileSettings, "mandelbox_color_R", params.fractal.mandelbox.doubles.colorFactorR);
	fprintfDot(fileSettings, "mandelbox_color_X", params.fractal.mandelbox.doubles.colorFactorX);
	fprintfDot(fileSettings, "mandelbox_color_Y", params.fractal.mandelbox.doubles.colorFactorY);
	fprintfDot(fileSettings, "mandelbox_color_Z", params.fractal.mandelbox.doubles.colorFactorZ);
	fprintfDot(fileSettings, "mandelbox_color_Sp1", params.fractal.mandelbox.doubles.colorFactorSp1);
	fprintfDot(fileSettings, "mandelbox_color_Sp2", params.fractal.mandelbox.doubles.colorFactorSp2);
	fprintf(fileSettings, "mandelbox_rotation_enabled %d;\n", params.fractal.mandelbox.rotationsEnabled);

	fprintfDot(fileSettings, "view_distance_max", params.doubles.viewDistanceMax);
	fprintfDot(fileSettings, "view_distance_min", params.doubles.viewDistanceMin);
	fprintf(fileSettings, "interior_mode %d;\n", params.fractal.interiorMode);
	fprintfDot(fileSettings, "FoldingIntPow_folding_factor", params.fractal.doubles.FoldingIntPowFoldFactor);
	fprintfDot(fileSettings, "FoldingIntPow_z_factor", params.fractal.doubles.FoldingIntPowZfactor);
	fprintf(fileSettings, "dynamic_DE_correction %d;\n", params.fractal.dynamicDEcorrection);
	fprintf(fileSettings, "linear_DE_mode %d;\n", params.fractal.linearDEmode);
	fprintf(fileSettings, "constant_DE_threshold %d;\n", params.fractal.constantDEThreshold);

	fprintf(fileSettings, "file_destination %s;\n", params.file_destination);
	fprintf(fileSettings, "file_background %s;\n", params.file_background);
	fprintf(fileSettings, "file_envmap %s;\n", params.file_envmap);
	fprintf(fileSettings, "file_lightmap %s;\n", params.file_lightmap);
	fprintf(fileSettings, "file_animation_path %s;\n", params.file_path);
	fprintf(fileSettings, "file_keyframes %s;\n", params.file_keyframes);
	fprintf(fileSettings, "file_sound %s;\n", params.file_sound);

	fprintf(fileSettings, "palette %s;\n", paletteString);

	fclose(fileSettings);

	delete[] paletteString;
}

bool LoadSettings(char *filename, sParamRender &params, bool disableMessages)
{
	DefaultValues(params);
	paletteLoadedFromSettingsFile = false;

	bool newMandelboxParametersLoaded = false;

	bool locale_dot = false;
	char str1[100];
	char str2[2000];
	char str3[100];

	FILE * fileSettings;
	fileSettings = fopen(filename, "r");

	int lineCounter = 0;

	if (fileSettings)
	{
		while (!feof(fileSettings))
		{
			lineCounter++;
			int c = fscanf(fileSettings, "%s", str1);
			if (c > 0)
			{
				c = fscanf(fileSettings, "%[ ]", str2);
				c = fscanf(fileSettings, "%[^;]", str2);

				//IFS params
				bool IFSresult = false;
				for (int i = 0; i < IFS_VECTOR_COUNT; i++)
				{
					c = sprintf(str3, "IFS_%d_x", i);
					if (!strcmp(str1, str3))
					{
						params.fractal.IFS.doubles.direction[i].x = atof2(str2, locale_dot);
						IFSresult = true;
						break;
					}
					c = sprintf(str3, "IFS_%d_y", i);
					if (!strcmp(str1, str3))
					{
						params.fractal.IFS.doubles.direction[i].y = atof2(str2, locale_dot);
						IFSresult = true;
						break;
					}
					c = sprintf(str3, "IFS_%d_z", i);
					if (!strcmp(str1, str3))
					{
						params.fractal.IFS.doubles.direction[i].z = atof2(str2, locale_dot);
						IFSresult = true;
						break;
					}
					c = sprintf(str3, "IFS_%d_alfa", i);
					if (!strcmp(str1, str3))
					{
						params.fractal.IFS.doubles.alfa[i] = atof2(str2, locale_dot);
						IFSresult = true;
						break;
					}
					c = sprintf(str3, "IFS_%d_beta", i);
					if (!strcmp(str1, str3))
					{
						params.fractal.IFS.doubles.beta[i] = atof2(str2, locale_dot);
						IFSresult = true;
						break;
					}
					c = sprintf(str3, "IFS_%d_gamma", i);
					if (!strcmp(str1, str3))
					{
						params.fractal.IFS.doubles.gamma[i] = atof2(str2, locale_dot);
						IFSresult = true;
						break;
					}
					c = sprintf(str3, "IFS_%d_distance", i);
					if (!strcmp(str1, str3))
					{
						params.fractal.IFS.doubles.distance[i] = atof2(str2, locale_dot);
						IFSresult = true;
						break;
					}
					c = sprintf(str3, "IFS_%d_intensity", i);
					if (!strcmp(str1, str3))
					{
						params.fractal.IFS.doubles.intensity[i] = atof2(str2, locale_dot);
						IFSresult = true;
						break;
					}
					c = sprintf(str3, "IFS_%d_enabled", i);
					if (!strcmp(str1, str3))
					{
						params.fractal.IFS.enabled[i] = atoi(str2);
						IFSresult = true;
						break;
					}
				}

				if (!strcmp(str1, "locale_test"))
				{
					double val = atof(str2);
					if (val == 0.5) locale_dot = true;
					else locale_dot = false;
				}
				else if (IFSresult) IFSresult = false;
				else if (!strcmp(str1, "image_width")) params.image_width = atoi(str2);
				else if (!strcmp(str1, "image_height")) params.image_height = atoi(str2);
				else if (!strcmp(str1, "x_min")) params.fractal.doubles.amin = atof2(str2, locale_dot);
				else if (!strcmp(str1, "x_max")) params.fractal.doubles.amax = atof2(str2, locale_dot);
				else if (!strcmp(str1, "y_min")) params.fractal.doubles.bmin = atof2(str2, locale_dot);
				else if (!strcmp(str1, "y_max")) params.fractal.doubles.bmax = atof2(str2, locale_dot);
				else if (!strcmp(str1, "z_min")) params.fractal.doubles.cmin = atof2(str2, locale_dot);
				else if (!strcmp(str1, "z_max")) params.fractal.doubles.cmax = atof2(str2, locale_dot);
				else if (!strcmp(str1, "view_point_x")) params.doubles.vp.x = atof2(str2, locale_dot);
				else if (!strcmp(str1, "view_point_y")) params.doubles.vp.y = atof2(str2, locale_dot);
				else if (!strcmp(str1, "view_point_z")) params.doubles.vp.z = atof2(str2, locale_dot);
				else if (!strcmp(str1, "angle_alfa")) params.doubles.alfa = atof2(str2, locale_dot) / 180.0 * M_PI;
				else if (!strcmp(str1, "angle_beta")) params.doubles.beta = atof2(str2, locale_dot) / 180.0 * M_PI;
				else if (!strcmp(str1, "angle_gamma")) params.doubles.gamma = atof2(str2, locale_dot) / 180.0 * M_PI;
				else if (!strcmp(str1, "zoom")) params.doubles.zoom = atof2(str2, locale_dot);
				else if (!strcmp(str1, "perspective")) params.doubles.persp = atof2(str2, locale_dot);
				else if (!strcmp(str1, "formula")) params.fractal.formula = (enumFractalFormula) atoi(str2);
				else if (!strcmp(str1, "power")) params.fractal.doubles.power = atof2(str2, locale_dot);
				else if (!strcmp(str1, "N")) params.fractal.N = atoi(str2);
				else if (!strcmp(str1, "minN")) params.fractal.minN = atoi(str2);
				else if (!strcmp(str1, "fractal_constant_factor")) params.fractal.doubles.constantFactor = atof2(str2, locale_dot);
				else if (!strcmp(str1, "quality")) params.doubles.quality = atof2(str2, locale_dot);
				else if (!strcmp(str1, "smoothness")) params.doubles.smoothness = atof2(str2, locale_dot);
				else if (!strcmp(str1, "julia_mode")) params.fractal.juliaMode = atoi(str2);
				else if (!strcmp(str1, "julia_a")) params.fractal.doubles.julia.x = atof2(str2, locale_dot);
				else if (!strcmp(str1, "julia_b")) params.fractal.doubles.julia.y = atof2(str2, locale_dot);
				else if (!strcmp(str1, "julia_c")) params.fractal.doubles.julia.z = atof2(str2, locale_dot);
				else if (!strcmp(str1, "tglad_folding_mode")) params.fractal.tgladFoldingMode = atoi(str2);
				else if (!strcmp(str1, "folding_limit")) params.fractal.doubles.foldingLimit = atof2(str2, locale_dot);
				else if (!strcmp(str1, "folding_value")) params.fractal.doubles.foldingValue = atof2(str2, locale_dot);
				else if (!strcmp(str1, "spherical_folding_mode")) params.fractal.sphericalFoldingMode = atoi(str2);
				else if (!strcmp(str1, "spherical_folding_fixed")) params.fractal.doubles.foldingSphericalFixed = atof2(str2, locale_dot);
				else if (!strcmp(str1, "spherical_folding_min")) params.fractal.doubles.foldingSphericalMin = atof2(str2, locale_dot);
				else if (!strcmp(str1, "IFS_folding_mode")) params.fractal.IFS.foldingMode = atoi(str2);
				else if (!strcmp(str1, "iteration_threshold_mode")) params.fractal.iterThresh = atoi(str2);
				else if (!strcmp(str1, "analityc_DE_mode")) params.fractal.analitycDE = atoi(str2);
				else if (!strcmp(str1, "DE_factor")) params.doubles.DE_factor = atof2(str2, locale_dot);
				else if (!strcmp(str1, "brightness")) params.doubles.imageAdjustments.brightness = atof2(str2, locale_dot);
				else if (!strcmp(str1, "gamma")) params.doubles.imageAdjustments.imageGamma = atof2(str2, locale_dot);
				else if (!strcmp(str1, "ambient")) params.doubles.imageAdjustments.ambient = atof2(str2, locale_dot);
				else if (!strcmp(str1, "reflect")) params.doubles.imageAdjustments.reflect = atof2(str2, locale_dot);
				else if (!strcmp(str1, "shadows_intensity")) params.doubles.imageAdjustments.directLight = atof2(str2, locale_dot);
				else if (!strcmp(str1, "ambient_occlusion")) params.doubles.imageAdjustments.globalIlum = atof2(str2, locale_dot);
				else if (!strcmp(str1, "ambient_occlusion_quality")) params.globalIlumQuality = atoi(str2);
				else if (!strcmp(str1, "shading")) params.doubles.imageAdjustments.shading = atof2(str2, locale_dot);
				else if (!strcmp(str1, "specular")) params.doubles.imageAdjustments.specular = atof2(str2, locale_dot);
				else if (!strcmp(str1, "glow_intensity")) params.doubles.imageAdjustments.glow_intensity = atof2(str2, locale_dot);
				else if (!strcmp(str1, "glow_color_1_R")) params.effectColours.glow_color1.R = atoi(str2);
				else if (!strcmp(str1, "glow_color_1_G")) params.effectColours.glow_color1.G = atoi(str2);
				else if (!strcmp(str1, "glow_color_1_B")) params.effectColours.glow_color1.B = atoi(str2);
				else if (!strcmp(str1, "glow_color_2_R")) params.effectColours.glow_color2.R = atoi(str2);
				else if (!strcmp(str1, "glow_color_2_G")) params.effectColours.glow_color2.G = atoi(str2);
				else if (!strcmp(str1, "glow_color_2_B")) params.effectColours.glow_color2.B = atoi(str2);
				else if (!strcmp(str1, "background_color_1_R")) params.background_color1.R = atoi(str2);
				else if (!strcmp(str1, "background_color_1_G")) params.background_color1.G = atoi(str2);
				else if (!strcmp(str1, "background_color_1_B")) params.background_color1.B = atoi(str2);
				else if (!strcmp(str1, "background_color_2_R")) params.background_color2.R = atoi(str2);
				else if (!strcmp(str1, "background_color_2_G")) params.background_color2.G = atoi(str2);
				else if (!strcmp(str1, "background_color_2_B")) params.background_color2.B = atoi(str2);
				else if (!strcmp(str1, "textured_background")) params.textured_background = atoi(str2);
				else if (!strcmp(str1, "shadows_enabled")) params.shadow = atoi(str2);
				else if (!strcmp(str1, "ambient_occlusion_enabled")) params.global_ilumination = atoi(str2);
				else if (!strcmp(str1, "fast_ambient_occlusion_mode")) params.fastGlobalIllumination = atoi(str2);
				else if (!strcmp(str1, "fractal_color")) params.imageSwitches.coloringEnabled = atoi(str2);
				else if (!strcmp(str1, "coloring_random_seed")) params.coloring_seed = atoi(str2);
				else if (!strcmp(str1, "coloring_speed")) params.doubles.imageAdjustments.coloring_speed = atof2(str2, locale_dot);
				else if (!strcmp(str1, "coloring_palette_offset")) params.doubles.imageAdjustments.paletteOffset = atof2(str2, locale_dot);
				else if (!strcmp(str1, "slow_shading")) params.slowShading = atoi(str2);
				else if (!strcmp(str1, "limits_enabled")) params.fractal.limits_enabled = atoi(str2);
				else if (!strcmp(str1, "post_fog_enabled")) params.imageSwitches.fogEnabled = atoi(str2);
				else if (!strcmp(str1, "post_fog_visibility")) params.doubles.imageAdjustments.fogVisibility = atof2(str2, locale_dot);
				else if (!strcmp(str1, "post_fog_visibility_front")) params.doubles.imageAdjustments.fogVisibilityFront = atof2(str2, locale_dot);
				else if (!strcmp(str1, "post_fog_color_R")) params.effectColours.fogColor.R = atoi(str2);
				else if (!strcmp(str1, "post_fog_color_G")) params.effectColours.fogColor.G = atoi(str2);
				else if (!strcmp(str1, "post_fog_color_B")) params.effectColours.fogColor.B = atoi(str2);
				else if (!strcmp(str1, "post_SSAO_enabled")) params.SSAOEnabled = atoi(str2);
				else if (!strcmp(str1, "post_SSAO_quality")) params.SSAOQuality = atoi(str2);
				else if (!strcmp(str1, "post_DOF_enabled")) params.DOFEnabled = atoi(str2);
				else if (!strcmp(str1, "post_DOF_focus")) params.doubles.DOFFocus = atof2(str2, locale_dot);
				else if (!strcmp(str1, "post_DOF_radius")) params.doubles.DOFRadius = atof2(str2, locale_dot);
				else if (!strcmp(str1, "main_light_intensity")) params.doubles.imageAdjustments.mainLightIntensity = atof2(str2, locale_dot);
				else if (!strcmp(str1, "main_light_alfa")) params.doubles.mainLightAlfa = atof2(str2, locale_dot);
				else if (!strcmp(str1, "main_light_beta")) params.doubles.mainLightBeta = atof2(str2, locale_dot);
				else if (!strcmp(str1, "main_light_colour_R")) params.effectColours.mainLightColour.R = atoi(str2);
				else if (!strcmp(str1, "main_light_colour_G")) params.effectColours.mainLightColour.G = atoi(str2);
				else if (!strcmp(str1, "main_light_colour_B")) params.effectColours.mainLightColour.B = atoi(str2);
				else if (!strcmp(str1, "aux_light_intensity")) params.doubles.auxLightIntensity = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_random_seed")) params.auxLightRandomSeed = atoi(str2);
				else if (!strcmp(str1, "aux_light_number")) params.auxLightNumber = atoi(str2);
				else if (!strcmp(str1, "aux_light_max_dist")) params.doubles.auxLightMaxDist = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_distribution_radius")) params.doubles.auxLightDistributionRadius = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_1_x")) params.doubles.auxLightPre1.x = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_1_y")) params.doubles.auxLightPre1.y = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_1_z")) params.doubles.auxLightPre1.z = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_1_intensity")) params.doubles.auxLightPre1intensity = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_2_x")) params.doubles.auxLightPre2.x = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_2_y")) params.doubles.auxLightPre2.y = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_2_z")) params.doubles.auxLightPre2.z = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_2_intensity")) params.doubles.auxLightPre2intensity = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_3_x")) params.doubles.auxLightPre3.x = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_3_y")) params.doubles.auxLightPre3.y = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_3_z")) params.doubles.auxLightPre3.z = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_3_intensity")) params.doubles.auxLightPre3intensity = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_4_x")) params.doubles.auxLightPre4.x = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_4_y")) params.doubles.auxLightPre4.y = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_4_z")) params.doubles.auxLightPre4.z = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_4_intensity")) params.doubles.auxLightPre4intensity = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_predefined_1_enabled")) params.auxLightPre1Enabled = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_2_enabled")) params.auxLightPre2Enabled = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_3_enabled")) params.auxLightPre3Enabled = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_4_enabled")) params.auxLightPre4Enabled = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_1_colour_R")) params.auxLightPre1Colour.R = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_1_colour_G")) params.auxLightPre1Colour.G = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_1_colour_B")) params.auxLightPre1Colour.B = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_2_colour_R")) params.auxLightPre2Colour.R = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_2_colour_G")) params.auxLightPre2Colour.G = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_2_colour_B")) params.auxLightPre2Colour.B = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_3_colour_R")) params.auxLightPre3Colour.R = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_3_colour_G")) params.auxLightPre3Colour.G = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_3_colour_B")) params.auxLightPre3Colour.B = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_4_colour_R")) params.auxLightPre4Colour.R = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_4_colour_G")) params.auxLightPre4Colour.G = atoi(str2);
				else if (!strcmp(str1, "aux_light_predefined_4_colour_B")) params.auxLightPre4Colour.B = atoi(str2);
				else if (!strcmp(str1, "aux_light_visibility")) params.doubles.auxLightVisibility = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_random_center_X")) params.doubles.auxLightRandomCenter.x = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_random_center_Y")) params.doubles.auxLightRandomCenter.y = atof2(str2, locale_dot);
				else if (!strcmp(str1, "aux_light_random_center_Z")) params.doubles.auxLightRandomCenter.z = atof2(str2, locale_dot);

				else if (!strcmp(str1, "IFS_scale")) params.fractal.IFS.doubles.scale = atof2(str2, locale_dot);
				else if (!strcmp(str1, "IFS_rot_alfa")) params.fractal.IFS.doubles.rotationAlfa = atof2(str2, locale_dot);
				else if (!strcmp(str1, "IFS_rot_beta")) params.fractal.IFS.doubles.rotationBeta = atof2(str2, locale_dot);
				else if (!strcmp(str1, "IFS_rot_gamma")) params.fractal.IFS.doubles.rotationGamma = atof2(str2, locale_dot);
				else if (!strcmp(str1, "IFS_offsetX")) params.fractal.IFS.doubles.offset.x = atof2(str2, locale_dot);
				else if (!strcmp(str1, "IFS_offsetY")) params.fractal.IFS.doubles.offset.y = atof2(str2, locale_dot);
				else if (!strcmp(str1, "IFS_offsetZ")) params.fractal.IFS.doubles.offset.z = atof2(str2, locale_dot);
				else if (!strcmp(str1, "IFS_absX")) params.fractal.IFS.absX = atof(str2);
				else if (!strcmp(str1, "IFS_absY")) params.fractal.IFS.absY = atof(str2);
				else if (!strcmp(str1, "IFS_absZ")) params.fractal.IFS.absZ = atof(str2);

				else if (!strcmp(str1, "start_frame")) params.startFrame = atoi(str2);
				else if (!strcmp(str1, "end_frame")) params.endFrame = atoi(str2);
				else if (!strcmp(str1, "frames_per_keyframe")) params.framesPerKeyframe = atoi(str2);

				else if (!strcmp(str1, "sound_animation_enabled")) params.soundEnabled = atoi(str2);
				else if (!strcmp(str1, "sound_animation_FPS")) params.doubles.soundFPS = atof2(str2, locale_dot);
				else if (!strcmp(str1, "sound_animation_band1_min")) params.soundBand1Min = atoi(str2);
				else if (!strcmp(str1, "sound_animation_band1_max")) params.soundBand1Max = atoi(str2);
				else if (!strcmp(str1, "sound_animation_band2_min")) params.soundBand2Min = atoi(str2);
				else if (!strcmp(str1, "sound_animation_band2_max")) params.soundBand2Max = atoi(str2);
				else if (!strcmp(str1, "sound_animation_band3_min")) params.soundBand3Min = atoi(str2);
				else if (!strcmp(str1, "sound_animation_band3_max")) params.soundBand3Max = atoi(str2);
				else if (!strcmp(str1, "sound_animation_band4_min")) params.soundBand4Min = atoi(str2);
				else if (!strcmp(str1, "sound_animation_band4_max")) params.soundBand4Max = atoi(str2);

			  else if (!strcmp(str1, "hybrid_cyclic")) params.fractal.hybridCyclic = atoi(str2);

				else if (!strcmp(str1, "fish_eye")) params.perspectiveType = (enumPerspectiveType)atoi(str2);

				else if (!strcmp(str1, "stereo_enabled")) params.stereoEnabled = atoi(str2);
				else if (!strcmp(str1, "stereo_eye_distance")) params.doubles.stereoEyeDistance = atof2(str2, locale_dot);

				else if (!strcmp(str1, "mandelbox_scale")) {params.fractal.mandelbox.doubles.scale = atof2(str2, locale_dot); newMandelboxParametersLoaded = true;}
				else if (!strcmp(str1, "mandelbox_folding_limit")) params.fractal.mandelbox.doubles.foldingLimit = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_folding_value")) params.fractal.mandelbox.doubles.foldingValue = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_folding_min_radius")) params.fractal.mandelbox.doubles.foldingSphericalMin = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_folding_fixed_radius")) params.fractal.mandelbox.doubles.foldingSphericalFixed = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_sharpness")) params.fractal.mandelbox.doubles.sharpness = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_color_R")) params.fractal.mandelbox.doubles.colorFactorR = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_color_X")) params.fractal.mandelbox.doubles.colorFactorX = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_color_Y")) params.fractal.mandelbox.doubles.colorFactorY = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_color_Z")) params.fractal.mandelbox.doubles.colorFactorZ = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_color_Sp1")) params.fractal.mandelbox.doubles.colorFactorSp1 = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_color_Sp2")) params.fractal.mandelbox.doubles.colorFactorSp2 = atof2(str2, locale_dot);
				else if (!strcmp(str1, "mandelbox_rotation_enabled")) params.fractal.mandelbox.rotationsEnabled = atoi(str2);

				else if (!strcmp(str1, "view_distance_max")) params.doubles.viewDistanceMax = atof2(str2, locale_dot);
				else if (!strcmp(str1, "view_distance_min")) params.doubles.viewDistanceMin = atof2(str2, locale_dot);
				else if (!strcmp(str1, "interior_mode")) params.fractal.interiorMode = atoi(str2);
				else if (!strcmp(str1, "dynamic_DE_correction")) params.fractal.dynamicDEcorrection = atoi(str2);
				else if (!strcmp(str1, "linear_DE_mode")) params.fractal.linearDEmode = atoi(str2);
				else if (!strcmp(str1, "constant_DE_threshold")) params.fractal.constantDEThreshold = atoi(str2);

				else if (!strcmp(str1, "FoldingIntPow_folding_factor")) params.fractal.doubles.FoldingIntPowFoldFactor = atof2(str2, locale_dot);
				else if (!strcmp(str1, "FoldingIntPow_z_factor")) params.fractal.doubles.FoldingIntPowZfactor = atof2(str2, locale_dot);

				else if (!strcmp(str1, "file_destination")) strcpy(params.file_destination, str2);
				else if (!strcmp(str1, "file_background")) strcpy(params.file_background, str2);
				else if (!strcmp(str1, "file_envmap")) strcpy(params.file_envmap, str2);
				else if (!strcmp(str1, "file_lightmap")) strcpy(params.file_lightmap, str2);
				else if (!strcmp(str1, "file_animation_path")) strcpy(params.file_path, str2);
				else if (!strcmp(str1, "file_keyframes")) strcpy(params.file_keyframes, str2);
				else if (!strcmp(str1, "file_sound")) strcpy(params.file_sound, str2);
				else if (!strcmp(str1, "palette")) GetPaletteFromString(params.palette, str2);
				else
				{
					int matched = false;
					char buf[100];
					for (int i = 1; i <= HYBRID_COUNT; ++i) {
						sprintf(buf, "hybrid_formula_%d", i);
						if (!strcmp(str1, buf)) {
							params.fractal.hybridFormula[i-1] = (enumFractalFormula) atoi(str2);
							matched = true;
							break;
						}
						sprintf(buf, "hybrid_iterations_%d", i);
						if (!strcmp(str1, buf)) {
							params.fractal.hybridIters[i-1] = atoi(str2);
							matched = true;
							break;
						}
						sprintf(buf, "hybrid_power_%d", i);
						if (!strcmp(str1, buf)) {
							params.fractal.doubles.hybridPower[i-1] = atof2(str2, locale_dot);
							matched = true;
							break;
						}
					}

					for (int component = 0; component < 3; ++component) {
						sprintf(buf, "mandelbox_rotation_main_%s", component_names[component]);
						if (!strcmp(str1, buf)) {
							params.fractal.mandelbox.doubles.rotationMain[component] = atof2(str2, locale_dot)/ 180.0 * M_PI;
							matched = true;
							break;
						}
					}

					for (int fold = 0; fold < MANDELBOX_FOLDS; ++fold) {
						for (int axis = 0; axis < 3; ++axis) {
							for (int component = 0; component < 3; ++component) {
								sprintf(buf, "mandelbox_rotation_%s%d_%s", axis_names[axis], fold + 1, component_names[component]);
								if (!strcmp(str1, buf)) {
									params.fractal.mandelbox.doubles.rotation[fold][axis][component] = atof2(str2, locale_dot)/ 180.0 * M_PI;
									matched = true;
									break;
								}
							}
							if (matched) break;
						}
						if (matched) break;
					}

					if (!matched) {
						printf("Warning! Unknown parameter: %s %s\n", str1, str2);
						WriteLog("Warning! Unknown parameter:");
						WriteLog(str1);
					}
				}
				c = fscanf(fileSettings, "%[^\n]", str2);
			}
		}
		fclose(fileSettings);

		params.doubles.max_y = 20.0 / params.doubles.zoom;
		params.doubles.resolution = 1.0 / params.image_width;

		if (params.fractal.formula == 1) params.fractal.analitycDE = true;
		else params.fractal.analitycDE = false;

		if (!paletteLoadedFromSettingsFile)
		{
			printf("Palette not found in settings file. Generating random palette\n");
			srand(params.coloring_seed);
			NowaPaleta(params.palette, 1.0);
		}

		if(!newMandelboxParametersLoaded)
		{
			params.fractal.mandelbox.doubles.scale = params.fractal.doubles.power;
			params.fractal.mandelbox.doubles.foldingLimit = params.fractal.doubles.foldingLimit;
			params.fractal.mandelbox.doubles.foldingValue = params.fractal.doubles.foldingValue;
			params.fractal.mandelbox.doubles.foldingSphericalFixed = params.fractal.doubles.foldingSphericalFixed;
			params.fractal.mandelbox.doubles.foldingSphericalMin = params.fractal.doubles.foldingSphericalMin;
			if(params.fractal.formula == tglad) params.fractal.tgladFoldingMode = false;
		}

		lightsPlaced = 0;

		if (lineCounter != 302)
		{
			printf("number of lines in settings file (should be 301): %d\n", lineCounter);
			if (!noGUI && !disableMessages)
			{
				GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_interface), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
						"Warning! Settings file was created in other version of Mandelbulber\nfile: %s", filename);
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
			}
		}
		return true;
	}
	else
	{
		//printf("Can't open settings file: %s\n", filename);
		return false;
	}
}

void DefaultValues(sParamRender &params)
{
	params.fractal.doubles.amin = -10; //fractal limits
	params.fractal.doubles.amax = 10;
	params.fractal.doubles.bmin = -10;
	params.fractal.doubles.bmax = 10;
	params.fractal.doubles.cmin = -10;
	params.fractal.doubles.cmax = 10;
	params.doubles.vp.x = 0; // viewpoint
	params.doubles.vp.y = 0;
	params.doubles.vp.z = 0;
	params.doubles.zoom = 2.5; //zoom
	params.doubles.min_y = -2; //range of depth;
	params.doubles.max_y = 20.0 / params.doubles.zoom;
	params.fractal.N = 255; //maximum number of iterations
	params.fractal.minN = 0; //minimum number of iterations
	params.fractal.iterThresh = false; //maxiter threshord mode
	params.fractal.doubles.power = 8; //power of fractal formula
	params.fractal.formula = trig_DE; //type of fractal formula
	params.fractal.analitycDE = true; //analytic DE mode
	params.fractal.juliaMode = false; //Julia mode
	params.fractal.tgladFoldingMode = false; //Tglad's folding mode
	params.fractal.sphericalFoldingMode = false; //spherical folding mode
	params.fractal.IFS.foldingMode = false;
	params.fractal.doubles.julia.x = 0; //Julia constant
	params.fractal.doubles.julia.y = 0;
	params.fractal.doubles.julia.z = 0;
	params.fractal.doubles.foldingLimit = 1; //parameters of Tg;ad's folding
	params.fractal.doubles.foldingValue = 2;
	params.fractal.doubles.foldingSphericalMin = 0.5; //parameters of spherical folding
	params.fractal.doubles.foldingSphericalFixed = 1;
	params.fractal.doubles.FoldingIntPowFoldFactor = 2.0;
	params.fractal.doubles.FoldingIntPowZfactor = 5.0;
	params.fractal.dynamicDEcorrection = false;
	params.fractal.linearDEmode = false;
	params.fractal.constantDEThreshold = false;
	params.doubles.DE_factor = 1.0; //factor for distance estimation steps
	params.image_width = 800; //image width
	params.image_height = 600; //image height
	params.doubles.dist_thresh = 1.0; //distance treshold
	params.doubles.resolution = 1.0 / params.image_width;
	params.doubles.imageAdjustments.brightness = 1.0; //image brightness
	params.doubles.imageAdjustments.ambient = 0.0; //simple ambient light
	params.doubles.imageAdjustments.reflect = 0.0; //reflection factor
	params.doubles.imageAdjustments.directLight = 0.7; //direct light intensity
	params.doubles.imageAdjustments.globalIlum = 1.0; //intensity of ambient occlusion
	params.globalIlumQuality = 3.0; //ambient occlusion quality
	params.doubles.imageAdjustments.shading = 1.0; //intensity of shading
	params.doubles.imageAdjustments.imageGamma = 1.0; //image gamma
	params.doubles.imageAdjustments.specular = 1.0; //intensity of specularity effect
	params.doubles.imageAdjustments.glow_intensity = 1.0; //intensity of glow effect
	params.effectColours.glow_color1.R = 40984; //glow colour
	params.effectColours.glow_color1.G = 44713;
	params.effectColours.glow_color1.B = 49490;
	params.effectColours.glow_color2.R = 57192;
	params.effectColours.glow_color2.G = 60888;
	params.effectColours.glow_color2.B = 62408;
	params.background_color1.R = 7891;
	params.background_color1.G = 37121;
	params.background_color1.B = 57898;//background colour
	params.background_color2.R = 54473;
	params.background_color2.G = 57812;
	params.background_color2.B = 59901;
	params.doubles.persp = 0.5; //perspective factor
	params.doubles.quality = 1.0; //DE threshold factor
	params.doubles.smoothness = 1.0;
	params.doubles.alfa = -20 * M_PI / 180.0; //rotation of fractal
	params.doubles.beta = 30 * M_PI / 180.0;
	params.doubles.gamma = 0 * M_PI / 180.0;
	params.shadow = true; //enable shadows
	params.global_ilumination = true; //enable global ilumination
	params.imageSwitches.coloringEnabled = true; //enable of surface colouring
	params.coloring_seed = 123456; //colouring random seed
	params.doubles.imageAdjustments.coloring_speed = 1.0; //colour change frequency
	params.doubles.imageAdjustments.paletteOffset = 0.0;
	params.fastGlobalIllumination = true; //enable fake global ilumination
	params.slowShading = false; //enable fake gradient calculation for shading
	params.fractal.limits_enabled = false; //enable limits (intersections)
	params.textured_background = false; //enable testured background
	params.doubles.imageAdjustments.fogVisibility = 29.0;
	params.doubles.imageAdjustments.fogVisibilityFront = 20.0;
	params.imageSwitches.fogEnabled = false;
	params.effectColours.fogColor.R = 0xFFFF;
	params.effectColours.fogColor.G = 0xFFFF;
	params.effectColours.fogColor.B = 0xFFFF;
	params.SSAOEnabled = false;
	params.SSAOQuality = 20.0;
	params.DOFEnabled = false;
	params.doubles.DOFFocus = 22.0;
	params.doubles.DOFRadius = 10.0;
	params.doubles.mainLightAlfa = -45 * M_PI / 180.0;
	params.doubles.mainLightBeta = 45 * M_PI / 180.0;
	params.effectColours.mainLightColour.R = 0xFFFF;
	params.effectColours.mainLightColour.G = 0xFFFF;
	params.effectColours.mainLightColour.B = 0xFFFF;
	params.doubles.imageAdjustments.mainLightIntensity = 1.0;
	params.doubles.auxLightIntensity = 1.0;
	params.auxLightRandomSeed = 123456;
	params.auxLightNumber = 0;
	params.doubles.auxLightMaxDist = 0.1;
	params.doubles.auxLightDistributionRadius = 3.0;
	params.doubles.auxLightVisibility = 1.0;
	params.doubles.auxLightPre1.x = 3;
	params.doubles.auxLightPre1.y = -3;
	params.doubles.auxLightPre1.z = -3;
	params.doubles.auxLightPre1intensity = 1.3;
	params.doubles.auxLightPre2.x = -3;
	params.doubles.auxLightPre2.y = -3;
	params.doubles.auxLightPre2.z = 0;
	params.doubles.auxLightPre2intensity = 1.0;
	params.doubles.auxLightPre3.x = -3;
	params.doubles.auxLightPre3.y = 3;
	params.doubles.auxLightPre3.z = -1;
	params.doubles.auxLightPre3intensity = 3.0;
	params.doubles.auxLightPre4.x = 0;
	params.doubles.auxLightPre4.y = -1;
	params.doubles.auxLightPre4.z = 0;
	params.doubles.auxLightPre4intensity = 2.0;
	params.auxLightPre1Enabled = false;
	params.auxLightPre2Enabled = false;
	params.auxLightPre3Enabled = false;
	params.auxLightPre4Enabled = false;
	params.auxLightPre1Colour.R = 45761;
	params.auxLightPre1Colour.G = 53633;
	params.auxLightPre1Colour.B = 59498;
	params.auxLightPre2Colour.R = 62875;
	params.auxLightPre2Colour.G = 55818;
	params.auxLightPre2Colour.B = 50083;
	params.auxLightPre3Colour.R = 64884;
	params.auxLightPre3Colour.G = 64928;
	params.auxLightPre3Colour.B = 48848;
	params.auxLightPre4Colour.R = 52704;
	params.auxLightPre4Colour.G = 62492;
	params.auxLightPre4Colour.B = 45654;
	params.doubles.auxLightRandomCenter.x = 0;
	params.doubles.auxLightRandomCenter.y = 0;
	params.doubles.auxLightRandomCenter.z = 0;
	params.startFrame = 0;
	params.endFrame = 1000;
	params.framesPerKeyframe = 100;

	params.soundEnabled = false;
	params.doubles.soundFPS = 25.0;
	params.soundBand1Min = 5;
	params.soundBand1Max = 50;
	params.soundBand2Min = 100;
	params.soundBand2Max = 300;
	params.soundBand3Min = 1000;
	params.soundBand3Max = 3000;
	params.soundBand4Min = 8000;
	params.soundBand4Max = 10000;

	params.fractal.IFS.doubles.offset.x = 1;
	params.fractal.IFS.doubles.offset.y = 0;
	params.fractal.IFS.doubles.offset.z = 0;
	params.fractal.IFS.doubles.scale = 2;
	params.fractal.IFS.doubles.rotationAlfa = 0;
	params.fractal.IFS.doubles.rotationBeta = 0;
	params.fractal.IFS.doubles.rotationGamma = 0;
	params.fractal.IFS.absX = false;
	params.fractal.IFS.absY = false;
	params.fractal.IFS.absZ = false;

	for (int i = 0; i < HYBRID_COUNT; ++i)  {
		params.fractal.hybridFormula[i] = none;
		params.fractal.hybridIters[i] = 1;
		params.fractal.doubles.hybridPower[i] = 2;
	}
	params.fractal.hybridFormula[HYBRID_COUNT - 1] = trig_optim;
	params.fractal.hybridCyclic = false;

	params.fractal.mandelbox.doubles.colorFactorX = 0.03;
	params.fractal.mandelbox.doubles.colorFactorY = 0.05;
	params.fractal.mandelbox.doubles.colorFactorZ = 0.07;
	params.fractal.mandelbox.doubles.colorFactorR = 0;
	params.fractal.mandelbox.doubles.colorFactorSp1 = 0.2;
	params.fractal.mandelbox.doubles.colorFactorSp2 = 0.2;

	for (int component = 0; component < 3; ++component)
		params.fractal.mandelbox.doubles.rotationMain[component] = 0;

	for (int rot = 0; rot < MANDELBOX_FOLDS; ++rot)
		for (int axis = 0; axis < 3; ++axis)
			for (int component = 0; component < 3; ++component)
				params.fractal.mandelbox.doubles.rotation[rot][axis][component] = 0;

	params.fractal.mandelbox.doubles.foldingLimit = 1;
	params.fractal.mandelbox.doubles.foldingValue = 2;
	params.fractal.mandelbox.doubles.foldingSphericalMin = 0.5;
	params.fractal.mandelbox.doubles.foldingSphericalFixed = 1.0;
	params.fractal.mandelbox.doubles.sharpness = 3.0;
	params.fractal.mandelbox.doubles.scale = 2.0;
	params.fractal.mandelbox.rotationsEnabled = false;

	params.doubles.stereoEyeDistance = 0.1;
	params.stereoEnabled = false;

	params.fractal.interiorMode = false;
	params.doubles.viewDistanceMin = 1e-15;
	params.doubles.viewDistanceMax = 50;

	params.fractal.doubles.constantFactor = 1.0;

	params.quiet = false;

	strcpy(params.file_background, "textures/background.jpg");
	strcpy(params.file_envmap, "textures/envmap.jpg");
	strcpy(params.file_lightmap, "textures/lightmap.jpg");
	strcpy(params.file_destination, "images/iamge");
	strcpy(params.file_path, "paths/path.txt");
	strcpy(params.file_keyframes, "keyframes/keyframe");
	strcpy(params.file_sound, "sounds/sound.wav");

	for (int i = 0; i < IFS_VECTOR_COUNT; i++)
	{
		params.fractal.IFS.doubles.direction[i].x = 1;
		params.fractal.IFS.doubles.direction[i].y = 0;
		params.fractal.IFS.doubles.direction[i].z = 0;
		params.fractal.IFS.doubles.alfa[i] = 0;
		params.fractal.IFS.doubles.beta[i] = 0;
		params.fractal.IFS.doubles.gamma[i] = 0;
		params.fractal.IFS.doubles.distance[i] = 0;
		params.fractal.IFS.doubles.intensity[i] = 1.0;
		params.fractal.IFS.enabled[i] = false;
	}
}

void KeepOtherSettings(sParamRender *params)
{
	double *doublesParamRender = new double[sizeof(sParamRenderD)];
	double *doublesFractal = new double[sizeof(sFractalD)];
	double *doublesMandelbox = new double[sizeof(sFractalMandelboxD)];
	double *doublesIFS = new double[sizeof(sFractalIFSD)];

	memcpy(doublesParamRender,&params->doubles,sizeof(sParamRenderD));
	memcpy(doublesFractal,&params->fractal.doubles,sizeof(sFractalD));
	memcpy(doublesMandelbox,&params->fractal.mandelbox.doubles,sizeof(sFractalMandelboxD));
	memcpy(doublesIFS,&params->fractal.IFS.doubles,sizeof(sFractalIFSD));

	ReadInterface(params);

	memcpy(&params->doubles,doublesParamRender,sizeof(sParamRenderD));
	memcpy(&params->fractal.doubles,doublesFractal,sizeof(sFractalD));
	memcpy(&params->fractal.mandelbox.doubles,doublesMandelbox,sizeof(sFractalMandelboxD));
	memcpy(&params->fractal.IFS.doubles,doublesIFS,sizeof(sFractalIFSD));

	delete[] doublesParamRender;
	delete[] doublesFractal;
	delete[] doublesMandelbox;
	delete[] doublesIFS;
}
