/*
 * morph.hpp
 *
 *  Created on: 2010-05-30
 *      Author: krzysztof
 */

#ifndef MORPH_HPP_
#define MORPH_HPP_

#include <vector>

class CMorph
{
public:
	CMorph(int size, int recordSize);
	void AddData(int index, double *data);
	void SetFramesPerKey(int frames) {framesPerKey = frames;}
	void Linear(int frame, double *destData);
	void CatmullRom(int frame, double *destData);

private:
	std::vector<std::vector<double> > dataSets;
	int dataSize;
	int count;
	int framesPerKey;
	std::vector<double> output;
};

#endif /* MORPH_HPP_ */
