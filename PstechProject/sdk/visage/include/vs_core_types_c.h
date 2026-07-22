/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
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

#ifndef __VS_CORE_TYPES_H__
#define __VS_CORE_TYPES_H__

#if !defined _CRT_SECURE_NO_DEPRECATE && defined _MSC_VER
#  if _MSC_VER > 1300
#    define _CRT_SECURE_NO_DEPRECATE /* to avoid multiple Visual Studio 2005 warnings */
#  endif
#endif


#ifndef SKIP_INCLUDES

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#if !defined _MSC_VER && !defined __BORLANDC__
#  include <stdint.h>
#endif

#if defined __ICL
#  define VS_ICC   __ICL
#elif defined __ICC
#  define VS_ICC   __ICC
#elif defined __ECL
#  define VS_ICC   __ECL
#elif defined __ECC
#  define VS_ICC   __ECC
#elif defined __INTEL_COMPILER
#  define VS_ICC   __INTEL_COMPILER
#endif

#if defined VS_ICC && !defined VS_ENABLE_UNROLLED
#  define VS_ENABLE_UNROLLED 0
#else
#  define VS_ENABLE_UNROLLED 1
#endif

#if (defined _M_X64 && defined _MSC_VER && _MSC_VER >= 1400) || (defined __GNUC__ && __GNUC__ >= 4 && defined __x86_64__)
#  if defined WIN32
#    include <intrin.h>
#  endif
#  if defined __SSE2__ || !defined __GNUC__
#    include <emmintrin.h>
#  endif
#endif

#include <math.h>

#endif // SKIP_INCLUDES

#if defined WIN32 || defined _WIN32
#  define VS_CDECL __cdecl
#  define VS_STDCALL __stdcall
#else
#  define VS_CDECL
#  define VS_STDCALL
#endif

#ifndef VS_EXTERN_C
#  ifdef __cplusplus
#    define VS_EXTERN_C extern "C"
#    define VS_DEFAULT(val) = val
#  else
#    define VS_EXTERN_C
#    define VS_DEFAULT(val)
#  endif
#endif

#ifndef VS_EXTERN_C_FUNCPTR
#  ifdef __cplusplus
#    define VS_EXTERN_C_FUNCPTR(x) extern "C" { typedef x; }
#  else
#    define VS_EXTERN_C_FUNCPTR(x) typedef x
#  endif
#endif

#ifndef VS_INLINE
#  if defined __cplusplus
#    define VS_INLINE inline
#  elif defined _MSC_VER
#    define VS_INLINE __inline
#  else
#    define VS_INLINE static
#  endif
#endif /* VS_INLINE */

#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined CVAPI_EXPORTS
#  define VS_EXPORTS __declspec(dllexport)
#else
#  define VS_EXPORTS
#endif

#ifndef CVAPI
#  define CVAPI(rettype) VS_EXTERN_C VS_EXPORTS rettype VS_CDECL
#endif

#ifndef VSAPI
#if (defined WIN32 || defined _WIN32 || defined WINCE)
#  define VSAPI(rettype) __declspec(dllexport) rettype VS_CDECL
#else
#  define VSAPI(rettype) VS_EXTERN_C VS_EXPORTS rettype VS_CDECL
#endif
#endif

#if defined _MSC_VER || defined __BORLANDC__
   typedef __int64 int64;
   typedef unsigned __int64 uint64;
#  define VS_BIG_INT(n)   n##I64
#  define VS_BIG_UINT(n)  n##UI64
#else
   typedef int64_t int64;
   typedef uint64_t uint64;
#  define VS_BIG_INT(n)   n##LL
#  define VS_BIG_UINT(n)  n##ULL
#endif

#ifndef HAVE_IPL
   typedef unsigned char uchar;
   typedef unsigned short ushort;
#endif

typedef signed char schar;

/* special informative macros for wrapper generators */
#define VS_CARRAY(counter)
#define VS_CUSTOM_CARRAY(args)
#define VS_EXPORTS_W VS_EXPORTS
#define VS_EXPORTS_W_SIMPLE VS_EXPORTS
#define VS_EXPORTS_AS(synonym) VS_EXPORTS
#define VS_EXPORTS_W_MAP VS_EXPORTS
#define VS_IN_OUT
#define VS_OUT
#define VS_PROP
#define VS_PROP_RW
#define VS_WRAP
#define VS_WRAP_AS(synonym)
#define VS_WRAP_DEFAULT(value)

/* VsArr* is used to pass arbitrary
 * array-like data structures
 * into functions where the particular
 * array type is recognized at runtime:
 */
typedef void VsArr;

typedef union Vs32suf
{
    int i;
    unsigned u;
    float f;
}
Vs32suf;

typedef union Vs64suf
{
    int64 i;
    uint64 u;
    double f;
}
Vs64suf;

typedef int VSStatus;

enum {
 VS_StsOk=                       0,  /* everithing is ok                */
 VS_StsBackTrace=               -1,  /* pseudo error for back trace     */
 VS_StsError=                   -2,  /* unknown /unspecified error      */
 VS_StsInternal=                -3,  /* internal error (bad state)      */
 VS_StsNoMem=                   -4,  /* insufficient memory             */
 VS_StsBadArg=                  -5,  /* function arg/param is bad       */
 VS_StsBadFunc=                 -6,  /* unsupported function            */
 VS_StsNoConv=                  -7,  /* iter. didn't converge           */
 VS_StsAutoTrace=               -8,  /* tracing                         */
 VS_HeaderIsNull=               -9,  /* image header is NULL            */
 VS_BadImageSize=              -10, /* image size is invalid           */
 VS_BadOffset=                 -11, /* offset is invalid               */
 VS_BadDataPtr=                -12, /**/
 VS_BadStep=                   -13, /**/
 VS_BadModelOrChSeq=           -14, /**/
 VS_BadNumChannels=            -15, /**/
 VS_BadNumChannel1U=           -16, /**/
 VS_BadDepth=                  -17, /**/
 VS_BadAlphaChannel=           -18, /**/
 VS_BadOrder=                  -19, /**/
 VS_BadOrigin=                 -20, /**/
 VS_BadAlign=                  -21, /**/
 VS_BadCallBack=               -22, /**/
 VS_BadTileSize=               -23, /**/
 VS_BadCOI=                    -24, /**/
 VS_BadROISize=                -25, /**/
 VS_MaskIsTiled=               -26, /**/
 VS_StsNullPtr=                -27, /* null pointer */
 VS_StsVecLengthErr=           -28, /* incorrect vector length */
 VS_StsFilterStructContentErr= -29, /* incorr. filter structure content */
 VS_StsKernelStructContentErr= -30, /* incorr. transform kernel content */
 VS_StsFilterOffsetErr=        -31, /* incorrect filter offset value */
 VS_StsBadSize=                -201, /* the input/output structure size is incorrect  */
 VS_StsDivByZero=              -202, /* division by zero */
 VS_StsInplaceNotSupported=    -203, /* in-place operation is not supported */
 VS_StsObjectNotFound=         -204, /* request can't be completed */
 VS_StsUnmatchedFormats=       -205, /* formats of input/output arrays differ */
 VS_StsBadFlag=                -206, /* flag is wrong or not supported */
 VS_StsBadPoint=               -207, /* bad VsPoint */
 VS_StsBadMask=                -208, /* bad format of mask (neither 8uC1 nor 8sC1)*/
 VS_StsUnmatchedSizes=         -209, /* sizes of input/output structures do not match */
 VS_StsUnsupportedFormat=      -210, /* the data format/type is not supported by the function*/
 VS_StsOutOfRange=             -211, /* some of parameters are out of range */
 VS_StsParseError=             -212, /* invalid syntax/structure of the parsed file */
 VS_StsNotImplemented=         -213, /* the requested function/feature is not implemented */
 VS_StsBadMemBlock=            -214, /* an allocated block has been corrupted */
 VS_StsAssert=                 -215, /* assertion failed */
 VS_GpuNotSupported=           -216,
 VS_GpuApiCallError=           -217,
 VS_OpenGlNotSupported=        -218,
 VS_OpenGlApiCallError=        -219,
 VS_OpenCLDoubleNotSupported=  -220,
 VS_OpenCLInitError=           -221,
 VS_OpenCLNoAMDBlasFft=        -222
};

/****************************************************************************************\
*                             Common macros and inline functions                         *
\****************************************************************************************/

#ifdef HAVE_TEGRA_OPTIMIZATION
#  include "tegra_round.hpp"
#endif

#define VS_PI   3.1415926535897932384626433832795
#define VS_LOG2 0.69314718055994530941723212145818

#define VS_SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))

#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

/* min & max without jumps */
#define  VS_IMIN(a, b)  ((a) ^ (((a)^(b)) & (((a) < (b)) - 1)))

#define  VS_IMAX(a, b)  ((a) ^ (((a)^(b)) & (((a) > (b)) - 1)))

/* absolute value without jumps */
#ifndef __cplusplus
#  define  VS_IABS(a)     (((a) ^ ((a) < 0 ? -1 : 0)) - ((a) < 0 ? -1 : 0))
#else
#  define  VS_IABS(a)     abs(a)
#endif
#define  VS_CMP(a,b)    (((a) > (b)) - ((a) < (b)))
#define  VS_SIGN(a)     VS_CMP((a),0)

VS_INLINE  int  vsRound( double value )
{
#if (defined _MSC_VER && defined _M_X64) || (defined __GNUC__ && defined __x86_64__ && defined __SSE2__ && !defined __APPLE__)
    __m128d t = _mm_set_sd( value );
    return _mm_cvtsd_si32(t);
#elif defined _MSC_VER && defined _M_IX86
    int t;
    __asm
    {
        fld value;
        fistp t;
    }
    return t;
#elif defined _MSC_VER && defined _M_ARM && defined HAVE_TEGRA_OPTIMIZATION
    TEGRA_ROUND(value);
#elif defined VS_ICC || defined __GNUC__
#  ifdef HAVE_TEGRA_OPTIMIZATION
    TEGRA_ROUND(value);
#  else
    return (int)lrint(value);
#  endif
#else
    double intpart, fractpart;
    fractpart = modf(value, &intpart);
    if ((fabs(fractpart) != 0.5) || ((((int)intpart) % 2) != 0))
        return (int)(value + (value >= 0 ? 0.5 : -0.5));
    else
        return (int)intpart;
#endif
}

#if defined __SSE2__ || (defined _M_IX86_FP && 2 == _M_IX86_FP)
#  include "emmintrin.h"
#endif

VS_INLINE  int  vsFloor( double value )
{
#if defined _MSC_VER && defined _M_X64 || (defined __GNUC__ && defined __SSE2__ && !defined __APPLE__)
    __m128d t = _mm_set_sd( value );
    int i = _mm_cvtsd_si32(t);
    return i - _mm_movemask_pd(_mm_cmplt_sd(t, _mm_cvtsi32_sd(t,i)));
#elif defined __GNUC__
    int i = (int)value;
    return i - (i > value);
#else
    int i = vsRound(value);
    float diff = (float)(value - i);
    return i - (diff < 0);
#endif
}


VS_INLINE  int  vsCeil( double value )
{
#if defined _MSC_VER && defined _M_X64 || (defined __GNUC__ && defined __SSE2__&& !defined __APPLE__)
    __m128d t = _mm_set_sd( value );
    int i = _mm_cvtsd_si32(t);
    return i + _mm_movemask_pd(_mm_cmplt_sd(_mm_cvtsi32_sd(t,i), t));
#elif defined __GNUC__
    int i = (int)value;
    return i + (i < value);
#else
    int i = vsRound(value);
    float diff = (float)(i - value);
    return i + (diff < 0);
#endif
}

#define vsInvSqrt(value) ((float)(1./sqrt(value)))
#define vsSqrt(value)  ((float)sqrt(value))

VS_INLINE int vsIsNaN( double value )
{
    Vs64suf ieee754;
    ieee754.f = value;
    return ((unsigned)(ieee754.u >> 32) & 0x7fffffff) +
           ((unsigned)ieee754.u != 0) > 0x7ff00000;
}


VS_INLINE int vsIsInf( double value )
{
    Vs64suf ieee754;
    ieee754.f = value;
    return ((unsigned)(ieee754.u >> 32) & 0x7fffffff) == 0x7ff00000 &&
           (unsigned)ieee754.u == 0;
}


/*************** Random number generation *******************/

typedef uint64 VsRNG;

#define VS_RNG_COEFF 4164903690U

VS_INLINE VsRNG vsRNG( int64 seed VS_DEFAULT(-1))
{
    VsRNG rng = seed ? (uint64)seed : (uint64)(int64)-1;
    return rng;
}

/* Return random 32-bit unsigned integer: */
VS_INLINE unsigned vsRandInt( VsRNG* rng )
{
    uint64 temp = *rng;
    temp = (uint64)(unsigned)temp*VS_RNG_COEFF + (temp >> 32);
    *rng = temp;
    return (unsigned)temp;
}

/* Returns random floating-point number between 0 and 1: */
VS_INLINE double vsRandReal( VsRNG* rng )
{
    return vsRandInt(rng)*2.3283064365386962890625e-10 /* 2^-32 */;
}

/****************************************************************************************\
*                                  Image type (VsImage)                                 *
\****************************************************************************************/

#ifndef HAVE_IPL

/*
 * The following definitions (until #endif)
 * is an extract from IPL headers.
 * Copyright (c) 1995 Intel Corporation.
 */
#define VS_DEPTH_SIGN 0x80000000

#define VS_DEPTH_1U     1
#define VS_DEPTH_8U     8
#define VS_DEPTH_16U   16
#define VS_DEPTH_32F   32

#define VS_DEPTH_8S  (VS_DEPTH_SIGN| 8)
#define VS_DEPTH_16S (VS_DEPTH_SIGN|16)
#define VS_DEPTH_32S (VS_DEPTH_SIGN|32)

#define VS_DATA_ORDER_PIXEL  0
#define VS_DATA_ORDER_PLANE  1

#define VS_ORIGIN_TL 0
#define VS_ORIGIN_BL 1

#define VS_ALIGN_4BYTES   4
#define VS_ALIGN_8BYTES   8
#define VS_ALIGN_16BYTES 16
#define VS_ALIGN_32BYTES 32

#define VS_ALIGN_DWORD   VS_ALIGN_4BYTES
#define VS_ALIGN_QWORD   VS_ALIGN_8BYTES

#define VS_BORDER_CONSTANT   0
#define VS_BORDER_REPLICATE  1
#define VS_BORDER_REFLECT    2
#define VS_BORDER_WRAP       3

/**
* Image store container
*/
typedef struct _VsImage
{
    int  nSize;             /**< sizeof(VsImage) */
    int  ID;                /**< version (=0)*/
    int  nChannels;         /**< Most of OpenCV functions support 1,2,3 or 4 channels */
    int  alphaChannel;      /**< Ignored by OpenCV */
    int  depth;             /**< Pixel depth in bits: VS_DEPTH_8U, VS_DEPTH_8S, VS_DEPTH_16S,
                               VS_DEPTH_32S, VS_DEPTH_32F and VS_DEPTH_64F are supported.  */
    char colorModel[4];     /**< Ignored by OpenCV */
    char channelSeq[4];     /**< Ignored by OpenCV */
    int  dataOrder;         /**< 0 - interleaved color channels, 1 - separate color channels.
                               vsCreateImage can only create interleaved images */
    int  origin;            /**< 0 - top-left origin,
                               1 - bottom-left origin (Windows bitmaps style).  */
    int  align;             /**< Alignment of image rows (4 or 8).
                               OpenCV ignores it and uses widthStep instead.    */
    int  width;             /**< Image width in pixels.                           */
    int  height;            /**< Image height in pixels.                          */
    struct _VsROI *roi;     /**< Image ROI. If NULL, the whole image is selected. */
    struct _VsImage *maskROI;      /**< Must be NULL. */
    void  *imageId;                 /**< Must be NULL. */
    struct _VsTileInfo *tileInfo;  /**< Must be NULL. */
    int  imageSize;         /**< Image data size in bytes
                               (==image->height*image->widthStep
                               in case of interleaved data)*/
    char *imageData;        /**< Pointer to aligned image data.         */
    int  widthStep;         /**< Size of aligned image row in bytes.    */
    int  BorderMode[4];     /**< Ignored by OpenCV.                     */
    int  BorderConst[4];    /**< Ignored by OpenCV.                                 */
    char *imageDataOrigin;  /**< Pointer to very origin of image data
                               (not necessarily aligned) -
                               needed for correct deallocation */
}
VsImage;

typedef struct _VsTileInfo VsTileInfo;

typedef struct _VsROI
{
    int  coi; /* 0 - no COI (all channels are selected), 1 - 0th channel is selected ...*/
    int  xOffset;
    int  yOffset;
    int  width;
    int  height;
}
VsROI;

typedef struct _VsConvKernel
{
    int  nCols;
    int  nRows;
    int  anchorX;
    int  anchorY;
    int *values;
    int  nShiftR;
}
VsConvKernel;

typedef struct _VsConvKernelFP
{
    int  nCols;
    int  nRows;
    int  anchorX;
    int  anchorY;
    float *values;
}
VsConvKernelFP;

#define VS_IMAGE_HEADER 1
#define VS_IMAGE_DATA   2
#define VS_IMAGE_ROI    4

#endif/*HAVE_IPL*/

/* extra border mode */
#define VS_BORDER_REFLECT_101    4
#define VS_BORDER_TRANSPARENT    5

#define VS_IMAGE_MAGIC_VAL  ((int)sizeof(VsImage))
#define VS_TYPE_NAME_IMAGE "opencv-image"

#define VS_IS_IMAGE_HDR(img) \
    ((img) != NULL && ((const VsImage*)(img))->nSize == sizeof(VsImage))

#define VS_IS_IMAGE(img) \
    (VS_IS_IMAGE_HDR(img) && ((VsImage*)img)->imageData != NULL)

/* for storing double-precision
   floating point data in VsImage's */
#define VS_DEPTH_64F  64

/* get reference to pixel at (col,row),
   for multi-channel images (col) should be multiplied by number of channels */
#define VS_IMAGE_ELEM( image, elemtype, row, col )       \
    (((elemtype*)((image)->imageData + (image)->widthStep*(row)))[(col)])

/****************************************************************************************\
*                                  Matrix type (VsMat)                                   *
\****************************************************************************************/

#define VS_CN_MAX     512
#define VS_CN_SHIFT   3
#define VS_DEPTH_MAX  (1 << VS_CN_SHIFT)

#define VS_8U   0
#define VS_8S   1
#define VS_16U  2
#define VS_16S  3
#define VS_32S  4
#define VS_32F  5
#define VS_64F  6
#define VS_USRTYPE1 7

#define VS_MAT_DEPTH_MASK       (VS_DEPTH_MAX - 1)
#define VS_MAT_DEPTH(flags)     ((flags) & VS_MAT_DEPTH_MASK)

#define VS_MAKETYPE(depth,cn) (VS_MAT_DEPTH(depth) + (((cn)-1) << VS_CN_SHIFT))
#define VS_MAKE_TYPE VS_MAKETYPE

#define VS_8UC1 VS_MAKETYPE(VS_8U,1)
#define VS_8UC2 VS_MAKETYPE(VS_8U,2)
#define VS_8UC3 VS_MAKETYPE(VS_8U,3)
#define VS_8UC4 VS_MAKETYPE(VS_8U,4)
#define VS_8UC(n) VS_MAKETYPE(VS_8U,(n))

#define VS_8SC1 VS_MAKETYPE(VS_8S,1)
#define VS_8SC2 VS_MAKETYPE(VS_8S,2)
#define VS_8SC3 VS_MAKETYPE(VS_8S,3)
#define VS_8SC4 VS_MAKETYPE(VS_8S,4)
#define VS_8SC(n) VS_MAKETYPE(VS_8S,(n))

#define VS_16UC1 VS_MAKETYPE(VS_16U,1)
#define VS_16UC2 VS_MAKETYPE(VS_16U,2)
#define VS_16UC3 VS_MAKETYPE(VS_16U,3)
#define VS_16UC4 VS_MAKETYPE(VS_16U,4)
#define VS_16UC(n) VS_MAKETYPE(VS_16U,(n))

#define VS_16SC1 VS_MAKETYPE(VS_16S,1)
#define VS_16SC2 VS_MAKETYPE(VS_16S,2)
#define VS_16SC3 VS_MAKETYPE(VS_16S,3)
#define VS_16SC4 VS_MAKETYPE(VS_16S,4)
#define VS_16SC(n) VS_MAKETYPE(VS_16S,(n))

#define VS_32SC1 VS_MAKETYPE(VS_32S,1)
#define VS_32SC2 VS_MAKETYPE(VS_32S,2)
#define VS_32SC3 VS_MAKETYPE(VS_32S,3)
#define VS_32SC4 VS_MAKETYPE(VS_32S,4)
#define VS_32SC(n) VS_MAKETYPE(VS_32S,(n))

#define VS_32FC1 VS_MAKETYPE(VS_32F,1)
#define VS_32FC2 VS_MAKETYPE(VS_32F,2)
#define VS_32FC3 VS_MAKETYPE(VS_32F,3)
#define VS_32FC4 VS_MAKETYPE(VS_32F,4)
#define VS_32FC(n) VS_MAKETYPE(VS_32F,(n))

#define VS_64FC1 VS_MAKETYPE(VS_64F,1)
#define VS_64FC2 VS_MAKETYPE(VS_64F,2)
#define VS_64FC3 VS_MAKETYPE(VS_64F,3)
#define VS_64FC4 VS_MAKETYPE(VS_64F,4)
#define VS_64FC(n) VS_MAKETYPE(VS_64F,(n))

#define VS_AUTO_STEP  0x7fffffff
#define VS_WHOLE_ARR  vsSlice( 0, 0x3fffffff )

#define VS_MAT_CN_MASK          ((VS_CN_MAX - 1) << VS_CN_SHIFT)
#define VS_MAT_CN(flags)        ((((flags) & VS_MAT_CN_MASK) >> VS_CN_SHIFT) + 1)
#define VS_MAT_TYPE_MASK        (VS_DEPTH_MAX*VS_CN_MAX - 1)
#define VS_MAT_TYPE(flags)      ((flags) & VS_MAT_TYPE_MASK)
#define VS_MAT_CONT_FLAG_SHIFT  14
#define VS_MAT_CONT_FLAG        (1 << VS_MAT_CONT_FLAG_SHIFT)
#define VS_IS_MAT_CONT(flags)   ((flags) & VS_MAT_CONT_FLAG)
#define VS_IS_CONT_MAT          VS_IS_MAT_CONT
#define VS_SUBMAT_FLAG_SHIFT    15
#define VS_SUBMAT_FLAG          (1 << VS_SUBMAT_FLAG_SHIFT)
#define VS_IS_SUBMAT(flags)     ((flags) & VS_MAT_SUBMAT_FLAG)

#define VS_MAGIC_MASK       0xFFFF0000
#define VS_MAT_MAGIC_VAL    0x42420000
#define VS_TYPE_NAME_MAT    "opencv-matrix"

typedef struct VsMat
{
    int type;
    int step;

    /* for internal use only */
    int* refcount;
    int hdr_refcount;

    union
    {
        uchar* ptr;
        short* s;
        int* i;
        float* fl;
        double* db;
    } data;

#ifdef __cplusplus
    union
    {
        int rows;
        int height;
    };

    union
    {
        int cols;
        int width;
    };
#else
    int rows;
    int cols;
#endif

}
VsMat;



#define VS_IS_MAT_HDR(mat) \
    ((mat) != NULL && \
    (((const VsMat*)(mat))->type & VS_MAGIC_MASK) == VS_MAT_MAGIC_VAL && \
    ((const VsMat*)(mat))->cols > 0 && ((const VsMat*)(mat))->rows > 0)

#define VS_IS_MAT_HDR_Z(mat) \
    ((mat) != NULL && \
    (((const VsMat*)(mat))->type & VS_MAGIC_MASK) == VS_MAT_MAGIC_VAL && \
    ((const VsMat*)(mat))->cols >= 0 && ((const VsMat*)(mat))->rows >= 0)

#define VS_IS_MAT(mat) \
    (VS_IS_MAT_HDR(mat) && ((const VsMat*)(mat))->data.ptr != NULL)

#define VS_IS_MASK_ARR(mat) \
    (((mat)->type & (VS_MAT_TYPE_MASK & ~VS_8SC1)) == 0)

#define VS_ARE_TYPES_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & VS_MAT_TYPE_MASK) == 0)

#define VS_ARE_CNS_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & VS_MAT_CN_MASK) == 0)

#define VS_ARE_DEPTHS_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & VS_MAT_DEPTH_MASK) == 0)

#define VS_ARE_SIZES_EQ(mat1, mat2) \
    ((mat1)->rows == (mat2)->rows && (mat1)->cols == (mat2)->cols)

#define VS_IS_MAT_CONST(mat)  \
    (((mat)->rows|(mat)->cols) == 1)

/* Size of each channel item,
   0x124489 = 1000 0100 0100 0010 0010 0001 0001 ~ array of sizeof(arr_type_elem) */
#define VS_ELEM_SIZE1(type) \
    ((((sizeof(size_t)<<28)|0x8442211) >> VS_MAT_DEPTH(type)*4) & 15)

/* 0x3a50 = 11 10 10 01 01 00 00 ~ array of log2(sizeof(arr_type_elem)) */
#define VS_ELEM_SIZE(type) \
    (VS_MAT_CN(type) << ((((sizeof(size_t)/4+1)*16384|0x3a50) >> VS_MAT_DEPTH(type)*2) & 3))

#define IPL2VS_DEPTH(depth) \
    ((((VS_8U)+(VS_16U<<4)+(VS_32F<<8)+(VS_64F<<16)+(VS_8S<<20)+ \
    (VS_16S<<24)+(VS_32S<<28)) >> ((((depth) & 0xF0) >> 2) + \
    (((depth) & VS_DEPTH_SIGN) ? 20 : 0))) & 15)

/* Inline constructor. No data is allocated internally!!!
 * (Use together with vsCreateData, or use vsCreateMat instead to
 * get a matrix with allocated data):
 */
VS_INLINE VsMat vsMat( int rows, int cols, int type, void* data VS_DEFAULT(NULL))
{
    VsMat m;

    assert( (unsigned)VS_MAT_DEPTH(type) <= VS_64F );
    type = VS_MAT_TYPE(type);
    m.type = VS_MAT_MAGIC_VAL | VS_MAT_CONT_FLAG | type;
    m.cols = cols;
    m.rows = rows;
    m.step = m.cols*VS_ELEM_SIZE(type);
    m.data.ptr = (uchar*)data;
    m.refcount = NULL;
    m.hdr_refcount = 0;

    return m;
}


#define VS_MAT_ELEM_PTR_FAST( mat, row, col, pix_size )  \
    (assert( (unsigned)(row) < (unsigned)(mat).rows &&   \
             (unsigned)(col) < (unsigned)(mat).cols ),   \
     (mat).data.ptr + (size_t)(mat).step*(row) + (pix_size)*(col))

#define VS_MAT_ELEM_PTR( mat, row, col )                 \
    VS_MAT_ELEM_PTR_FAST( mat, row, col, VS_ELEM_SIZE((mat).type) )

#define VS_MAT_ELEM( mat, elemtype, row, col )           \
    (*(elemtype*)VS_MAT_ELEM_PTR_FAST( mat, row, col, sizeof(elemtype)))


VS_INLINE  double  vsmGet( const VsMat* mat, int row, int col )
{
    int type;

    type = VS_MAT_TYPE(mat->type);
    assert( (unsigned)row < (unsigned)mat->rows &&
            (unsigned)col < (unsigned)mat->cols );

    if( type == VS_32FC1 )
        return ((float*)(void*)(mat->data.ptr + (size_t)mat->step*row))[col];
    else
    {
        assert( type == VS_64FC1 );
        return ((double*)(void*)(mat->data.ptr + (size_t)mat->step*row))[col];
    }
}


VS_INLINE  void  vsmSet( VsMat* mat, int row, int col, double value )
{
    int type;
    type = VS_MAT_TYPE(mat->type);
    assert( (unsigned)row < (unsigned)mat->rows &&
            (unsigned)col < (unsigned)mat->cols );

    if( type == VS_32FC1 )
        ((float*)(void*)(mat->data.ptr + (size_t)mat->step*row))[col] = (float)value;
    else
    {
        assert( type == VS_64FC1 );
        ((double*)(void*)(mat->data.ptr + (size_t)mat->step*row))[col] = value;
    }
}


VS_INLINE int vsIplDepth( int type )
{
    int depth = VS_MAT_DEPTH(type);
    return VS_ELEM_SIZE1(depth)*8 | (depth == VS_8S || depth == VS_16S ||
           depth == VS_32S ? VS_DEPTH_SIGN : 0);
}


/****************************************************************************************\
*                       Multi-dimensional dense array (VsMatND)                          *
\****************************************************************************************/

#define VS_MATND_MAGIC_VAL    0x42430000
#define VS_TYPE_NAME_MATND    "opencv-nd-matrix"

#define VS_MAX_DIM            32
#define VS_MAX_DIM_HEAP       1024

/****************************************************************************************\
*                                         Histogram                                      *
\****************************************************************************************/

typedef int VsHistType;

#define VS_HIST_MAGIC_VAL     0x42450000
#define VS_HIST_UNIFORM_FLAG  (1 << 10)

/* indicates whether bin ranges are set already or not */
#define VS_HIST_RANGES_FLAG   (1 << 11)

#define VS_HIST_ARRAY         0
#define VS_HIST_SPARSE        1
#define VS_HIST_TREE          VS_HIST_SPARSE

/* should be used as a parameter only,
   it turns to VS_HIST_UNIFORM_FLAG of hist->type */
#define VS_HIST_UNIFORM       1



#define VS_IS_HIST( hist ) \
    ((hist) != NULL  && \
     (((VsHistogram*)(hist))->type & VS_MAGIC_MASK) == VS_HIST_MAGIC_VAL && \
     (hist)->bins != NULL)

#define VS_IS_UNIFORM_HIST( hist ) \
    (((hist)->type & VS_HIST_UNIFORM_FLAG) != 0)

#define VS_IS_SPARSE_HIST( hist ) \
    VS_IS_SPARSE_MAT((hist)->bins)

#define VS_HIST_HAS_RANGES( hist ) \
    (((hist)->type & VS_HIST_RANGES_FLAG) != 0)

/****************************************************************************************\
*                      Other supplementary data type definitions                         *
\****************************************************************************************/

/*************************************** VsRect *****************************************/

typedef struct VsRect
{
    int x;
    int y;
    int width;
    int height;
}
VsRect;

VS_INLINE  VsRect  vsRect( int x, int y, int width, int height )
{
    VsRect r;

    r.x = x;
    r.y = y;
    r.width = width;
    r.height = height;

    return r;
}


VS_INLINE  VsROI  vsRectToROI( VsRect rect, int coi )
{
    VsROI roi;
    roi.xOffset = rect.x;
    roi.yOffset = rect.y;
    roi.width = rect.width;
    roi.height = rect.height;
    roi.coi = coi;

    return roi;
}


VS_INLINE  VsRect  vsROIToRect( VsROI roi )
{
    return vsRect( roi.xOffset, roi.yOffset, roi.width, roi.height );
}

/* VsRect32f*/

typedef struct VsRect32f
{
	float x;
	float y;
	float width;
	float height;
}
VsRect32f;

VS_INLINE  VsRect32f  vsRect32f(float x, float y, float width, float height)
{
	VsRect32f r;

	r.x = x;
	r.y = y;
	r.width = width;
	r.height = height;

	return r;
}


VS_INLINE  VsROI  vsRect32fToROI(VsRect32f rect, float coi)
{
	VsROI roi;
	roi.xOffset = static_cast<int>(rect.x);
	roi.yOffset = static_cast<int>(rect.y);
	roi.width = static_cast<int>(rect.width);
	roi.height = static_cast<int>(rect.height);
	roi.coi = static_cast<int>(coi);

	return roi;
}


VS_INLINE  VsRect32f  vsROIToRect32f(VsROI roi)
{
	return vsRect32f(static_cast<float>(roi.xOffset), static_cast<float>(roi.yOffset), static_cast<float>(roi.width), static_cast<float>(roi.height));
}

/*********************************** VsTermCriteria *************************************/

#define VS_TERMCRIT_ITER    1
#define VS_TERMCRIT_NUMBER  VS_TERMCRIT_ITER
#define VS_TERMCRIT_EPS     2

typedef struct VsTermCriteria
{
    int    type;  /* may be combination of
                     VS_TERMCRIT_ITER
                     VS_TERMCRIT_EPS */
    int    max_iter;
    double epsilon;
}
VsTermCriteria;

VS_INLINE  VsTermCriteria  vsTermCriteria( int type, int max_iter, double epsilon )
{
    VsTermCriteria t;

    t.type = type;
    t.max_iter = max_iter;
    t.epsilon = (float)epsilon;

    return t;
}


/******************************* VsPoint and variants ***********************************/

typedef struct VsPoint
{
    int x;
    int y;
}
VsPoint;


VS_INLINE  VsPoint  vsPoint( int x, int y )
{
    VsPoint p;

    p.x = x;
    p.y = y;

    return p;
}


typedef struct VsPoint2D32f
{
    float x;
    float y;
}
VsPoint2D32f;


VS_INLINE  VsPoint2D32f  vsPoint2D32f( double x, double y )
{
    VsPoint2D32f p;

    p.x = (float)x;
    p.y = (float)y;

    return p;
}


VS_INLINE  VsPoint2D32f  vsPointTo32f( VsPoint point )
{
    return vsPoint2D32f( (float)point.x, (float)point.y );
}


VS_INLINE  VsPoint  vsPointFrom32f( VsPoint2D32f point )
{
    VsPoint ipt;
    ipt.x = vsRound(point.x);
    ipt.y = vsRound(point.y);

    return ipt;
}


typedef struct VsPoint3D32f
{
    float x;
    float y;
    float z;
}
VsPoint3D32f;


VS_INLINE  VsPoint3D32f  vsPoint3D32f( double x, double y, double z )
{
    VsPoint3D32f p;

    p.x = (float)x;
    p.y = (float)y;
    p.z = (float)z;

    return p;
}


typedef struct VsPoint2D64f
{
    double x;
    double y;
}
VsPoint2D64f;


VS_INLINE  VsPoint2D64f  vsPoint2D64f( double x, double y )
{
    VsPoint2D64f p;

    p.x = x;
    p.y = y;

    return p;
}


typedef struct VsPoint3D64f
{
    double x;
    double y;
    double z;
}
VsPoint3D64f;


VS_INLINE  VsPoint3D64f  vsPoint3D64f( double x, double y, double z )
{
    VsPoint3D64f p;

    p.x = x;
    p.y = y;
    p.z = z;

    return p;
}


/******************************** VsSize's & CvBox **************************************/

typedef struct VsSize
{
    int width;
    int height;
}
VsSize;

VS_INLINE  VsSize  vsSize( int width, int height )
{
    VsSize s;

    s.width = width;
    s.height = height;

    return s;
}

typedef struct VsSize2D32f
{
    float width;
    float height;
}
VsSize2D32f;


VS_INLINE  VsSize2D32f  vsSize2D32f( double width, double height )
{
    VsSize2D32f s;

    s.width = (float)width;
    s.height = (float)height;

    return s;
}

typedef struct VsBox2D
{
    VsPoint2D32f center;  /* Center of the box.                          */
    VsSize2D32f  size;    /* Box width and length.                       */
    float angle;          /* Angle between the horizontal axis           */
                          /* and the first side (i.e. length) in degrees */
}
VsBox2D;


/* Line iterator state: */
typedef struct VsLineIterator
{
    /* Pointer to the current point: */
    uchar* ptr;

    /* Bresenham algorithm state: */
    int  err;
    int  plus_delta;
    int  minus_delta;
    int  plus_step;
    int  minus_step;
}
VsLineIterator;



/************************************* VsSlice ******************************************/

typedef struct VsSlice
{
    int  start_index, end_index;
}
VsSlice;

VS_INLINE  VsSlice  vsSlice( int start, int end )
{
    VsSlice slice;
    slice.start_index = start;
    slice.end_index = end;

    return slice;
}

#define VS_WHOLE_SEQ_END_INDEX 0x3fffffff
#define VS_WHOLE_SEQ  vsSlice(0, VS_WHOLE_SEQ_END_INDEX)


/************************************* VsScalar *****************************************/

typedef struct VsScalar
{
    double val[4];
}
VsScalar;

VS_INLINE  VsScalar  vsScalar( double val0, double val1 VS_DEFAULT(0),
                               double val2 VS_DEFAULT(0), double val3 VS_DEFAULT(0))
{
    VsScalar scalar;
    scalar.val[0] = val0; scalar.val[1] = val1;
    scalar.val[2] = val2; scalar.val[3] = val3;
    return scalar;
}


VS_INLINE  VsScalar  vsRealScalar( double val0 )
{
    VsScalar scalar;
    scalar.val[0] = val0;
    scalar.val[1] = scalar.val[2] = scalar.val[3] = 0;
    return scalar;
}

VS_INLINE  VsScalar  vsScalarAll( double val0123 )
{
    VsScalar scalar;
    scalar.val[0] = val0123;
    scalar.val[1] = val0123;
    scalar.val[2] = val0123;
    scalar.val[3] = val0123;
    return scalar;
}

/****************************************************************************************\
*                                   Dynamic Data structures                              *
\****************************************************************************************/

/******************************** Memory storage ****************************************/

typedef struct VsMemBlock
{
    struct VsMemBlock*  prev;
    struct VsMemBlock*  next;
}
VsMemBlock;

#define VS_STORAGE_MAGIC_VAL    0x42890000

typedef struct VsMemStorage
{
    int signature;
    VsMemBlock* bottom;           /* First allocated block.                   */
    VsMemBlock* top;              /* Current memory block - top of the stack. */
    struct  VsMemStorage* parent; /* We get new blocks from parent as needed. */
    int block_size;               /* Block size.                              */
    int free_space;               /* Remaining free space in current block.   */
}
VsMemStorage;

#define VS_IS_STORAGE(storage)  \
    ((storage) != NULL &&       \
    (((VsMemStorage*)(storage))->signature & VS_MAGIC_MASK) == VS_STORAGE_MAGIC_VAL)


typedef struct VsMemStoragePos
{
    VsMemBlock* top;
    int free_space;
}
VsMemStoragePos;


/*********************************** Sequence *******************************************/

typedef struct VsSeqBlock
{
    struct VsSeqBlock*  prev; /* Previous sequence block.                   */
    struct VsSeqBlock*  next; /* Next sequence block.                       */
  int    start_index;         /* Index of the first element in the block +  */
                              /* sequence->first->start_index.              */
    int    count;             /* Number of elements in the block.           */
    schar* data;              /* Pointer to the first element of the block. */
}
VsSeqBlock;


#define VS_TREE_NODE_FIELDS(node_type)                               \
    int       flags;             /* Miscellaneous flags.     */      \
    int       header_size;       /* Size of sequence header. */      \
    struct    node_type* h_prev; /* Previous sequence.       */      \
    struct    node_type* h_next; /* Next sequence.           */      \
    struct    node_type* v_prev; /* 2nd previous sequence.   */      \
    struct    node_type* v_next  /* 2nd next sequence.       */

/*
   Read/Write sequence.
   Elements can be dynamically inserted to or deleted from the sequence.
*/
#define VS_SEQUENCE_FIELDS()                                              \
    VS_TREE_NODE_FIELDS(VsSeq);                                           \
    int       total;          /* Total number of elements.            */  \
    int       elem_size;      /* Size of sequence element in bytes.   */  \
    schar*    block_max;      /* Maximal bound of the last block.     */  \
    schar*    ptr;            /* Current write pointer.               */  \
    int       delta_elems;    /* Grow seq this many at a time.        */  \
    VsMemStorage* storage;    /* Where the seq is stored.             */  \
    VsSeqBlock* free_blocks;  /* Free blocks list.                    */  \
    VsSeqBlock* first;        /* Pointer to the first sequence block. */

#define VS_TYPE_NAME_SEQ             "vs-sequence"
#define VS_TYPE_NAME_SEQ_TREE        "vs-sequence-tree"

/*************************************** Set ********************************************/
/*
  Set.
  Order is not preserved. There can be gaps between sequence elements.
  After the element has been inserted it stays in the same place all the time.
  The MSB(most-significant or sign bit) of the first field (flags) is 0 iff the element exists.
*/
#define VS_SET_ELEM_FIELDS(elem_type)   \
    int  flags;                         \
    struct elem_type* next_free;

typedef struct VsSetElem
{
    VS_SET_ELEM_FIELDS(VsSetElem)
}
VsSetElem;

#define VS_SET_FIELDS()      \
    VS_SEQUENCE_FIELDS()     \
    VsSetElem* free_elems;   \
    int active_count;

typedef struct VsSet
{
    VS_SET_FIELDS()
}
VsSet;


#define VS_SET_ELEM_IDX_MASK   ((1 << 26) - 1)
#define VS_SET_ELEM_FREE_FLAG  (1 << (sizeof(int)*8-1))

/* Checks whether the element pointed by ptr belongs to a set or not */
#define VS_IS_SET_ELEM( ptr )  (((VsSetElem*)(ptr))->flags >= 0)

/****************************************************************************************\
*                                    Sequence types                                      *
\****************************************************************************************/

#define VS_SET_MAGIC_VAL             0x42980000
#define VS_IS_SET(set) \
    ((set) != NULL && (((VsSeq*)(set))->flags & VS_MAGIC_MASK) == VS_SET_MAGIC_VAL)

/****************************************************************************************\
*             Data structures for persistence (a.k.a serialization) functionality        *
\****************************************************************************************/


#define VS_NODE_NONE        0
#define VS_NODE_INT         1
#define VS_NODE_INTEGER     VS_NODE_INT
#define VS_NODE_REAL        2
#define VS_NODE_FLOAT       VS_NODE_REAL
#define VS_NODE_STR         3
#define VS_NODE_STRING      VS_NODE_STR
#define VS_NODE_REF         4 /* not used */
#define VS_NODE_SEQ         5
#define VS_NODE_MAP         6
#define VS_NODE_TYPE_MASK   7

#define VS_NODE_TYPE(flags)  ((flags) & VS_NODE_TYPE_MASK)

/* file node flags */
#define VS_NODE_FLOW        8 /* Used only for writing structures in YAML format. */
#define VS_NODE_USER        16
#define VS_NODE_EMPTY       32
#define VS_NODE_NAMED       64

#define VS_NODE_IS_INT(flags)        (VS_NODE_TYPE(flags) == VS_NODE_INT)
#define VS_NODE_IS_REAL(flags)       (VS_NODE_TYPE(flags) == VS_NODE_REAL)
#define VS_NODE_IS_STRING(flags)     (VS_NODE_TYPE(flags) == VS_NODE_STRING)
#define VS_NODE_IS_SEQ(flags)        (VS_NODE_TYPE(flags) == VS_NODE_SEQ)
#define VS_NODE_IS_MAP(flags)        (VS_NODE_TYPE(flags) == VS_NODE_MAP)
#define VS_NODE_IS_COLLECTION(flags) (VS_NODE_TYPE(flags) >= VS_NODE_SEQ)
#define VS_NODE_IS_FLOW(flags)       (((flags) & VS_NODE_FLOW) != 0)
#define VS_NODE_IS_EMPTY(flags)      (((flags) & VS_NODE_EMPTY) != 0)
#define VS_NODE_IS_USER(flags)       (((flags) & VS_NODE_USER) != 0)
#define VS_NODE_HAS_NAME(flags)      (((flags) & VS_NODE_NAMED) != 0)

#define VS_NODE_SEQ_SIMPLE 256
#define VS_NODE_SEQ_IS_SIMPLE(seq) (((seq)->flags & VS_NODE_SEQ_SIMPLE) != 0)

typedef struct VsString
{
    int len;
    char* ptr;
}
VsString;

#endif /*__VS_CORE_TYPES_H__*/

/* End of file. */
