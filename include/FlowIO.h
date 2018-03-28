/*
Copyright (C) Chen Wang

FlowIO.h

Functions used to Read/Write/Display Flow Files

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

#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace std;
//using namespace cv;	// cv 命名空间中会有Vector同名，因此在各处使用cv::

/*	read flow from a file */
float** readFlowFile(const char* filename, int *width, int *height)
{
	FILE *fid = fopen(filename, "rb");
	if (fid == 0) 
	{
		fprintf(stderr, "readFlow() error: could not open file  %s\n", filename);
		exit(1);
	}
	float help;
	fread(&help, sizeof(float), 1, fid);	//小端模式！
	if (help != 202021.25)
	{
		cout << "Invalid Flow File!" << endl;
		exit(1);
	}
	//PIEH = 50 49 45 48
	//小端：48 45 49 50 =
	//0100 1000 0100 0101 0100 1001 0101 0000 = 
	//0 10010000 10001010100100101010000 = 
	//+1.10001010100100101010000 * 2^(144-127) = 
	//+1.10001010100100101010000 * 2^17 =	右移17位
	//+110001010100100101.010000 = 202021.25

	//char help;
	//for(int i = 0; i < 4; i++)
	//	fread(&help, sizeof(char), 1, fid);

	fread(width, sizeof(int), 1, fid);
	fread(height, sizeof(int), 1, fid);

	float** flow = (float**) malloc(sizeof(float*)*2);	//分配内存
	flow[0] = (float*)malloc(sizeof(float)*(*width)*(*height));	//	u
	flow[1] = (float*)malloc(sizeof(float)*(*width)*(*height));	//	v

	for (int y = 0; y < *height; y++)
	{
		for (int x = 0; x < *width; x++) 
		{
			fread(&(flow[0][y*(*width) + x]), sizeof(float), 1, fid);
			fread(&(flow[1][y*(*width) + x]), sizeof(float), 1, fid);

			if( flow[0][y*(*width) + x] > 1000 || flow[0][y*(*width) + x] < -1000 ||
				flow[1][y*(*width) + x] > 1000 || flow[1][y*(*width) + x] < -1000)	//Occlusion
			{
				flow[0][y*(*width) + x] = 0;
				flow[1][y*(*width) + x] = 0;
			}
		}
	}
	fclose(fid);
	return flow;
}


/* write flow to a file */
void writeFlowFile(const char *filename, const float *flow_x, const float *flow_y, int width, int height)
{
	FILE *stream = fopen(filename, "wb");
	if (stream == 0) 
	{
		printf("Error while opening %s\n",filename);
		exit(1);
	}
	float help = 202021.25;
	fwrite(&help, sizeof(float), 1, stream);

	fwrite(&width, sizeof(int), 1, stream);
	fwrite(&height, sizeof(int), 1, stream);

	for (int y = 0; y < height ; y++)
	{
		for (int x = 0; x < width ; x++) 
		{
			fwrite(&flow_x[y*width + x], sizeof(float), 1, stream);
			fwrite(&flow_y[y*width + x], sizeof(float), 1, stream);
		}
	}
	fclose(stream);
}


inline bool isFlowCorrect(cv::Point2f u)
{
	return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
}

static cv::Vec3b computeColor(float fx, float fy)
{
	static bool first = true;

	// relative lengths of color transitions:
	// these are chosen based on perceptual similarity
	// (e.g. one can distinguish more shades between red and yellow
	//  than between yellow and green)
	const int RY = 15;
	const int YG = 6;
	const int GC = 4;
	const int CB = 11;
	const int BM = 13;
	const int MR = 6;
	const int NCOLS = RY + YG + GC + CB + BM + MR;
	static cv::Vec3i colorWheel[NCOLS];

	if (first)
	{
		int k = 0;

		for (int i = 0; i < RY; ++i, ++k)
			colorWheel[k] = cv::Vec3i(255, 255 * i / RY, 0);

		for (int i = 0; i < YG; ++i, ++k)
			colorWheel[k] = cv::Vec3i(255 - 255 * i / YG, 255, 0);

		for (int i = 0; i < GC; ++i, ++k)
			colorWheel[k] = cv::Vec3i(0, 255, 255 * i / GC);

		for (int i = 0; i < CB; ++i, ++k)
			colorWheel[k] = cv::Vec3i(0, 255 - 255 * i / CB, 255);

		for (int i = 0; i < BM; ++i, ++k)
			colorWheel[k] = cv::Vec3i(255 * i / BM, 0, 255);

		for (int i = 0; i < MR; ++i, ++k)
			colorWheel[k] = cv::Vec3i(255, 0, 255 - 255 * i / MR);

		first = false;
	}

	const float rad = sqrt(fx * fx + fy * fy);
	const float a = atan2(-fy, -fx) / (float) CV_PI;

	const float fk = (a + 1.0f) / 2.0f * (NCOLS - 1);
	const int k0 = static_cast<int>(fk);
	const int k1 = (k0 + 1) % NCOLS;
	const float f = fk - k0;

	cv::Vec3b pix;

	for (int b = 0; b < 3; b++)
	{
		const float col0 = colorWheel[k0][b] / 255.0f;
		const float col1 = colorWheel[k1][b] / 255.0f;

		float col = (1 - f) * col0 + f * col1;

		if (rad <= 1)
			col = 1 - rad * (1 - col); // increase saturation with radius
		else
			col *= .75; // out of range

		pix[2 - b] = static_cast<uchar>(255.0 * col);
	}

	return pix;
}

static void drawOpticalFlow(const cv::Mat_<float>& flowx, const cv::Mat_<float>& flowy, cv::Mat& dst, float maxmotion = -1)
{
	dst.create(flowx.size(), CV_8UC3);
	dst.setTo(cv::Scalar::all(0));

	// determine motion range:
	float maxrad = maxmotion;

	if (maxmotion <= 0)
	{
		maxrad = 1;
		for (int y = 0; y < flowx.rows; ++y)
		{
			for (int x = 0; x < flowx.cols; ++x)
			{
				cv::Point2f u(flowx(y, x), flowy(y, x));

				if (!isFlowCorrect(u))
					continue;

				maxrad = max(maxrad, sqrt(u.x * u.x + u.y * u.y));
			}
		}
	}

	for (int y = 0; y < flowx.rows; ++y)
	{
		for (int x = 0; x < flowx.cols; ++x)
		{
			cv::Point2f u(flowx(y, x), flowy(y, x));

			if (isFlowCorrect(u))
				dst.at<cv::Vec3b>(y, x) = computeColor(u.x / maxrad, u.y / maxrad);
		}
	}
}

static void showFlow(const char* name, const cv::Mat& flowx, const cv::Mat& flowy)
{
	cv::Mat out;
	drawOpticalFlow(flowx, flowy, out);

	cv::imshow(name, out);
}
