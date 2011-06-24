/*
 * settings.h
 *
 *  Created on: 2010-01-24
 *      Author: krzysztof marczak
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Render3D.h"

extern char data_directory[1000];

void fprintfDot(FILE *file, const char *string, double value, double defaultVal, bool compare);
void fprintfInt(FILE *file, const char *string, int value, int defaultVal, bool compare);
double atof2(const char *str, bool locale_dot);
void SaveSettings(const char *filename, const sParamRender& params, bool compare);
bool LoadSettings(const char *filename, sParamRender &params, bool disableMessages = false);
bool LoadSettings2(const char *filename, sParamRender &params, bool disableMessages);
bool LoadOneSetting(const char *str1, const char *s2, sParamRender &params, bool disableMessages = false);
void LoadSettingsPost(sParamRender &params);
void IFSToMorph(double *IFSdouble, const sFractal &fractal);
void MorphToIFS(double *IFSdouble, sFractal &fractal);
void MakePaletteString(const sRGB *palette, char *paletteString);
void GetPaletteFromString(sRGB *palette, const char *paletteString);
void KeepOtherSettings(sParamRender *params);
#endif /* SETTINGS_H_ */
