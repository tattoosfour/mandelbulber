/*
 * cimage.hpp
 *
 *  Created on: 2010-08-29
 *      Author: krzysztof
 */

#ifndef CIMAGE_HPP_
#define CIMAGE_HPP_

#include <gtk-2.0/gtk/gtk.h>

//global variables
struct sRGB8
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

struct sRGB16
{
	unsigned short R;
	unsigned short G;
	unsigned short B;
};

struct sRGB
{
	int R;
	int G;
	int B;
};

struct sComplexImage
{
	float zBuffer;
	unsigned short shadowsBuf16;
	unsigned short shadingBuf16;
	unsigned short specularBuf16;
	sRGB16 auxLight;
	sRGB16 auxSpecular;
	unsigned short glowBuf16;
	sRGB8 reflectBuf8;
	sRGB16 ambientBuf16;
	unsigned short colorIndexBuf16;
	sRGB16 backgroundBuf16;
	unsigned short alpha;
};

struct sImageAdjustments
{
  double shading;
  double directLight;
  double ambient;
  double specular;
  double reflect;
  double globalIlum;
  double brightness;
  double glow_intensity;
  double fogVisibility;
  double fogVisibilityFront;
  double coloring_speed;
  double imageGamma;
  double paletteOffset;
  double mainLightIntensity;
};

struct sEffectColours
{
	sRGB glow_color1;
	sRGB glow_color2;
	sRGB fogColor;
	sRGB mainLightColour;
};

struct sImageSwitches
{
	bool coloringEnabled;
	bool fogEnabled;
};

class cImage
{
public:
	cImage(int w, int h);
	~cImage();
	void ChangeSize(int w, int h);
	void ClearImage(void);
	inline void PutPixelImage(int x, int y, sRGB16 pixel)	{if (x >= 0 && x < width && y >= 0 && y < height) image16[x + y * width] = pixel;	}
	inline void PutPixelZBuffer(int x, int y, float pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].zBuffer = pixel;}
	inline void PutPixelShadow(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].shadowsBuf16 = pixel;}
	inline void PutPixelShading(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].shadingBuf16 = pixel;}
	inline void PutPixelSpecular(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].specularBuf16 = pixel;}
	inline void PutPixelAuxLight(int x, int y, sRGB16 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].auxLight = pixel;}
	inline void PutPixelAuxSpecular(int x, int y, sRGB16 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].auxSpecular = pixel;}
	inline void PutPixelGlow(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].glowBuf16 = pixel;}
	inline void PutPixelReflect(int x, int y, sRGB8 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].reflectBuf8 = pixel;}
	inline void PutPixelAmbient(int x, int y, sRGB16 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].ambientBuf16 = pixel;}
	inline void PutPixelColor(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].colorIndexBuf16 = pixel;}
	inline void PutPixelBackground(int x, int y, sRGB16 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].backgroundBuf16 = pixel;}
	inline void PutPixelAlpha(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].alpha = pixel;}
  inline sRGB16 GetPixelImage(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return image16[x + y * width];}
  inline short int GetPixelAlpha(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return complexImage[x + y * width].alpha;}
  inline float GetPixelZBuffer(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return complexImage[x + y * width].zBuffer;}
  void CompileImage(void);
  void SetPalette(sRGB *palette);
  sRGB* GetPalettePtr() {return palette;}
  sRGB IndexToColour(int index);
  int GetWidth(void) {return width;}
  int GetHeight(void) {return height;}
  int GetPreviewWidth(void) {return previewWidth;}
  int GetPreviewHeight(void) {return previewHeight;}
  int GetUsedMB(void);
  void SetImageParameters(sImageAdjustments adjustments, sEffectColours effectColours, sImageSwitches switches);
  void SetImageAdjustments(sImageAdjustments adjustments) {adj = adjustments;}
  sImageAdjustments* GetImageAdjustments(void) {return &adj;}
  sEffectColours* GetEffectColours(void) {return &ecol;}
  sImageSwitches* GetImageSwitches(void) {return &sw;}
  unsigned char* ConvertTo8bit(void);
  unsigned char* CreatePreview(double scale);
  void UpdatePreview(void);
  unsigned char* GetPreviewPtr(void);
  bool IsPreview(void);
  void RedrawInWidget(GtkWidget *dareaWidget);
  double GetPreviewScale() {return previewScale;}

private:
  sRGB8 Interpolation(float x, float y);
	void AllocMem(void);
  sComplexImage *complexImage;
	sRGB16 *image16;
	sRGB8 *image8;
	sRGB8 *preview;
	sImageAdjustments adj;
	sEffectColours ecol;
	sImageSwitches sw;
	sRGB *palette;
	int width;
	int height;
	int *gammaTable;
	bool previewAllocated;
	int previewWidth;
	int previewHeight;
	double previewScale;
};

sRGB PostRendering_Fog(double z, double min, double max, sRGB fog_color, sRGB color_before);

#endif /* CIMAGE_HPP_ */
