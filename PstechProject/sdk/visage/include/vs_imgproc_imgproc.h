/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

//TAKEN FROM opencv\sources\modules\imgproc\include\opencv2\imgproc\imgproc.hpp

#ifndef __VSDK_IMGPROC_HPP__
#define __VSDK_IMGPROC_HPP__

#include "vs_core_core.h"
#include "vs_imgproc_types_c.h"

#ifdef VISAGE_STATIC
#define VISAGE_DECLSPEC
#else

#ifdef VISAGE_EXPORTS
#define VISAGE_DECLSPEC __declspec(dllexport)
#else
#define VISAGE_DECLSPEC __declspec(dllimport)
#endif

#endif

#ifdef __cplusplus

/*! \namespace cv
 Namespace where all the C++ OpenCV functionality resides
 */
namespace vs
{
	//! various border interpolation methods
	enum { BORDER_REPLICATE=VS_BORDER_REPLICATE, BORDER_CONSTANT=VS_BORDER_CONSTANT,
		BORDER_REFLECT=VS_BORDER_REFLECT, BORDER_WRAP=VS_BORDER_WRAP,
		BORDER_REFLECT_101=VS_BORDER_REFLECT_101, BORDER_REFLECT101=BORDER_REFLECT_101,
		BORDER_TRANSPARENT=VS_BORDER_TRANSPARENT,
		BORDER_DEFAULT=BORDER_REFLECT_101, BORDER_ISOLATED=16 };

	//! type of the threshold operation
	enum { THRESH_BINARY=VS_THRESH_BINARY, THRESH_BINARY_INV=VS_THRESH_BINARY_INV,
		THRESH_TRUNC=VS_THRESH_TRUNC, THRESH_TOZERO=VS_THRESH_TOZERO,
		THRESH_TOZERO_INV=VS_THRESH_TOZERO_INV, THRESH_MASK=VS_THRESH_MASK,
		THRESH_OTSU=VS_THRESH_OTSU };

	//! interpolation algorithm
	enum
	{
		INTER_NEAREST=VS_INTER_NN, //!< nearest neighbor interpolation
		INTER_LINEAR=VS_INTER_LINEAR, //!< bilinear interpolation
		INTER_CUBIC=VS_INTER_CUBIC, //!< bicubic interpolation
		INTER_AREA=VS_INTER_AREA, //!< area-based (or super) interpolation
		INTER_LANCZOS4=VS_INTER_LANCZOS4, //!< Lanczos interpolation over 8x8 neighborhood
		INTER_MAX=7,
		WARP_INVERSE_MAP=VS_WARP_INVERSE_MAP
	};

	//! type of the kernel
	enum { KERNEL_GENERAL=0, KERNEL_SYMMETRICAL=1, KERNEL_ASYMMETRICAL=2,
		KERNEL_SMOOTH=4, KERNEL_INTEGER=8 };

	enum
	{
		INTER_BITS=5, INTER_BITS2=INTER_BITS*2,
		INTER_TAB_SIZE=(1<<INTER_BITS),
		INTER_TAB_SIZE2=INTER_TAB_SIZE*INTER_TAB_SIZE
	};

	/*!
	The Base Class for 1D or Row-wise Filters

	This is the base class for linear or non-linear filters that process 1D data.
	In particular, such filters are used for the "horizontal" filtering parts in separable filters.

	Several functions in OpenCV return Ptr<BaseRowFilter> for the specific types of filters,
	and those pointers can be used directly or within vs::FilterEngine.
	*/
	class VS_EXPORTS BaseRowFilter
	{
	public:
		//! the default constructor
		BaseRowFilter();
		//! the destructor
		virtual ~BaseRowFilter();
		//! the filtering operator. Must be overrided in the derived classes. The horizontal border interpolation is done outside of the class.
		virtual void operator()(const uchar* src, uchar* dst,
			int width, int cn) = 0;
		int ksize, anchor;
	};


	/*!
	The Base Class for Column-wise Filters

	This is the base class for linear or non-linear filters that process columns of 2D arrays.
	Such filters are used for the "vertical" filtering parts in separable filters.

	Several functions in OpenCV return Ptr<BaseColumnFilter> for the specific types of filters,
	and those pointers can be used directly or within vs::FilterEngine.

	Unlike vs::BaseRowFilter, vs::BaseColumnFilter may have some context information,
	i.e. box filter keeps the sliding sum of elements. To reset the state BaseColumnFilter::reset()
	must be called (e.g. the method is called by vs::FilterEngine)
	*/
	class VS_EXPORTS BaseColumnFilter
	{
	public:
		//! the default constructor
		BaseColumnFilter();
		//! the destructor
		virtual ~BaseColumnFilter();
		//! the filtering operator. Must be overrided in the derived classes. The vertical border interpolation is done outside of the class.
		virtual void operator()(const uchar** src, uchar* dst, int dststep,
			int dstcount, int width) = 0;
		//! resets the internal buffers, if any
		virtual void reset();
		int ksize, anchor;
	};

	/*!
	The Base Class for Non-Separable 2D Filters.

	This is the base class for linear or non-linear 2D filters.

	Several functions in OpenCV return Ptr<BaseFilter> for the specific types of filters,
	and those pointers can be used directly or within vs::FilterEngine.

	Similar to vs::BaseColumnFilter, the class may have some context information,
	that should be reset using BaseFilter::reset() method before processing the new array.
	*/
	class VS_EXPORTS BaseFilter
	{
	public:
		//! the default constructor
		BaseFilter();
		//! the destructor
		virtual ~BaseFilter();
		//! the filtering operator. The horizontal and the vertical border interpolation is done outside of the class.
		virtual void operator()(const uchar** src, uchar* dst, int dststep,
			int dstcount, int width, int cn) = 0;
		//! resets the internal buffers, if any
		virtual void reset();
		Size ksize;
		Point anchor;
	};

	/*!
	The Main Class for Image Filtering.

	The class can be used to apply an arbitrary filtering operation to an image.
	It contains all the necessary intermediate buffers, it computes extrapolated values
	of the "virtual" pixels outside of the image etc.
	Pointers to the initialized vs::FilterEngine instances
	are returned by various OpenCV functions, such as vs::createSeparableLinearFilter(),
	vs::createLinearFilter(), vs::createGaussianFilter(), vs::createDerivFilter(),
	vs::createBoxFilter() and vs::createMorphologyFilter().

	Using the class you can process large images by parts and build complex pipelines
	that include filtering as some of the stages. If all you need is to apply some pre-defined
	filtering operation, you may use vs::filter2D(), vs::erode(), vs::dilate() etc.
	functions that create FilterEngine internally.

	Here is the example on how to use the class to implement Laplacian operator, which is the sum of
	second-order derivatives. More complex variant for different types is implemented in vs::Laplacian().

	\code
	void laplace_f(const Mat& src, Mat& dst)
	{
	VS_Assert( src.type() == VS_32F );
	// make sure the destination array has the proper size and type
	dst.create(src.size(), src.type());

	// get the derivative and smooth kernels for d2I/dx2.
	// for d2I/dy2 we could use the same kernels, just swapped
	Mat kd, ks;
	getSobelKernels( kd, ks, 2, 0, ksize, false, ktype );

	// let's process 10 source rows at once
	int DELTA = std::min(10, src.rows);
	Ptr<FilterEngine> Fxx = createSeparableLinearFilter(src.type(),
	dst.type(), kd, ks, Point(-1,-1), 0, borderType, borderType, Scalar() );
	Ptr<FilterEngine> Fyy = createSeparableLinearFilter(src.type(),
	dst.type(), ks, kd, Point(-1,-1), 0, borderType, borderType, Scalar() );

	int y = Fxx->start(src), dsty = 0, dy = 0;
	Fyy->start(src);
	const uchar* sptr = src.data + y*src.step;

	// allocate the buffers for the spatial image derivatives;
	// the buffers need to have more than DELTA rows, because at the
	// last iteration the output may take max(kd.rows-1,ks.rows-1)
	// rows more than the input.
	Mat Ixx( DELTA + kd.rows - 1, src.cols, dst.type() );
	Mat Iyy( DELTA + kd.rows - 1, src.cols, dst.type() );

	// inside the loop we always pass DELTA rows to the filter
	// (note that the "proceed" method takes care of possibe overflow, since
	// it was given the actual image height in the "start" method)
	// on output we can get:
	//  * < DELTA rows (the initial buffer accumulation stage)
	//  * = DELTA rows (settled state in the middle)
	//  * > DELTA rows (then the input image is over, but we generate
	//                  "virtual" rows using the border mode and filter them)
	// this variable number of output rows is dy.
	// dsty is the current output row.
	// sptr is the pointer to the first input row in the portion to process
	for( ; dsty < dst.rows; sptr += DELTA*src.step, dsty += dy )
	{
	Fxx->proceed( sptr, (int)src.step, DELTA, Ixx.data, (int)Ixx.step );
	dy = Fyy->proceed( sptr, (int)src.step, DELTA, d2y.data, (int)Iyy.step );
	if( dy > 0 )
	{
	Mat dstripe = dst.rowRange(dsty, dsty + dy);
	add(Ixx.rowRange(0, dy), Iyy.rowRange(0, dy), dstripe);
	}
	}
	}
	\endcode
	*/
	class VS_EXPORTS FilterEngine
	{
	public:
		//! the default constructor
		FilterEngine();
		//! the full constructor. Either _filter2D or both _rowFilter and _columnFilter must be non-empty.
		FilterEngine(const Ptr<BaseFilter>& _filter2D,
			const Ptr<BaseRowFilter>& _rowFilter,
			const Ptr<BaseColumnFilter>& _columnFilter,
			int srcType, int dstType, int bufType,
			int _rowBorderType=BORDER_REPLICATE,
			int _columnBorderType=-1,
			const Scalar& _borderValue=Scalar());
		//! the destructor
		virtual ~FilterEngine();
		//! reinitializes the engine. The previously assigned filters are released.
		void init(const Ptr<BaseFilter>& _filter2D,
			const Ptr<BaseRowFilter>& _rowFilter,
			const Ptr<BaseColumnFilter>& _columnFilter,
			int srcType, int dstType, int bufType,
			int _rowBorderType=BORDER_REPLICATE, int _columnBorderType=-1,
			const Scalar& _borderValue=Scalar());
		//! starts filtering of the specified ROI of an image of size wholeSize.
		virtual int start(Size wholeSize, Rect roi, int maxBufRows=-1);
		//! starts filtering of the specified ROI of the specified image.
		virtual int start(const Mat& src, const Rect& srcRoi=Rect(0,0,-1,-1),
			bool isolated=false, int maxBufRows=-1);
		//! processes the next srcCount rows of the image.
		virtual int proceed(const uchar* src, int srcStep, int srcCount,
			uchar* dst, int dstStep);
		//! applies filter to the specified ROI of the image. if srcRoi=(0,0,-1,-1), the whole image is filtered.
		virtual void apply( const Mat& src, Mat& dst,
			const Rect& srcRoi=Rect(0,0,-1,-1),
			Point dstOfs=Point(0,0),
			bool isolated=false);
		//! returns true if the filter is separable
		bool isSeparable() const { return (const BaseFilter*)filter2D == 0; }
		//! returns the number
		int remainingInputRows() const;
		int remainingOutputRows() const;

		int srcType, dstType, bufType;
		Size ksize;
		Point anchor;
		int maxWidth;
		Size wholeSize;
		Rect roi;
		int dx1, dx2;
		int rowBorderType, columnBorderType;
		vector<int> borderTab;
		int borderElemSize;
		vector<uchar> ringBuf;
		vector<uchar> srcRow;
		vector<uchar> constBorderValue;
		vector<uchar> constBorderRow;
		int bufStep, startY, startY0, endY, rowCount, dstY;
		vector<uchar*> rows;

		Ptr<BaseFilter> filter2D;
		Ptr<BaseRowFilter> rowFilter;
		Ptr<BaseColumnFilter> columnFilter;
	};

	//! returns type (one of KERNEL_*) of 1D or 2D kernel specified by its coefficients.
	int getKernelType(InputArray kernel, Point anchor);

	//! returns the primitive row filter with the specified kernel
	Ptr<BaseRowFilter> getLinearRowFilter(int srcType, int bufType, InputArray kernel, int anchor, int symmetryType);

	//! returns the primitive column filter with the specified kernel
	Ptr<BaseColumnFilter> getLinearColumnFilter(int bufType, int dstType, InputArray kernel, int anchor, int symmetryType, double delta=0, int bits=0);

	//! returns 2D filter with the specified kernel
	Ptr<BaseFilter> getLinearFilter(int srcType, int dstType, InputArray kernel, Point anchor=Point(-1,-1), double delta=0, int bits=0);

	//! returns the separable linear filter engine
	Ptr<FilterEngine> createSeparableLinearFilter(int srcType, int dstType, InputArray rowKernel, InputArray columnKernel, Point anchor=Point(-1,-1), double delta=0, int rowBorderType=BORDER_DEFAULT, int columnBorderType=-1, const Scalar& borderValue=Scalar());

	//! returns the non-separable linear filter engine
	Ptr<FilterEngine> createLinearFilter(int srcType, int dstType, InputArray kernel, Point _anchor=Point(-1,-1), double delta=0, int rowBorderType=BORDER_DEFAULT, int columnBorderType=-1, const Scalar& borderValue=Scalar());

	//! applies non-separable 2D linear filter to the image
	void filter2D( InputArray src, OutputArray dst, int ddepth, InputArray kernel, Point anchor=Point(-1,-1), double delta=0, int borderType=BORDER_DEFAULT );

	//! applies separable 2D linear filter to the image
	void sepFilter2D( InputArray src, OutputArray dst, int ddepth, InputArray kernelX, InputArray kernelY, Point anchor=Point(-1,-1), double delta=0, int borderType=BORDER_DEFAULT );

	//! copies 2D array to a larger destination array with extrapolation of the outer part of src using the specified border mode
	void copyMakeBorder( InputArray src, OutputArray dst, int top, int bottom, int left, int right, int borderType, const Scalar& value=Scalar() );

	//! 1D interpolation function: returns coordinate of the "donor" pixel for the specified location p.
	int borderInterpolate( int p, int len, int borderType );

	//! smooths and downsamples the image
	void pyrDown( InputArray src, OutputArray dst, const Size& dstsize=Size(), int borderType=BORDER_DEFAULT );

	//! smooths the image using Gaussian filter.
	void gaussianBlur( InputArray src, OutputArray dst, Size ksize, double sigmaX, double sigmaY=0, int borderType=BORDER_DEFAULT );

	//! returns the Gaussian filter engine
	Ptr<FilterEngine> createGaussianFilter( int type, Size ksize, double sigma1, double sigma2=0, int borderType=BORDER_DEFAULT );

	//! c-interface version of vs::gaussianBlur for compatibility (not part of imgproc)
	VISAGE_DECLSPEC void gaussianBlur( const VsArr* src, VsArr* dst, int ksize = 3 );

	//! returns the Gaussian kernel with the specified parameters
	Mat getGaussianKernel( int ksize, double sigma, int ktype=VS_64F );

	//! applies generalized Sobel operator to the image
	void sobel( InputArray src, OutputArray dst, int ddepth,int dx, int dy, int ksize=3,double scale=1, double delta=0, int borderType=BORDER_DEFAULT );

	//! c-interface version of sobel for compatibility (not part of imgproc)
	void sobel( const VsArr* src, VsArr* dst, int xorder, int yorder, int aperture_size VS_DEFAULT(3));

	//! normalizes the grayscale image brightness and contrast by normalizing its histogram
	void equalizeHist( InputArray src, OutputArray dst );

	//! c-interface version of equalizeHist for compatibility (not part of imgproc)
	VISAGE_DECLSPEC void equalizeHist( const VsArr* src, VsArr* dst );

	//! applies fixed threshold to the image
	double threshold( InputArray src, OutputArray dst, double thresh, double maxval, int type );

	//! c-interface version of threshold for compatibility (not part of imgproc)
	double threshold( const VsArr*  src, VsArr*  dst, double  threshold, double  max_value, int threshold_type );

	//! warps the image using affine transformation
	void warpAffine( InputArray src, OutputArray dst, InputArray M, Size dsize, int flags=INTER_LINEAR, int borderMode=BORDER_CONSTANT, const Scalar& borderValue=Scalar());

	//! computes 2x3 affine transformation matrix that is inverse to the specified 2x3 affine transformation.
	VS_EXPORTS_W void invertAffineTransform(InputArray M, OutputArray iM);

	//! returns 2x3 affine transformation matrix for the planar rotation.
	Mat getRotationMatrix2D( Point2f center, double angle, double scale );

	//! c-interface version of getRotationMatrix2D for compatibility (not part of imgproc)
	VsMat* getRotationMatrix2D( VsPoint2D32f center, double angle, double scale, VsMat* map_matrix );

	//! converts image from one color space to another
	void cvtColor( InputArray src, OutputArray dst, int code, int dstCn=0 );

	//! resizes the image
	void resize( InputArray src, OutputArray dst, Size dsize, double fx=0, double fy=0, int interpolation=INTER_LINEAR );
}

#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

	//! c-interface version of cvtColor for compatibility (not part of imgproc)
	VISAGE_DECLSPEC void vsCvtColor( const VsArr* src, VsArr* dst, int code );

	//! c-interface version of resize for compatibility (not part of imgproc)
	VISAGE_DECLSPEC void vsResize( const VsArr* src, VsArr* dst, int interpolation VS_DEFAULT(VS_INTER_LINEAR));

	//! c-interface version of copyMakeBorder for compatibility (not part of imgproc)
	/* Copies source 2D array inside of the larger destination array and
	makes a border of the specified type (IPL_BORDER_*) around the copied area. */
	VISAGE_DECLSPEC void vsCopyMakeBorder(const VsArr* src, VsArr* dst, VsPoint offset, int bordertype, VsScalar value VS_DEFAULT(vsScalarAll(0)));

	//! c-interface version of warpAffine for compatibility (not part of imgproc)
	VISAGE_DECLSPEC void vsWarpAffine(const VsArr* src, VsArr* dst, const VsMat* map_matrix, int flags VS_DEFAULT(VS_INTER_LINEAR + VS_WARP_FILL_OUTLIERS), VsScalar fillval VS_DEFAULT(vsScalarAll(0)));

#ifdef __cplusplus
}
#endif

#endif // __VSDK_IMGPROC_HPP__