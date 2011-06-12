/*
 * texture.hpp
 *
 *  Created on: 2010-02-07
 *      Author: krzysztof marczak
 */

#ifndef TEXTURE_HPP_
#define TEXTURE_HPP_

#include "cimage.hpp"

class cTexture
{
public:
	cTexture(const char *filename);
	~cTexture(void);
	int Height(void) {return height;}
	int Width(void) {return width;}
	sRGB8 Pixel(double x, double y);
	sRGB8 FastPixel(int x, int y);
	bool IsLoaded(void) {return loaded;}
private:
	sRGB8 Interpolation(double x, double y);
	sRGB8 *bitmap;
	int width;
	int height;
	bool loaded;
};

#endif /* TEXTURE_HPP_ */
