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

#ifndef __VS_IMGPROC_TYPES_C_H__
#define __VS_IMGPROC_TYPES_C_H__

#include "vs_core_core_c.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Image smooth methods */
enum
{
    VS_BLUR_NO_SCALE =0,
    VS_BLUR  =1,
    VS_GAUSSIAN  =2,
    VS_MEDIAN =3,
    VS_BILATERAL =4
};

/* Filters used in pyramid decomposition */
enum
{
    VS_GAUSSIAN_5x5 = 7
};

/* Special filters */
enum
{
    VS_SCHARR =-1,
    VS_MAX_SOBEL_KSIZE =7
};

/* Constants for color conversion */
enum
{
    VS_BGR2BGRA    =0,
    VS_RGB2RGBA    =VS_BGR2BGRA,

    VS_BGRA2BGR    =1,
    VS_RGBA2RGB    =VS_BGRA2BGR,

    VS_BGR2RGBA    =2,
    VS_RGB2BGRA    =VS_BGR2RGBA,

    VS_RGBA2BGR    =3,
    VS_BGRA2RGB    =VS_RGBA2BGR,

    VS_BGR2RGB     =4,
    VS_RGB2BGR     =VS_BGR2RGB,

    VS_BGRA2RGBA   =5,
    VS_RGBA2BGRA   =VS_BGRA2RGBA,

    VS_BGR2GRAY    =6,
    VS_RGB2GRAY    =7,
    VS_GRAY2BGR    =8,
    VS_GRAY2RGB    =VS_GRAY2BGR,
    VS_GRAY2BGRA   =9,
    VS_GRAY2RGBA   =VS_GRAY2BGRA,
    VS_BGRA2GRAY   =10,
    VS_RGBA2GRAY   =11,

    VS_BGR2BGR565  =12,
    VS_RGB2BGR565  =13,
    VS_BGR5652BGR  =14,
    VS_BGR5652RGB  =15,
    VS_BGRA2BGR565 =16,
    VS_RGBA2BGR565 =17,
    VS_BGR5652BGRA =18,
    VS_BGR5652RGBA =19,

    VS_GRAY2BGR565 =20,
    VS_BGR5652GRAY =21,

    VS_BGR2BGR555  =22,
    VS_RGB2BGR555  =23,
    VS_BGR5552BGR  =24,
    VS_BGR5552RGB  =25,
    VS_BGRA2BGR555 =26,
    VS_RGBA2BGR555 =27,
    VS_BGR5552BGRA =28,
    VS_BGR5552RGBA =29,

    VS_GRAY2BGR555 =30,
    VS_BGR5552GRAY =31,

    VS_BGR2XYZ     =32,
    VS_RGB2XYZ     =33,
    VS_XYZ2BGR     =34,
    VS_XYZ2RGB     =35,

    VS_BGR2YCrCb   =36,
    VS_RGB2YCrCb   =37,
    VS_YCrCb2BGR   =38,
    VS_YCrCb2RGB   =39,

    VS_BGR2HSV     =40,
    VS_RGB2HSV     =41,

    VS_BGR2Lab     =44,
    VS_RGB2Lab     =45,

    VS_BayerBG2BGR =46,
    VS_BayerGB2BGR =47,
    VS_BayerRG2BGR =48,
    VS_BayerGR2BGR =49,

    VS_BayerBG2RGB =VS_BayerRG2BGR,
    VS_BayerGB2RGB =VS_BayerGR2BGR,
    VS_BayerRG2RGB =VS_BayerBG2BGR,
    VS_BayerGR2RGB =VS_BayerGB2BGR,

    VS_BGR2Luv     =50,
    VS_RGB2Luv     =51,
    VS_BGR2HLS     =52,
    VS_RGB2HLS     =53,

    VS_HSV2BGR     =54,
    VS_HSV2RGB     =55,

    VS_Lab2BGR     =56,
    VS_Lab2RGB     =57,
    VS_Luv2BGR     =58,
    VS_Luv2RGB     =59,
    VS_HLS2BGR     =60,
    VS_HLS2RGB     =61,

    VS_BayerBG2BGR_VNG =62,
    VS_BayerGB2BGR_VNG =63,
    VS_BayerRG2BGR_VNG =64,
    VS_BayerGR2BGR_VNG =65,

    VS_BayerBG2RGB_VNG =VS_BayerRG2BGR_VNG,
    VS_BayerGB2RGB_VNG =VS_BayerGR2BGR_VNG,
    VS_BayerRG2RGB_VNG =VS_BayerBG2BGR_VNG,
    VS_BayerGR2RGB_VNG =VS_BayerGB2BGR_VNG,

    VS_BGR2HSV_FULL = 66,
    VS_RGB2HSV_FULL = 67,
    VS_BGR2HLS_FULL = 68,
    VS_RGB2HLS_FULL = 69,

    VS_HSV2BGR_FULL = 70,
    VS_HSV2RGB_FULL = 71,
    VS_HLS2BGR_FULL = 72,
    VS_HLS2RGB_FULL = 73,

    VS_LBGR2Lab     = 74,
    VS_LRGB2Lab     = 75,
    VS_LBGR2Luv     = 76,
    VS_LRGB2Luv     = 77,

    VS_Lab2LBGR     = 78,
    VS_Lab2LRGB     = 79,
    VS_Luv2LBGR     = 80,
    VS_Luv2LRGB     = 81,

    VS_BGR2YUV      = 82,
    VS_RGB2YUV      = 83,
    VS_YUV2BGR      = 84,
    VS_YUV2RGB      = 85,

    VS_BayerBG2GRAY = 86,
    VS_BayerGB2GRAY = 87,
    VS_BayerRG2GRAY = 88,
    VS_BayerGR2GRAY = 89,

    //YUV 4:2:0 formats family
    VS_YUV2RGB_NV12 = 90,
    VS_YUV2BGR_NV12 = 91,
    VS_YUV2RGB_NV21 = 92,
    VS_YUV2BGR_NV21 = 93,
    VS_YUV420sp2RGB = VS_YUV2RGB_NV21,
    VS_YUV420sp2BGR = VS_YUV2BGR_NV21,

    VS_YUV2RGBA_NV12 = 94,
    VS_YUV2BGRA_NV12 = 95,
    VS_YUV2RGBA_NV21 = 96,
    VS_YUV2BGRA_NV21 = 97,
    VS_YUV420sp2RGBA = VS_YUV2RGBA_NV21,
    VS_YUV420sp2BGRA = VS_YUV2BGRA_NV21,

    VS_YUV2RGB_YV12 = 98,
    VS_YUV2BGR_YV12 = 99,
    VS_YUV2RGB_IYUV = 100,
    VS_YUV2BGR_IYUV = 101,
    VS_YUV2RGB_I420 = VS_YUV2RGB_IYUV,
    VS_YUV2BGR_I420 = VS_YUV2BGR_IYUV,
    VS_YUV420p2RGB = VS_YUV2RGB_YV12,
    VS_YUV420p2BGR = VS_YUV2BGR_YV12,

    VS_YUV2RGBA_YV12 = 102,
    VS_YUV2BGRA_YV12 = 103,
    VS_YUV2RGBA_IYUV = 104,
    VS_YUV2BGRA_IYUV = 105,
    VS_YUV2RGBA_I420 = VS_YUV2RGBA_IYUV,
    VS_YUV2BGRA_I420 = VS_YUV2BGRA_IYUV,
    VS_YUV420p2RGBA = VS_YUV2RGBA_YV12,
    VS_YUV420p2BGRA = VS_YUV2BGRA_YV12,

    VS_YUV2GRAY_420 = 106,
    VS_YUV2GRAY_NV21 = VS_YUV2GRAY_420,
    VS_YUV2GRAY_NV12 = VS_YUV2GRAY_420,
    VS_YUV2GRAY_YV12 = VS_YUV2GRAY_420,
    VS_YUV2GRAY_IYUV = VS_YUV2GRAY_420,
    VS_YUV2GRAY_I420 = VS_YUV2GRAY_420,
    VS_YUV420sp2GRAY = VS_YUV2GRAY_420,
    VS_YUV420p2GRAY = VS_YUV2GRAY_420,

    //YUV 4:2:2 formats family
    VS_YUV2RGB_UYVY = 107,
    VS_YUV2BGR_UYVY = 108,
    //VS_YUV2RGB_VYUY = 109,
    //VS_YUV2BGR_VYUY = 110,
    VS_YUV2RGB_Y422 = VS_YUV2RGB_UYVY,
    VS_YUV2BGR_Y422 = VS_YUV2BGR_UYVY,
    VS_YUV2RGB_UYNV = VS_YUV2RGB_UYVY,
    VS_YUV2BGR_UYNV = VS_YUV2BGR_UYVY,

    VS_YUV2RGBA_UYVY = 111,
    VS_YUV2BGRA_UYVY = 112,
    //VS_YUV2RGBA_VYUY = 113,
    //VS_YUV2BGRA_VYUY = 114,
    VS_YUV2RGBA_Y422 = VS_YUV2RGBA_UYVY,
    VS_YUV2BGRA_Y422 = VS_YUV2BGRA_UYVY,
    VS_YUV2RGBA_UYNV = VS_YUV2RGBA_UYVY,
    VS_YUV2BGRA_UYNV = VS_YUV2BGRA_UYVY,

    VS_YUV2RGB_YUY2 = 115,
    VS_YUV2BGR_YUY2 = 116,
    VS_YUV2RGB_YVYU = 117,
    VS_YUV2BGR_YVYU = 118,
    VS_YUV2RGB_YUYV = VS_YUV2RGB_YUY2,
    VS_YUV2BGR_YUYV = VS_YUV2BGR_YUY2,
    VS_YUV2RGB_YUNV = VS_YUV2RGB_YUY2,
    VS_YUV2BGR_YUNV = VS_YUV2BGR_YUY2,

    VS_YUV2RGBA_YUY2 = 119,
    VS_YUV2BGRA_YUY2 = 120,
    VS_YUV2RGBA_YVYU = 121,
    VS_YUV2BGRA_YVYU = 122,
    VS_YUV2RGBA_YUYV = VS_YUV2RGBA_YUY2,
    VS_YUV2BGRA_YUYV = VS_YUV2BGRA_YUY2,
    VS_YUV2RGBA_YUNV = VS_YUV2RGBA_YUY2,
    VS_YUV2BGRA_YUNV = VS_YUV2BGRA_YUY2,

    VS_YUV2GRAY_UYVY = 123,
    VS_YUV2GRAY_YUY2 = 124,
    //VS_YUV2GRAY_VYUY = VS_YUV2GRAY_UYVY,
    VS_YUV2GRAY_Y422 = VS_YUV2GRAY_UYVY,
    VS_YUV2GRAY_UYNV = VS_YUV2GRAY_UYVY,
    VS_YUV2GRAY_YVYU = VS_YUV2GRAY_YUY2,
    VS_YUV2GRAY_YUYV = VS_YUV2GRAY_YUY2,
    VS_YUV2GRAY_YUNV = VS_YUV2GRAY_YUY2,

    // alpha premultiplication
    VS_RGBA2mRGBA = 125,
    VS_mRGBA2RGBA = 126,

    VS_RGB2YUV_I420 = 127,
    VS_BGR2YUV_I420 = 128,
    VS_RGB2YUV_IYUV = VS_RGB2YUV_I420,
    VS_BGR2YUV_IYUV = VS_BGR2YUV_I420,

    VS_RGBA2YUV_I420 = 129,
    VS_BGRA2YUV_I420 = 130,
    VS_RGBA2YUV_IYUV = VS_RGBA2YUV_I420,
    VS_BGRA2YUV_IYUV = VS_BGRA2YUV_I420,
    VS_RGB2YUV_YV12  = 131,
    VS_BGR2YUV_YV12  = 132,
    VS_RGBA2YUV_YV12 = 133,
    VS_BGRA2YUV_YV12 = 134,

    VS_COLORCVT_MAX  = 135
};


/* Sub-pixel interpolation methods */
enum
{
    VS_INTER_NN        =0,
    VS_INTER_LINEAR    =1,
    VS_INTER_CUBIC     =2,
    VS_INTER_AREA      =3,
    VS_INTER_LANCZOS4  =4
};

/* ... and other image warping flags */
enum
{
    VS_WARP_FILL_OUTLIERS =8,
    VS_WARP_INVERSE_MAP  =16
};

/* Shapes of a structuring element for morphological operations */
enum
{
    VS_SHAPE_RECT      =0,
    VS_SHAPE_CROSS     =1,
    VS_SHAPE_ELLIPSE   =2,
    VS_SHAPE_CUSTOM    =100
};

/* Morphological operations */
enum
{
    VS_MOP_ERODE        =0,
    VS_MOP_DILATE       =1,
    VS_MOP_OPEN         =2,
    VS_MOP_CLOSE        =3,
    VS_MOP_GRADIENT     =4,
    VS_MOP_TOPHAT       =5,
    VS_MOP_BLACKHAT     =6
};

/* Template matching methods */
enum
{
    VS_TM_SQDIFF        =0,
    VS_TM_SQDIFF_NORMED =1,
    VS_TM_CCORR         =2,
    VS_TM_CCORR_NORMED  =3,
    VS_TM_CCOEFF        =4,
    VS_TM_CCOEFF_NORMED =5
};

typedef float (VS_CDECL * VsDistanceFunction)( const float* a, const float* b, void* user_param );

/* Contour retrieval modes */
enum
{
    VS_RETR_EXTERNAL=0,
    VS_RETR_LIST=1,
    VS_RETR_CCOMP=2,
    VS_RETR_TREE=3,
    VS_RETR_FLOODFILL=4
};

/* Shape orientation */
enum
{
    VS_CLOCKWISE         =1,
    VS_COUNTER_CLOCKWISE =2
};

/* Histogram comparison methods */
enum
{
    VS_COMP_CORREL        =0,
    VS_COMP_CHISQR        =1,
    VS_COMP_INTERSECT     =2,
    VS_COMP_BHATTACHARYYA =3,
    VS_COMP_HELLINGER     =VS_COMP_BHATTACHARYYA
};

/* Mask size for distance transform */
enum
{
    VS_DIST_MASK_3   =3,
    VS_DIST_MASK_5   =5,
    VS_DIST_MASK_PRECISE =0
};

/* Content of output label array: connected components or pixels */
enum
{
  VS_DIST_LABEL_CCOMP = 0,
  VS_DIST_LABEL_PIXEL = 1
};

/* Distance types for Distance Transform and M-estimators */
enum
{
    VS_DIST_USER    =-1,  /* User defined distance */
    VS_DIST_L1      =1,   /* distance = |x1-x2| + |y1-y2| */
    VS_DIST_L2      =2,   /* the simple euclidean distance */
    VS_DIST_C       =3,   /* distance = max(|x1-x2|,|y1-y2|) */
    VS_DIST_L12     =4,   /* L1-L2 metric: distance = 2(sqrt(1+x*x/2) - 1)) */
    VS_DIST_FAIR    =5,   /* distance = c^2(|x|/c-log(1+|x|/c)), c = 1.3998 */
    VS_DIST_WELSCH  =6,   /* distance = c^2/2(1-exp(-(x/c)^2)), c = 2.9846 */
    VS_DIST_HUBER   =7    /* distance = |x|<c ? x^2/2 : c(|x|-c/2), c=1.345 */
};


/* Threshold types */
enum
{
    VS_THRESH_BINARY      =0,  /* value = value > threshold ? max_value : 0       */
    VS_THRESH_BINARY_INV  =1,  /* value = value > threshold ? 0 : max_value       */
    VS_THRESH_TRUNC       =2,  /* value = value > threshold ? threshold : value   */
    VS_THRESH_TOZERO      =3,  /* value = value > threshold ? value : 0           */
    VS_THRESH_TOZERO_INV  =4,  /* value = value > threshold ? 0 : value           */
    VS_THRESH_MASK        =7,
    VS_THRESH_OTSU        =8  /* use Otsu algorithm to choose the optimal threshold value;
                                 combine the flag with one of the above VS_THRESH_* values */
};

/* Adaptive threshold methods */
enum
{
    VS_ADAPTIVE_THRESH_MEAN_C  =0,
    VS_ADAPTIVE_THRESH_GAUSSIAN_C  =1
};

/* FloodFill flags */
enum
{
    VS_FLOODFILL_FIXED_RANGE =(1 << 16),
    VS_FLOODFILL_MASK_ONLY   =(1 << 17)
};


/* Canny edge detector flags */
enum
{
    VS_CANNY_L2_GRADIENT  =(1 << 31)
};

/* Variants of a Hough transform */
enum
{
    VS_HOUGH_STANDARD =0,
    VS_HOUGH_PROBABILISTIC =1,
    VS_HOUGH_MULTI_SCALE =2,
    VS_HOUGH_GRADIENT =3
};


/* Fast search data structures  */
struct CvFeatureTree;
struct CvLSH;
struct CvLSHOperations;

#ifdef __cplusplus
}
#endif

#endif
