/*! \file core.hpp
    \brief The Core Functionality
 */
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
// Copyright (C) 2009-2011, Willow Garage Inc., all rights reserved.
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

#ifndef __VS_CORE_HPP__
#define __VS_CORE_HPP__

#include "vs_core_types_c.h"
#include "vs_core_version.h"

#ifdef __cplusplus

#ifndef SKIP_INCLUDES
#include <limits.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <complex>
#include <map>
#include <new>
#include <string>
#include <vector>
#include <sstream>
#endif // SKIP_INCLUDES

/*! \namespace vs
    Namespace where all the C++ OpenCV functionality resides
*/
namespace vs {

#undef abs
#undef min
#undef max
#undef Complex

using std::vector;
using std::string;
using std::ptrdiff_t;

template<typename _Tp> class Size_;
template<typename _Tp> class Point_;
template<typename _Tp> class Rect_;
template<typename _Tp, int cn> class Vec;
template<typename _Tp, int m, int n> class Matx;

typedef std::string String;

#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined MAT_EXPORTS
#  define VS_MAT_EXPORTS __declspec(dllexport)
#else
#  define VS_MAT_EXPORTS
#endif

class Mat;

class VS_EXPORTS MatExpr;
class VS_EXPORTS MatOp_Base;
class VS_EXPORTS MatArg;

template<typename _Tp> class Mat_;
template<typename _Tp> class MatIterator_;
template<typename _Tp> class MatCommaInitializer_;

#if !defined(ANDROID) || (defined(_GLIBCXX_USE_WCHAR_T) && _GLIBCXX_USE_WCHAR_T)
typedef std::basic_string<wchar_t> WString;

VS_EXPORTS string fromUtf16(const WString& str);
VS_EXPORTS WString toUtf16(const string& str);
#endif

VS_EXPORTS string format( const char* fmt, ... );
VS_EXPORTS string tempfile( const char* suffix VS_DEFAULT(0));

// matrix decomposition types
enum { DECOMP_LU=0, DECOMP_SVD=1, DECOMP_EIG=2, DECOMP_CHOLESKY=3, DECOMP_QR=4, DECOMP_NORMAL=16 };
enum { NORM_INF=1, NORM_L1=2, NORM_L2=4, NORM_L2SQR=5, NORM_HAMMING=6, NORM_HAMMING2=7, NORM_TYPE_MASK=7, NORM_RELATIVE=8, NORM_MINMAX=32 };
enum { CMP_EQ=0, CMP_GT=1, CMP_GE=2, CMP_LT=3, CMP_LE=4, CMP_NE=5 };
enum { GEMM_1_T=1, GEMM_2_T=2, GEMM_3_T=4 };
enum { DFT_INVERSE=1, DFT_SCALE=2, DFT_ROWS=4, DFT_COMPLEX_OUTPUT=16, DFT_REAL_OUTPUT=32,
    DCT_INVERSE = DFT_INVERSE, DCT_ROWS=DFT_ROWS };


/*!
 The standard OpenCV exception class.
 Instances of the class are thrown by various functions and methods in the case of critical errors.
 */
class VS_EXPORTS Exception : public std::exception
{
public:
    /*!
     Default constructor
     */
    Exception();
    /*!
     Full constructor. Normally the constuctor is not called explicitly.
     Instead, the macros VS_Error(), VS_Error_() and VS_Assert() are used.
    */
    Exception(int _code, const string& _err, const string& _func, const string& _file, int _line);
    virtual ~Exception() throw();

    /*!
     \return the error description and the context as a text string.
    */
    virtual const char *what() const throw();
    void formatMessage();

    string msg; ///< the formatted error message

    int code; ///< error code @see VSStatus
    string err; ///< error description
    string func; ///< function name. Available only when the compiler supports getting it
    string file; ///< source file name where the error has occured
    int line; ///< line number in the source file where the error has occured
};


//! Signals an error and raises the exception.

/*!
  By default the function prints information about the error to stderr,
  then it either stops if setBreakOnError() had been called before or raises the exception.
  It is possible to alternate error processing by using redirectError().

  \param exc the exception raisen.
 */
VS_EXPORTS void error( const Exception& exc );

//! Sets/resets the break-on-error mode.

/*!
  When the break-on-error mode is set, the default error handler
  issues a hardware exception, which can make debugging more convenient.

  \return the previous state
 */
VS_EXPORTS bool setBreakOnError(bool flag);

typedef int (VS_CDECL *ErrorCallback)( int status, const char* func_name,
                                       const char* err_msg, const char* file_name,
                                       int line, void* userdata );

//! Sets the new error handler and the optional user data.

/*!
  The function sets the new error handler, called from vs::error().

  \param errCallback the new error handler. If NULL, the default error handler is used.
  \param userdata the optional user data pointer, passed to the callback.
  \param prevUserdata the optional output parameter where the previous user data pointer is stored

  \return the previous error handler
*/
VS_EXPORTS ErrorCallback redirectError( ErrorCallback errCallback,
                                        void* userdata=0, void** prevUserdata=0);


#if defined __GNUC__
#define VS_Func __func__
#elif defined _MSC_VER
#define VS_Func __FUNCTION__
#else
#define VS_Func ""
#endif

#define VS_Error( code, msg ) vs::error( vs::Exception(code, msg, VS_Func, __FILE__, __LINE__) )
#define VS_Error_( code, args ) vs::error( vs::Exception(code, vs::format args, VS_Func, __FILE__, __LINE__) )
#define VS_Assert( expr ) if(!!(expr)) ; else vs::error( vs::Exception(VS_StsAssert, #expr, VS_Func, __FILE__, __LINE__) )

#ifdef _DEBUG
#define VS_DbgAssert(expr) VS_Assert(expr)
#else
#define VS_DbgAssert(expr)
#endif

VS_EXPORTS void glob(String pattern, std::vector<String>& result, bool recursive = false);

VS_EXPORTS_W void setNumThreads(int nthreads);
VS_EXPORTS_W int getNumThreads();
VS_EXPORTS_W int getThreadNum();

VS_EXPORTS_W const string& getBuildInformation();

//! Returns the number of ticks.

/*!
  The function returns the number of ticks since the certain event (e.g. when the machine was turned on).
  It can be used to initialize vs::RNG or to measure a function execution time by reading the tick count
  before and after the function call. The granularity of ticks depends on the hardware and OS used. Use
  vs::getTickFrequency() to convert ticks to seconds.
*/
VS_EXPORTS_W int64 getTickCount();

/*!
  Returns the number of ticks per seconds.

  The function returns the number of ticks (as returned by vs::getTickCount()) per second.
  The following code computes the execution time in milliseconds:

  \code
  double exec_time = (double)getTickCount();
  // do something ...
  exec_time = ((double)getTickCount() - exec_time)*1000./getTickFrequency();
  \endcode
*/
VS_EXPORTS_W double getTickFrequency();

/*!
  Returns the number of CPU ticks.

  On platforms where the feature is available, the function returns the number of CPU ticks
  since the certain event (normally, the system power-on moment). Using this function
  one can accurately measure the execution time of very small code fragments,
  for which vs::getTickCount() granularity is not enough.
*/
VS_EXPORTS_W int64 getCPUTickCount();

/*!
  Returns SSE etc. support status

  The function returns true if certain hardware features are available.
  Currently, the following features are recognized:
  - VS_CPU_MMX - MMX
  - VS_CPU_SSE - SSE
  - VS_CPU_SSE2 - SSE 2
  - VS_CPU_SSE3 - SSE 3
  - VS_CPU_SSSE3 - SSSE 3
  - VS_CPU_SSE4_1 - SSE 4.1
  - VS_CPU_SSE4_2 - SSE 4.2
  - VS_CPU_POPCNT - POPCOUNT
  - VS_CPU_AVX - AVX
  - VS_CPU_AVX2 - AVX2

  \note {Note that the function output is not static. Once you called vs::useOptimized(false),
  most of the hardware acceleration is disabled and thus the function will returns false,
  until you call vs::useOptimized(true)}
*/
VS_EXPORTS_W bool checkHardwareSupport(int feature);

//! returns the number of CPUs (including hyper-threading)
VS_EXPORTS_W int getNumberOfCPUs();

/*!
  Allocates memory buffer

  This is specialized OpenCV memory allocation function that returns properly aligned memory buffers.
  The usage is identical to malloc(). The allocated buffers must be freed with vs::fastFree().
  If there is not enough memory, the function calls vs::error(), which raises an exception.

  \param bufSize buffer size in bytes
  \return the allocated memory buffer.
*/
VS_EXPORTS void* fastMalloc(size_t bufSize);

/*!
  Frees the memory allocated with vs::fastMalloc

  This is the corresponding deallocation function for vs::fastMalloc().
  When ptr==NULL, the function has no effect.
*/
VS_EXPORTS void fastFree(void* ptr);

template<typename _Tp> static inline _Tp* allocate(size_t n)
{
    return new _Tp[n];
}

template<typename _Tp> static inline void deallocate(_Tp* ptr, size_t)
{
    delete[] ptr;
}

/*!
  Aligns pointer by the certain number of bytes

  This small inline function aligns the pointer by the certian number of bytes by shifting
  it forward by 0 or a positive offset.
*/
template<typename _Tp> static inline _Tp* alignPtr(_Tp* ptr, int n=(int)sizeof(_Tp))
{
    return (_Tp*)(((size_t)ptr + n-1) & -n);
}

/*!
  Aligns buffer size by the certain number of bytes

  This small inline function aligns a buffer size by the certian number of bytes by enlarging it.
*/
static inline size_t alignSize(size_t sz, int n)
{
    assert((n & (n - 1)) == 0); // n is a power of 2
    return (sz + n-1) & -n;
}

/*!
  Turns on/off available optimization

  The function turns on or off the optimized code in OpenCV. Some optimization can not be enabled
  or disabled, but, for example, most of SSE code in OpenCV can be temporarily turned on or off this way.

  \note{Since optimization may imply using special data structures, it may be unsafe
  to call this function anywhere in the code. Instead, call it somewhere at the top level.}
*/
VS_EXPORTS_W void setUseOptimized(bool onoff);

/*!
  Returns the current optimization status

  The function returns the current optimization status, which is controlled by vs::setUseOptimized().
*/
VS_EXPORTS_W bool useOptimized();

/*!
  The STL-compilant memory Allocator based on vs::fastMalloc() and vs::fastFree()
*/
template<typename _Tp> class Allocator
{
public:
    typedef _Tp value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    template<typename U> class rebind { typedef Allocator<U> other; };

    explicit Allocator() {}
    ~Allocator() {}
    explicit Allocator(Allocator const&) {}
    template<typename U>
    explicit Allocator(Allocator<U> const&) {}

    // address
    pointer address(reference r) { return &r; }
    const_pointer address(const_reference r) { return &r; }

    pointer allocate(size_type count, const void* =0)
    { return reinterpret_cast<pointer>(fastMalloc(count * sizeof (_Tp))); }

    void deallocate(pointer p, size_type) {fastFree(p); }

    size_type max_size() const
    { return max(static_cast<_Tp>(-1)/sizeof(_Tp), 1); }

    void construct(pointer p, const _Tp& v) { new(static_cast<void*>(p)) _Tp(v); }
    void destroy(pointer p) { p->~_Tp(); }
};

/////////////////////// Vec (used as element of multi-channel images /////////////////////

/*!
  A helper class for vs::DataType

  The class is specialized for each fundamental numerical data type supported by OpenCV.
  It provides DataDepth<T>::value constant.
*/
template<typename _Tp> class DataDepth {};

template<> class DataDepth<bool> { public: enum { value = VS_8U, fmt=(int)'u' }; };
template<> class DataDepth<uchar> { public: enum { value = VS_8U, fmt=(int)'u' }; };
template<> class DataDepth<schar> { public: enum { value = VS_8S, fmt=(int)'c' }; };
template<> class DataDepth<char> { public: enum { value = VS_8S, fmt=(int)'c' }; };
template<> class DataDepth<ushort> { public: enum { value = VS_16U, fmt=(int)'w' }; };
template<> class DataDepth<short> { public: enum { value = VS_16S, fmt=(int)'s' }; };
template<> class DataDepth<int> { public: enum { value = VS_32S, fmt=(int)'i' }; };
// this is temporary solution to support 32-bit unsigned integers
template<> class DataDepth<unsigned> { public: enum { value = VS_32S, fmt=(int)'i' }; };
template<> class DataDepth<float> { public: enum { value = VS_32F, fmt=(int)'f' }; };
template<> class DataDepth<double> { public: enum { value = VS_64F, fmt=(int)'d' }; };
template<typename _Tp> class DataDepth<_Tp*> { public: enum { value = VS_USRTYPE1, fmt=(int)'r' }; };


////////////////////////////// Small Matrix ///////////////////////////

/*!
 A short numerical vector.

 This template class represents short numerical vectors (of 1, 2, 3, 4 ... elements)
 on which you can perform basic arithmetical operations, access individual elements using [] operator etc.
 The vectors are allocated on stack, as opposite to std::valarray, std::vector, vs::Mat etc.,
 which elements are dynamically allocated in the heap.

 The template takes 2 parameters:
 -# _Tp element type
 -# cn the number of elements

 In addition to the universal notation like Vec<float, 3>, you can use shorter aliases
 for the most popular specialized variants of Vec, e.g. Vec3f ~ Vec<float, 3>.
 */

struct VS_EXPORTS Matx_AddOp {};
struct VS_EXPORTS Matx_SubOp {};
struct VS_EXPORTS Matx_ScaleOp {};
struct VS_EXPORTS Matx_MulOp {};
struct VS_EXPORTS Matx_MatMulOp {};
struct VS_EXPORTS Matx_TOp {};

template<typename _Tp, int m, int n> class Matx
{
public:
    typedef _Tp value_type;
    typedef Matx<_Tp, (m < n ? m : n), 1> diag_type;
    typedef Matx<_Tp, m, n> mat_type;
    enum { depth = DataDepth<_Tp>::value, rows = m, cols = n, channels = rows*cols,
           type = VS_MAKETYPE(depth, channels) };

    //! default constructor
    Matx();

    Matx(_Tp v0); //!< 1x1 matrix
    Matx(_Tp v0, _Tp v1); //!< 1x2 or 2x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2); //!< 1x3 or 3x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3); //!< 1x4, 2x2 or 4x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4); //!< 1x5 or 5x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5); //!< 1x6, 2x3, 3x2 or 6x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6); //!< 1x7 or 7x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7); //!< 1x8, 2x4, 4x2 or 8x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8); //!< 1x9, 3x3 or 9x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8, _Tp v9); //!< 1x10, 2x5 or 5x2 or 10x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3,
         _Tp v4, _Tp v5, _Tp v6, _Tp v7,
         _Tp v8, _Tp v9, _Tp v10, _Tp v11); //!< 1x12, 2x6, 3x4, 4x3, 6x2 or 12x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3,
         _Tp v4, _Tp v5, _Tp v6, _Tp v7,
         _Tp v8, _Tp v9, _Tp v10, _Tp v11,
         _Tp v12, _Tp v13, _Tp v14, _Tp v15); //!< 1x16, 4x4 or 16x1 matrix
    explicit Matx(const _Tp* vals); //!< initialize from a plain array

    static Matx all(_Tp alpha);
    static Matx zeros();
    static Matx ones();
    static Matx eye();
    static Matx diag(const diag_type& d);
    static Matx randu(_Tp a, _Tp b);
    static Matx randn(_Tp a, _Tp b);

    //! dot product computed with the default precision
    _Tp dot(const Matx<_Tp, m, n>& v) const;

    //! dot product computed in double-precision arithmetics
    double ddot(const Matx<_Tp, m, n>& v) const;

    //! conversion to another data type
    template<typename T2> operator Matx<T2, m, n>() const;

    //! change the matrix shape
    template<int m1, int n1> Matx<_Tp, m1, n1> reshape() const;

    //! extract part of the matrix
    template<int m1, int n1> Matx<_Tp, m1, n1> get_minor(int i, int j) const;

    //! extract the matrix row
    Matx<_Tp, 1, n> row(int i) const;

    //! extract the matrix column
    Matx<_Tp, m, 1> col(int i) const;

    //! extract the matrix diagonal
    diag_type diag() const;

    //! transpose the matrix
    Matx<_Tp, n, m> t() const;

    //! invert matrix the matrix
    Matx<_Tp, n, m> inv(int method=DECOMP_LU) const;

    //! solve linear system
    template<int l> Matx<_Tp, n, l> solve(const Matx<_Tp, m, l>& rhs, int flags=DECOMP_LU) const;
    Vec<_Tp, n> solve(const Vec<_Tp, m>& rhs, int method) const;

    //! multiply two matrices element-wise
    Matx<_Tp, m, n> mul(const Matx<_Tp, m, n>& a) const;

    //! element access
    const _Tp& operator ()(int i, int j) const;
    _Tp& operator ()(int i, int j);

    //! 1D element access
    const _Tp& operator ()(int i) const;
    _Tp& operator ()(int i);

    Matx(const Matx<_Tp, m, n>& a, const Matx<_Tp, m, n>& b, Matx_AddOp);
    Matx(const Matx<_Tp, m, n>& a, const Matx<_Tp, m, n>& b, Matx_SubOp);
    template<typename _T2> Matx(const Matx<_Tp, m, n>& a, _T2 alpha, Matx_ScaleOp);
    Matx(const Matx<_Tp, m, n>& a, const Matx<_Tp, m, n>& b, Matx_MulOp);
    template<int l> Matx(const Matx<_Tp, m, l>& a, const Matx<_Tp, l, n>& b, Matx_MatMulOp);
    Matx(const Matx<_Tp, n, m>& a, Matx_TOp);

    _Tp val[m*n]; //< matrix elements
};


typedef Matx<float, 1, 2> Matx12f;
typedef Matx<double, 1, 2> Matx12d;
typedef Matx<float, 1, 3> Matx13f;
typedef Matx<double, 1, 3> Matx13d;
typedef Matx<float, 1, 4> Matx14f;
typedef Matx<double, 1, 4> Matx14d;
typedef Matx<float, 1, 6> Matx16f;
typedef Matx<double, 1, 6> Matx16d;

typedef Matx<float, 2, 1> Matx21f;
typedef Matx<double, 2, 1> Matx21d;
typedef Matx<float, 3, 1> Matx31f;
typedef Matx<double, 3, 1> Matx31d;
typedef Matx<float, 4, 1> Matx41f;
typedef Matx<double, 4, 1> Matx41d;
typedef Matx<float, 6, 1> Matx61f;
typedef Matx<double, 6, 1> Matx61d;

typedef Matx<float, 2, 2> Matx22f;
typedef Matx<double, 2, 2> Matx22d;
typedef Matx<float, 2, 3> Matx23f;
typedef Matx<double, 2, 3> Matx23d;
typedef Matx<float, 3, 2> Matx32f;
typedef Matx<double, 3, 2> Matx32d;

typedef Matx<float, 3, 3> Matx33f;
typedef Matx<double, 3, 3> Matx33d;

typedef Matx<float, 3, 4> Matx34f;
typedef Matx<double, 3, 4> Matx34d;
typedef Matx<float, 4, 3> Matx43f;
typedef Matx<double, 4, 3> Matx43d;

typedef Matx<float, 4, 4> Matx44f;
typedef Matx<double, 4, 4> Matx44d;
typedef Matx<float, 6, 6> Matx66f;
typedef Matx<double, 6, 6> Matx66d;


/*!
  A short numerical vector.

  This template class represents short numerical vectors (of 1, 2, 3, 4 ... elements)
  on which you can perform basic arithmetical operations, access individual elements using [] operator etc.
  The vectors are allocated on stack, as opposite to std::valarray, std::vector, vs::Mat etc.,
  which elements are dynamically allocated in the heap.

  The template takes 2 parameters:
  -# _Tp element type
  -# cn the number of elements

  In addition to the universal notation like Vec<float, 3>, you can use shorter aliases
  for the most popular specialized variants of Vec, e.g. Vec3f ~ Vec<float, 3>.
*/
template<typename _Tp, int cn> class Vec : public Matx<_Tp, cn, 1>
{
public:
    typedef _Tp value_type;
    enum { depth = DataDepth<_Tp>::value, channels = cn, type = VS_MAKETYPE(depth, channels) };

    //! default constructor
    Vec();

    Vec(_Tp v0); //!< 1-element vector constructor
    Vec(_Tp v0, _Tp v1); //!< 2-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2); //!< 3-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3); //!< 4-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4); //!< 5-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5); //!< 6-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6); //!< 7-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7); //!< 8-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8); //!< 9-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8, _Tp v9); //!< 10-element vector constructor
    explicit Vec(const _Tp* values);

    Vec(const Vec<_Tp, cn>& v);

    static Vec all(_Tp alpha);

    //! per-element multiplication
    Vec mul(const Vec<_Tp, cn>& v) const;

    //! conjugation (makes sense for complex numbers and quaternions)
    Vec conj() const;

    /*!
      cross product of the two 3D vectors.

      For other dimensionalities the exception is raised
    */
    Vec cross(const Vec& v) const;
    //! conversion to another data type
    template<typename T2> operator Vec<T2, cn>() const;
    //! conversion to 4-element VsScalar.
    operator VsScalar() const;

    /*! element access */
    const _Tp& operator [](int i) const;
    _Tp& operator[](int i);
    const _Tp& operator ()(int i) const;
    _Tp& operator ()(int i);

    Vec(const Matx<_Tp, cn, 1>& a, const Matx<_Tp, cn, 1>& b, Matx_AddOp);
    Vec(const Matx<_Tp, cn, 1>& a, const Matx<_Tp, cn, 1>& b, Matx_SubOp);
    template<typename _T2> Vec(const Matx<_Tp, cn, 1>& a, _T2 alpha, Matx_ScaleOp);
};


/* \typedef

   Shorter aliases for the most popular specializations of Vec<T,n>
*/
typedef Vec<uchar, 2> Vec2b;
typedef Vec<uchar, 3> Vec3b;
typedef Vec<uchar, 4> Vec4b;

typedef Vec<short, 2> Vec2s;
typedef Vec<short, 3> Vec3s;
typedef Vec<short, 4> Vec4s;

typedef Vec<ushort, 2> Vec2w;
typedef Vec<ushort, 3> Vec3w;
typedef Vec<ushort, 4> Vec4w;

typedef Vec<int, 2> Vec2i;
typedef Vec<int, 3> Vec3i;
typedef Vec<int, 4> Vec4i;
typedef Vec<int, 6> Vec6i;
typedef Vec<int, 8> Vec8i;

typedef Vec<float, 2> Vec2f;
typedef Vec<float, 3> Vec3f;
typedef Vec<float, 4> Vec4f;
typedef Vec<float, 6> Vec6f;

typedef Vec<double, 2> Vec2d;
typedef Vec<double, 3> Vec3d;
typedef Vec<double, 4> Vec4d;
typedef Vec<double, 6> Vec6d;


//////////////////////////////// Complex //////////////////////////////

/*!
  A complex number class.

  The template class is similar and compatible with std::complex, however it provides slightly
  more convenient access to the real and imaginary parts using through the simple field access, as opposite
  to std::complex::real() and std::complex::imag().
*/
template<typename _Tp> class Complex
{
public:

    //! constructors
    Complex();
    Complex( _Tp _re, _Tp _im=0 );
    Complex( const std::complex<_Tp>& c );

    //! conversion to another data type
    template<typename T2> operator Complex<T2>() const;
    //! conjugation
    Complex conj() const;
    //! conversion to std::complex
    operator std::complex<_Tp>() const;

    _Tp re, im; //< the real and the imaginary parts
};


typedef Complex<float> Complexf;
typedef Complex<double> Complexd;


//////////////////////////////// Point_ ////////////////////////////////

/*!
  template 2D point class.

  The class defines a point in 2D space. Data type of the point coordinates is specified
  as a template parameter. There are a few shorter aliases available for user convenience.
  See vs::Point, vs::Point2i, vs::Point2f and vs::Point2d.
*/
template<typename _Tp> class Point_
{
public:
    typedef _Tp value_type;

    // various constructors
    Point_();
    Point_(_Tp _x, _Tp _y);
    Point_(const Point_& pt);
    Point_(const VsPoint& pt);
    Point_(const VsPoint2D32f& pt);
    Point_(const Size_<_Tp>& sz);
    Point_(const Vec<_Tp, 2>& v);

    Point_& operator = (const Point_& pt);
    //! conversion to another data type
    template<typename _Tp2> operator Point_<_Tp2>() const;

    //! conversion to the old-style C structures
    operator VsPoint() const;
    operator VsPoint2D32f() const;
    operator Vec<_Tp, 2>() const;

    //! dot product
    _Tp dot(const Point_& pt) const;
    //! dot product computed in double-precision arithmetics
    double ddot(const Point_& pt) const;
    //! cross-product
    double cross(const Point_& pt) const;
    //! checks whether the point is inside the specified rectangle
    bool inside(const Rect_<_Tp>& r) const;

    _Tp x, y; //< the point coordinates
};

/*!
  template 3D point class.

  The class defines a point in 3D space. Data type of the point coordinates is specified
  as a template parameter.

  \see vs::Point3i, vs::Point3f and vs::Point3d
*/
template<typename _Tp> class Point3_
{
public:
    typedef _Tp value_type;

    // various constructors
    Point3_();
    Point3_(_Tp _x, _Tp _y, _Tp _z);
    Point3_(const Point3_& pt);
    explicit Point3_(const Point_<_Tp>& pt);
    Point3_(const VsPoint3D32f& pt);
    Point3_(const Vec<_Tp, 3>& v);

    Point3_& operator = (const Point3_& pt);
    //! conversion to another data type
    template<typename _Tp2> operator Point3_<_Tp2>() const;
    //! conversion to the old-style VsPoint...
    operator VsPoint3D32f() const;
    //! conversion to vs::Vec<>
    operator Vec<_Tp, 3>() const;

    //! dot product
    _Tp dot(const Point3_& pt) const;
    //! dot product computed in double-precision arithmetics
    double ddot(const Point3_& pt) const;
    //! cross product of the 2 3D points
    Point3_ cross(const Point3_& pt) const;

    _Tp x, y, z; //< the point coordinates
};

//////////////////////////////// Size_ ////////////////////////////////

/*!
  The 2D size class

  The class represents the size of a 2D rectangle, image size, matrix size etc.
  Normally, vs::Size ~ vs::Size_<int> is used.
*/
template<typename _Tp> class Size_
{
public:
    typedef _Tp value_type;

    //! various constructors
    Size_();
    Size_(_Tp _width, _Tp _height);
    Size_(const Size_& sz);
    Size_(const VsSize& sz);
    Size_(const VsSize2D32f& sz);
    Size_(const Point_<_Tp>& pt);

    Size_& operator = (const Size_& sz);
    //! the area (width*height)
    _Tp area() const;

    //! conversion of another data type.
    template<typename _Tp2> operator Size_<_Tp2>() const;

    //! conversion to the old-style OpenCV types
    operator VsSize() const;
    operator VsSize2D32f() const;

    _Tp width, height; // the width and the height
};

//////////////////////////////// Rect_ ////////////////////////////////

/*!
  The 2D up-right rectangle class

  The class represents a 2D rectangle with coordinates of the specified data type.
  Normally, vs::Rect ~ vs::Rect_<int> is used.
*/
template<typename _Tp> class Rect_
{
public:
    typedef _Tp value_type;

    //! various constructors
    Rect_();
    Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height);
    Rect_(const Rect_& r);
    Rect_(const VsRect& r);
    Rect_(const Point_<_Tp>& org, const Size_<_Tp>& sz);
    Rect_(const Point_<_Tp>& pt1, const Point_<_Tp>& pt2);

    Rect_& operator = ( const Rect_& r );
    //! the top-left corner
    Point_<_Tp> tl() const;
    //! the bottom-right corner
    Point_<_Tp> br() const;

    //! size (width, height) of the rectangle
    Size_<_Tp> size() const;
    //! area (width*height) of the rectangle
    _Tp area() const;

    //! conversion to another data type
    template<typename _Tp2> operator Rect_<_Tp2>() const;
    //! conversion to the old-style VsRect
    operator VsRect() const;

    //! checks whether the rectangle contains the point
    bool contains(const Point_<_Tp>& pt) const;

    _Tp x, y, width, height; //< the top-left corner, as well as width and height of the rectangle
};


typedef Point_<int> Point2i;
typedef Point2i Point;
typedef Size_<int> Size2i;
typedef Size_<double> Size2d;
typedef Size2i Size;
typedef Rect_<int> Rect;
typedef Point_<float> Point2f;
typedef Point_<double> Point2d;
typedef Size_<float> Size2f;
typedef Point3_<int> Point3i;
typedef Point3_<float> Point3f;
typedef Point3_<double> Point3d;


/*!
  The rotated 2D rectangle.

  The class represents rotated (i.e. not up-right) rectangles on a plane.
  Each rectangle is described by the center point (mass center), length of each side
  (represented by vs::Size2f structure) and the rotation angle in degrees.
*/
class VS_EXPORTS RotatedRect
{
public:
    //! various constructors
    RotatedRect();
    RotatedRect(const Point2f& center, const Size2f& size, float angle);
    RotatedRect(const VsBox2D& box);

    //! returns 4 vertices of the rectangle
    void points(Point2f pts[]) const;
    //! returns the minimal up-right rectangle containing the rotated rectangle
    Rect boundingRect() const;
    //! conversion to the old-style VsBox2D structure
    operator VsBox2D() const;

    Point2f center; //< the rectangle mass center
    Size2f size;    //< width and height of the rectangle
    float angle;    //< the rotation angle. When the angle is 0, 90, 180, 270 etc., the rectangle becomes an up-right rectangle.
};

//////////////////////////////// Scalar_ ///////////////////////////////

/*!
   The template scalar class.

   This is partially specialized vs::Vec class with the number of elements = 4, i.e. a short vector of four elements.
   Normally, vs::Scalar ~ vs::Scalar_<double> is used.
*/
template<typename _Tp> class Scalar_ : public Vec<_Tp, 4>
{
public:
    //! various constructors
    Scalar_();
    Scalar_(_Tp v0, _Tp v1, _Tp v2=0, _Tp v3=0);
    Scalar_(const VsScalar& s);
    Scalar_(_Tp v0);

    //! returns a scalar with all elements set to v0
    static Scalar_<_Tp> all(_Tp v0);
    //! conversion to the old-style VsScalar
    operator VsScalar() const;

    //! conversion to another data type
    template<typename T2> operator Scalar_<T2>() const;

    //! per-element product
    Scalar_<_Tp> mul(const Scalar_<_Tp>& t, double scale=1 ) const;

    // returns (v0, -v1, -v2, -v3)
    Scalar_<_Tp> conj() const;

    // returns true iff v1 == v2 == v3 == 0
    bool isReal() const;
};

typedef Scalar_<double> Scalar;

VS_EXPORTS void scalarToRawData(const Scalar& s, void* buf, int type, int unroll_to=0);

//////////////////////////////// Range /////////////////////////////////

/*!
   The 2D range class

   This is the class used to specify a continuous subsequence, i.e. part of a contour, or a column span in a matrix.
*/
class VS_EXPORTS Range
{
public:
    Range();
    Range(int _start, int _end);
    Range(const VsSlice& slice);
    int size() const;
    bool empty() const;
    static Range all();
    operator VsSlice() const;

    int start, end;
};

/////////////////////////////// DataType ////////////////////////////////

/*!
   Informative template class for OpenCV "scalars".

   The class is specialized for each primitive numerical type supported by OpenCV (such as unsigned char or float),
   as well as for more complex types, like vs::Complex<>, std::complex<>, vs::Vec<> etc.
   The common property of all such types (called "scalars", do not confuse it with vs::Scalar_)
   is that each of them is basically a tuple of numbers of the same type. Each "scalar" can be represented
   by the depth id (VS_8U ... VS_64F) and the number of channels.
   OpenCV matrices, 2D or nD, dense or sparse, can store "scalars",
   as long as the number of channels does not exceed VS_CN_MAX.
*/
template<typename _Tp> class DataType
{
public:
    typedef _Tp value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 1, depth = -1, channels = 1, fmt=0,
        type = VS_MAKETYPE(depth, channels) };
};

template<> class DataType<bool>
{
public:
    typedef bool value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
};

template<> class DataType<uchar>
{
public:
    typedef uchar value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
};

template<> class DataType<schar>
{
public:
    typedef schar value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
};

template<> class DataType<char>
{
public:
    typedef schar value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
};

template<> class DataType<ushort>
{
public:
    typedef ushort value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
};

template<> class DataType<short>
{
public:
    typedef short value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
};

template<> class DataType<int>
{
public:
    typedef int value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
};

template<> class DataType<float>
{
public:
    typedef float value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
};

template<> class DataType<double>
{
public:
    typedef double value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
};

template<typename _Tp, int m, int n> class DataType<Matx<_Tp, m, n> >
{
public:
    typedef Matx<_Tp, m, n> value_type;
    typedef Matx<typename DataType<_Tp>::work_type, m, n> work_type;
    typedef _Tp channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = m*n,
        fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
        type = VS_MAKETYPE(depth, channels) };
};

template<typename _Tp, int cn> class DataType<Vec<_Tp, cn> >
{
public:
    typedef Vec<_Tp, cn> value_type;
    typedef Vec<typename DataType<_Tp>::work_type, cn> work_type;
    typedef _Tp channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = cn,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
};

template<typename _Tp> class DataType<std::complex<_Tp> >
{
public:
    typedef std::complex<_Tp> value_type;
    typedef value_type work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Complex<_Tp> >
{
public:
    typedef Complex<_Tp> value_type;
    typedef value_type work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Point_<_Tp> >
{
public:
    typedef Point_<_Tp> value_type;
    typedef Point_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Point3_<_Tp> >
{
public:
    typedef Point3_<_Tp> value_type;
    typedef Point3_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 3,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Size_<_Tp> >
{
public:
    typedef Size_<_Tp> value_type;
    typedef Size_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Rect_<_Tp> >
{
public:
    typedef Rect_<_Tp> value_type;
    typedef Rect_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 4,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Scalar_<_Tp> >
{
public:
    typedef Scalar_<_Tp> value_type;
    typedef Scalar_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 4,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<> class DataType<Range>
{
public:
    typedef Range value_type;
    typedef value_type work_type;
    typedef int channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = VS_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

//////////////////// generic_type ref-counting pointer class for C/C++ objects ////////////////////////

/*!
  Smart pointer to dynamically allocated objects.

  This is template pointer-wrapping class that stores the associated reference counter along with the
  object pointer. The class is similar to std::smart_ptr<> from the recent addons to the C++ standard,
  but is shorter to write :) and self-contained (i.e. does add any dependency on the compiler or an external library).

  Basically, you can use "Ptr<MyObjectType> ptr" (or faster "const Ptr<MyObjectType>& ptr" for read-only access)
  everywhere instead of "MyObjectType* ptr", where MyObjectType is some C structure or a C++ class.
  To make it all work, you need to specialize Ptr<>::delete_obj(), like:

  \code
  template<> void Ptr<MyObjectType>::delete_obj() { call_destructor_func(obj); }
  \endcode

  \note{if MyObjectType is a C++ class with a destructor, you do not need to specialize delete_obj(),
  since the default implementation calls "delete obj;"}

  \note{Another good property of the class is that the operations on the reference counter are atomic,
  i.e. it is safe to use the class in multi-threaded applications}
*/
template<typename _Tp> class Ptr
{
public:
    //! empty constructor
    Ptr();
    //! take ownership of the pointer. The associated reference counter is allocated and set to 1
    Ptr(_Tp* _obj);
    //! calls release()
    ~Ptr();
    //! copy constructor. Copies the members and calls addref()
    Ptr(const Ptr& ptr);
    template<typename _Tp2> Ptr(const Ptr<_Tp2>& ptr);
    //! copy operator. Calls ptr.addref() and release() before copying the members
    Ptr& operator = (const Ptr& ptr);
    //! increments the reference counter
    void addref();
    //! decrements the reference counter. If it reaches 0, delete_obj() is called
    void release();
    //! deletes the object. Override if needed
    void delete_obj();
    //! returns true iff obj==NULL
    bool empty() const;

    //! cast pointer to another type
    template<typename _Tp2> Ptr<_Tp2> ptr();
    template<typename _Tp2> const Ptr<_Tp2> ptr() const;

    //! helper operators making "Ptr<T> ptr" use very similar to "T* ptr".
    _Tp* operator -> ();
    const _Tp* operator -> () const;

    operator _Tp* ();
    operator const _Tp*() const;

    _Tp* obj; //< the object pointer.
    int* refcount; //< the associated reference counter
};

template<typename T>
Ptr<T> makePtr();

template<typename T, typename A1>
Ptr<T> makePtr(const A1& a1);

template<typename T, typename A1, typename A2>
Ptr<T> makePtr(const A1& a1, const A2& a2);

template<typename T, typename A1, typename A2, typename A3>
Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3);

template<typename T, typename A1, typename A2, typename A3, typename A4>
Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3, const A4& a4);

template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5);

template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6);

template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7);

template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8);

template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9);

template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10);

//////////////////////// Input/Output Array Arguments /////////////////////////////////

/*!
 Proxy datatype for passing Mat's and vector<>'s as input parameters
 */
class VS_EXPORTS _InputArray
{
public:
    enum {
        KIND_SHIFT = 16,
        FIXED_TYPE = 0x8000 << KIND_SHIFT,
        FIXED_SIZE = 0x4000 << KIND_SHIFT,
        KIND_MASK = ~(FIXED_TYPE|FIXED_SIZE) - (1 << KIND_SHIFT) + 1,

        NONE              = 0 << KIND_SHIFT,
        MAT               = 1 << KIND_SHIFT,
        MATX              = 2 << KIND_SHIFT,
        STD_VECTOR        = 3 << KIND_SHIFT,
        STD_VECTOR_VECTOR = 4 << KIND_SHIFT,
        STD_VECTOR_MAT    = 5 << KIND_SHIFT,
        EXPR              = 6 << KIND_SHIFT,
        OPENGL_BUFFER     = 7 << KIND_SHIFT,
        OPENGL_TEXTURE    = 8 << KIND_SHIFT,
        GPU_MAT           = 9 << KIND_SHIFT,
        OCL_MAT           =10 << KIND_SHIFT
    };
    _InputArray();

    _InputArray(const Mat& m);
    _InputArray(const MatExpr& expr);
    template<typename _Tp> _InputArray(const _Tp* vec, int n);
    template<typename _Tp> _InputArray(const vector<_Tp>& vec);
    template<typename _Tp> _InputArray(const vector<vector<_Tp> >& vec);
    _InputArray(const vector<Mat>& vec);
    template<typename _Tp> _InputArray(const vector<Mat_<_Tp> >& vec);
    template<typename _Tp> _InputArray(const Mat_<_Tp>& m);
    template<typename _Tp, int m, int n> _InputArray(const Matx<_Tp, m, n>& matx);
    _InputArray(const Scalar& s);
    _InputArray(const double& val);

    virtual Mat getMat(int i=-1) const;
    virtual void getMatVector(vector<Mat>& mv) const;

    virtual int kind() const;
    virtual Size size(int i=-1) const;
    virtual size_t total(int i=-1) const;
    virtual int type(int i=-1) const;
    virtual int depth(int i=-1) const;
    virtual int channels(int i=-1) const;
    virtual bool empty() const;

#ifdef OPENVS_CAN_BREAK_BINARY_COMPATIBILITY
    virtual ~_InputArray();
#endif

    int flags;
    void* obj;
    Size sz;
};


enum
{
    DEPTH_MASK_8U = 1 << VS_8U,
    DEPTH_MASK_8S = 1 << VS_8S,
    DEPTH_MASK_16U = 1 << VS_16U,
    DEPTH_MASK_16S = 1 << VS_16S,
    DEPTH_MASK_32S = 1 << VS_32S,
    DEPTH_MASK_32F = 1 << VS_32F,
    DEPTH_MASK_64F = 1 << VS_64F,
    DEPTH_MASK_ALL = (DEPTH_MASK_64F<<1)-1,
    DEPTH_MASK_ALL_BUT_8S = DEPTH_MASK_ALL & ~DEPTH_MASK_8S,
    DEPTH_MASK_FLT = DEPTH_MASK_32F + DEPTH_MASK_64F
};


/*!
 Proxy datatype for passing Mat's and vector<>'s as input parameters
 */
class VS_EXPORTS _OutputArray : public _InputArray
{
public:
    _OutputArray();

    _OutputArray(Mat& m);
    template<typename _Tp> _OutputArray(vector<_Tp>& vec);
    template<typename _Tp> _OutputArray(vector<vector<_Tp> >& vec);
    _OutputArray(vector<Mat>& vec);
    template<typename _Tp> _OutputArray(vector<Mat_<_Tp> >& vec);
    template<typename _Tp> _OutputArray(Mat_<_Tp>& m);
    template<typename _Tp, int m, int n> _OutputArray(Matx<_Tp, m, n>& matx);
    template<typename _Tp> _OutputArray(_Tp* vec, int n);

    _OutputArray(const Mat& m);
    template<typename _Tp> _OutputArray(const vector<_Tp>& vec);
    template<typename _Tp> _OutputArray(const vector<vector<_Tp> >& vec);
    _OutputArray(const vector<Mat>& vec);
    template<typename _Tp> _OutputArray(const vector<Mat_<_Tp> >& vec);
    template<typename _Tp> _OutputArray(const Mat_<_Tp>& m);
    template<typename _Tp, int m, int n> _OutputArray(const Matx<_Tp, m, n>& matx);
    template<typename _Tp> _OutputArray(const _Tp* vec, int n);

    virtual bool fixedSize() const;
    virtual bool fixedType() const;
    virtual bool needed() const;
    virtual Mat& getMatRef(int i=-1) const;

    virtual void create(Size sz, int type, int i=-1, bool allowTransposed=false, int fixedDepthMask=0) const;
    virtual void create(int rows, int cols, int type, int i=-1, bool allowTransposed=false, int fixedDepthMask=0) const;
    virtual void create(int dims, const int* size, int type, int i=-1, bool allowTransposed=false, int fixedDepthMask=0) const;
    virtual void release() const;
    virtual void clear() const;

#ifdef OPENVS_CAN_BREAK_BINARY_COMPATIBILITY
    virtual ~_OutputArray();
#endif
};

typedef const _InputArray& InputArray;
typedef InputArray InputArrayOfArrays;
typedef const _OutputArray& OutputArray;
typedef OutputArray OutputArrayOfArrays;
typedef OutputArray InputOutputArray;
typedef OutputArray InputOutputArrayOfArrays;

VS_EXPORTS OutputArray noArray();

/////////////////////////////////////// Mat ///////////////////////////////////////////

enum { MAGIC_MASK=0xFFFF0000, TYPE_MASK=0x00000FFF, DEPTH_MASK=7 };

static inline size_t getElemSize(int type) { return VS_ELEM_SIZE(type); }

/*!
   Custom array allocator

*/
class VS_EXPORTS MatAllocator
{
public:
    MatAllocator() {}
    virtual ~MatAllocator() {}
    virtual void allocate(int dims, const int* sizes, int type, int*& refcount,
                          uchar*& datastart, uchar*& data, size_t* step) = 0;
    virtual void deallocate(int* refcount, uchar* datastart, uchar* data) = 0;
};

/*!
   The n-dimensional matrix class.

   The class represents an n-dimensional dense numerical array that can act as
   a matrix, image, optical flow map, 3-focal tensor etc.
   It is very similar to VsMat and VsMatND types from earlier versions of OpenCV,
   and similarly to those types, the matrix can be multi-channel. It also fully supports ROI mechanism.

   There are many different ways to create vs::Mat object. Here are the some popular ones:
   <ul>
   <li> using vs::Mat::create(nrows, ncols, type) method or
     the similar constructor vs::Mat::Mat(nrows, ncols, type[, fill_value]) constructor.
     A new matrix of the specified size and specifed type will be allocated.
     "type" has the same meaning as in vsCreateMat function,
     e.g. VS_8UC1 means 8-bit single-channel matrix, VS_32FC2 means 2-channel (i.e. complex)
     floating-point matrix etc:

     \code
     // make 7x7 complex matrix filled with 1+3j.
     vs::Mat M(7,7,VS_32FC2,Scalar(1,3));
     // and now turn M to 100x60 15-channel 8-bit matrix.
     // The old content will be deallocated
     M.create(100,60,VS_8UC(15));
     \endcode

     As noted in the introduction of this chapter, Mat::create()
     will only allocate a new matrix when the current matrix dimensionality
     or type are different from the specified.

   <li> by using a copy constructor or assignment operator, where on the right side it can
     be a matrix or expression, see below. Again, as noted in the introduction,
     matrix assignment is O(1) operation because it only copies the header
     and increases the reference counter. vs::Mat::clone() method can be used to get a full
     (a.k.a. deep) copy of the matrix when you need it.

   <li> by constructing a header for a part of another matrix. It can be a single row, single column,
     several rows, several columns, rectangular region in the matrix (called a minor in algebra) or
     a diagonal. Such operations are also O(1), because the new header will reference the same data.
     You can actually modify a part of the matrix using this feature, e.g.

     \code
     // add 5-th row, multiplied by 3 to the 3rd row
     M.row(3) = M.row(3) + M.row(5)*3;

     // now copy 7-th column to the 1-st column
     // M.col(1) = M.col(7); // this will not work
     Mat M1 = M.col(1);
     M.col(7).copyTo(M1);

     // create new 320x240 image
     vs::Mat img(Size(320,240),VS_8UC3);
     // select a roi
     vs::Mat roi(img, Rect(10,10,100,100));
     // fill the ROI with (0,255,0) (which is green in RGB space);
     // the original 320x240 image will be modified
     roi = Scalar(0,255,0);
     \endcode

     Thanks to the additional vs::Mat::datastart and vs::Mat::dataend members, it is possible to
     compute the relative sub-matrix position in the main "container" matrix using vs::Mat::locateROI():

     \code
     Mat A = Mat::eye(10, 10, VS_32S);
     // extracts A columns, 1 (inclusive) to 3 (exclusive).
     Mat B = A(Range::all(), Range(1, 3));
     // extracts B rows, 5 (inclusive) to 9 (exclusive).
     // that is, C ~ A(Range(5, 9), Range(1, 3))
     Mat C = B(Range(5, 9), Range::all());
     Size size; Point ofs;
     C.locateROI(size, ofs);
     // size will be (width=10,height=10) and the ofs will be (x=1, y=5)
     \endcode

     As in the case of whole matrices, if you need a deep copy, use vs::Mat::clone() method
     of the extracted sub-matrices.

   <li> by making a header for user-allocated-data. It can be useful for
      <ol>
      <li> processing "foreign" data using OpenCV (e.g. when you implement
         a DirectShow filter or a processing module for gstreamer etc.), e.g.

         \code
         void process_video_frame(const unsigned char* pixels,
                                  int width, int height, int step)
         {
            vs::Mat img(height, width, VS_8UC3, pixels, step);
            vs::GaussianBlur(img, img, vs::Size(7,7), 1.5, 1.5);
         }
         \endcode

      <li> for quick initialization of small matrices and/or super-fast element access

         \code
         double m[3][3] = {{a, b, c}, {d, e, f}, {g, h, i}};
         vs::Mat M = vs::Mat(3, 3, VS_64F, m).inv();
         \endcode
      </ol>

       partial yet very common cases of this "user-allocated data" case are conversions
       from VsMat and VsImage to vs::Mat. For this purpose there are special constructors
       taking pointers to VsMat or VsImage and the optional
       flag indicating whether to copy the data or not.

       Backward conversion from vs::Mat to VsMat or VsImage is provided via cast operators
       vs::Mat::operator VsMat() an vs::Mat::operator VsImage().
       The operators do not copy the data.


       \code
       VsImage* img = cvLoadImage("greatwave.jpg", 1);
       Mat mtx(img); // convert VsImage* -> vs::Mat
       VsMat oldmat = mtx; // convert vs::Mat -> VsMat
       VS_Assert(oldmat.cols == img->width && oldmat.rows == img->height &&
           oldmat.data.ptr == (uchar*)img->imageData && oldmat.step == img->widthStep);
       \endcode

   <li> by using MATLAB-style matrix initializers, vs::Mat::zeros(), vs::Mat::ones(), vs::Mat::eye(), e.g.:

   \code
   // create a double-precision identity martix and add it to M.
   M += Mat::eye(M.rows, M.cols, VS_64F);
   \endcode

   <li> by using comma-separated initializer:

   \code
   // create 3x3 double-precision identity matrix
   Mat M = (Mat_<double>(3,3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);
   \endcode

   here we first call constructor of vs::Mat_ class (that we describe further) with the proper matrix,
   and then we just put "<<" operator followed by comma-separated values that can be constants,
   variables, expressions etc. Also, note the extra parentheses that are needed to avoid compiler errors.

   </ul>

   Once matrix is created, it will be automatically managed by using reference-counting mechanism
   (unless the matrix header is built on top of user-allocated data,
   in which case you should handle the data by yourself).
   The matrix data will be deallocated when no one points to it;
   if you want to release the data pointed by a matrix header before the matrix destructor is called,
   use vs::Mat::release().

   The next important thing to learn about the matrix class is element access. Here is how the matrix is stored.
   The elements are stored in row-major order (row by row). The vs::Mat::data member points to the first element of the first row,
   vs::Mat::rows contains the number of matrix rows and vs::Mat::cols - the number of matrix columns. There is yet another member,
   vs::Mat::step that is used to actually compute address of a matrix element. vs::Mat::step is needed because the matrix can be
   a part of another matrix or because there can some padding space in the end of each row for a proper alignment.

   Given these parameters, address of the matrix element M_{ij} is computed as following:

   addr(M_{ij})=M.data + M.step*i + j*M.elemSize()

   if you know the matrix element type, e.g. it is float, then you can use vs::Mat::at() method:

   addr(M_{ij})=&M.at<float>(i,j)

   (where & is used to convert the reference returned by vs::Mat::at() to a pointer).
   if you need to process a whole row of matrix, the most efficient way is to get
   the pointer to the row first, and then just use plain C operator []:

   \code
   // compute sum of positive matrix elements
   // (assuming that M is double-precision matrix)
   double sum=0;
   for(int i = 0; i < M.rows; i++)
   {
       const double* Mi = M.ptr<double>(i);
       for(int j = 0; j < M.cols; j++)
           sum += std::max(Mi[j], 0.);
   }
   \endcode

   Some operations, like the above one, do not actually depend on the matrix shape,
   they just process elements of a matrix one by one (or elements from multiple matrices
   that are sitting in the same place, e.g. matrix addition). Such operations are called
   element-wise and it makes sense to check whether all the input/output matrices are continuous,
   i.e. have no gaps in the end of each row, and if yes, process them as a single long row:

   \code
   // compute sum of positive matrix elements, optimized variant
   double sum=0;
   int cols = M.cols, rows = M.rows;
   if(M.isContinuous())
   {
       cols *= rows;
       rows = 1;
   }
   for(int i = 0; i < rows; i++)
   {
       const double* Mi = M.ptr<double>(i);
       for(int j = 0; j < cols; j++)
           sum += std::max(Mi[j], 0.);
   }
   \endcode
   in the case of continuous matrix the outer loop body will be executed just once,
   so the overhead will be smaller, which will be especially noticeable in the case of small matrices.

   Finally, there are STL-style iterators that are smart enough to skip gaps between successive rows:
   \code
   // compute sum of positive matrix elements, iterator-based variant
   double sum=0;
   MatConstIterator_<double> it = M.begin<double>(), it_end = M.end<double>();
   for(; it != it_end; ++it)
       sum += std::max(*it, 0.);
   \endcode

   The matrix iterators are random-access iterators, so they can be passed
   to any STL algorithm, including std::sort().
*/
class VS_MAT_EXPORTS Mat
{
public:
    //! default constructor
    Mat();
    //! constructs 2D matrix of the specified size and type
    // (_type is VS_8UC1, VS_64FC3, VS_32SC(12) etc.)
    Mat(int rows, int cols, int type);
    Mat(Size size, int type);
    //! constucts 2D matrix and fills it with the specified value _s.
    Mat(int rows, int cols, int type, const Scalar& s);
    Mat(Size size, int type, const Scalar& s);

    //! constructs n-dimensional matrix
    Mat(int ndims, const int* sizes, int type);
    Mat(int ndims, const int* sizes, int type, const Scalar& s);

    //! copy constructor
    Mat(const Mat& m);
    //! constructor for matrix headers pointing to user-allocated data
    Mat(int rows, int cols, int type, void* data, size_t step=AUTO_STEP);
    Mat(Size size, int type, void* data, size_t step=AUTO_STEP);
    Mat(int ndims, const int* sizes, int type, void* data, const size_t* steps=0);

    //! creates a matrix header for a part of the bigger matrix
    Mat(const Mat& m, const Range& rowRange, const Range& colRange=Range::all());
    Mat(const Mat& m, const Rect& roi);
    Mat(const Mat& m, const Range* ranges);
    //! converts old-style VsMat to the new matrix; the data is not copied by default
    Mat(const VsMat* m, bool copyData=false);
    //! converts old-style VsImage to the new matrix; the data is not copied by default
    Mat(const VsImage* img, bool copyData=false);
    //! builds matrix from std::vector with or without copying the data
    template<typename _Tp> explicit Mat(const vector<_Tp>& vec, bool copyData=false);
    //! builds matrix from vs::Vec; the data is copied by default
    template<typename _Tp, int n> explicit Mat(const Vec<_Tp, n>& vec, bool copyData=true);
    //! builds matrix from vs::Matx; the data is copied by default
    template<typename _Tp, int m, int n> explicit Mat(const Matx<_Tp, m, n>& mtx, bool copyData=true);
    //! builds matrix from a 2D point
    template<typename _Tp> explicit Mat(const Point_<_Tp>& pt, bool copyData=true);
    //! builds matrix from a 3D point
    template<typename _Tp> explicit Mat(const Point3_<_Tp>& pt, bool copyData=true);
    //! builds matrix from comma initializer
    template<typename _Tp> explicit Mat(const MatCommaInitializer_<_Tp>& commaInitializer);

    //! destructor - calls release()
    ~Mat();
    //! assignment operators
    Mat& operator = (const Mat& m);
    Mat& operator = (const MatExpr& expr);

    //! returns a new matrix header for the specified row
    Mat row(int y) const;
    //! returns a new matrix header for the specified column
    Mat col(int x) const;
    //! ... for the specified row span
    Mat rowRange(int startrow, int endrow) const;
    Mat rowRange(const Range& r) const;
    //! ... for the specified column span
    Mat colRange(int startcol, int endcol) const;
    Mat colRange(const Range& r) const;
    //! ... for the specified diagonal
    // (d=0 - the main diagonal,
    //  >0 - a diagonal from the lower half,
    //  <0 - a diagonal from the upper half)
    Mat diag(int d=0) const;
    //! constructs a square diagonal matrix which main diagonal is vector "d"
    static Mat diag(const Mat& d);

    //! returns deep copy of the matrix, i.e. the data is copied
    Mat clone() const;
    //! copies the matrix content to "m".
    // It calls m.create(this->size(), this->type()).
    void copyTo( OutputArray m ) const;
    //! copies those matrix elements to "m" that are marked with non-zero mask elements.
    void copyTo( OutputArray m, InputArray mask ) const;
    //! converts matrix to another datatype with optional scalng. See vsConvertScale.
    void convertTo( OutputArray m, int rtype, double alpha=1, double beta=0 ) const;

    void assignTo( Mat& m, int type=-1 ) const;

    //! sets every matrix element to s
    Mat& operator = (const Scalar& s);
    //! sets some of the matrix elements to s, according to the mask
    Mat& setTo(InputArray value, InputArray mask=noArray());
    //! creates alternative matrix header for the same data, with different
    // number of channels and/or different number of rows. see vsReshape.
    Mat reshape(int cn, int rows=0) const;
    Mat reshape(int cn, int newndims, const int* newsz) const;

    //! matrix transposition by means of matrix expressions
    MatExpr t() const;
    //! matrix inversion by means of matrix expressions
    MatExpr inv(int method=DECOMP_LU) const;
    //! per-element matrix multiplication by means of matrix expressions
    MatExpr mul(InputArray m, double scale=1) const;

    //! computes cross-product of 2 3D vectors
    Mat cross(InputArray m) const;
    //! computes dot-product
    double dot(InputArray m) const;

    //! Matlab-style matrix initialization
    static MatExpr zeros(int rows, int cols, int type);
    static MatExpr zeros(Size size, int type);
    static MatExpr zeros(int ndims, const int* sz, int type);
    static MatExpr ones(int rows, int cols, int type);
    static MatExpr ones(Size size, int type);
    static MatExpr ones(int ndims, const int* sz, int type);
    static MatExpr eye(int rows, int cols, int type);
    static MatExpr eye(Size size, int type);

    //! allocates new matrix data unless the matrix already has specified size and type.
    // previous data is unreferenced if needed.
    void create(int rows, int cols, int type);
    void create(Size size, int type);
    void create(int ndims, const int* sizes, int type);

    //! increases the reference counter; use with care to avoid memleaks
    void addref();
    //! decreases reference counter;
    // deallocates the data when reference counter reaches 0.
    void release();

    //! deallocates the matrix data
    void deallocate();
    //! internal use function; properly re-allocates _size, _step arrays
    void copySize(const Mat& m);

    //! reserves enough space to fit sz hyper-planes
    void reserve(size_t sz);
    //! resizes matrix to the specified number of hyper-planes
    void resize(size_t sz);
    //! resizes matrix to the specified number of hyper-planes; initializes the newly added elements
    void resize(size_t sz, const Scalar& s);
    //! internal function
    void push_back_(const void* elem);
    //! adds element to the end of 1d matrix (or possibly multiple elements when _Tp=Mat)
    template<typename _Tp> void push_back(const _Tp& elem);
    template<typename _Tp> void push_back(const Mat_<_Tp>& elem);
    void push_back(const Mat& m);
    //! removes several hyper-planes from bottom of the matrix
    void pop_back(size_t nelems=1);

    //! locates matrix header within a parent matrix. See below
    void locateROI( Size& wholeSize, Point& ofs ) const;
    //! moves/resizes the current matrix ROI inside the parent matrix.
    Mat& adjustROI( int dtop, int dbottom, int dleft, int dright );
    //! extracts a rectangular sub-matrix
    // (this is a generalized form of row, rowRange etc.)
    Mat operator()( Range rowRange, Range colRange ) const;
    Mat operator()( const Rect& roi ) const;
    Mat operator()( const Range* ranges ) const;

    //! converts header to VsMat; no data is copied
    operator VsMat() const;
    //! converts header to VsImage; no data is copied
    operator VsImage() const;

    template<typename _Tp> operator vector<_Tp>() const;
    template<typename _Tp, int n> operator Vec<_Tp, n>() const;
    template<typename _Tp, int m, int n> operator Matx<_Tp, m, n>() const;

    //! returns true iff the matrix data is continuous
    // (i.e. when there are no gaps between successive rows).
    // similar to VS_IS_MAT_CONT(vsmat->type)
    bool isContinuous() const;

    //! returns true if the matrix is a submatrix of another matrix
    bool isSubmatrix() const;

    //! returns element size in bytes,
    // similar to VS_ELEM_SIZE(vsmat->type)
    size_t elemSize() const;
    //! returns the size of element channel in bytes.
    size_t elemSize1() const;
    //! returns element type, similar to VS_MAT_TYPE(vsmat->type)
    int type() const;
    //! returns element type, similar to VS_MAT_DEPTH(vsmat->type)
    int depth() const;
    //! returns element type, similar to VS_MAT_CN(vsmat->type)
    int channels() const;
    //! returns step/elemSize1()
    size_t step1(int i=0) const;
    //! returns true if matrix data is NULL
    bool empty() const;
    //! returns the total number of matrix elements
    size_t total() const;

    //! returns N if the matrix is 1-channel (N x ptdim) or ptdim-channel (1 x N) or (N x 1); negative number otherwise
    int checkVector(int elemChannels, int depth=-1, bool requireContinuous=true) const;

    //! returns pointer to i0-th submatrix along the dimension #0
    uchar* ptr(int i0=0);
    const uchar* ptr(int i0=0) const;

    //! returns pointer to (i0,i1) submatrix along the dimensions #0 and #1
    uchar* ptr(int i0, int i1);
    const uchar* ptr(int i0, int i1) const;

    //! returns pointer to (i0,i1,i3) submatrix along the dimensions #0, #1, #2
    uchar* ptr(int i0, int i1, int i2);
    const uchar* ptr(int i0, int i1, int i2) const;

    //! returns pointer to the matrix element
    uchar* ptr(const int* idx);
    //! returns read-only pointer to the matrix element
    const uchar* ptr(const int* idx) const;

    template<int n> uchar* ptr(const Vec<int, n>& idx);
    template<int n> const uchar* ptr(const Vec<int, n>& idx) const;

    //! template version of the above method
    template<typename _Tp> _Tp* ptr(int i0=0);
    template<typename _Tp> const _Tp* ptr(int i0=0) const;

    template<typename _Tp> _Tp* ptr(int i0, int i1);
    template<typename _Tp> const _Tp* ptr(int i0, int i1) const;

    template<typename _Tp> _Tp* ptr(int i0, int i1, int i2);
    template<typename _Tp> const _Tp* ptr(int i0, int i1, int i2) const;

    template<typename _Tp> _Tp* ptr(const int* idx);
    template<typename _Tp> const _Tp* ptr(const int* idx) const;

    template<typename _Tp, int n> _Tp* ptr(const Vec<int, n>& idx);
    template<typename _Tp, int n> const _Tp* ptr(const Vec<int, n>& idx) const;

    //! the same as above, with the pointer dereferencing
    template<typename _Tp> _Tp& at(int i0=0);
    template<typename _Tp> const _Tp& at(int i0=0) const;

    template<typename _Tp> _Tp& at(int i0, int i1);
    template<typename _Tp> const _Tp& at(int i0, int i1) const;

    template<typename _Tp> _Tp& at(int i0, int i1, int i2);
    template<typename _Tp> const _Tp& at(int i0, int i1, int i2) const;

    template<typename _Tp> _Tp& at(const int* idx);
    template<typename _Tp> const _Tp& at(const int* idx) const;

    template<typename _Tp, int n> _Tp& at(const Vec<int, n>& idx);
    template<typename _Tp, int n> const _Tp& at(const Vec<int, n>& idx) const;

    //! special versions for 2D arrays (especially convenient for referencing image pixels)
    template<typename _Tp> _Tp& at(Point pt);
    template<typename _Tp> const _Tp& at(Point pt) const;

    //! template methods for iteration over matrix elements.
    // the iterators take care of skipping gaps in the end of rows (if any)
    template<typename _Tp> MatIterator_<_Tp> begin();
    template<typename _Tp> MatIterator_<_Tp> end();

    enum { MAGIC_VAL=0x42FF0000, AUTO_STEP=0, CONTINUOUS_FLAG=VS_MAT_CONT_FLAG, SUBMATRIX_FLAG=VS_SUBMAT_FLAG };

    /*! includes several bit-fields:
         - the magic signature
         - continuity flag
         - depth
         - number of channels
     */
    int flags;
    //! the matrix dimensionality, >= 2
    int dims;
    //! the number of rows and columns or (-1, -1) when the matrix has more than 2 dimensions
    int rows, cols;
    //! pointer to the data
    uchar* data;

    //! pointer to the reference counter;
    // when matrix points to user-allocated data, the pointer is NULL
    int* refcount;

    //! helper fields used in locateROI and adjustROI
    uchar* datastart;
    uchar* dataend;
    uchar* datalimit;

    //! custom allocator
    MatAllocator* allocator;

    struct VS_EXPORTS MSize
    {
        MSize(int* _p);
        Size operator()() const;
        const int& operator[](int i) const;
        int& operator[](int i);
        operator const int*() const;
        bool operator == (const MSize& sz) const;
        bool operator != (const MSize& sz) const;

        int* p;
    };

    struct VS_EXPORTS MStep
    {
        MStep();
        MStep(size_t s);
        const size_t& operator[](int i) const;
        size_t& operator[](int i);
        operator size_t() const;
        MStep& operator = (size_t s);

        size_t* p;
        size_t buf[2];
    protected:
        MStep& operator = (const MStep&);
    };

    MSize size;
    MStep step;

protected:
    void initEmpty();
};


/*!
   Random Number Generator

   The class implements RNG using Multiply-with-Carry algorithm
*/
class VS_EXPORTS RNG
{
public:
    enum { UNIFORM=0, NORMAL=1 };

    RNG();
    RNG(uint64 state);
    //! updates the state and returns the next 32-bit unsigned integer random number
    unsigned next();

    operator uchar();
    operator schar();
    operator ushort();
    operator short();
    operator unsigned();
    //! returns a random integer sampled uniformly from [0, N).
    unsigned operator ()(unsigned N);
    unsigned operator ()();
    operator int();
    operator float();
    operator double();
    //! returns uniformly distributed integer random number from [a,b) range
    int uniform(int a, int b);
    //! returns uniformly distributed floating-point random number from [a,b) range
    float uniform(float a, float b);
    //! returns uniformly distributed double-precision floating-point random number from [a,b) range
    double uniform(double a, double b);
    void fill( InputOutputArray mat, int distType, InputArray a, InputArray b, bool saturateRange=false );
    //! returns Gaussian random variate with mean zero.
    double gaussian(double sigma);

    uint64 state;
};

/*!
   Random Number Generator - MT

   The class implements RNG using the Mersenne Twister algorithm
*/
class VS_EXPORTS RNG_MT19937
{
public:
    RNG_MT19937();
    RNG_MT19937(unsigned s);
    void seed(unsigned s);

    unsigned next();

    operator int();
    operator unsigned();
    operator float();
    operator double();

    unsigned operator ()(unsigned N);
    unsigned operator ()();

    //! returns uniformly distributed integer random number from [a,b) range
    int uniform(int a, int b);
    //! returns uniformly distributed floating-point random number from [a,b) range
    float uniform(float a, float b);
    //! returns uniformly distributed double-precision floating-point random number from [a,b) range
    double uniform(double a, double b);

private:
    enum PeriodParameters {N = 624, M = 397};
    unsigned state[N];
    int mti;
};

/*!
 Termination criteria in iterative algorithms
 */
class VS_EXPORTS TermCriteria
{
public:
    enum
    {
        COUNT=1, //!< the maximum number of iterations or elements to compute
        MAX_ITER=COUNT, //!< ditto
        EPS=2 //!< the desired accuracy or change in parameters at which the iterative algorithm stops
    };

    //! default constructor
    TermCriteria();
    //! full constructor
    TermCriteria(int type, int maxCount, double epsilon);
    //! conversion from VsTermCriteria
    TermCriteria(const VsTermCriteria& criteria);
    //! conversion to VsTermCriteria
    operator VsTermCriteria() const;

    int type; //!< the type of termination criteria: COUNT, EPS or COUNT + EPS
    int maxCount; // the maximum number of iterations/elements
    double epsilon; // the desired accuracy
};


typedef void (*BinaryFunc)(const uchar* src1, size_t step1,
                           const uchar* src2, size_t step2,
                           uchar* dst, size_t step, Size sz,
                           void*);

VS_EXPORTS BinaryFunc getConvertFunc(int sdepth, int ddepth);
VS_EXPORTS BinaryFunc getConvertScaleFunc(int sdepth, int ddepth);
VS_EXPORTS BinaryFunc getCopyMaskFunc(size_t esz);

//! swaps two matrices
VS_EXPORTS void swap(Mat& a, Mat& b);

//! converts array (VsMat or VsImage) to vs::Mat
VS_EXPORTS Mat vsarrToMat(const VsArr* arr, bool copyData=false,
                          bool allowND=true, int coiMode=0);
//! extracts Channel of Interest from VsMat or VsImage and makes vs::Mat out of it.
VS_EXPORTS void extractImageCOI(const VsArr* arr, OutputArray coiimg, int coi=-1);
//! inserts single-channel vs::Mat into a multi-channel VsMat or VsImage
VS_EXPORTS void insertImageCOI(InputArray coiimg, VsArr* arr, int coi=-1);

//! adds one matrix to another (dst = src1 + src2)
VS_EXPORTS_W void add(InputArray src1, InputArray src2, OutputArray dst,
                      InputArray mask=noArray(), int dtype=-1);
//! subtracts one matrix from another (dst = src1 - src2)
VS_EXPORTS_W void subtract(InputArray src1, InputArray src2, OutputArray dst,
                           InputArray mask=noArray(), int dtype=-1);

//! computes element-wise weighted product of the two arrays (dst = scale*src1*src2)
VS_EXPORTS_W void multiply(InputArray src1, InputArray src2,
                           OutputArray dst, double scale=1, int dtype=-1);

//! computes element-wise weighted quotient of the two arrays (dst = scale*src1/src2)
VS_EXPORTS_W void divide(InputArray src1, InputArray src2, OutputArray dst,
                         double scale=1, int dtype=-1);

//! computes element-wise weighted reciprocal of an array (dst = scale/src2)
VS_EXPORTS_W void divide(double scale, InputArray src2,
                         OutputArray dst, int dtype=-1);

//! adds scaled array to another one (dst = alpha*src1 + src2)
VS_EXPORTS_W void scaleAdd(InputArray src1, double alpha, InputArray src2, OutputArray dst);

//! computes weighted sum of two arrays (dst = alpha*src1 + beta*src2 + gamma)
VS_EXPORTS_W void addWeighted(InputArray src1, double alpha, InputArray src2,
                              double beta, double gamma, OutputArray dst, int dtype=-1);

//! scales array elements, computes absolute values and converts the results to 8-bit unsigned integers: dst(i)=saturate_cast<uchar>abs(src(i)*alpha+beta)
VS_EXPORTS_W void convertScaleAbs(InputArray src, OutputArray dst,
                                  double alpha=1, double beta=0);
//! transforms array of numbers using a lookup table: dst(i)=lut(src(i))
VS_EXPORTS_W void LUT(InputArray src, InputArray lut, OutputArray dst,
                      int interpolation=0);

//! computes sum of array elements
VS_EXPORTS_AS(sumElems) Scalar sum(InputArray src);
//! computes the number of nonzero array elements
VS_EXPORTS_W int countNonZero( InputArray src );
//! returns the list of locations of non-zero pixels
VS_EXPORTS_W void findNonZero( InputArray src, OutputArray idx );

//! computes mean value of selected array elements
VS_EXPORTS_W Scalar mean(InputArray src, InputArray mask=noArray());
//! computes mean value and standard deviation of all or selected array elements
VS_EXPORTS_W void meanStdDev(InputArray src, OutputArray mean, OutputArray stddev,
                             InputArray mask=noArray());
//! computes norm of the selected array part
VS_EXPORTS_W double norm(InputArray src1, int normType=NORM_L2, InputArray mask=noArray());
//! computes norm of selected part of the difference between two arrays
VS_EXPORTS_W double norm(InputArray src1, InputArray src2,
                         int normType=NORM_L2, InputArray mask=noArray());

//! naive nearest neighbor finder
VS_EXPORTS_W void batchDistance(InputArray src1, InputArray src2,
                                OutputArray dist, int dtype, OutputArray nidx,
                                int normType=NORM_L2, int K=0,
                                InputArray mask=noArray(), int update=0,
                                bool crosscheck=false);

//! scales and shifts array elements so that either the specified norm (alpha) or the minimum (alpha) and maximum (beta) array values get the specified values
VS_EXPORTS_W void normalize( InputArray src, OutputArray dst, double alpha=1, double beta=0,
                             int norm_type=NORM_L2, int dtype=-1, InputArray mask=noArray());

//! finds global minimum and maximum array elements and returns their values and their locations
VS_EXPORTS_W void minMaxLoc(InputArray src, VS_OUT double* minVal,
                           VS_OUT double* maxVal=0, VS_OUT Point* minLoc=0,
                           VS_OUT Point* maxLoc=0, InputArray mask=noArray());
VS_EXPORTS void minMaxIdx(InputArray src, double* minVal, double* maxVal,
                          int* minIdx=0, int* maxIdx=0, InputArray mask=noArray());

//! transforms 2D matrix to 1D row or column vector by taking sum, minimum, maximum or mean value over all the rows
VS_EXPORTS_W void reduce(InputArray src, OutputArray dst, int dim, int rtype, int dtype=-1);

//! makes multi-channel array out of several single-channel arrays
VS_EXPORTS void merge(const Mat* mv, size_t count, OutputArray dst);
VS_EXPORTS void merge(const vector<Mat>& mv, OutputArray dst );

//! makes multi-channel array out of several single-channel arrays
VS_EXPORTS_W void merge(InputArrayOfArrays mv, OutputArray dst);

//! copies each plane of a multi-channel array to a dedicated array
VS_EXPORTS void split(const Mat& src, Mat* mvbegin);
VS_EXPORTS void split(const Mat& m, vector<Mat>& mv );

//! copies each plane of a multi-channel array to a dedicated array
VS_EXPORTS_W void split(InputArray m, OutputArrayOfArrays mv);

//! copies selected channels from the input arrays to the selected channels of the output arrays
VS_EXPORTS void mixChannels(const Mat* src, size_t nsrcs, Mat* dst, size_t ndsts,
                            const int* fromTo, size_t npairs);
VS_EXPORTS void mixChannels(const vector<Mat>& src, vector<Mat>& dst,
                            const int* fromTo, size_t npairs);
VS_EXPORTS_W void mixChannels(InputArrayOfArrays src, InputArrayOfArrays dst,
                              const vector<int>& fromTo);

//! extracts a single channel from src (coi is 0-based index)
VS_EXPORTS_W void extractChannel(InputArray src, OutputArray dst, int coi);

//! inserts a single channel to dst (coi is 0-based index)
VS_EXPORTS_W void insertChannel(InputArray src, InputOutputArray dst, int coi);

//! reverses the order of the rows, columns or both in a matrix
VS_EXPORTS_W void flip(InputArray src, OutputArray dst, int flipCode);

//! replicates the input matrix the specified number of times in the horizontal and/or vertical direction
VS_EXPORTS_W void repeat(InputArray src, int ny, int nx, OutputArray dst);
VS_EXPORTS Mat repeat(const Mat& src, int ny, int nx);

VS_EXPORTS void hconcat(const Mat* src, size_t nsrc, OutputArray dst);
VS_EXPORTS void hconcat(InputArray src1, InputArray src2, OutputArray dst);
VS_EXPORTS_W void hconcat(InputArrayOfArrays src, OutputArray dst);

VS_EXPORTS void vconcat(const Mat* src, size_t nsrc, OutputArray dst);
VS_EXPORTS void vconcat(InputArray src1, InputArray src2, OutputArray dst);
VS_EXPORTS_W void vconcat(InputArrayOfArrays src, OutputArray dst);

//! computes bitwise conjunction of the two arrays (dst = src1 & src2)
VS_EXPORTS_W void bitwise_and(InputArray src1, InputArray src2,
                              OutputArray dst, InputArray mask=noArray());
//! computes bitwise disjunction of the two arrays (dst = src1 | src2)
VS_EXPORTS_W void bitwise_or(InputArray src1, InputArray src2,
                             OutputArray dst, InputArray mask=noArray());
//! computes bitwise exclusive-or of the two arrays (dst = src1 ^ src2)
VS_EXPORTS_W void bitwise_xor(InputArray src1, InputArray src2,
                              OutputArray dst, InputArray mask=noArray());
//! inverts each bit of array (dst = ~src)
VS_EXPORTS_W void bitwise_not(InputArray src, OutputArray dst,
                              InputArray mask=noArray());
//! computes element-wise absolute difference of two arrays (dst = abs(src1 - src2))
VS_EXPORTS_W void absdiff(InputArray src1, InputArray src2, OutputArray dst);
//! set mask elements for those array elements which are within the element-specific bounding box (dst = lowerb <= src && src < upperb)
VS_EXPORTS_W void inRange(InputArray src, InputArray lowerb,
                          InputArray upperb, OutputArray dst);
//! compares elements of two arrays (dst = src1 \<cmpop\> src2)
VS_EXPORTS_W void compare(InputArray src1, InputArray src2, OutputArray dst, int cmpop);
//! computes per-element minimum of two arrays (dst = min(src1, src2))
VS_EXPORTS_W void min(InputArray src1, InputArray src2, OutputArray dst);
//! computes per-element maximum of two arrays (dst = max(src1, src2))
VS_EXPORTS_W void max(InputArray src1, InputArray src2, OutputArray dst);

//! computes per-element minimum of two arrays (dst = min(src1, src2))
VS_EXPORTS void min(const Mat& src1, const Mat& src2, Mat& dst);
//! computes per-element minimum of array and scalar (dst = min(src1, src2))
VS_EXPORTS void min(const Mat& src1, double src2, Mat& dst);
//! computes per-element maximum of two arrays (dst = max(src1, src2))
VS_EXPORTS void max(const Mat& src1, const Mat& src2, Mat& dst);
//! computes per-element maximum of array and scalar (dst = max(src1, src2))
VS_EXPORTS void max(const Mat& src1, double src2, Mat& dst);

//! computes square root of each matrix element (dst = src**0.5)
VS_EXPORTS_W void sqrt(InputArray src, OutputArray dst);
//! raises the input matrix elements to the specified power (b = a**power)
VS_EXPORTS_W void pow(InputArray src, double power, OutputArray dst);
//! computes exponent of each matrix element (dst = e**src)
VS_EXPORTS_W void exp(InputArray src, OutputArray dst);
//! computes natural logarithm of absolute value of each matrix element: dst = log(abs(src))
VS_EXPORTS_W void log(InputArray src, OutputArray dst);
//! computes cube root of the argument
VS_EXPORTS_W float cubeRoot(float val);
//! computes the angle in degrees (0..360) of the vector (x,y)
VS_EXPORTS_W float fastAtan2(float y, float x);

VS_EXPORTS void exp(const float* src, float* dst, int n);
VS_EXPORTS void log(const float* src, float* dst, int n);
VS_EXPORTS void fastAtan2(const float* y, const float* x, float* dst, int n, bool angleInDegrees);
VS_EXPORTS void magnitude(const float* x, const float* y, float* dst, int n);

//! converts polar coordinates to Cartesian
VS_EXPORTS_W void polarToCart(InputArray magnitude, InputArray angle,
                              OutputArray x, OutputArray y, bool angleInDegrees=false);
//! converts Cartesian coordinates to polar
VS_EXPORTS_W void cartToPolar(InputArray x, InputArray y,
                              OutputArray magnitude, OutputArray angle,
                              bool angleInDegrees=false);
//! computes angle (angle(i)) of each (x(i), y(i)) vector
VS_EXPORTS_W void phase(InputArray x, InputArray y, OutputArray angle,
                        bool angleInDegrees=false);
//! computes magnitude (magnitude(i)) of each (x(i), y(i)) vector
VS_EXPORTS_W void magnitude(InputArray x, InputArray y, OutputArray magnitude);
//! checks that each matrix element is within the specified range.
VS_EXPORTS_W bool checkRange(InputArray a, bool quiet=true, VS_OUT Point* pos=0,
                            double minVal=-DBL_MAX, double maxVal=DBL_MAX);
//! converts NaN's to the given number
VS_EXPORTS_W void patchNaNs(InputOutputArray a, double val=0);

//! implements generalized matrix product algorithm GEMM from BLAS
VS_EXPORTS_W void gemm(InputArray src1, InputArray src2, double alpha,
                       InputArray src3, double beta, OutputArray dst, int flags=0);
//! multiplies matrix by its transposition from the left or from the right
VS_EXPORTS_W void mulTransposed( InputArray src, OutputArray dst, bool aTa,
                                 InputArray delta=noArray(),
                                 double scale=1, int dtype=-1 );
//! transposes the matrix
VS_EXPORTS_W void transpose(InputArray src, OutputArray dst);
//! performs affine transformation of each element of multi-channel input matrix
VS_EXPORTS_W void transform(InputArray src, OutputArray dst, InputArray m );
//! performs perspective transformation of each element of multi-channel input matrix
VS_EXPORTS_W void perspectiveTransform(InputArray src, OutputArray dst, InputArray m );

//! extends the symmetrical matrix from the lower half or from the upper half
VS_EXPORTS_W void completeSymm(InputOutputArray mtx, bool lowerToUpper=false);
//! initializes scaled identity matrix
VS_EXPORTS_W void setIdentity(InputOutputArray mtx, const Scalar& s=Scalar(1));
//! computes determinant of a square matrix
VS_EXPORTS_W double determinant(InputArray mtx);
//! computes trace of a matrix
VS_EXPORTS_W Scalar trace(InputArray mtx);
//! computes inverse or pseudo-inverse matrix
VS_EXPORTS_W double invert(InputArray src, OutputArray dst, int flags=DECOMP_LU);
//! solves linear system or a least-square problem
VS_EXPORTS_W bool solve(InputArray src1, InputArray src2,
                        OutputArray dst, int flags=DECOMP_LU);

enum
{
    SORT_EVERY_ROW=0,
    SORT_EVERY_COLUMN=1,
    SORT_ASCENDING=0,
    SORT_DESCENDING=16
};

//! sorts independently each matrix row or each matrix column
VS_EXPORTS_W void sort(InputArray src, OutputArray dst, int flags);
//! sorts independently each matrix row or each matrix column
VS_EXPORTS_W void sortIdx(InputArray src, OutputArray dst, int flags);
//! finds real roots of a cubic polynomial
VS_EXPORTS_W int solveCubic(InputArray coeffs, OutputArray roots);
//! finds real and complex roots of a polynomial
VS_EXPORTS_W double solvePoly(InputArray coeffs, OutputArray roots, int maxIters=300);
//! finds eigenvalues of a symmetric matrix
VS_EXPORTS bool eigen(InputArray src, OutputArray eigenvalues, int lowindex=-1,
                      int highindex=-1);
//! finds eigenvalues and eigenvectors of a symmetric matrix
VS_EXPORTS bool eigen(InputArray src, OutputArray eigenvalues,
                      OutputArray eigenvectors,
                      int lowindex=-1, int highindex=-1);
VS_EXPORTS_W bool eigen(InputArray src, bool computeEigenvectors,
                        OutputArray eigenvalues, OutputArray eigenvectors);

enum
{
    COVAR_SCRAMBLED=0,
    COVAR_NORMAL=1,
    COVAR_USE_AVG=2,
    COVAR_SCALE=4,
    COVAR_ROWS=8,
    COVAR_COLS=16
};

//! computes covariation matrix of a set of samples
VS_EXPORTS void calcCovarMatrix( const Mat* samples, int nsamples, Mat& covar, Mat& mean,
                                 int flags, int ctype=VS_64F);
//! computes covariation matrix of a set of samples
VS_EXPORTS_W void calcCovarMatrix( InputArray samples, OutputArray covar,
                                   OutputArray mean, int flags, int ctype=VS_64F);

/*!
    Principal Component Analysis

    The class PCA is used to compute the special basis for a set of vectors.
    The basis will consist of eigenvectors of the covariance matrix computed
    from the input set of vectors. After PCA is performed, vectors can be transformed from
    the original high-dimensional space to the subspace formed by a few most
    prominent eigenvectors (called the principal components),
    corresponding to the largest eigenvalues of the covariation matrix.
    Thus the dimensionality of the vector and the correlation between the coordinates is reduced.

    The following sample is the function that takes two matrices. The first one stores the set
    of vectors (a row per vector) that is used to compute PCA, the second one stores another
    "test" set of vectors (a row per vector) that are first compressed with PCA,
    then reconstructed back and then the reconstruction error norm is computed and printed for each vector.

    \code
    using namespace vs;

    PCA compressPCA(const Mat& pcaset, int maxComponents,
                    const Mat& testset, Mat& compressed)
    {
        PCA pca(pcaset, // pass the data
                Mat(), // we do not have a pre-computed mean vector,
                       // so let the PCA engine to compute it
                VS_PCA_DATA_AS_ROW, // indicate that the vectors
                                    // are stored as matrix rows
                                    // (use VS_PCA_DATA_AS_COL if the vectors are
                                    // the matrix columns)
                maxComponents // specify, how many principal components to retain
                );
        // if there is no test data, just return the computed basis, ready-to-use
        if( !testset.data )
            return pca;
        VS_Assert( testset.cols == pcaset.cols );

        compressed.create(testset.rows, maxComponents, testset.type());

        Mat reconstructed;
        for( int i = 0; i < testset.rows; i++ )
        {
            Mat vec = testset.row(i), coeffs = compressed.row(i), reconstructed;
            // compress the vector, the result will be stored
            // in the i-th row of the output matrix
            pca.project(vec, coeffs);
            // and then reconstruct it
            pca.backProject(coeffs, reconstructed);
            // and measure the error
            printf("%d. diff = %g\n", i, norm(vec, reconstructed, NORM_L2));
        }
        return pca;
    }
    \endcode
*/
/*
class VS_EXPORTS PCA
{
public:
    //! default constructor
    PCA();
    //! the constructor that performs PCA
    PCA(InputArray data, InputArray mean, int flags, int maxComponents=0);
    PCA(InputArray data, InputArray mean, int flags, double retainedVariance);
    //! operator that performs PCA. The previously stored data, if any, is released
    PCA& operator()(InputArray data, InputArray mean, int flags, int maxComponents=0);
    PCA& computeVar(InputArray data, InputArray mean, int flags, double retainedVariance);
    //! projects vector from the original space to the principal components subspace
    Mat project(InputArray vec) const;
    //! projects vector from the original space to the principal components subspace
    void project(InputArray vec, OutputArray result) const;
    //! reconstructs the original vector from the projection
    Mat backProject(InputArray vec) const;
    //! reconstructs the original vector from the projection
    void backProject(InputArray vec, OutputArray result) const;

    Mat eigenvectors; //!< eigenvectors of the covariation matrix
    Mat eigenvalues; //!< eigenvalues of the covariation matrix
    Mat mean; //!< mean value subtracted before the projection and added after the back projection
};

VS_EXPORTS_W void PCACompute(InputArray data, VS_OUT InputOutputArray mean,
                             OutputArray eigenvectors, int maxComponents=0);

VS_EXPORTS_W void PCAComputeVar(InputArray data, VS_OUT InputOutputArray mean,
                             OutputArray eigenvectors, double retainedVariance);

VS_EXPORTS_W void PCAProject(InputArray data, InputArray mean,
                             InputArray eigenvectors, OutputArray result);

VS_EXPORTS_W void PCABackProject(InputArray data, InputArray mean,
                                 InputArray eigenvectors, OutputArray result);
*/

/*!
    Singular Value Decomposition class

    The class is used to compute Singular Value Decomposition of a floating-point matrix and then
    use it to solve least-square problems, under-determined linear systems, invert matrices,
    compute condition numbers etc.

    For a bit faster operation you can pass flags=SVD::MODIFY_A|... to modify the decomposed matrix
    when it is not necessarily to preserve it. If you want to compute condition number of a matrix
    or absolute value of its determinant - you do not need SVD::u or SVD::vt,
    so you can pass flags=SVD::NO_UV|... . Another flag SVD::FULL_UV indicates that the full-size SVD::u and SVD::vt
    must be computed, which is not necessary most of the time.
*/
class VS_EXPORTS SVD
{
public:
    enum { MODIFY_A=1, NO_UV=2, FULL_UV=4 };
    //! the default constructor
    SVD();
    //! the constructor that performs SVD
    SVD( InputArray src, int flags=0 );
    //! the operator that performs SVD. The previously allocated SVD::u, SVD::w are SVD::vt are released.
    SVD& operator ()( InputArray src, int flags=0 );

    //! decomposes matrix and stores the results to user-provided matrices
    static void compute( InputArray src, OutputArray w,
                         OutputArray u, OutputArray vt, int flags=0 );
    //! computes singular values of a matrix
    static void compute( InputArray src, OutputArray w, int flags=0 );
    //! performs back substitution
    static void backSubst( InputArray w, InputArray u,
                           InputArray vt, InputArray rhs,
                           OutputArray dst );

    template<typename _Tp, int m, int n, int nm> static void compute( const Matx<_Tp, m, n>& a,
        Matx<_Tp, nm, 1>& w, Matx<_Tp, m, nm>& u, Matx<_Tp, n, nm>& vt );
    template<typename _Tp, int m, int n, int nm> static void compute( const Matx<_Tp, m, n>& a,
        Matx<_Tp, nm, 1>& w );
    template<typename _Tp, int m, int n, int nm, int nb> static void backSubst( const Matx<_Tp, nm, 1>& w,
        const Matx<_Tp, m, nm>& u, const Matx<_Tp, n, nm>& vt, const Matx<_Tp, m, nb>& rhs, Matx<_Tp, n, nb>& dst );

    //! finds dst = arg min_{|dst|=1} |m*dst|
    static void solveZ( InputArray src, OutputArray dst );
    //! performs back substitution, so that dst is the solution or pseudo-solution of m*dst = rhs, where m is the decomposed matrix
    void backSubst( InputArray rhs, OutputArray dst ) const;

    Mat u, w, vt;
};

//! computes SVD of src
VS_EXPORTS_W void SVDecomp( InputArray src, VS_OUT OutputArray w,
    VS_OUT OutputArray u, VS_OUT OutputArray vt, int flags=0 );

//! performs back substitution for the previously computed SVD
VS_EXPORTS_W void SVBackSubst( InputArray w, InputArray u, InputArray vt,
                               InputArray rhs, VS_OUT OutputArray dst );

//! computes Mahalanobis distance between two vectors: sqrt((v1-v2)'*icovar*(v1-v2)), where icovar is the inverse covariation matrix
VS_EXPORTS_W double Mahalanobis(InputArray v1, InputArray v2, InputArray icovar);
//! a synonym for Mahalanobis
VS_EXPORTS double Mahalonobis(InputArray v1, InputArray v2, InputArray icovar);

//! performs forward or inverse 1D or 2D Discrete Fourier Transformation
VS_EXPORTS_W void dft(InputArray src, OutputArray dst, int flags=0, int nonzeroRows=0);
//! performs inverse 1D or 2D Discrete Fourier Transformation
VS_EXPORTS_W void idft(InputArray src, OutputArray dst, int flags=0, int nonzeroRows=0);
//! performs forward or inverse 1D or 2D Discrete Cosine Transformation
VS_EXPORTS_W void dct(InputArray src, OutputArray dst, int flags=0);
//! performs inverse 1D or 2D Discrete Cosine Transformation
VS_EXPORTS_W void idct(InputArray src, OutputArray dst, int flags=0);
//! computes element-wise product of the two Fourier spectrums. The second spectrum can optionally be conjugated before the multiplication
VS_EXPORTS_W void mulSpectrums(InputArray a, InputArray b, OutputArray c,
                               int flags, bool conjB=false);
//! computes the minimal vector size vecsize1 >= vecsize so that the dft() of the vector of length vecsize1 can be computed efficiently
VS_EXPORTS_W int getOptimalDFTSize(int vecsize);

/*!
 Various k-Means flags
*/
enum
{
    KMEANS_RANDOM_CENTERS=0, // Chooses random centers for k-Means initialization
    KMEANS_PP_CENTERS=2,     // Uses k-Means++ algorithm for initialization
    KMEANS_USE_INITIAL_LABELS=1 // Uses the user-provided labels for K-Means initialization
};
//! clusters the input data using k-Means algorithm
VS_EXPORTS_W double kmeans( InputArray data, int K, VS_OUT InputOutputArray bestLabels,
                            TermCriteria criteria, int attempts,
                            int flags, OutputArray centers=noArray() );

//! returns the thread-local Random number generator
VS_EXPORTS RNG& theRNG();

//! returns the next unifomly-distributed random number of the specified type
template<typename _Tp> static inline _Tp randu() { return (_Tp)theRNG(); }

//! fills array with uniformly-distributed random numbers from the range [low, high)
VS_EXPORTS_W void randu(InputOutputArray dst, InputArray low, InputArray high);

//! fills array with normally-distributed random numbers with the specified mean and the standard deviation
VS_EXPORTS_W void randn(InputOutputArray dst, InputArray mean, InputArray stddev);

//! shuffles the input array elements
VS_EXPORTS void randShuffle(InputOutputArray dst, double iterFactor=1., RNG* rng=0);
VS_EXPORTS_AS(randShuffle) void randShuffle_(InputOutputArray dst, double iterFactor=1.);

//! draws the line segment (pt1, pt2) in the image
VS_EXPORTS_W void line(VS_IN_OUT Mat& img, Point pt1, Point pt2, const Scalar& color,
                     int thickness=1, int lineType=8, int shift=0);

//! draws an arrow from pt1 to pt2 in the image
VS_EXPORTS_W void arrowedLine(VS_IN_OUT Mat& img, Point pt1, Point pt2, const Scalar& color,
                     int thickness=1, int line_type=8, int shift=0, double tipLength=0.1);

//! draws the rectangle outline or a solid rectangle with the opposite corners pt1 and pt2 in the image
VS_EXPORTS_W void rectangle(VS_IN_OUT Mat& img, Point pt1, Point pt2,
                          const Scalar& color, int thickness=1,
                          int lineType=8, int shift=0);

//! draws the rectangle outline or a solid rectangle covering rec in the image
VS_EXPORTS void rectangle(VS_IN_OUT Mat& img, Rect rec,
                          const Scalar& color, int thickness=1,
                          int lineType=8, int shift=0);

//! draws the circle outline or a solid circle in the image
VS_EXPORTS_W void circle(VS_IN_OUT Mat& img, Point center, int radius,
                       const Scalar& color, int thickness=1,
                       int lineType=8, int shift=0);

//! draws an elliptic arc, ellipse sector or a rotated ellipse in the image
VS_EXPORTS_W void ellipse(VS_IN_OUT Mat& img, Point center, Size axes,
                        double angle, double startAngle, double endAngle,
                        const Scalar& color, int thickness=1,
                        int lineType=8, int shift=0);

//! draws a rotated ellipse in the image
VS_EXPORTS_W void ellipse(VS_IN_OUT Mat& img, const RotatedRect& box, const Scalar& color,
                        int thickness=1, int lineType=8);

//! draws a filled convex polygon in the image
VS_EXPORTS void fillConvexPoly(Mat& img, const Point* pts, int npts,
                               const Scalar& color, int lineType=8,
                               int shift=0);
VS_EXPORTS_W void fillConvexPoly(InputOutputArray img, InputArray points,
                                 const Scalar& color, int lineType=8,
                                 int shift=0);

//! fills an area bounded by one or more polygons
VS_EXPORTS void fillPoly(Mat& img, const Point** pts,
                         const int* npts, int ncontours,
                         const Scalar& color, int lineType=8, int shift=0,
                         Point offset=Point() );

VS_EXPORTS_W void fillPoly(InputOutputArray img, InputArrayOfArrays pts,
                           const Scalar& color, int lineType=8, int shift=0,
                           Point offset=Point() );

//! draws one or more polygonal curves
VS_EXPORTS void polylines(Mat& img, const Point** pts, const int* npts,
                          int ncontours, bool isClosed, const Scalar& color,
                          int thickness=1, int lineType=8, int shift=0 );

VS_EXPORTS_W void polylines(InputOutputArray img, InputArrayOfArrays pts,
                            bool isClosed, const Scalar& color,
                            int thickness=1, int lineType=8, int shift=0 );

//! clips the line segment by the rectangle Rect(0, 0, imgSize.width, imgSize.height)
VS_EXPORTS bool clipLine(Size imgSize, VS_IN_OUT Point& pt1, VS_IN_OUT Point& pt2);

//! clips the line segment by the rectangle imgRect
VS_EXPORTS_W bool clipLine(Rect imgRect, VS_OUT VS_IN_OUT Point& pt1, VS_OUT VS_IN_OUT Point& pt2);

/*!
   Line iterator class

   The class is used to iterate over all the pixels on the raster line
   segment connecting two specified points.
*/
class VS_EXPORTS LineIterator
{
public:
    //! intializes the iterator
    LineIterator( const Mat& img, Point pt1, Point pt2,
                  int connectivity=8, bool leftToRight=false );
    //! returns pointer to the current pixel
    uchar* operator *();
    //! prefix increment operator (++it). shifts iterator to the next pixel
    LineIterator& operator ++();
    //! postfix increment operator (it++). shifts iterator to the next pixel
    LineIterator operator ++(int);
    //! returns coordinates of the current pixel
    Point pos() const;

    uchar* ptr;
    const uchar* ptr0;
    int step, elemSize;
    int err, count;
    int minusDelta, plusDelta;
    int minusStep, plusStep;
};

//! converts elliptic arc to a polygonal curve
VS_EXPORTS_W void ellipse2Poly( Point center, Size axes, int angle,
                                int arcStart, int arcEnd, int delta,
                                VS_OUT vector<Point>& pts );

enum
{
    FONT_HERSHEY_SIMPLEX = 0,
    FONT_HERSHEY_PLAIN = 1,
    FONT_HERSHEY_DUPLEX = 2,
    FONT_HERSHEY_COMPLEX = 3,
    FONT_HERSHEY_TRIPLEX = 4,
    FONT_HERSHEY_COMPLEX_SMALL = 5,
    FONT_HERSHEY_SCRIPT_SIMPLEX = 6,
    FONT_HERSHEY_SCRIPT_COMPLEX = 7,
    FONT_ITALIC = 16
};

//! renders text string in the image
VS_EXPORTS_W void putText( Mat& img, const string& text, Point org,
                         int fontFace, double fontScale, Scalar color,
                         int thickness=1, int lineType=8,
                         bool bottomLeftOrigin=false );

//! returns bounding box of the text string
VS_EXPORTS_W Size getTextSize(const string& text, int fontFace,
                            double fontScale, int thickness,
                            VS_OUT int* baseLine);

///////////////////////////////// Mat_<_Tp> ////////////////////////////////////

/*!
 Template matrix class derived from Mat

 The class Mat_ is a "thin" template wrapper on top of vs::Mat. It does not have any extra data fields,
 nor it or vs::Mat have any virtual methods and thus references or pointers to these two classes
 can be safely converted one to another. But do it with care, for example:

 \code
 // create 100x100 8-bit matrix
 Mat M(100,100,VS_8U);
 // this will compile fine. no any data conversion will be done.
 Mat_<float>& M1 = (Mat_<float>&)M;
 // the program will likely crash at the statement below
 M1(99,99) = 1.f;
 \endcode

 While vs::Mat is sufficient in most cases, vs::Mat_ can be more convenient if you use a lot of element
 access operations and if you know matrix type at compile time.
 Note that vs::Mat::at\<_Tp\>(int y, int x) and vs::Mat_\<_Tp\>::operator ()(int y, int x) do absolutely the
 same thing and run at the same speed, but the latter is certainly shorter:

 \code
 Mat_<double> M(20,20);
 for(int i = 0; i < M.rows; i++)
    for(int j = 0; j < M.cols; j++)
       M(i,j) = 1./(i+j+1);
 Mat E, V;
 eigen(M,E,V);
 cout << E.at<double>(0,0)/E.at<double>(M.rows-1,0);
 \endcode

 It is easy to use Mat_ for multi-channel images/matrices - just pass vs::Vec as vs::Mat_ template parameter:

 \code
 // allocate 320x240 color image and fill it with green (in RGB space)
 Mat_<Vec3b> img(240, 320, Vec3b(0,255,0));
 // now draw a diagonal white line
 for(int i = 0; i < 100; i++)
     img(i,i)=Vec3b(255,255,255);
 // and now modify the 2nd (red) channel of each pixel
 for(int i = 0; i < img.rows; i++)
    for(int j = 0; j < img.cols; j++)
       img(i,j)[2] ^= (uchar)(i ^ j); // img(y,x)[c] accesses c-th channel of the pixel (x,y)
 \endcode
*/
template<typename _Tp> class Mat_ : public Mat
{
public:
    typedef _Tp value_type;
    typedef typename DataType<_Tp>::channel_type channel_type;
    typedef MatIterator_<_Tp> iterator;

    //! default constructor
    Mat_();
    //! equivalent to Mat(_rows, _cols, DataType<_Tp>::type)
    Mat_(int _rows, int _cols);
    //! constructor that sets each matrix element to specified value
    Mat_(int _rows, int _cols, const _Tp& value);
    //! equivalent to Mat(_size, DataType<_Tp>::type)
    explicit Mat_(Size _size);
    //! constructor that sets each matrix element to specified value
    Mat_(Size _size, const _Tp& value);
    //! n-dim array constructor
    Mat_(int _ndims, const int* _sizes);
    //! n-dim array constructor that sets each matrix element to specified value
    Mat_(int _ndims, const int* _sizes, const _Tp& value);
    //! copy/conversion contructor. If m is of different type, it's converted
    Mat_(const Mat& m);
    //! copy constructor
    Mat_(const Mat_& m);
    //! constructs a matrix on top of user-allocated data. step is in bytes(!!!), regardless of the type
    Mat_(int _rows, int _cols, _Tp* _data, size_t _step=AUTO_STEP);
    //! constructs n-dim matrix on top of user-allocated data. steps are in bytes(!!!), regardless of the type
    Mat_(int _ndims, const int* _sizes, _Tp* _data, const size_t* _steps=0);
    //! selects a submatrix
    Mat_(const Mat_& m, const Range& rowRange, const Range& colRange=Range::all());
    //! selects a submatrix
    Mat_(const Mat_& m, const Rect& roi);
    //! selects a submatrix, n-dim version
    Mat_(const Mat_& m, const Range* ranges);
    //! from a matrix expression
    explicit Mat_(const MatExpr& e);
    //! makes a matrix out of Vec, std::vector, Point_ or Point3_. The matrix will have a single column
    explicit Mat_(const vector<_Tp>& vec, bool copyData=false);
    template<int n> explicit Mat_(const Vec<typename DataType<_Tp>::channel_type, n>& vec, bool copyData=true);
    template<int m, int n> explicit Mat_(const Matx<typename DataType<_Tp>::channel_type, m, n>& mtx, bool copyData=true);
    explicit Mat_(const Point_<typename DataType<_Tp>::channel_type>& pt, bool copyData=true);
    explicit Mat_(const Point3_<typename DataType<_Tp>::channel_type>& pt, bool copyData=true);
    explicit Mat_(const MatCommaInitializer_<_Tp>& commaInitializer);

    Mat_& operator = (const Mat& m);
    Mat_& operator = (const Mat_& m);
    //! set all the elements to s.
    Mat_& operator = (const _Tp& s);
    //! assign a matrix expression
    Mat_& operator = (const MatExpr& e);

    //! iterators; they are smart enough to skip gaps in the end of rows
    iterator begin();
    iterator end();
//    const_iterator begin() const;
//    const_iterator end() const;

    //! equivalent to Mat::create(_rows, _cols, DataType<_Tp>::type)
    void create(int _rows, int _cols);
    //! equivalent to Mat::create(_size, DataType<_Tp>::type)
    void create(Size _size);
    //! equivalent to Mat::create(_ndims, _sizes, DatType<_Tp>::type)
    void create(int _ndims, const int* _sizes);
    //! cross-product
    Mat_ cross(const Mat_& m) const;
    //! data type conversion
    template<typename T2> operator Mat_<T2>() const;
    //! overridden forms of Mat::row() etc.
    Mat_ row(int y) const;
    Mat_ col(int x) const;
    Mat_ diag(int d=0) const;
    Mat_ clone() const;

    //! overridden forms of Mat::elemSize() etc.
    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t step1(int i=0) const;
    //! returns step()/sizeof(_Tp)
    size_t stepT(int i=0) const;

    //! overridden forms of Mat::zeros() etc. Data type is omitted, of course
    static MatExpr zeros(int rows, int cols);
    static MatExpr zeros(Size size);
    static MatExpr zeros(int _ndims, const int* _sizes);
    static MatExpr ones(int rows, int cols);
    static MatExpr ones(Size size);
    static MatExpr ones(int _ndims, const int* _sizes);
    static MatExpr eye(int rows, int cols);
    static MatExpr eye(Size size);

    //! some more overriden methods
    Mat_& adjustROI( int dtop, int dbottom, int dleft, int dright );
    Mat_ operator()( const Range& rowRange, const Range& colRange ) const;
    Mat_ operator()( const Rect& roi ) const;
    Mat_ operator()( const Range* ranges ) const;

    //! more convenient forms of row and element access operators
    _Tp* operator [](int y);
    const _Tp* operator [](int y) const;

    //! returns reference to the specified element
    _Tp& operator ()(const int* idx);
    //! returns read-only reference to the specified element
    const _Tp& operator ()(const int* idx) const;

    //! returns reference to the specified element
    template<int n> _Tp& operator ()(const Vec<int, n>& idx);
    //! returns read-only reference to the specified element
    template<int n> const _Tp& operator ()(const Vec<int, n>& idx) const;

    //! returns reference to the specified element (1D case)
    _Tp& operator ()(int idx0);
    //! returns read-only reference to the specified element (1D case)
    const _Tp& operator ()(int idx0) const;
    //! returns reference to the specified element (2D case)
    _Tp& operator ()(int idx0, int idx1);
    //! returns read-only reference to the specified element (2D case)
    const _Tp& operator ()(int idx0, int idx1) const;
    //! returns reference to the specified element (3D case)
    _Tp& operator ()(int idx0, int idx1, int idx2);
    //! returns read-only reference to the specified element (3D case)
    const _Tp& operator ()(int idx0, int idx1, int idx2) const;

    _Tp& operator ()(Point pt);
    const _Tp& operator ()(Point pt) const;

    //! conversion to vector.
    operator vector<_Tp>() const;
    //! conversion to Vec
    template<int n> operator Vec<typename DataType<_Tp>::channel_type, n>() const;
    //! conversion to Matx
    template<int m, int n> operator Matx<typename DataType<_Tp>::channel_type, m, n>() const;
};

typedef Mat_<uchar> Mat1b;
typedef Mat_<Vec2b> Mat2b;
typedef Mat_<Vec3b> Mat3b;
typedef Mat_<Vec4b> Mat4b;

typedef Mat_<short> Mat1s;
typedef Mat_<Vec2s> Mat2s;
typedef Mat_<Vec3s> Mat3s;
typedef Mat_<Vec4s> Mat4s;

typedef Mat_<ushort> Mat1w;
typedef Mat_<Vec2w> Mat2w;
typedef Mat_<Vec3w> Mat3w;
typedef Mat_<Vec4w> Mat4w;

typedef Mat_<int>   Mat1i;
typedef Mat_<Vec2i> Mat2i;
typedef Mat_<Vec3i> Mat3i;
typedef Mat_<Vec4i> Mat4i;

typedef Mat_<float> Mat1f;
typedef Mat_<Vec2f> Mat2f;
typedef Mat_<Vec3f> Mat3f;
typedef Mat_<Vec4f> Mat4f;

typedef Mat_<double> Mat1d;
typedef Mat_<Vec2d> Mat2d;
typedef Mat_<Vec3d> Mat3d;
typedef Mat_<Vec4d> Mat4d;

//////////// Iterators & Comma initializers //////////////////

/*!
 Comma-separated Matrix Initializer

 The class instances are usually not created explicitly.
 Instead, they are created on "matrix << firstValue" operator.

 The sample below initializes 2x2 rotation matrix:

 \code
 double angle = 30, a = cos(angle*VS_PI/180), b = sin(angle*VS_PI/180);
 Mat R = (Mat_<double>(2,2) << a, -b, b, a);
 \endcode
*/
template<typename _Tp> class MatCommaInitializer_
{
public:
    //! the constructor, created by "matrix << firstValue" operator, where matrix is vs::Mat
    MatCommaInitializer_(Mat_<_Tp>* _m);
    //! the operator that takes the next value and put it to the matrix
    template<typename T2> MatCommaInitializer_<_Tp>& operator , (T2 v);
    //! another form of conversion operator
    Mat_<_Tp> operator *() const;
    operator Mat_<_Tp>() const;
protected:
    MatIterator_<_Tp> it;
};


template<typename _Tp, int m, int n> class MatxCommaInitializer
{
public:
    MatxCommaInitializer(Matx<_Tp, m, n>* _mtx);
    template<typename T2> MatxCommaInitializer<_Tp, m, n>& operator , (T2 val);
    Matx<_Tp, m, n> operator *() const;

    Matx<_Tp, m, n>* dst;
    int idx;
};

template<typename _Tp, int m> class VecCommaInitializer : public MatxCommaInitializer<_Tp, m, 1>
{
public:
    VecCommaInitializer(Vec<_Tp, m>* _vec);
    template<typename T2> VecCommaInitializer<_Tp, m>& operator , (T2 val);
    Vec<_Tp, m> operator *() const;
};

/*!
 Automatically Allocated Buffer Class

 The class is used for temporary buffers in functions and methods.
 If a temporary buffer is usually small (a few K's of memory),
 but its size depends on the parameters, it makes sense to create a small
 fixed-size array on stack and use it if it's large enough. If the required buffer size
 is larger than the fixed size, another buffer of sufficient size is allocated dynamically
 and released after the processing. Therefore, in typical cases, when the buffer size is small,
 there is no overhead associated with malloc()/free().
 At the same time, there is no limit on the size of processed data.

 This is what AutoBuffer does. The template takes 2 parameters - type of the buffer elements and
 the number of stack-allocated elements. Here is how the class is used:

 \code
 void my_func(const vs::Mat& m)
 {
    vs::AutoBuffer<float, 1000> buf; // create automatic buffer containing 1000 floats

    buf.allocate(m.rows); // if m.rows <= 1000, the pre-allocated buffer is used,
                          // otherwise the buffer of "m.rows" floats will be allocated
                          // dynamically and deallocated in vs::AutoBuffer destructor
    ...
 }
 \endcode
*/
template<typename _Tp, size_t fixed_size=4096/sizeof(_Tp)+8> class AutoBuffer
{
public:
    typedef _Tp value_type;
    enum { buffer_padding = (int)((16 + sizeof(_Tp) - 1)/sizeof(_Tp)) };

    //! the default contructor
    AutoBuffer();
    //! constructor taking the real buffer size
    AutoBuffer(size_t _size);
    //! destructor. calls deallocate()
    ~AutoBuffer();

    //! allocates the new buffer of size _size. if the _size is small enough, stack-allocated buffer is used
    void allocate(size_t _size);
    //! deallocates the buffer if it was dynamically allocated
    void deallocate();
    //! returns pointer to the real buffer, stack-allocated or head-allocated
    operator _Tp* ();
    //! returns read-only pointer to the real buffer, stack-allocated or head-allocated
    operator const _Tp* () const;

protected:
    //! pointer to the real buffer, can point to buf if the buffer is small enough
    _Tp* ptr;
    //! size of the real buffer
    size_t size;
    //! pre-allocated buffer
    _Tp buf[fixed_size+buffer_padding];
};

/////////////////////////// multi-dimensional dense matrix //////////////////////////

/*!
 n-Dimensional Dense Matrix Iterator Class.

 The class vs::NAryMatIterator is used for iterating over one or more n-dimensional dense arrays (vs::Mat's).

 The iterator is completely different from vs::Mat_ and vs::SparseMat_ iterators.
 It iterates through the slices (or planes), not the elements, where "slice" is a continuous part of the arrays.

 Here is the example on how the iterator can be used to normalize 3D histogram:

 \code
 void normalizeColorHist(Mat& hist)
 {
 #if 1
     // intialize iterator (the style is different from STL).
     // after initialization the iterator will contain
     // the number of slices or planes
     // the iterator will go through
     Mat* arrays[] = { &hist, 0 };
     Mat planes[1];
     NAryMatIterator it(arrays, planes);
     double s = 0;
     // iterate through the matrix. on each iteration
     // it.planes[i] (of type Mat) will be set to the current plane of
     // i-th n-dim matrix passed to the iterator constructor.
     for(int p = 0; p < it.nplanes; p++, ++it)
        s += sum(it.planes[0])[0];
     it = NAryMatIterator(hist);
     s = 1./s;
     for(int p = 0; p < it.nplanes; p++, ++it)
        it.planes[0] *= s;
 #elif 1
     // this is a shorter implementation of the above
     // using built-in operations on Mat
     double s = sum(hist)[0];
     hist.convertTo(hist, hist.type(), 1./s, 0);
 #else
     // and this is even shorter one
     // (assuming that the histogram elements are non-negative)
     normalize(hist, hist, 1, 0, NORM_L1);
 #endif
 }
 \endcode

 You can iterate through several matrices simultaneously as long as they have the same geometry
 (dimensionality and all the dimension sizes are the same), which is useful for binary
 and n-ary operations on such matrices. Just pass those matrices to vs::MatNDIterator.
 Then, during the iteration it.planes[0], it.planes[1], ... will
 be the slices of the corresponding matrices
*/
class VS_EXPORTS NAryMatIterator
{
public:
    //! the default constructor
    NAryMatIterator();
    //! the full constructor taking arbitrary number of n-dim matrices
    NAryMatIterator(const Mat** arrays, uchar** ptrs, int narrays=-1);
    //! the full constructor taking arbitrary number of n-dim matrices
    NAryMatIterator(const Mat** arrays, Mat* planes, int narrays=-1);
    //! the separate iterator initialization method
    void init(const Mat** arrays, Mat* planes, uchar** ptrs, int narrays=-1);

    //! proceeds to the next plane of every iterated matrix
    NAryMatIterator& operator ++();
    //! proceeds to the next plane of every iterated matrix (postfix increment operator)
    NAryMatIterator operator ++(int);

    //! the iterated arrays
    const Mat** arrays;
    //! the current planes
    Mat* planes;
    //! data pointers
    uchar** ptrs;
    //! the number of arrays
    int narrays;
    //! the number of hyper-planes that the iterator steps through
    size_t nplanes;
    //! the size of each segment (in elements)
    size_t size;
protected:
    int iterdepth;
    size_t idx;
};

typedef void (*ConvertData)(const void* from, void* to, int cn);
typedef void (*ConvertScaleData)(const void* from, void* to, int cn, double alpha, double beta);

//! returns the function for converting pixels from one data type to another
VS_EXPORTS ConvertData getConvertElem(int fromType, int toType);
//! returns the function for converting pixels from one data type to another with the optional scaling
VS_EXPORTS ConvertScaleData getConvertScaleElem(int fromType, int toType);

/////////////////////////////// Parallel Primitives //////////////////////////////////

// a base body class
class VS_EXPORTS ParallelLoopBody
{
public:
    virtual ~ParallelLoopBody();
    virtual void operator() (const Range& range) const = 0;
};

VS_EXPORTS void parallel_for_(const Range& range, const ParallelLoopBody& body, double nstripes=-1.);

/////////////////////////// Synchronization Primitives ///////////////////////////////

class VS_EXPORTS Mutex
{
public:
    Mutex();
    ~Mutex();
    Mutex(const Mutex& m);
    Mutex& operator = (const Mutex& m);

    void lock();
    bool trylock();
    void unlock();

    struct Impl;
protected:
    Impl* impl;
};

class VS_EXPORTS AutoLock
{
public:
    AutoLock(Mutex& m) : mutex(&m) { mutex->lock(); }
    ~AutoLock() { mutex->unlock(); }
protected:
    Mutex* mutex;
private:
    AutoLock(const AutoLock&);
    AutoLock& operator = (const AutoLock&);
};

}

#endif // __cplusplus

#include "vs_core_operations.h"
#include "vs_core_mat.h"

#endif /*__VS_CORE_HPP__*/
