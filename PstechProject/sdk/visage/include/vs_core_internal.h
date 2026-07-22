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

/* The header is for internal use and it is likely to change.
   It contains some macro definitions that are used in cxcore, cv, cvaux
   and, probably, other libraries. If you need some of this functionality,
   the safe way is to copy it into your code and rename the macros.
*/
#ifndef __VS_CORE_INTERNAL_HPP__
#define __VS_CORE_INTERNAL_HPP__

#include <vector>

#include "vs_core_core.h"
#include "vs_core_types_c.h"

#if defined WIN32 || defined _WIN32
#  ifndef WIN32
#    define WIN32
#  endif
#  ifndef _WIN32
#    define _WIN32
#  endif
#endif

#if !defined WIN32 && !defined WINCE
#  include <pthread.h>
#endif

#ifdef __BORLANDC__
#  ifndef WIN32
#    define WIN32
#  endif
#  ifndef _WIN32
#    define _WIN32
#  endif
#  define VS_DLL
#  undef _VS_ALWAYS_PROFILE_
#  define _VS_ALWAYS_NO_PROFILE_
#endif

#ifndef FALSE
#  define FALSE 0
#endif
#ifndef TRUE
#  define TRUE 1
#endif

#ifndef IPPI_CALL
#  define IPPI_CALL(func) VS_Assert((func) >= 0)
#endif

#if defined __SSE2__ || defined _M_X64  || (defined _M_IX86_FP && _M_IX86_FP >= 2)
#  include "emmintrin.h"
#  define VS_SSE 1
#  define VS_SSE2 1
#  if defined __SSE3__ || (defined _MSC_VER && _MSC_VER >= 1500)
#    include "pmmintrin.h"
#    define VS_SSE3 1
#  endif
#  if defined __SSSE3__  || (defined _MSC_VER && _MSC_VER >= 1500)
#    include "tmmintrin.h"
#    define VS_SSSE3 1
#  endif
#  if defined __SSE4_1__ || (defined _MSC_VER && _MSC_VER >= 1500)
#    include <smmintrin.h>
#    define VS_SSE4_1 1
#  endif
#  if defined __SSE4_2__ || (defined _MSC_VER && _MSC_VER >= 1500)
#    include <nmmintrin.h>
#    define VS_SSE4_2 1
#  endif
#  if defined __AVX__ || (defined _MSC_FULL_VER && _MSC_FULL_VER >= 160040219)
// MS Visual Studio 2010 (2012?) has no macro pre-defined to identify the use of /arch:AVX
// See: http://connect.microsoft.com/VisualStudio/feedback/details/605858/arch-avx-should-define-a-predefined-macro-in-x64-and-set-a-unique-value-for-m-ix86-fp-in-win32
#    include <immintrin.h>
#    define VS_AVX 1
#    if defined(_XCR_XFEATURE_ENABLED_MASK)
#      define __xgetbv() _xgetbv(_XCR_XFEATURE_ENABLED_MASK)
#    else
#      define __xgetbv() 0
#    endif
#  endif
#  if defined __AVX2__
#    include <immintrin.h>
#    define VS_AVX2 1
#  endif
#endif


#if (defined WIN32 || defined _WIN32) && defined(_M_ARM)
# include <Intrin.h>
# include "arm_neon.h"
# define VS_NEON 1
# define CPU_HAS_NEON_FEATURE (true)
#elif defined(__ARM_NEON__) || defined(__ARM_NEON)
#  include <arm_neon.h>
#  define VS_NEON 1
#  define CPU_HAS_NEON_FEATURE (true)
#endif

#ifndef VS_SSE
#  define VS_SSE 0
#endif
#ifndef VS_SSE2
#  define VS_SSE2 0
#endif
#ifndef VS_SSE3
#  define VS_SSE3 0
#endif
#ifndef VS_SSSE3
#  define VS_SSSE3 0
#endif
#ifndef VS_SSE4_1
#  define VS_SSE4_1 0
#endif
#ifndef VS_SSE4_2
#  define VS_SSE4_2 0
#endif
#ifndef VS_AVX
#  define VS_AVX 0
#endif
#ifndef VS_AVX2
#  define VS_AVX2 0
#endif
#ifndef VS_NEON
#  define VS_NEON 0
#endif

#ifdef HAVE_TBB
#  include "tbb/tbb_stddef.h"
#  if TBB_VERSION_MAJOR*100 + TBB_VERSION_MINOR >= 202
#    include "tbb/tbb.h"
#    include "tbb/task.h"
#    undef min
#    undef max
#  else
#    undef HAVE_TBB
#  endif
#endif

#ifdef HAVE_EIGEN
#  if defined __GNUC__ && defined __APPLE__
#    pragma GCC diagnostic ignored "-Wshadow"
#  endif
#  include <Eigen/Core>
#  include "opencv2/core/eigen.hpp"
#endif

#ifdef __cplusplus

namespace vs
{
#ifdef HAVE_TBB

    typedef tbb::blocked_range<int> BlockedRange;

    template<typename Body> static inline
    void parallel_for( const BlockedRange& range, const Body& body )
    {
        tbb::parallel_for(range, body);
    }

    template<typename Iterator, typename Body> static inline
    void parallel_do( Iterator first, Iterator last, const Body& body )
    {
        tbb::parallel_do(first, last, body);
    }

    typedef tbb::split Split;

    template<typename Body> static inline
    void parallel_reduce( const BlockedRange& range, Body& body )
    {
        tbb::parallel_reduce(range, body);
    }

    typedef tbb::concurrent_vector<Rect> ConcurrentRectVector;
    typedef tbb::concurrent_vector<double> ConcurrentDoubleVector;
#else
    class BlockedRange
    {
    public:
        BlockedRange() : _begin(0), _end(0), _grainsize(0) {}
        BlockedRange(int b, int e, int g=1) : _begin(b), _end(e), _grainsize(g) {}
        int begin() const { return _begin; }
        int end() const { return _end; }
        int grainsize() const { return _grainsize; }

    protected:
        int _begin, _end, _grainsize;
    };

    template<typename Body> static inline
    void parallel_for( const BlockedRange& range, const Body& body )
    {
        body(range);
    }
    typedef std::vector<Rect> ConcurrentRectVector;
    typedef std::vector<double> ConcurrentDoubleVector;

    template<typename Iterator, typename Body> static inline
    void parallel_do( Iterator first, Iterator last, const Body& body )
    {
        for( ; first != last; ++first )
            body(*first);
    }

    class Split {};

    template<typename Body> static inline
    void parallel_reduce( const BlockedRange& range, Body& body )
    {
        body(range);
    }
#endif

    // Returns a static string if there is a parallel framework,
    // NULL otherwise.
    VS_EXPORTS const char* currentParallelFramework();
} //namespace vs

#define VS_INIT_ALGORITHM(classname, algname, memberinit) \
    static ::vs::Algorithm* create##classname() \
    { \
        return new classname; \
    } \
    \
    static ::vs::AlgorithmInfo& classname##_info() \
    { \
        static ::vs::AlgorithmInfo classname##_info_var(algname, create##classname); \
        return classname##_info_var; \
    } \
    \
    static ::vs::AlgorithmInfo& classname##_info_auto = classname##_info(); \
    \
    ::vs::AlgorithmInfo* classname::info() const \
    { \
        static volatile bool initialized = false; \
        \
        if( !initialized ) \
        { \
            initialized = true; \
            classname obj; \
            memberinit; \
        } \
        return &classname##_info(); \
    }

#endif //__cplusplus

/* maximal size of vector to run matrix operations on it inline (i.e. w/o ipp calls) */
#define  VS_MAX_INLINE_MAT_OP_SIZE  10

/* maximal linear size of matrix to allocate it on stack. */
#define  VS_MAX_LOCAL_MAT_SIZE  32

/* maximal size of local memory storage */
#define  VS_MAX_LOCAL_SIZE  \
    (VS_MAX_LOCAL_MAT_SIZE*VS_MAX_LOCAL_MAT_SIZE*(int)sizeof(double))

/* default image row align (in bytes) */
#define  VS_DEFAULT_IMAGE_ROW_ALIGN  4

/* matrices are continuous by default */
#define  VS_DEFAULT_MAT_ROW_ALIGN  1

/* maximum size of dynamic memory buffer.
   vsAlloc reports an error if a larger block is requested. */
#define  VS_MAX_ALLOC_SIZE    (((size_t)1 << (sizeof(size_t)*8-2)))

/* the alignment of all the allocated buffers */
#define  VS_MALLOC_ALIGN    16

/* default alignment for dynamic data strucutures, resided in storages. */
#define  VS_STRUCT_ALIGN    ((int)sizeof(double))

/* default storage block size */
#define  VS_STORAGE_BLOCK_SIZE   ((1<<16) - 128)

/* default memory block for sparse array elements */
#define  VS_SPARSE_MAT_BLOCK    (1<<12)

/* initial hash table size */
#define  VS_SPARSE_HASH_SIZE0    (1<<10)

/* maximal average node_count/hash_size ratio beyond which hash table is resized */
#define  VS_SPARSE_HASH_RATIO    3

/* max length of strings */
#define  VS_MAX_STRLEN  1024

#if 0 /*def  VS_CHECK_FOR_NANS*/
#  define VS_CHECK_NANS( arr ) vsCheckArray((arr))
#else
#  define VS_CHECK_NANS( arr )
#endif

/****************************************************************************************\
*                                  Common declarations                                   *
\****************************************************************************************/

#ifdef __GNUC__
#  define VS_DECL_ALIGNED(x) __attribute__ ((aligned (x)))
#elif defined _MSC_VER
#  define VS_DECL_ALIGNED(x) __declspec(align(x))
#else
#  define VS_DECL_ALIGNED(x)
#endif

#ifndef VS_IMPL
#  define VS_IMPL VS_EXTERN_C
#endif

#define VS_DBG_BREAK() { volatile int* crashMe = 0; *crashMe = 0; }

/* default step, set in case of continuous data
   to work around checks for valid step in some ipp functions */
#define  VS_STUB_STEP     (1 << 30)

#define  VS_SIZEOF_FLOAT ((int)sizeof(float))
#define  VS_SIZEOF_SHORT ((int)sizeof(short))

#define  VS_ORIGIN_TL  0
#define  VS_ORIGIN_BL  1

/* IEEE754 constants and macros */
#define  VS_POS_INF       0x7f800000
#define  VS_NEG_INF       0x807fffff /* VS_TOGGLE_FLT(0xff800000) */
#define  VS_1F            0x3f800000
#define  VS_TOGGLE_FLT(x) ((x)^((int)(x) < 0 ? 0x7fffffff : 0))
#define  VS_TOGGLE_DBL(x) \
    ((x)^((int64)(x) < 0 ? VS_BIG_INT(0x7fffffffffffffff) : 0))

#define  VS_NOP(a)      (a)
#define  VS_ADD(a, b)   ((a) + (b))
#define  VS_SUB(a, b)   ((a) - (b))
#define  VS_MUL(a, b)   ((a) * (b))
#define  VS_AND(a, b)   ((a) & (b))
#define  VS_OR(a, b)    ((a) | (b))
#define  VS_XOR(a, b)   ((a) ^ (b))
#define  VS_ANDN(a, b)  (~(a) & (b))
#define  VS_ORN(a, b)   (~(a) | (b))
#define  VS_SQR(a)      ((a) * (a))

#define  VS_LT(a, b)    ((a) < (b))
#define  VS_LE(a, b)    ((a) <= (b))
#define  VS_EQ(a, b)    ((a) == (b))
#define  VS_NE(a, b)    ((a) != (b))
#define  VS_GT(a, b)    ((a) > (b))
#define  VS_GE(a, b)    ((a) >= (b))

#define  VS_NONZERO(a)      ((a) != 0)
#define  VS_NONZERO_FLT(a)  (((a)+(a)) != 0)

/* general-purpose saturation macros */
#define  VS_CAST_8U(t)  (uchar)(!((t) & ~255) ? (t) : (t) > 0 ? 255 : 0)
#define  VS_CAST_8S(t)  (schar)(!(((t)+128) & ~255) ? (t) : (t) > 0 ? 127 : -128)
#define  VS_CAST_16U(t) (ushort)(!((t) & ~65535) ? (t) : (t) > 0 ? 65535 : 0)
#define  VS_CAST_16S(t) (short)(!(((t)+32768) & ~65535) ? (t) : (t) > 0 ? 32767 : -32768)
#define  VS_CAST_32S(t) (int)(t)
#define  VS_CAST_64S(t) (int64)(t)
#define  VS_CAST_32F(t) (float)(t)
#define  VS_CAST_64F(t) (double)(t)

#define  VS_PASTE2(a,b) a##b
#define  VS_PASTE(a,b)  VS_PASTE2(a,b)

#define  VS_EMPTY
#define  VS_MAKE_STR(a) #a

#define  VS_ZERO_OBJ(x) memset((x), 0, sizeof(*(x)))

#define  VS_DIM(static_array) ((int)(sizeof(static_array)/sizeof((static_array)[0])))

#define  vsUnsupportedFormat "vsUnsupportedFormat"

VS_INLINE void* vsAlignPtr( const void* ptr, int align VS_DEFAULT(32) )
{
    assert( (align & (align-1)) == 0 );
    return (void*)( ((size_t)ptr + align - 1) & ~(size_t)(align-1) );
}

VS_INLINE int vsAlign( int size, int align )
{
    assert( (align & (align-1)) == 0 && size < INT_MAX );
    return (size + align - 1) & -align;
}

VS_INLINE  VsSize  vsGetMatSize( const VsMat* mat )
{
    VsSize size;
    size.width = mat->cols;
    size.height = mat->rows;
    return size;
}

#define  VS_DESCALE(x,n)     (((x) + (1 << ((n)-1))) >> (n))
#define  VS_FLT_TO_FIX(x,n)  vsRound((x)*(1<<(n)))

/****************************************************************************************\

  Generic implementation of QuickSort algorithm.
  ----------------------------------------------
  Using this macro user can declare customized sort function that can be much faster
  than built-in qsort function because of lower overhead on elements
  comparison and exchange. The macro takes less_than (or LT) argument - a macro or function
  that takes 2 arguments returns non-zero if the first argument should be before the second
  one in the sorted sequence and zero otherwise.

  Example:

    Suppose that the task is to sort points by ascending of y coordinates and if
    y's are equal x's should ascend.

    The code is:
    ------------------------------------------------------------------------------
           #define cmp_pts( pt1, pt2 ) \
               ((pt1).y < (pt2).y || ((pt1).y < (pt2).y && (pt1).x < (pt2).x))

           [static] VS_IMPLEMENT_QSORT( icvSortPoints, VsPoint, cmp_pts )
    ------------------------------------------------------------------------------

    After that the function "void icvSortPoints( VsPoint* array, size_t total, int aux );"
    is available to user.

  aux is an additional parameter, which can be used when comparing elements.
  The current implementation was derived from *BSD system qsort():

    * Copyright (c) 1992, 1993
    *  The Regents of the University of California.  All rights reserved.
    *
    * Redistribution and use in source and binary forms, with or without
    * modification, are permitted provided that the following conditions
    * are met:
    * 1. Redistributions of source code must retain the above copyright
    *    notice, this list of conditions and the following disclaimer.
    * 2. Redistributions in binary form must reproduce the above copyright
    *    notice, this list of conditions and the following disclaimer in the
    *    documentation and/or other materials provided with the distribution.
    * 3. All advertising materials mentioning features or use of this software
    *    must display the following acknowledgement:
    *  This product includes software developed by the University of
    *  California, Berkeley and its contributors.
    * 4. Neither the name of the University nor the names of its contributors
    *    may be used to endorse or promote products derived from this software
    *    without specific prior written permission.
    *
    * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
    * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
    * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    * SUCH DAMAGE.

\****************************************************************************************/

#define VS_IMPLEMENT_QSORT_EX( func_name, T, LT, user_data_type )                   \
void func_name( T *array, size_t total, user_data_type aux )                        \
{                                                                                   \
    int isort_thresh = 7;                                                           \
    T t;                                                                            \
    int sp = 0;                                                                     \
                                                                                    \
    struct                                                                          \
    {                                                                               \
        T *lb;                                                                      \
        T *ub;                                                                      \
    }                                                                               \
    stack[48];                                                                      \
                                                                                    \
    aux = aux;                                                                      \
                                                                                    \
    if( total <= 1 )                                                                \
        return;                                                                     \
                                                                                    \
    stack[0].lb = array;                                                            \
    stack[0].ub = array + (total - 1);                                              \
                                                                                    \
    while( sp >= 0 )                                                                \
    {                                                                               \
        T* left = stack[sp].lb;                                                     \
        T* right = stack[sp--].ub;                                                  \
                                                                                    \
        for(;;)                                                                     \
        {                                                                           \
            int i, n = (int)(right - left) + 1, m;                                  \
            T* ptr;                                                                 \
            T* ptr2;                                                                \
                                                                                    \
            if( n <= isort_thresh )                                                 \
            {                                                                       \
            insert_sort:                                                            \
                for( ptr = left + 1; ptr <= right; ptr++ )                          \
                {                                                                   \
                    for( ptr2 = ptr; ptr2 > left && LT(ptr2[0],ptr2[-1]); ptr2--)   \
                        VS_SWAP( ptr2[0], ptr2[-1], t );                            \
                }                                                                   \
                break;                                                              \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                T* left0;                                                           \
                T* left1;                                                           \
                T* right0;                                                          \
                T* right1;                                                          \
                T* pivot;                                                           \
                T* a;                                                               \
                T* b;                                                               \
                T* c;                                                               \
                int swap_cnt = 0;                                                   \
                                                                                    \
                left0 = left;                                                       \
                right0 = right;                                                     \
                pivot = left + (n/2);                                               \
                                                                                    \
                if( n > 40 )                                                        \
                {                                                                   \
                    int d = n / 8;                                                  \
                    a = left, b = left + d, c = left + 2*d;                         \
                    left = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))     \
                                      : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));    \
                                                                                    \
                    a = pivot - d, b = pivot, c = pivot + d;                        \
                    pivot = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))    \
                                      : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));    \
                                                                                    \
                    a = right - 2*d, b = right - d, c = right;                      \
                    right = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))    \
                                      : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));    \
                }                                                                   \
                                                                                    \
                a = left, b = pivot, c = right;                                     \
                pivot = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))        \
                                   : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));       \
                if( pivot != left0 )                                                \
                {                                                                   \
                    VS_SWAP( *pivot, *left0, t );                                   \
                    pivot = left0;                                                  \
                }                                                                   \
                left = left1 = left0 + 1;                                           \
                right = right1 = right0;                                            \
                                                                                    \
                for(;;)                                                             \
                {                                                                   \
                    while( left <= right && !LT(*pivot, *left) )                    \
                    {                                                               \
                        if( !LT(*left, *pivot) )                                    \
                        {                                                           \
                            if( left > left1 )                                      \
                                VS_SWAP( *left1, *left, t );                        \
                            swap_cnt = 1;                                           \
                            left1++;                                                \
                        }                                                           \
                        left++;                                                     \
                    }                                                               \
                                                                                    \
                    while( left <= right && !LT(*right, *pivot) )                   \
                    {                                                               \
                        if( !LT(*pivot, *right) )                                   \
                        {                                                           \
                            if( right < right1 )                                    \
                                VS_SWAP( *right1, *right, t );                      \
                            swap_cnt = 1;                                           \
                            right1--;                                               \
                        }                                                           \
                        right--;                                                    \
                    }                                                               \
                                                                                    \
                    if( left > right )                                              \
                        break;                                                      \
                    VS_SWAP( *left, *right, t );                                    \
                    swap_cnt = 1;                                                   \
                    left++;                                                         \
                    right--;                                                        \
                }                                                                   \
                                                                                    \
                if( swap_cnt == 0 )                                                 \
                {                                                                   \
                    left = left0, right = right0;                                   \
                    goto insert_sort;                                               \
                }                                                                   \
                                                                                    \
                n = MIN( (int)(left1 - left0), (int)(left - left1) );               \
                for( i = 0; i < n; i++ )                                            \
                    VS_SWAP( left0[i], left[i-n], t );                              \
                                                                                    \
                n = MIN( (int)(right0 - right1), (int)(right1 - right) );           \
                for( i = 0; i < n; i++ )                                            \
                    VS_SWAP( left[i], right0[i-n+1], t );                           \
                n = (int)(left - left1);                                            \
                m = (int)(right1 - right);                                          \
                if( n > 1 )                                                         \
                {                                                                   \
                    if( m > 1 )                                                     \
                    {                                                               \
                        if( n > m )                                                 \
                        {                                                           \
                            stack[++sp].lb = left0;                                 \
                            stack[sp].ub = left0 + n - 1;                           \
                            left = right0 - m + 1, right = right0;                  \
                        }                                                           \
                        else                                                        \
                        {                                                           \
                            stack[++sp].lb = right0 - m + 1;                        \
                            stack[sp].ub = right0;                                  \
                            left = left0, right = left0 + n - 1;                    \
                        }                                                           \
                    }                                                               \
                    else                                                            \
                        left = left0, right = left0 + n - 1;                        \
                }                                                                   \
                else if( m > 1 )                                                    \
                    left = right0 - m + 1, right = right0;                          \
                else                                                                \
                    break;                                                          \
            }                                                                       \
        }                                                                           \
    }                                                                               \
}

#define VS_IMPLEMENT_QSORT( func_name, T, cmp )  \
    VS_IMPLEMENT_QSORT_EX( func_name, T, cmp, int )

/****************************************************************************************\
*                     Structures and macros for integration with IPP                     *
\****************************************************************************************/

/* IPP-compatible return codes */
typedef enum VsStatus
{
    VS_BADMEMBLOCK_ERR          = -113,
    VS_INPLACE_NOT_SUPPORTED_ERR= -112,
    VS_UNMATCHED_ROI_ERR        = -111,
    VS_NOTFOUND_ERR             = -110,
    VS_BADCONVERGENCE_ERR       = -109,

    VS_BADDEPTH_ERR             = -107,
    VS_BADROI_ERR               = -106,
    VS_BADHEADER_ERR            = -105,
    VS_UNMATCHED_FORMATS_ERR    = -104,
    VS_UNSUPPORTED_COI_ERR      = -103,
    VS_UNSUPPORTED_CHANNELS_ERR = -102,
    VS_UNSUPPORTED_DEPTH_ERR    = -101,
    VS_UNSUPPORTED_FORMAT_ERR   = -100,

    VS_BADARG_ERR               = -49,  //ipp comp
    VS_NOTDEFINED_ERR           = -48,  //ipp comp

    VS_BADCHANNELS_ERR          = -47,  //ipp comp
    VS_BADRANGE_ERR             = -44,  //ipp comp
    VS_BADSTEP_ERR              = -29,  //ipp comp

    VS_BADFLAG_ERR              =  -12,
    VS_DIV_BY_ZERO_ERR          =  -11, //ipp comp
    VS_BADCOEF_ERR              =  -10,

    VS_BADFACTOR_ERR            =  -7,
    VS_BADPOINT_ERR             =  -6,
    VS_BADSCALE_ERR             =  -4,
    VS_OUTOFMEM_ERR             =  -3,
    VS_NULLPTR_ERR              =  -2,
    VS_BADSIZE_ERR              =  -1,
    VS_NO_ERR                   =   0,
    VS_OK                       =   VS_NO_ERR
}
VsStatus;

#define VS_NOTHROW throw()

typedef struct VsFuncTable
{
    void*   fn_2d[VS_DEPTH_MAX];
}
VsFuncTable;

typedef struct VsBigFuncTable
{
    void*   fn_2d[VS_DEPTH_MAX*4];
} VsBigFuncTable;

#define VS_INIT_FUNC_TAB( tab, FUNCNAME, FLAG )         \
    (tab).fn_2d[VS_8U] = (void*)FUNCNAME##_8u##FLAG;    \
    (tab).fn_2d[VS_8S] = 0;                             \
    (tab).fn_2d[VS_16U] = (void*)FUNCNAME##_16u##FLAG;  \
    (tab).fn_2d[VS_16S] = (void*)FUNCNAME##_16s##FLAG;  \
    (tab).fn_2d[VS_32S] = (void*)FUNCNAME##_32s##FLAG;  \
    (tab).fn_2d[VS_32F] = (void*)FUNCNAME##_32f##FLAG;  \
    (tab).fn_2d[VS_64F] = (void*)FUNCNAME##_64f##FLAG

#endif // __VS_CORE_INTERNAL_HPP__
