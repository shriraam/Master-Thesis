/*
 * speed.cpp
 *
 *  Created on: 13 Oct 2015
 *      Author: raam
 */

#include <iostream>
#include <png++/png.hpp>
#include <stdlib.h>
#include <thread>
#include <fstream>
#include "emmintrin.h"
#include <sys/time.h>

using namespace std;


class UCarray
{
public:
	unsigned char *Pix;
	UCarray(int r, int c);
	~UCarray();
	void ImagetoArray(png::image<png::gray_pixel>* input);
	void operator=(const UCarray& other);
	void ArraytoImage(png::image<png::gray_pixel>* output);
	void ArrayErode(UCarray &b);
	void ArrayDilate(UCarray &b);
private:
	int row, column;

};


UCarray::UCarray(int r, int c)
{
	row = r;
	column = c;
	Pix = (unsigned char*)malloc(row * column * sizeof(unsigned char));
	if (Pix == NULL)
	{
		fprintf(stderr, "out of memory\n");
	}

}

UCarray::~UCarray()
{
	free(Pix);
}

void UCarray::ImagetoArray(png::image<png::gray_pixel>* input)
{
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			*(Pix + i*column +j) = (*input)[i][j];
		}
	}
}

void UCarray::ArraytoImage(png::image<png::gray_pixel>* output)
{
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			(*output)[i][j] = *(Pix + i*column +j);
		}
	}
}

void UCarray:: operator =(const UCarray&  other)
{
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			*(Pix+i*column+j) = *(other.Pix+i*column+j);
		}
	}
}

void UCarray::ArrayErode(UCarray &b)
{
	unsigned char min;
	for (int i = 1; i <= row - 2; i++)
	{
		for (int j = 1; j <= column - 2; j++)
		{
			min = *(Pix+i*column+j);
			for (int k = -1; k <= 1; k++)
			{
				for (int l = -1; l <= 1; l++)
				{
					if (*(Pix+(i + k)*column+j + l) < min)
					{
						min = *(Pix + (i + k)*column + j + l);
					}
				}
			}
			*(b.Pix+i*column+j) = min;
		}
	}
}

void UCarray::ArrayDilate(UCarray &b)
{
	unsigned char max;
	for (int i = 1; i <= row - 2; i++)
	{
		for (int j = 1; j <= column - 2; j++)
		{
			max = *(Pix + i*column + j);
			for (int k = -1; k <= 1; k++)
			{
				for (int l = -1; l <= 1; l++)
				{
					if (*(Pix + (i + k)*column + j + l) > max)
					{
						max = *(Pix + (i + k)*column + j + l);
					}
				}
			}
			*(b.Pix + i*column + j) = max;
		}
	}
}

void InitializeULArray(unsigned long * Pix, int row, int column)
{
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				*(Pix+(i*column+j)*2+k) = 0;
			}

		}
	}
}

void Census11(unsigned long * OPic, UCarray &IPic, int row, int column)
{
	int BitCnt;
	unsigned long * OPicc;

	for (int i = 5; i <= row - 6; i++)
	{
		for (int j = 5; j <= column - 6; j++)
		{
			BitCnt = 0;
			OPicc = OPic+i*column*2+j*2;

			for (int k = -5; k <= 5; k++)
			{
				for (int l = -5; l <= 5; l++)
				{
					if (~(k == 0 && l == 0))
					{
						*OPicc = *OPicc << 1;

						if (*(IPic.Pix +(i + k)*column+j + l) < *(IPic.Pix+i*column+j))
						{
							*OPicc = *OPicc + 1;
						}

						BitCnt++;

						if (BitCnt%64==0)
						OPicc = OPicc + 1;

					}
				}
			}
		}
	}
}

void InitializeUIArray(unsigned int* Pix, int row, int column)
{
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			*(Pix+i*column+j) = 0;
		}
	}
}

void ArrayUItoImage(png::image<png::gray_pixel>* input, unsigned int* Pix, int row, int column)
{
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			(*input)[i][j] = (unsigned char)(*(Pix+i*column+j));
		}
	}
}

void SHDRighttoLeft13(unsigned int* DMap, __m128i  *LPic, __m128i  *RPic, int row, int column, unsigned int* HammDist, unsigned int* XorSum, unsigned int* HammDistY)
{
	unsigned int *HammDistt,*XorSumm, HammDistCalc, VDmy[4]={0,0,0,0};
	int DMin=0, DMax=100;
	__m128i  *LPicc,*RPicc, Xor, VSum, *VElmt,*HammDistyy;
	unsigned long *SigfLong;
	SigfLong = (unsigned long*)&Xor;

	HammDistt = HammDist +6*(column + 1);

	// Initialize Hamming distance
	for (int i = 6; i < row - 6; i++)
	{
		for (int j = 6; j < column - 6; j++)
		{
			*HammDistt = 20449;
			HammDistt++;
		}
		HammDistt=HammDistt+12;
	}


	for (int DRange = DMax; DRange >= DMin; DRange--)
	{
		LPicc = LPic +DRange;
		RPicc = RPic;
		XorSumm=XorSum;

		//Xor and add for each pixel
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < column - DRange; j++)
			{
				Xor = _mm_xor_si128 (*LPicc, *RPicc);
				*XorSumm= __builtin_popcountl(*SigfLong);
				*XorSumm+= __builtin_popcountl(*(SigfLong+1));
				LPicc++;
				RPicc++;
				XorSumm++;
			}
			LPicc= LPicc+ DRange;
			RPicc= RPicc+ DRange;
			XorSumm= XorSumm+ DRange;
		}

		//Add vertically, calculating partial hamming distance for each pixel
		for (int i = 6; i < row - 6; i++)
		{
			for (int j = 0; j < (column - DRange)/4; j++)
			{
				VSum = _mm_load_si128 ((__m128i*) &VDmy);
				VElmt = (__m128i*)XorSum + (i-6)*column/4 + j;
				HammDistyy = (__m128i*)HammDistY + i*column/4 + j;
				for (int k = 12; k >= 0; k--)
				{
					VSum = _mm_add_epi32 (VSum,*VElmt);
					VElmt = VElmt + column/4;
				}

                *HammDistyy = VSum;
			}
		}

		//Calculate the hamming distance and update disparity map
		for (int i = 6; i < row - 6; i++)
		{
			for (int j = 6; j < column - DRange - 6; j++)
			{
				HammDistCalc = 0;
				XorSumm = HammDistY + i*column + j-6;
				HammDistt = HammDist + i*column + j;
				for (int k = 12; k >= 0; k--)
				{
					HammDistCalc+= *XorSumm;
					if (HammDistCalc>*HammDistt) break;
					XorSumm++;
				}

				if (HammDistCalc<*HammDistt)
				{
					*HammDistt = HammDistCalc;
					*(DMap+i*column + j) = DRange;
				}
			}
		}
	}
}

void PreProcessImage(png::image<png::gray_pixel>* image, UCarray &UCInput, UCarray &UCOutput, unsigned long * CTImage, int row, int column)
{
	struct timeval tim;
	double t1, t2;
	UCInput.ImagetoArray(image);
	UCInput.ArrayErode(UCOutput);
	UCInput = UCOutput;
	UCInput.ArrayDilate(UCOutput);
	InitializeULArray(CTImage, row, column);
	gettimeofday(&tim, NULL);
	t1 = tim.tv_sec + (tim.tv_usec / 1000000.0);
    Census11(CTImage,UCOutput,row,column);
    gettimeofday(&tim, NULL);
    t2 = tim.tv_sec + (tim.tv_usec / 1000000.0);
    t2= t2 - t1;
    cout<<"ct time:"<<t2<<endl;
}

int main()
{
	struct timeval tim;
	double t1, t2, t3, t4, t5, t6;
	gettimeofday(&tim, NULL);
	t1 = tim.tv_sec + (tim.tv_usec / 1000000.0);

	png::image< png::gray_pixel > imageL("frame_1_left.png");
	png::image< png::gray_pixel > imageR("frame_1_right.png");

	gettimeofday(&tim, NULL);
	t3 = tim.tv_sec + (tim.tv_usec / 1000000.0);

	int row = imageL.get_height(), column = imageL.get_width();

	UCarray UCLInput(row, column), UCLOutput(row, column), UCRInput(row, column), UCROutput(row, column);
	unsigned long *LCTImage, *RCTImage;

	LCTImage = (unsigned long*)malloc(row * column* 2* sizeof(unsigned long ));
	RCTImage = (unsigned long*)malloc(row * column*2*sizeof(unsigned long));
	if (LCTImage == NULL||RCTImage == NULL)
	{
		fprintf(stderr, "out of memory\n");
	}

	unsigned int* DMap, *HammDist,*XorSum, *HammDistY;

	DMap = (unsigned int*)malloc(row * column * sizeof(unsigned int));
	HammDist = (unsigned int*)malloc(row*column*sizeof(unsigned int));
	XorSum = (unsigned int*)malloc(row*column*sizeof(unsigned int));
	HammDistY = (unsigned int*)malloc(row*column*sizeof(unsigned int));
	if (DMap == NULL)
	{
		fprintf(stderr, "out of memory\n");
	}

	gettimeofday(&tim, NULL);
	t5 = tim.tv_sec + (tim.tv_usec / 1000000.0);

	PreProcessImage(&imageL, UCLInput, UCLOutput, LCTImage, row,column);
	PreProcessImage(&imageR, UCRInput, UCROutput, RCTImage, row, column);

	gettimeofday(&tim, NULL);
	t6 = tim.tv_sec + (tim.tv_usec / 1000000.0) - t5;

	InitializeUIArray(DMap,row,column);

	//SHDRighttoLeft13(O,LC,RC,row,column);
	//average (O,row,column);



	std::thread a(SHDRighttoLeft13, DMap							, (__m128i*)LCTImage							, (__m128i*)RCTImage							, row / 4 + 6	, column, HammDist								, XorSum							, HammDistY);
	std::thread b(SHDRighttoLeft13, DMap + (row / 4 - 6)*column		, (__m128i*)LCTImage + (row / 4 - 6)*column		, (__m128i*)RCTImage + (row / 4 - 6)*column		, row / 4 + 12	, column, HammDist + (row / 4 - 6)*column		, XorSum + (row / 4 - 6)*column		, HammDistY + (row / 4 - 6)*column);
	std::thread c(SHDRighttoLeft13, DMap + (row / 2 - 6)*column		, (__m128i*)LCTImage + (row / 2 - 6)*column		, (__m128i*)RCTImage + (row / 2 - 6)*column		, row / 4 + 12	, column, HammDist + (row / 2 - 6)*column		, XorSum + (row / 2 - 6)*column		, HammDistY + (row / 2 - 6)*column);
	std::thread d(SHDRighttoLeft13, DMap + (3 * row / 4 - 6)*column	, (__m128i*)LCTImage + (3 * row / 4 - 6)*column	, (__m128i*)RCTImage + (3 * row / 4 - 6)*column	, row / 4 + 6	, column, HammDist + (3 * row / 4 - 6)*column	, XorSum + (3 * row / 4 - 6)*column	, HammDistY + (3 * row / 4 - 6)*column);

	a.join();
	b.join();
	c.join();
	d.join();

	ArrayUItoImage(&imageL,DMap,row,column);

	gettimeofday(&tim, NULL);
	t4 = tim.tv_sec + (tim.tv_usec / 1000000.0);
	t4 = t4-t3;

	imageL.write("output.png");

	cout<<endl<<sizeof(unsigned long long)<<endl;



	gettimeofday(&tim, NULL);
	t2 = tim.tv_sec + (tim.tv_usec / 1000000.0);
    t2= t2 - t1;

	cout<<endl<<"Total program run time : "<<t2<<endl;
	cout<<endl<<"Total processing time : "<<t4<<endl;
	cout<<endl<<"Total preprocessing time : "<<t6<<endl;

}




