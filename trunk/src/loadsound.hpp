/*
 * loadsound.hpp
 *
 *  Created on: 2010-06-26
 *      Author: krzysztof
 */

#ifndef LOADSOUND_H_
#define LOADSOUND_H_

/*
class CSound
{
public:
	bool Load(char* file);
	void SetFPS(double newFPS);
	void CreateEnvelope(void);
	double GetEnvelope(int frameNumber);
	double GetSpectrumA(int frameNumber);
	double GetSpectrumB(int frameNumber);
	double GetSpectrumC(int frameNumber);
	double GetSpectrumD(int frameNumber);
	double GetLength();
	void NormalizeVolume(double *samples);
	bool IsLoaded(void);
	void DoFFT(int bandMin[4], int bandMax[4]);
	int animframes;
private:
	double *wave;
	double *envelope;
	double *FFTbufferIn;
	double *FFTbufferOut;
	double *spectrumA;
	double *spectrumB;
	double *spectrumC;
	double *spectrumD;
	bool loaded;
	int frames;
	int sampleRate;
	double length;
	int channels;
	double fps;
};

extern CSound sound;

void fft(double* data, unsigned long nn);
*/

#endif /* LOADSOUND_H_ */
