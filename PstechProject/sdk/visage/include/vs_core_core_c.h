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


#ifndef __VS_CORE_C_H__
#define __VS_CORE_C_H__

#include "vs_core_types_c.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************\
*          Array allocation, deallocation, initialization and access to elements         *
\****************************************************************************************/

/* <malloc> wrapper.
   If there is no enough memory, the function
   (as well as other OpenCV functions that call vsAlloc)
   raises an error. */
CVAPI(void*)  vsAlloc( size_t size );

/* <free> wrapper.
   Here and further all the memory releasing functions
   (that all call vsFree) take double pointer in order to
   to clear pointer to the data after releasing it.
   Passing pointer to NULL pointer is Ok: nothing happens in this case
*/
CVAPI(void)   vsFree_( void* ptr );
#define vsFree(ptr) (vsFree_(*(ptr)), *(ptr)=0)

/* Allocates and initializes VsImage header */
VSAPI(VsImage*)  vsCreateImageHeader( VsSize size, int depth, int channels );

/* Inializes VsImage header */
CVAPI(VsImage*) vsInitImageHeader( VsImage* image, VsSize size, int depth,
                                   int channels, int origin VS_DEFAULT(0),
                                   int align VS_DEFAULT(4));

/* Creates VsImage (header and data) */
VSAPI(VsImage*)  vsCreateImage( VsSize size, int depth, int channels );

/* Releases (i.e. deallocates) VsImage header */
VSAPI(void)  vsReleaseImageHeader( VsImage** image );

/* Releases VsImage header and data */
VSAPI(void)  vsReleaseImage( VsImage** image );

/* Creates a copy of VsImage (widthStep may differ) */
VSAPI(VsImage*) vsCloneImage( const VsImage* image );

/* Sets a Channel Of Interest (only a few functions support COI) -
   use vsCopy to extract the selected channel and/or put it back */
CVAPI(void)  vsSetImageCOI( VsImage* image, int coi );

/* Retrieves image Channel Of Interest */
CVAPI(int)  vsGetImageCOI( const VsImage* image );

/* Sets image ROI (region of interest) (COI is not changed) */
VSAPI(void)  vsSetImageROI( VsImage* image, VsRect rect );

/* Resets image ROI and COI */
VSAPI(void)  vsResetImageROI( VsImage* image );

/* Retrieves image ROI */
CVAPI(VsRect) vsGetImageROI( const VsImage* image );

/* Allocates and initializes VsMat header */
VSAPI(VsMat*)  vsCreateMatHeader( int rows, int cols, int type );

#define VS_AUTOSTEP  0x7fffffff

/* Initializes VsMat header */
CVAPI(VsMat*) vsInitMatHeader( VsMat* mat, int rows, int cols,
                              int type, void* data VS_DEFAULT(NULL),
                              int step VS_DEFAULT(VS_AUTOSTEP) );

/* Allocates and initializes VsMat header and allocates data */
VSAPI(VsMat*)  vsCreateMat( int rows, int cols, int type );

/* Releases VsMat header and deallocates matrix data
   (reference counting is used for data) */
VSAPI(void)  vsReleaseMat( VsMat** mat );

/* Decrements VsMat data reference counter and deallocates the data if
   it reaches 0 */
VS_INLINE  void  vsDecRefData( VsArr* arr )
{
    if( VS_IS_MAT( arr ))
    {
        VsMat* mat = (VsMat*)arr;
        mat->data.ptr = NULL;
        if( mat->refcount != NULL && --*mat->refcount == 0 )
            vsFree( &mat->refcount );
        mat->refcount = NULL;
    }
}

/* Increments VsMat data reference counter */
VS_INLINE  int  vsIncRefData( VsArr* arr )
{
    int refcount = 0;
    if( VS_IS_MAT( arr ))
    {
        VsMat* mat = (VsMat*)arr;
        if( mat->refcount != NULL )
            refcount = ++*mat->refcount;
    }
    return refcount;
}


/* Creates an exact copy of the input matrix (except, may be, step value) */
VSAPI(VsMat*) vsCloneMat( const VsMat* mat );


/* Makes a new matrix from <rect> subrectangle of input array.
   No data is copied */
CVAPI(VsMat*) vsGetSubRect( const VsArr* arr, VsMat* submat, VsRect rect );
#define vsGetSubArr vsGetSubRect

/* low-level scalar <-> raw data conversion functions */
CVAPI(void) vsScalarToRawData( const VsScalar* scalar, void* data, int type,
                              int extend_to_12 VS_DEFAULT(0) );

CVAPI(void) vsRawDataToScalar( const void* data, int type, VsScalar* scalar );


/* ptr = &arr(idx0,idx1,...). All indexes are zero-based,
   the major dimensions go first (e.g. (y,x) for 2D, (z,y,x) for 3D */
CVAPI(uchar*) vsPtr1D( const VsArr* arr, int idx0, int* type VS_DEFAULT(NULL));
CVAPI(uchar*) vsPtr2D( const VsArr* arr, int idx0, int idx1, int* type VS_DEFAULT(NULL) );

/* value = arr(idx0,idx1,...) */
CVAPI(VsScalar) vsGet1D( const VsArr* arr, int idx0 );
VSAPI(VsScalar) vsGet2D( const VsArr* arr, int idx0, int idx1 );

/* arr(idx0,idx1,...) = value */
VSAPI(void) vsSet1D( VsArr* arr, int idx0, VsScalar value );
VSAPI(void) vsSet2D( VsArr* arr, int idx0, int idx1, VsScalar value );

/* Converts VsArr (VsImage or VsMat,...) to VsMat.
   If the last parameter is non-zero, function can
   convert multi(>2)-dimensional array to VsMat as long as
   the last array's dimension is continous. The resultant
   matrix will be have appropriate (a huge) number of rows */
VSAPI(VsMat*) vsGetMat( const VsArr* arr, VsMat* header,
                       int* coi VS_DEFAULT(NULL),
                       int allowND VS_DEFAULT(0));

/* Converts VsArr (VsImage or VsMat) to VsImage */
CVAPI(VsImage*) vsGetImage( const VsArr* arr, VsImage* image_header );

/* Repeats source 2d array several times in both horizontal and
   vertical direction to fill destination array */
VSAPI(void) vsRepeat( const VsArr* src, VsArr* dst );

/* Allocates array data */
CVAPI(void)  vsCreateData( VsArr* arr );

/* Releases array data */
CVAPI(void)  vsReleaseData( VsArr* arr );

/* Attaches user data to the array header. The step is reffered to
   the pre-last dimension. That is, all the planes of the array
   must be joint (w/o gaps) */
VSAPI(void)  vsSetData( VsArr* arr, void* data, int step );
#define vsSetImageData vsSetData

/* Retrieves raw data of VsMat, VsImage or VsMatND.
   In the latter case the function raises an error if
   the array can not be represented as a matrix */
CVAPI(void) vsGetRawData( const VsArr* arr, uchar** data,
                         int* step VS_DEFAULT(NULL),
                         VsSize* roi_size VS_DEFAULT(NULL));
#define vsGetImageRawData vsGetRawData

/* Returns width and height of array in elements */
CVAPI(VsSize) vsGetSize( const VsArr* arr );

/* Copies source array to destination array */
VSAPI(void)  vsCopy( const VsArr* src, VsArr* dst,
                     const VsArr* mask VS_DEFAULT(NULL) );
#define vsCopyImage( src, dst )  vsCopy( src, dst, 0 )

/* Sets all or "masked" elements of input array
   to the same value*/
CVAPI(void)  vsSet( VsArr* arr, VsScalar value,
                    const VsArr* mask VS_DEFAULT(NULL) );

/* Clears all the array elements (sets them to 0) */
VSAPI(void)  vsSetZero( VsArr* arr );
#define vsZero  vsSetZero

/* Performs linear transformation on every source array element:
   dst(x,y,c) = scale*src(x,y,c)+shift.
   Arbitrary combination of input and output array depths are allowed
   (number of channels must be the same), thus the function can be used
   for type conversion */
VSAPI(void)  vsConvertScale( const VsArr* src, VsArr* dst,
                             double scale VS_DEFAULT(1),
                             double shift VS_DEFAULT(0) );
#define vsCvtScale vsConvertScale
#define vsScale  vsConvertScale
#define vsConvert( src, dst )  vsConvertScale( (src), (dst), 1, 0 )


/* Performs linear transformation on every source array element,
   stores absolute value of the result:
   dst(x,y,c) = abs(scale*src(x,y,c)+shift).
   destination array must have 8u type.
   In other cases one may use vsConvertScale + vsAbsDiffS */
CVAPI(void)  vsConvertScaleAbs( const VsArr* src, VsArr* dst,
                                double scale VS_DEFAULT(1),
                                double shift VS_DEFAULT(0) );
#define vsCvtScaleAbs  vsConvertScaleAbs

/****************************************************************************************\
*                   Arithmetic, logic and comparison operations                          *
\****************************************************************************************/

/* dst(mask) = src1(mask) + src2(mask) */
CVAPI(void)  vsAdd( const VsArr* src1, const VsArr* src2, VsArr* dst,
                    const VsArr* mask VS_DEFAULT(NULL));

/* dst(mask) = src(mask) + value */
CVAPI(void)  vsAddS( const VsArr* src, VsScalar value, VsArr* dst,
                     const VsArr* mask VS_DEFAULT(NULL));

/* dst(mask) = src1(mask) - src2(mask) */
VSAPI(void)  vsSub( const VsArr* src1, const VsArr* src2, VsArr* dst,
                    const VsArr* mask VS_DEFAULT(NULL));

/* dst(mask) = src(mask) - value = src(mask) + (-value) */
VS_INLINE  void  vsSubS( const VsArr* src, VsScalar value, VsArr* dst,
                         const VsArr* mask VS_DEFAULT(NULL))
{
    vsAddS( src, vsScalar( -value.val[0], -value.val[1], -value.val[2], -value.val[3]),
            dst, mask );
}

/* dst(mask) = value - src(mask) */
CVAPI(void)  vsSubRS( const VsArr* src, VsScalar value, VsArr* dst,
                      const VsArr* mask VS_DEFAULT(NULL));

/* dst(idx) = src1(idx) * src2(idx) * scale
   (scaled element-wise multiplication of 2 arrays) */
CVAPI(void)  vsMul( const VsArr* src1, const VsArr* src2,
                    VsArr* dst, double scale VS_DEFAULT(1) );

/* element-wise division/inversion with scaling:
    dst(idx) = src1(idx) * scale / src2(idx)
    or dst(idx) = scale / src2(idx) if src1 == 0 */
VSAPI(void)  vsDiv( const VsArr* src1, const VsArr* src2,
                    VsArr* dst, double scale VS_DEFAULT(1));

/* dst = src1 * scale + src2 */
CVAPI(void)  vsScaleAdd( const VsArr* src1, VsScalar scale,
                         const VsArr* src2, VsArr* dst );
#define vsAXPY( A, real_scalar, B, C ) vsScaleAdd(A, vsRealScalar(real_scalar), B, C)

/* dst = src1 * alpha + src2 * beta + gamma */
CVAPI(void)  vsAddWeighted( const VsArr* src1, double alpha,
                            const VsArr* src2, double beta,
                            double gamma, VsArr* dst );

/* result = sum_i(src1(i) * src2(i)) (results for all channels are accumulated together) */
VSAPI(double)  vsDotProduct( const VsArr* src1, const VsArr* src2 );

/* dst(idx) = src1(idx) & src2(idx) */
CVAPI(void) vsAnd( const VsArr* src1, const VsArr* src2,
                  VsArr* dst, const VsArr* mask VS_DEFAULT(NULL));

/* dst(idx) = src(idx) & value */
CVAPI(void) vsAndS( const VsArr* src, VsScalar value,
                   VsArr* dst, const VsArr* mask VS_DEFAULT(NULL));

/* dst(idx) = src1(idx) | src2(idx) */
CVAPI(void) vsOr( const VsArr* src1, const VsArr* src2,
                 VsArr* dst, const VsArr* mask VS_DEFAULT(NULL));

/* dst(idx) = src(idx) | value */
CVAPI(void) vsOrS( const VsArr* src, VsScalar value,
                  VsArr* dst, const VsArr* mask VS_DEFAULT(NULL));

/* dst(idx) = src1(idx) ^ src2(idx) */
CVAPI(void) vsXor( const VsArr* src1, const VsArr* src2,
                  VsArr* dst, const VsArr* mask VS_DEFAULT(NULL));

/* dst(idx) = src(idx) ^ value */
CVAPI(void) vsXorS( const VsArr* src, VsScalar value,
                   VsArr* dst, const VsArr* mask VS_DEFAULT(NULL));

/* dst(idx) = ~src(idx) */
CVAPI(void) vsNot( const VsArr* src, VsArr* dst );

/* dst(idx) = lower(idx) <= src(idx) < upper(idx) */
CVAPI(void) vsInRange( const VsArr* src, const VsArr* lower,
                      const VsArr* upper, VsArr* dst );

/* dst(idx) = lower <= src(idx) < upper */
CVAPI(void) vsInRangeS( const VsArr* src, VsScalar lower,
                       VsScalar upper, VsArr* dst );

#define VS_CMP_EQ   0
#define VS_CMP_GT   1
#define VS_CMP_GE   2
#define VS_CMP_LT   3
#define VS_CMP_LE   4
#define VS_CMP_NE   5

/* The comparison operation support single-channel arrays only.
   Destination image should be 8uC1 or 8sC1 */

/* dst(idx) = src1(idx) _cmp_op_ src2(idx) */
CVAPI(void) vsCmp( const VsArr* src1, const VsArr* src2, VsArr* dst, int cmp_op );

/* dst(idx) = src1(idx) _cmp_op_ value */
CVAPI(void) vsCmpS( const VsArr* src, double value, VsArr* dst, int cmp_op );

/* dst(idx) = min(src1(idx),src2(idx)) */
CVAPI(void) vsMin( const VsArr* src1, const VsArr* src2, VsArr* dst );

/* dst(idx) = max(src1(idx),src2(idx)) */
CVAPI(void) vsMax( const VsArr* src1, const VsArr* src2, VsArr* dst );

/* dst(idx) = min(src(idx),value) */
CVAPI(void) vsMinS( const VsArr* src, double value, VsArr* dst );

/* dst(idx) = max(src(idx),value) */
CVAPI(void) vsMaxS( const VsArr* src, double value, VsArr* dst );

/* dst(x,y,c) = abs(src1(x,y,c) - src2(x,y,c)) */
CVAPI(void) vsAbsDiff( const VsArr* src1, const VsArr* src2, VsArr* dst );

/* dst(x,y,c) = abs(src(x,y,c) - value(c)) */
CVAPI(void) vsAbsDiffS( const VsArr* src, VsArr* dst, VsScalar value );
#define vsAbs( src, dst ) vsAbsDiffS( (src), (dst), vsScalarAll(0))

/****************************************************************************************\
*                                Math operations                                         *
\****************************************************************************************/

#define VS_RAND_UNI      0
#define VS_RAND_NORMAL   1

/****************************************************************************************\
*                                Matrix operations                                       *
\****************************************************************************************/

/* Matrix transform: dst = A*B + C, C is optional */
#define vsMatMulAdd( src1, src2, src3, dst ) vsGEMM( (src1), (src2), 1., (src3), 1., (dst), 0 )
#define vsMatMul( src1, src2, dst )  vsMatMulAdd( (src1), (src2), NULL, (dst))

#define VS_GEMM_A_T 1
#define VS_GEMM_B_T 2
#define VS_GEMM_C_T 4
/* Extended matrix transform:
   dst = alpha*op(A)*op(B) + beta*op(C), where op(X) is X or X^T */
VSAPI(void)  vsGEMM( const VsArr* src1, const VsArr* src2, double alpha,
                     const VsArr* src3, double beta, VsArr* dst,
                     int tABC VS_DEFAULT(0));
#define vsMatMulAddEx vsGEMM

/* Tranposes matrix. Square matrices can be transposed in-place */
VSAPI(void)  vsTranspose( const VsArr* src, VsArr* dst );
#define vsT vsTranspose

/* Mirror array data around horizontal (flip=0),
   vertical (flip=1) or both(flip=-1) axises:
   vsFlip(src) flips images vertically and sequences horizontally (inplace) */
CVAPI(void)  vsFlip( const VsArr* src, VsArr* dst VS_DEFAULT(NULL),
                     int flip_mode VS_DEFAULT(0));
#define vsMirror vsFlip

#define VS_LU  0
#define VS_SVD 1
#define VS_SVD_SYM 2
#define VS_CHOLESKY 3
#define VS_QR  4
#define VS_NORMAL 16

/* Inverts matrix */
CVAPI(double)  vsInvert( const VsArr* src, VsArr* dst,
                         int method VS_DEFAULT(VS_LU));
#define vsInv vsInvert

/* Solves linear system (src1)*(dst) = (src2)
   (returns 0 if src1 is a singular and VS_LU method is used) */
VSAPI(int)  vsSolve( const VsArr* src1, const VsArr* src2, VsArr* dst,
                     int method VS_DEFAULT(VS_LU));

/* Makes an identity matrix (mat_ij = i == j) */
CVAPI(void)  vsSetIdentity( VsArr* mat, VsScalar value VS_DEFAULT(vsRealScalar(1)) );


/****************************************************************************************\
*                                    Array Statistics                                    *
\****************************************************************************************/

/* Calculates mean value of array elements */
CVAPI(VsScalar)  vsAvg( const VsArr* arr, const VsArr* mask VS_DEFAULT(NULL) );

/* Calculates mean and standard deviation of pixel values */
CVAPI(void)  vsAvgSdv( const VsArr* arr, VsScalar* mean, VsScalar* std_dev,
                       const VsArr* mask VS_DEFAULT(NULL) );

/* Finds global minimum, maximum and their positions */
CVAPI(void)  vsMinMaxLoc( const VsArr* arr, double* min_val, double* max_val,
                          VsPoint* min_loc VS_DEFAULT(NULL),
                          VsPoint* max_loc VS_DEFAULT(NULL),
                          const VsArr* mask VS_DEFAULT(NULL) );

/* types of array norm */
#define VS_C            1
#define VS_L1           2
#define VS_L2           4
#define VS_NORM_MASK    7
#define VS_RELATIVE     8
#define VS_DIFF         16
#define VS_MINMAX       32

#define VS_DIFF_C       (VS_DIFF | VS_C)
#define VS_DIFF_L1      (VS_DIFF | VS_L1)
#define VS_DIFF_L2      (VS_DIFF | VS_L2)
#define VS_RELATIVE_C   (VS_RELATIVE | VS_C)
#define VS_RELATIVE_L1  (VS_RELATIVE | VS_L1)
#define VS_RELATIVE_L2  (VS_RELATIVE | VS_L2)

/* Finds norm, difference norm or relative difference norm for an array (or two arrays) */
VSAPI(double)  vsNorm( const VsArr* arr1, const VsArr* arr2 VS_DEFAULT(NULL),
                       int norm_type VS_DEFAULT(VS_L2),
                       const VsArr* mask VS_DEFAULT(NULL) );

#define VS_REDUCE_SUM 0
#define VS_REDUCE_AVG 1
#define VS_REDUCE_MAX 2
#define VS_REDUCE_MIN 3

VSAPI(void)  vsReduce(const VsArr* src, VsArr* dst, int dim VS_DEFAULT(-1),
	int op VS_DEFAULT(VS_REDUCE_SUM));

/****************************************************************************************\
*                      Discrete Linear Transforms and Related Functions                  *
\****************************************************************************************/

#define VS_DXT_FORWARD  0
#define VS_DXT_INVERSE  1
#define VS_DXT_SCALE    2 /* divide result by size of array */
#define VS_DXT_INV_SCALE (VS_DXT_INVERSE + VS_DXT_SCALE)
#define VS_DXT_INVERSE_SCALE VS_DXT_INV_SCALE
#define VS_DXT_ROWS     4 /* transform each row individually */
#define VS_DXT_MUL_CONJ 8 /* conjugate the second argument of vsMulSpectrums */

/****************************************************************************************\
*                              Dynamic data structures                                   *
\****************************************************************************************/

/* Remember a storage "free memory" position */
CVAPI(void)  vsSaveMemStoragePos( const VsMemStorage* storage, VsMemStoragePos* pos );

/* Restore a storage "free memory" position */
CVAPI(void)  vsRestoreMemStoragePos( VsMemStorage* storage, VsMemStoragePos* pos );

#define VS_FRONT 1
#define VS_BACK 0

/* a < b ? -1 : a > b ? 1 : 0 */
typedef int (VS_CDECL* VsCmpFunc)(const void* a, const void* b, void* userdata );

/* Creates a new set */
CVAPI(VsSet*)  vsCreateSet( int set_flags, int header_size,
                            int elem_size, VsMemStorage* storage );

/* Adds new element to the set and returns pointer to it */
CVAPI(int)  vsSetAdd( VsSet* set_header, VsSetElem* elem VS_DEFAULT(NULL),
                      VsSetElem** inserted_elem VS_DEFAULT(NULL) );

/* Fast variant of vsSetAdd */
VS_INLINE  VsSetElem* vsSetNew( VsSet* set_header )
{
    VsSetElem* elem = set_header->free_elems;
    if( elem )
    {
        set_header->free_elems = elem->next_free;
        elem->flags = elem->flags & VS_SET_ELEM_IDX_MASK;
        set_header->active_count++;
    }
    else
        vsSetAdd( set_header, NULL, &elem );
    return elem;
}

/* Removes set element given its pointer */
VS_INLINE  void vsSetRemoveByPtr( VsSet* set_header, void* elem )
{
    VsSetElem* _elem = (VsSetElem*)elem;
    assert( _elem->flags >= 0 /*&& (elem->flags & VS_SET_ELEM_IDX_MASK) < set_header->total*/ );
    _elem->next_free = set_header->free_elems;
    _elem->flags = (_elem->flags & VS_SET_ELEM_IDX_MASK) | VS_SET_ELEM_FREE_FLAG;
    set_header->free_elems = _elem;
    set_header->active_count--;
}

/* Removes element from the set by its index  */
CVAPI(void)   vsSetRemove( VsSet* set_header, int index );

/****************************************************************************************\
*                                    System functions                                    *
\****************************************************************************************/

typedef void* (VS_CDECL *VsAllocFunc)(size_t size, void* userdata);
typedef int (VS_CDECL *VsFreeFunc)(void* pptr, void* userdata);

/* Set user-defined memory managment functions (substitutors for malloc and free) that
   will be called by vsAlloc, vsFree and higher-level functions (e.g. vsCreateImage) */
CVAPI(void) vsSetMemoryManager( VsAllocFunc alloc_func VS_DEFAULT(NULL),
                               VsFreeFunc free_func VS_DEFAULT(NULL),
                               void* userdata VS_DEFAULT(NULL));


typedef VsImage* (VS_STDCALL* Vs_vsCreateImageHeader)
                            (int,int,int,char*,char*,int,int,int,int,int,
                            VsROI*,VsImage*,void*,VsTileInfo*);
typedef void (VS_STDCALL* Vs_vsAllocateImageData)(VsImage*,int,int);
typedef void (VS_STDCALL* Vs_vsDeallocate)(VsImage*,int);
typedef VsROI* (VS_STDCALL* Vs_vsCreateROI)(int,int,int,int,int);
typedef VsImage* (VS_STDCALL* Vs_vsCloneImage)(const VsImage*);

/* Makes OpenCV use IPL functions for VsImage allocation/deallocation */
CVAPI(void) vsSetIPLAllocators( Vs_vsCreateImageHeader create_header,
                               Vs_vsAllocateImageData allocate_data,
                               Vs_vsDeallocate deallocate,
                               Vs_vsCreateROI create_roi,
                               Vs_vsCloneImage clone_image );

#define VS_TURN_ON_IPL_COMPATIBILITY()                                  \
    vsSetIPLAllocators( vsCreateImageHeader, vsAllocateImage,         \
                        vsDeallocate, vsCreateROI, vsCloneImage )

/*********************************** CPU capabilities ***********************************/

#define VS_CPU_NONE    0
#define VS_CPU_MMX     1
#define VS_CPU_SSE     2
#define VS_CPU_SSE2    3
#define VS_CPU_SSE3    4
#define VS_CPU_SSSE3   5
#define VS_CPU_SSE4_1  6
#define VS_CPU_SSE4_2  7
#define VS_CPU_POPCNT  8
#define VS_CPU_AVX    10
#define VS_CPU_AVX2   11
#define VS_HARDWARE_MAX_FEATURE 255

/********************************** Error Handling **************************************/

/* Get current OpenCV error status */
CVAPI(int) vsGetErrStatus( void );

/* Sets error status silently */
CVAPI(void) vsSetErrStatus( int status );

#define VS_ErrModeLeaf     0   /* Print error and exit program */
#define VS_ErrModeParent   1   /* Print error and continue */
#define VS_ErrModeSilent   2   /* Don't print and continue */

/* Retrives current error processing mode */
CVAPI(int)  vsGetErrMode( void );

/* Sets error processing mode, returns previously used mode */
CVAPI(int) vsSetErrMode( int mode );

/* Sets error status and performs some additonal actions (displaying message box,
 writing message to stderr, terminating application etc.)
 depending on the current error mode */
CVAPI(void) vsError( int status, const char* func_name,
                    const char* err_msg, const char* file_name, int line );

/* Retrieves textual description of the error given its code */
CVAPI(const char*) vsErrorStr( int status );

/* Retrieves detailed information about the last error occured */
CVAPI(int) vsGetErrInfo( const char** errcode_desc, const char** description,
                        const char** filename, int* line );

/* Maps IPP error codes to the counterparts from OpenCV */
CVAPI(int) vsErrorFromIppStatus( int ipp_status );

typedef int (VS_CDECL *VsErrorCallback)( int status, const char* func_name,
                                        const char* err_msg, const char* file_name, int line, void* userdata );

/* Assigns a new error-handling function */
CVAPI(VsErrorCallback) vsRedirectError( VsErrorCallback error_handler,
                                       void* userdata VS_DEFAULT(NULL),
                                       void** prev_userdata VS_DEFAULT(NULL) );

/*
 Output to:
 vsNulDevReport - nothing
 vsStdErrReport - console(fprintf(stderr,...))
 vsGuiBoxReport - MessageBox(WIN32)
 */
CVAPI(int) vsNulDevReport( int status, const char* func_name, const char* err_msg,
                          const char* file_name, int line, void* userdata );

CVAPI(int) vsStdErrReport( int status, const char* func_name, const char* err_msg,
                          const char* file_name, int line, void* userdata );

CVAPI(int) vsGuiBoxReport( int status, const char* func_name, const char* err_msg,
                          const char* file_name, int line, void* userdata );

#define OPENVS_ERROR(status,func,context)                           \
vsError((status),(func),(context),__FILE__,__LINE__)

#define OPENVS_ERRCHK(func,context)                                 \
{if (vsGetErrStatus() >= 0)                         \
{OPENVS_ERROR(VS_StsBackTrace,(func),(context));}}

#define OPENVS_ASSERT(expr,func,context)                            \
{if (! (expr))                                      \
{OPENVS_ERROR(VS_StsInternal,(func),(context));}}

#define OPENVS_RSTERR() (vsSetErrStatus(VS_StsOk))

#define OPENVS_CALL( Func )                                         \
{                                                                   \
Func;                                                           \
}


/* VS_FUNCNAME macro defines icvFuncName constant which is used by VS_ERROR macro */
#ifdef VS_NO_FUNC_NAMES
#define VS_FUNCNAME( Name )
#define vsFuncName ""
#else
#define VS_FUNCNAME( Name )  \
static char vsFuncName[] = Name
#endif


/*
 VS_ERROR macro unconditionally raises error with passed code and message.
 After raising error, control will be transferred to the exit label.
 */
#define VS_ERROR( Code, Msg )                                       \
{                                                                   \
    vsError( (Code), vsFuncName, Msg, __FILE__, __LINE__ );        \
    __VS_EXIT__;                                                   \
}

/* Simplified form of VS_ERROR */
#define VS_ERROR_FROM_CODE( code )   \
    VS_ERROR( code, "" )

/*
 VS_CHECK macro checks error status after CV (or IPL)
 function call. If error detected, control will be transferred to the exit
 label.
 */
#define VS_CHECK()                                                  \
{                                                                   \
    if( vsGetErrStatus() < 0 )                                      \
        VS_ERROR( VS_StsBackTrace, "Inner function failed." );      \
}


/*
 VS_CALL macro calls CV (or IPL) function, checks error status and
 signals a error if the function failed. Useful in "parent node"
 error procesing mode
 */
#define VS_CALL( Func )                                             \
{                                                                   \
    Func;                                                           \
    VS_CHECK();                                                     \
}


/* Runtime assertion macro */
#define VS_ASSERT( Condition )                                          \
{                                                                       \
    if( !(Condition) )                                                  \
        VS_ERROR( VS_StsInternal, "Assertion: " #Condition " failed" ); \
}

#define __VS_BEGIN__       {
#define __VS_END__         goto exit; exit: ; }
#define __VS_EXIT__        goto exit

#ifdef __cplusplus
}

#endif

#endif
