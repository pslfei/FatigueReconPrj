///////////////////////////////////////////////////////////////////////////////
// 
// (c) Visage Technologies AB 2002 - 2023  All rights reserved. 
// 
// This file is part of visage|SDK(tm). 
// Unauthorized copying of this file, via any medium is strictly prohibited. 
// 
// No warranty, explicit or implicit, provided. 
// 
// This is proprietary software. No part of this software may be used or 
// reproduced in any form or by any means otherwise than in accordance with
// any written license granted by Visage Technologies AB. 
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef __Common_h__
#define __Common_h__

#ifdef VISAGE_STATIC
	#define VISAGE_DECLSPEC
#else

	#ifdef VISAGE_EXPORTS
		#define VISAGE_DECLSPEC __declspec(dllexport)
	#else
		#define VISAGE_DECLSPEC __declspec(dllimport)
	#endif

#endif

#include <fstream>
#include <sstream>
#ifdef EMSCRIPTEN
#include <cstddef>
#include <vector>
#endif
#include <string>
#include <stdio.h>
#include <time.h>

#include "FaceData.h"

#ifdef WIN32
	#include <direct.h>
	#define GetCurrentDir _getcwd
	#define ChangeDir _chdir
#else
	#include <unistd.h>
	#define GetCurrentDir getcwd
	#define ChangeDir chdir
#endif

namespace VNN
{
	class ViNNIEChooser;
}


namespace VisageSDK
{

#if defined(EMSCRIPTEN)
	VsImage* vsLoadImage(const char* name);
	int vsNamedWindow(const char* name);
	void vsShowImage(const char* name, VsImage* image);
	int vsWaitKey(int delay=0 );
	void vsDestroyWindow(const char* name);
	int vsSaveImage(const char* filename, const VsArr* image, const int* params=0 );
#endif

class FDP;

//structures used for the spline calculation 
typedef struct CubicPoly
{
	float c0, c1, c2, c3;

	float eval(float t)
	{
		float t2 = t*t;
		float t3 = t2 * t;
		return c0 + c1*t + c2*t2 + c3*t3;
	}
} CubicPoly;

typedef struct Vec2D
{
	Vec2D(float _x, float _y) : x(_x), y(_y) {}
	float x, y;
} Vec2D;


/**
* Provides common functionalities for VisageTrack
*/

class VISAGE_DECLSPEC Common
{
public:

	//functions added
	static void InitCubicPoly(float x0, float x1, float t0, float t1, CubicPoly &p);
	static void InitCatmullRom(float x0, float x1, float x2, float x3, CubicPoly &p);
	static void InitNonuniformCatmullRom(float x0, float x1, float x2, float x3, float dt0, float dt1, float dt2, CubicPoly &p);
	static float VecDistSquared(const Vec2D& p, const Vec2D& q);
	static void InitCentripetalCR(const Vec2D& p0, const Vec2D& p1, const Vec2D& p2, const Vec2D& p3, CubicPoly &px, CubicPoly &py);
	static void calcSpline(std::vector <float>& inputPoints, int ratio, std::vector <float>& outputPoints);

	static void drawLandmarkPoints(VsImage* grayImage, float bbX, float bbY, float s, float* landmarkCols, float* landmarkRows, int numPoints, int pointSize = 1, int intensity = 255);
	static void drawInitShape(VsImage** blankImage, float* cols, float* rows, int numPoints);
	static void drawFDP(VsImage* grayImage, FDP* fdp, bool flipY = false, int pointSize = 1, int intensity = 255, int groupInd = -1);
	static void drawRectangle(VsImage* grayImage, VsRect* rectangle, int intensity = 255);
	static void drawCircle(VsImage* grayImage, int x, int y, int r, int intensity = 255);
	static void drawLine(VsImage* grayImage, VsPoint p1, VsPoint p2, int intensity = 255);

	static void displayErrorMessageAndExit(VNN::ViNNIEChooser* chooser, const std::string& strModelPrefix);

	static bool isKeyPressed(int key);
	static void printArray(std::string name, float* array, int n, bool indices=false);

	/**	Given a greyscale, 32f image, compute the mean pixel value and subtract it from each pixel, then normalize
		to one.
		@param inputImage the input image
		@param mask 

	*/
	static void SubtractAvgNorm(VsImage* inputImage, VsImage* mask = NULL);

	/**	Given a greyscale, 32f image, compute the mean pixel value and subtract it from each pixel.
		@param inputImage the input image
		@param mask 

	*/
	static void SubtractAvg(VsImage* inputImage, VsImage* mask = NULL);

	/**	Given a greyscale, 32f image, compute the mean pixel value and subtract it from each pixel, setting the 
		mean to zero, then set the variance to one.

		@param inputImage the input image
		@param mask 

	*/
	static void SubtractAvgNormalizeStd(VsImage* inputImage, VsImage* mask);

	/**	Performs bilinear interploation of image img at location (x,y), works both for greyscale,
		BGR and Normalized BGR color images. Whe the input image is greyscale, the function returns the 
		interpolated value, while the arguments b,g,r are set to -1. On the other hand, when the image has 3 channels,
		the reuqested values are stored in b,g,r and the function returns 1.
		If (x,y) is on the image border, the pixel value is replicated.
		If all the neighbours of the pixel containing (x,y) are outside the image, -1 is returned and bgr are also set to -1
	
		@param img VsImage to be sampled, can be greyscale or BGR or NBGR, only float32 represenation
		@param x X coordinate of the requested position>
		@param y Y coordinate of the requested position
		@param b Interpolated value for the Blue channel, set to -1 if the image is greyscale or if we try to sample outside the image borders
		@param g Interpolated value for the Green channel, set to -1 if the image is greyscale or if we try to sample outside the image borders
		@param r Interpolated value for the Red channel, set to -1 if the image is greyscale or if we try to sample outside the image borders
		@Returns The interpolated value if the image in input is greyscale, 1 for a color image and -1 if the wished position for samplign is entirely outside the image
	*/
	static float Interpolate2d(const VsImage& img, float x, float y, float* b = NULL, float* g = NULL, float* r = NULL);

	/**	Bilinear interpolation at point (x,y) for a greyscale 32f image
	*/
	static float Interpolate2dGRAY32(const VsImage& img, float x, float y);

	/**	Bilinear interpolation at point (x,y) for a greyscale 8uchar image
	*/
	static uchar Interpolate2dGRAY8(const VsImage& img, float x, float y);

	/**	Convert a BGR unsiged char image to a NormnalizedBGR float32 image
		 Input and output images shall have the right depth and number of channels
		@param in The input BGR 8u image
		@param out The outpuyt NBGR 32f image		
	*/
	static void ConvertImageToNormalizedBGR32f(const VsImage* in, VsImage* out);


	/**	Normalizes a grey 32f image, i.e., pixel values are normalized between 0 and 1
		@param in The input 32f grey image
		@param out The output 32f grey image, all the pixels will be between 0 and 1
	*/
	static void NormalizeImg32f(VsImage* in, VsImage* out, VsImage* mask = NULL);


	/**	Converts a BGR 8u image into a grey 32f one, the resulting image is normalized (all the pixels are between 0 and 1)
				 The images passed to the function should be already allocated and with the right depth/num of channels
		@param in The BGR 8u input image
		@param out The greyscale float32 oputput image
		@param temp Optional temporary image, if preallocated speeds up the processing, especially if the conversion has to be performed 
							for each frame in a sequence and the frames have all the same size
	*/
	static void ConvertImageFrom8u3To32f1(const VsImage* in, VsImage* out, VsImage* temp = 0);


	/**	Converts a generic image to a grey 32f one, the resulting image is normalized (all the pixels are between 0 and 1)
		The images passed to the function should be already allocated and with the right depth/num of channels
		@param in Input image (can be BGR or RGB 8u or 32f)
		@param out The greyscale float32 oputput image
		*/
	static void ConvertImageTo32f1(const VsImage* in, VsImage* out);

	/**	Converts a grey 32f image to a grey 8u image, the resulting image is normalized (all the pixels are between 0 and 255)
		The images passed to the function should be already allocated and with the right depth/num of channels
		@param in Input image (should be grey 32f)
		@param out The greyscale 8u oputput image
		@param mask permits to consider only part of the pixels
		*/
	static void ConvertImageFrom32fTo8u(const VsImage* in, VsImage* out, VsImage* mask);


	static void FillImageSides(const VsImage* in, VsImage* out, VsImage* mask, VsImage* mask2);


	#pragma region Utility functions for I/O

	/// \brief Extracts the filename extension from a path+filename.
	///
	/// Ruthlessly stolen from osgDB.
	inline static std::string getFileExtension(const std::string& fileName)
	{
			std::string::size_type dot = fileName.find_last_of('.');
			if (dot==std::string::npos) return std::string("");
			return std::string(fileName.begin()+dot+1,fileName.end());
	}

	/// \brief Extracts the filename extension (in lower case) from a path+filename.
	///
	/// Ruthlessly stolen from osgDB.
	inline static std::string getLowerCaseFileExtension(const std::string& filename)
	{
			std::string ext = Common::getFileExtension(filename);
			for(std::string::iterator itr=ext.begin();
					itr!=ext.end();
					++itr)
			{
					*itr = static_cast<char>(tolower(*itr));
			}
			return ext;
	}
	/** Sets the path to where the config file is located. Copies the
		path in the char array cfgDirPath which must be atleast buffer_size long.
		@param cfgPath Input char array containing path to the configuration file
		@param cfgDirPath Output char array containing path to the configuration file
		*/
	inline static void setCfgDirPath(const char* cfgPath, char* cfgDirPath) {
		const int buffer_size = 1024;
		char cfgPathCopy[buffer_size] = { 0 };
		strncpy(cfgPathCopy, cfgPath, buffer_size - 1);
		cfgDirPath[0] = '\0';
		std::string fnstr((const char*)cfgPathCopy);
		size_t slashPos = fnstr.find_last_of( "/\\" );
		if( slashPos != std::string::npos )
			strncpy(cfgDirPath, fnstr.substr( 0, slashPos + 1).c_str(), buffer_size - 1);
	}

	inline static void getPath(const char* fname, char* path) {
		// this code doesn't work right
		// see, that's what happens when you use pointers and arrays
		// instead of strings and containers
		/*char *p1 = strrchr(path,'\\');
		char *p2 = strrchr(path,'/');
		if(p2 > p1) p1 = p2;
		if(p1) *p1 = 0; else path[0] = 0;*/
		std::string fnstr( fname ); 
		size_t pi = fnstr.find_last_of( "/\\" );
		if( pi == std::string::npos ) path[0] = 0;
		else strcpy( path, fnstr.substr( 0, pi ).c_str() );
	}

	///gets the next line in a stream
	static inline std::string getNextLine( std::ifstream& is )
	{
		std::string line;
		is.ignore(10000, '\n');
		do
		{
			std::getline( is, line );
		}
		while( line.length() == 0 );
		return line;
	}

	///Int to string 
	static inline void IntToString( std::string &s, int n ){
		std::ostringstream oss;
		oss << n;
		std::string tmp = oss.str();
		s.swap( tmp );
	}


	/// A commented line is a line starting with a given character (defaults to '#').
	/// Empty lines are skipped as well.
	static inline void skipComments( std::istream& is, char comment = '#' )
	{
		char next = static_cast<char>(is.peek());
		while( next == '\n' || next == comment ) {
			is.ignore( 10000, '\n' );
			next = static_cast<char>(is.peek());
		} 
	}


	static inline void skipComments( FILE* is, char comment = '#' )
	{

		int ch;
	
		ch = getc( is );        
		if ( ch==comment ) {
					
			do { 
				 ch=fgetc( is ); 
			 } 
			while( ch!=EOF );
			ungetc( ch, is );

			/*do { 
				ch=fgetc( is ); 
			 } while( ch==' ' || ch=='\t' );
			 ungetc( ch, is );*/

			//SkipRestOfLine();                        
			//SkipComments();
			//skipComments(is);

		} else {
			ungetc( ch, is );
		}

	}

#pragma endregion

	static void CLAHE(VsImage* dest, VsImage* src, int di = 8, int dj = 8, unsigned char s = 2);

	static void verticalFlip(FDP* featurePoints);

	static void RedirectIOToConsole();

	static void RedirectIOToFile(const char* fileName);

#if defined(WIN32) || defined(LINUX) || defined(ANDROID)
	static double getticks();

	class Stopwatch
	{
	private:
		double startTics;
		double elapsedms;

	public:
		void start() {startTics = getticks();}
		double getTime() {elapsedms = (getticks() - startTics); return elapsedms;}
	};
#endif

private:
	static void RCLAHEM(unsigned char imap[], unsigned char im[], int i0, int j0, int i1, int j1, int ldim, unsigned char s);

};

}

#endif // __Common_h__
