/*
 * loadsound.cpp
 *
 *  Created on: 2010-06-26
 *      Author: krzysztof
 */

#include "loadsound.hpp"
#include "sndfile.h"
#include "math.h"

CSound sound;

bool CSound::Load(char *file)
{
	SF_INFO sfinfo;
	SNDFILE *sndfile;

	sfinfo.format = 0;
	sndfile = sf_open(file, SFM_READ, &sfinfo);
	if (sndfile == NULL)
	{
		fprintf(stderr, "Could not open soundfile '%s'\n", file);
		loaded = false;
	}
	else
	{
		frames = sfinfo.frames;
		length = (double) frames / sfinfo.samplerate;
		sampleRate = sfinfo.samplerate;
		printf("Sound loaded. Channels: %d, sample rate %dHz, length: %fs\n", sfinfo.channels, sfinfo.samplerate, length);
		channels = sfinfo.channels;
		int buffer_size = frames * channels;
		double *sndBuff = new double[buffer_size];
		sf_count_t framesRead = sf_readf_double(sndfile, sndBuff, frames);
		printf("Read %d sound samples\n", (int) framesRead);

		if(loaded)
		{
			delete wave;
			delete envelope;
			delete spectrumA;
			delete spectrumB;
			delete spectrumC;
			delete spectrumD;
			animframes = 0;
		}
		wave = new double[frames];

		for (int i = 0; i < frames; i++)
		{
			double sample = 0;
			for (int channel = 0; channel < channels; channel++)
			{
				sample += sndBuff[i * channels + channel];
			}
			sample /= channels;
			wave[i] = sample;
		}
		delete sndBuff;
		loaded = true;
	}
	return loaded;
}

void CSound::SetFPS(double newFPS)
{
	fps = newFPS;
}

void CSound::CreateEnvelope(void)
{
	animframes = length * fps;
	envelope = new double[animframes];
	double samplesPerFrame = (double) sampleRate / fps;
	for (int i = 0; i < animframes; i++)
	{
		int start = i * samplesPerFrame;
		double sample = 0;
		for (int k = 0; k < samplesPerFrame; k++)
		{
			sample += fabs(wave[start + k]);
		}
		sample /= samplesPerFrame;
		envelope[i] = sample;
		//printf("sample[%d] = %f\n", i, sample);
	}
	NormalizeVolume(envelope);
}

double CSound::GetEnvelope(int frameNumber)
{
	double result = 0;
	if (frameNumber < animframes)
	{
		result = envelope[frameNumber];
	}
	return result;
}

double CSound::GetSpectrumA(int frameNumber)
{
	double result = 0;
	if (frameNumber < animframes)
	{
		result = spectrumA[frameNumber];
	}
	return result;
}

double CSound::GetSpectrumB(int frameNumber)
{
	double result = 0;
	if (frameNumber < animframes)
	{
		result = spectrumB[frameNumber];
	}
	return result;
}

double CSound::GetSpectrumC(int frameNumber)
{
	double result = 0;
	if (frameNumber < animframes)
	{
		result = spectrumC[frameNumber];
	}
	return result;
}

double CSound::GetSpectrumD(int frameNumber)
{
	double result = 0;
	if (frameNumber < animframes)
	{
		result = spectrumD[frameNumber];
	}
	return result;
}


void CSound::NormalizeVolume(double *samples)
{
	double min = samples[0];
	double max = samples[0];
	for (int i = 0; i < animframes; i++)
	{
		if (samples[i] > max) max = samples[i];
		if (samples[i] < min) min = samples[i];
	}

	double gain = 1.0 / (max - min);
	for (int i = 0; i < animframes; i++)
	{
		samples[i] = (samples[i] - min) * gain;
	}
}

bool CSound::IsLoaded(void)
{
	return loaded;
}

double CSound::GetLength(void)
{
	return length;
}

void CSound::DoFFT(int bandMin[4], int bandMax[4])
{
	spectrumA = new double[animframes];
	spectrumB = new double[animframes];
	spectrumC = new double[animframes];
	spectrumD = new double[animframes];

	int N = 2048;
	int size = 2 * N;
	FFTbufferIn = new double[size];
	FFTbufferOut = new double[N];
	double samplesPerFrame = (double) sampleRate / fps;
	for (int i = 0; i < animframes; i++)
	{
		//prepare FFT buffer
		int offset = i * samplesPerFrame;
		for (int k = 0; k < size; k += 2)
		{
			if (k + offset < frames)
			{
				FFTbufferIn[k] = wave[k + offset];
				FFTbufferIn[k + 1] = 0;
			}
			else
			{
				FFTbufferIn[k] = 0;
				FFTbufferIn[k + 1] = 0;
			}
		}
		//przekształcenie FFT
		fft(FFTbufferIn, N);
		//obliczenie modułów wartosci zespolonych
		for (int k = 0; k < size; k += 2)
		{
			FFTbufferOut[k / 2] = sqrt(FFTbufferIn[k] * FFTbufferIn[k] + FFTbufferIn[k + 1] * FFTbufferIn[k + 1]);
		}

		int counter = 0;
		double scale = 0.5*N/sampleRate;
		spectrumA[i] = 0;
		for (int k = scale*bandMin[0]; k < scale*bandMax[0]; k++)
		{
			spectrumA[i] += FFTbufferOut[k];
			counter++;
		}
		spectrumA[i] /= counter;

		counter = 0;
		spectrumB[i] = 0;
		for (int k = scale*bandMin[1]; k < scale*bandMax[1]; k++)
		{
			spectrumB[i] += FFTbufferOut[k];
			counter++;
		}
		spectrumB[i] /= counter;

		counter = 0;
		spectrumC[i] = 0;
		for (int k = scale*bandMin[2]; k < scale*bandMax[2]; k++)
		{
			spectrumC[i] += FFTbufferOut[k];
			counter++;
		}
		spectrumC[i] /= counter;

		counter = 0;
		spectrumD[i] = 0;
		for (int k =scale*bandMin[3]; k < scale*bandMax[3]; k++)
		{
			spectrumD[i] += FFTbufferOut[k];
			counter++;
		}
		spectrumD[i] /= counter;

	}
	NormalizeVolume(spectrumA);
	NormalizeVolume(spectrumB);
	NormalizeVolume(spectrumC);
	NormalizeVolume(spectrumD);

	delete FFTbufferIn;
	delete FFTbufferOut;
}

//----------- swap ----------------
inline void swap(double &a, double &b)
{
	double temp = a;
	a = b;
	b = temp;
}

void fft(double* data, unsigned long nn)
{
	unsigned long n, mmax, m, j, istep, i;
	double wtemp, wr, wpr, wpi, wi, theta;
	double tempr, tempi;

	// reverse-binary reindexing
	n = nn << 1;
	j = 1;
	for (i = 1; i < n; i += 2)
	{
		if (j > i)
		{
			swap(data[j - 1], data[i - 1]);
			swap(data[j], data[i]);
		}
		m = nn;
		while (m >= 2 && j > m)
		{
			j -= m;
			m >>= 1;
		}
		j += m;
	};

	// here begins the Danielson-Lanczos section
	mmax = 2;
	while (n > mmax)
	{
		istep = mmax << 1;
		theta = -(2 * M_PI / mmax);
		wtemp = sin(0.5 * theta);
		wpr = -2.0 * wtemp * wtemp;
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;
		for (m = 1; m < mmax; m += 2)
		{
			for (i = m; i <= n; i += istep)
			{
				j = i + mmax;
				tempr = wr * data[j - 1] - wi * data[j];
				tempi = wr * data[j] + wi * data[j - 1];
				data[j - 1] = data[i - 1] - tempr;
				data[j] = data[i] - tempi;
				data[i - 1] += tempr;
				data[i] += tempi;
			}
			wtemp = wr;
			wr += wr * wpr - wi * wpi;
			wi += wi * wpr + wtemp * wpi;
		}
		mmax = istep;
	}
}
