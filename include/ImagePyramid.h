/*
Copyright (C) Chen Wang

ImagePyramid.h

Functions to Image Pyramid used by Liu Ce

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/
#ifndef _IMAGEPYRAMID_H_
#define _IMAGEPYRAMID_H_
#include "Image.h"

template <class T>
class ImagePyramid
{
private:
	Image<T>* ImPyramid;
	int nLevels;
	float fRatio;
public:
	ImagePyramid(void){ ImPyramid = NULL; };
	~ImagePyramid(void){ if (ImPyramid != NULL) delete[]ImPyramid; };
	inline Image<T>& operator[](int level) { return ImPyramid[level]; };
	void ConstructPyramid(const Image<T>& image, float ratio = 0.8, int minWidth = 30);
	void ConstructPyramidLevels(const Image<T>& image, float ratio = 0.8, int _nLevels = 2);
	void displayTop(const char* filename){ ImPyramid[nLevels - 1].imwrite(filename); };
	inline int nlevels() const { return nLevels; };
	inline float ratio() const { return fRatio; };
	inline Image<T> getImage(const int level){ return ImPyramid[level]; }
};

typedef ImagePyramid<float> FImagePyramid;
typedef ImagePyramid<double> DImagePyramid;

//---------------------------------------------------------------------------------------
// function to construct the pyramid
// this is the fast way
//---------------------------------------------------------------------------------------
template <class T>
void ImagePyramid<T>::ConstructPyramid(const Image<T>& image, float ratio, int minWidth)
{
	// the ratio cannot be arbitrary numbers
	if (ratio > 0.98 || ratio < 0.4)
		ratio = 0.75;
	// first decide how many levels
	nLevels = log((float)minWidth / image.width()) / log(ratio);
	fRatio = ratio;
	if (ImPyramid != NULL)
		delete[]ImPyramid;
	ImPyramid = new Image<T>[nLevels];
	ImPyramid[0].copyData(image);
	float baseSigma = (1 / ratio - 1);
	int n = log(0.25) / log(ratio);
	float nSigma = baseSigma*n;
	for (int i = 1; i < nLevels; i++)
	{
		Image<T> foo;
		if (i <= n)
		{
			float sigma = baseSigma * i;
			image.GaussianSmoothing(foo, sigma, sigma * 3);
			foo.imresize(ImPyramid[i], pow(ratio, i));
		}
		else
		{
			ImPyramid[i - n].GaussianSmoothing(foo, nSigma, nSigma * 3);
			float rate = (float)pow(ratio, i)*image.width() / foo.width();
			foo.imresize(ImPyramid[i], rate);
		}
	}
}

template <class T>
void ImagePyramid<T>::ConstructPyramidLevels(const Image<T>& image, float ratio = 0.8, int _nLevels = 2)
{
	// the ratio cannot be arbitrary numbers
	if (ratio > 0.98 || ratio < 0.4)
		ratio = 0.75;
	nLevels = _nLevels;
	fRatio = ratio;
	if (ImPyramid != NULL)
		delete[]ImPyramid;
	ImPyramid = new Image<T>[nLevels];
	ImPyramid[0].copyData(image);
	float baseSigma = (1 / ratio - 1);
	int n = log(0.25) / log(ratio);
	float nSigma = baseSigma*n;
	for (int i = 1; i < nLevels; i++)
	{
		Image<T> foo;
		if (i <= n)
		{
			float sigma = baseSigma*i;
			image.GaussianSmoothing(foo, sigma, sigma * 3);
			foo.imresize(ImPyramid[i], pow(ratio, i));
		}
		else
		{
			ImPyramid[i - n].GaussianSmoothing(foo, nSigma, nSigma * 3);
			float rate = (float)pow(ratio, i)*image.width() / foo.width();
			foo.imresize(ImPyramid[i], rate);
		}
	}
}

//---------------------------------------------------------------------------------------
// function to construct the pyramid
// this is the slow way
//---------------------------------------------------------------------------------------
/*
void GaussianPyramid::ConstructPyramid(const DImage &image, float ratio, int minWidth)
{
// the ratio cannot be arbitrary numbers
if (ratio > 0.98 || ratio < 0.4)
ratio = 0.75;
// first decide how many levels
nLevels = log((float)minWidth / image.width()) / log(ratio);
if (ImPyramid != NULL)
delete[]ImPyramid;
ImPyramid = new DImage[nLevels];
ImPyramid[0].copyData(image);
float baseSigma = (1 / ratio - 1);
for (int i = 1; i < nLevels; i++)
{
DImage foo;
float sigma = baseSigma*i;
image.GaussianSmoothing(foo, sigma, sigma*2.5);
foo.imresize(ImPyramid[i], pow(ratio, i));
}
}//*/

#endif	//_IMAGEPYRAMID_H_