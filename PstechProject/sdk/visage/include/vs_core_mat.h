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

#ifndef __VS_CORE_MATRIX_OPERATIONS_HPP__
#define __VS_CORE_MATRIX_OPERATIONS_HPP__

#ifndef SKIP_INCLUDES
#include <limits.h>
#include <string.h>
#endif // SKIP_INCLUDES

#ifdef __cplusplus

namespace vs
{

//////////////////////////////// Mat ////////////////////////////////

inline void Mat::initEmpty()
{
    flags = MAGIC_VAL;
    dims = rows = cols = 0;
    data = datastart = dataend = datalimit = 0;
    refcount = 0;
    allocator = 0;
}

inline Mat::Mat() : size(&rows)
{
    initEmpty();
}

inline Mat::Mat(int _rows, int _cols, int _type) : size(&rows)
{
    initEmpty();
    create(_rows, _cols, _type);
}

inline Mat::Mat(int _rows, int _cols, int _type, const Scalar& _s) : size(&rows)
{
    initEmpty();
    create(_rows, _cols, _type);
    *this = _s;
}

inline Mat::Mat(Size _sz, int _type) : size(&rows)
{
    initEmpty();
    create( _sz.height, _sz.width, _type );
}

inline Mat::Mat(Size _sz, int _type, const Scalar& _s) : size(&rows)
{
    initEmpty();
    create(_sz.height, _sz.width, _type);
    *this = _s;
}

inline Mat::Mat(int _dims, const int* _sz, int _type) : size(&rows)
{
    initEmpty();
    create(_dims, _sz, _type);
}

inline Mat::Mat(int _dims, const int* _sz, int _type, const Scalar& _s) : size(&rows)
{
    initEmpty();
    create(_dims, _sz, _type);
    *this = _s;
}

inline Mat::Mat(const Mat& m)
    : flags(m.flags), dims(m.dims), rows(m.rows), cols(m.cols), data(m.data),
    refcount(m.refcount), datastart(m.datastart), dataend(m.dataend),
    datalimit(m.datalimit), allocator(m.allocator), size(&rows)
{
    if( refcount )
        VS_XADD(refcount, 1);
    if( m.dims <= 2 )
    {
        step[0] = m.step[0]; step[1] = m.step[1];
    }
    else
    {
        dims = 0;
        copySize(m);
    }
}

inline Mat::Mat(int _rows, int _cols, int _type, void* _data, size_t _step)
    : flags(MAGIC_VAL + (_type & TYPE_MASK)), dims(2), rows(_rows), cols(_cols),
    data((uchar*)_data), refcount(0), datastart((uchar*)_data), dataend(0),
    datalimit(0), allocator(0), size(&rows)
{
    size_t esz = VS_ELEM_SIZE(_type), minstep = cols*esz;
    if( _step == AUTO_STEP )
    {
        _step = minstep;
        flags |= CONTINUOUS_FLAG;
    }
    else
    {
        if( rows == 1 ) _step = minstep;
        VS_DbgAssert( _step >= minstep );
        flags |= _step == minstep ? CONTINUOUS_FLAG : 0;
    }
    step[0] = _step; step[1] = esz;
    datalimit = datastart + _step*rows;
    dataend = datalimit - _step + minstep;
}

inline Mat::Mat(Size _sz, int _type, void* _data, size_t _step)
    : flags(MAGIC_VAL + (_type & TYPE_MASK)), dims(2), rows(_sz.height), cols(_sz.width),
    data((uchar*)_data), refcount(0), datastart((uchar*)_data), dataend(0),
    datalimit(0), allocator(0), size(&rows)
{
    size_t esz = VS_ELEM_SIZE(_type), minstep = cols*esz;
    if( _step == AUTO_STEP )
    {
        _step = minstep;
        flags |= CONTINUOUS_FLAG;
    }
    else
    {
        if( rows == 1 ) _step = minstep;
        VS_DbgAssert( _step >= minstep );
        flags |= _step == minstep ? CONTINUOUS_FLAG : 0;
    }
    step[0] = _step; step[1] = esz;
    datalimit = datastart + _step*rows;
    dataend = datalimit - _step + minstep;
}


template<typename _Tp> inline Mat::Mat(const vector<_Tp>& vec, bool copyData)
    : flags(MAGIC_VAL | DataType<_Tp>::type | VS_MAT_CONT_FLAG),
    dims(2), rows((int)vec.size()), cols(1), data(0), refcount(0),
    datastart(0), dataend(0), allocator(0), size(&rows)
{
    if(vec.empty())
        return;
    if( !copyData )
    {
        step[0] = step[1] = sizeof(_Tp);
        data = datastart = (uchar*)&vec[0];
        datalimit = dataend = datastart + rows*step[0];
    }
    else
        Mat((int)vec.size(), 1, DataType<_Tp>::type, (uchar*)&vec[0]).copyTo(*this);
}


template<typename _Tp, int n> inline Mat::Mat(const Vec<_Tp, n>& vec, bool copyData)
    : flags(MAGIC_VAL | DataType<_Tp>::type | VS_MAT_CONT_FLAG),
    dims(2), rows(n), cols(1), data(0), refcount(0),
    datastart(0), dataend(0), allocator(0), size(&rows)
{
    if( !copyData )
    {
        step[0] = step[1] = sizeof(_Tp);
        data = datastart = (uchar*)vec.val;
        datalimit = dataend = datastart + rows*step[0];
    }
    else
        Mat(n, 1, DataType<_Tp>::type, (void*)vec.val).copyTo(*this);
}


template<typename _Tp, int m, int n> inline Mat::Mat(const Matx<_Tp,m,n>& M, bool copyData)
    : flags(MAGIC_VAL | DataType<_Tp>::type | VS_MAT_CONT_FLAG),
    dims(2), rows(m), cols(n), data(0), refcount(0),
    datastart(0), dataend(0), allocator(0), size(&rows)
{
    if( !copyData )
    {
        step[0] = cols*sizeof(_Tp);
        step[1] = sizeof(_Tp);
        data = datastart = (uchar*)M.val;
        datalimit = dataend = datastart + rows*step[0];
    }
    else
        Mat(m, n, DataType<_Tp>::type, (uchar*)M.val).copyTo(*this);
}


template<typename _Tp> inline Mat::Mat(const Point_<_Tp>& pt, bool copyData)
    : flags(MAGIC_VAL | DataType<_Tp>::type | VS_MAT_CONT_FLAG),
    dims(2), rows(2), cols(1), data(0), refcount(0),
    datastart(0), dataend(0), allocator(0), size(&rows)
{
    if( !copyData )
    {
        step[0] = step[1] = sizeof(_Tp);
        data = datastart = (uchar*)&pt.x;
        datalimit = dataend = datastart + rows*step[0];
    }
    else
    {
        create(2, 1, DataType<_Tp>::type);
        ((_Tp*)data)[0] = pt.x;
        ((_Tp*)data)[1] = pt.y;
    }
}


template<typename _Tp> inline Mat::Mat(const Point3_<_Tp>& pt, bool copyData)
    : flags(MAGIC_VAL | DataType<_Tp>::type | VS_MAT_CONT_FLAG),
    dims(2), rows(3), cols(1), data(0), refcount(0),
    datastart(0), dataend(0), allocator(0), size(&rows)
{
    if( !copyData )
    {
        step[0] = step[1] = sizeof(_Tp);
        data = datastart = (uchar*)&pt.x;
        datalimit = dataend = datastart + rows*step[0];
    }
    else
    {
        create(3, 1, DataType<_Tp>::type);
        ((_Tp*)data)[0] = pt.x;
        ((_Tp*)data)[1] = pt.y;
        ((_Tp*)data)[2] = pt.z;
    }
}


template<typename _Tp> inline Mat::Mat(const MatCommaInitializer_<_Tp>& commaInitializer)
    : flags(MAGIC_VAL | DataType<_Tp>::type | VS_MAT_CONT_FLAG),
    dims(0), rows(0), cols(0), data(0), refcount(0),
    datastart(0), dataend(0), allocator(0), size(&rows)
{
    *this = *commaInitializer;
}

inline Mat::~Mat()
{
    release();
    if( step.p != step.buf )
        fastFree(step.p);
}

inline Mat& Mat::operator = (const Mat& m)
{
    if( this != &m )
    {
        if( m.refcount )
            VS_XADD(m.refcount, 1);
        release();
        flags = m.flags;
        if( dims <= 2 && m.dims <= 2 )
        {
            dims = m.dims;
            rows = m.rows;
            cols = m.cols;
            step[0] = m.step[0];
            step[1] = m.step[1];
        }
        else
            copySize(m);
        data = m.data;
        datastart = m.datastart;
        dataend = m.dataend;
        datalimit = m.datalimit;
        refcount = m.refcount;
        allocator = m.allocator;
    }
    return *this;
}

inline Mat Mat::row(int y) const { return Mat(*this, Range(y, y+1), Range::all()); }
inline Mat Mat::col(int x) const { return Mat(*this, Range::all(), Range(x, x+1)); }
inline Mat Mat::rowRange(int startrow, int endrow) const
    { return Mat(*this, Range(startrow, endrow), Range::all()); }
inline Mat Mat::rowRange(const Range& r) const
    { return Mat(*this, r, Range::all()); }
inline Mat Mat::colRange(int startcol, int endcol) const
    { return Mat(*this, Range::all(), Range(startcol, endcol)); }
inline Mat Mat::colRange(const Range& r) const
    { return Mat(*this, Range::all(), r); }

inline Mat Mat::diag(const Mat& d)
{
    VS_Assert( d.cols == 1 || d.rows == 1 );
    int len = d.rows + d.cols - 1;
    Mat m(len, len, d.type(), Scalar(0)), md = m.diag();
    if( d.cols == 1 )
        d.copyTo(md);
    else
        transpose(d, md);
    return m;
}

inline Mat Mat::clone() const
{
    Mat m;
    copyTo(m);
    return m;
}

inline void Mat::assignTo( Mat& m, int _type ) const
{
    if( _type < 0 )
        m = *this;
    else
        convertTo(m, _type);
}

inline void Mat::create(int _rows, int _cols, int _type)
{
    _type &= TYPE_MASK;
    if( dims <= 2 && rows == _rows && cols == _cols && type() == _type && data )
        return;
    int sz[] = {_rows, _cols};
    create(2, sz, _type);
}

inline void Mat::create(Size _sz, int _type)
{
    create(_sz.height, _sz.width, _type);
}

inline void Mat::addref()
{ if( refcount ) VS_XADD(refcount, 1); }

inline void Mat::release()
{
    if( refcount && VS_XADD(refcount, -1) == 1 )
        deallocate();
    data = datastart = dataend = datalimit = 0;
    for(int i = 0; i < dims; i++)
        size.p[i] = 0;
    refcount = 0;
}

inline Mat Mat::operator()( Range _rowRange, Range _colRange ) const
{
    return Mat(*this, _rowRange, _colRange);
}

inline Mat Mat::operator()( const Rect& roi ) const
{ return Mat(*this, roi); }

inline Mat Mat::operator()(const Range* ranges) const
{
    return Mat(*this, ranges);
}

inline Mat::operator VsMat() const
{
    VS_DbgAssert(dims <= 2);
    VsMat m = vsMat(rows, dims == 1 ? 1 : cols, type(), data);
    m.step = (int)step[0];
    m.type = (m.type & ~CONTINUOUS_FLAG) | (flags & CONTINUOUS_FLAG);
    return m;
}

inline bool Mat::isContinuous() const { return (flags & CONTINUOUS_FLAG) != 0; }
inline bool Mat::isSubmatrix() const { return (flags & SUBMATRIX_FLAG) != 0; }
inline size_t Mat::elemSize() const { return dims > 0 ? step.p[dims-1] : 0; }
inline size_t Mat::elemSize1() const { return VS_ELEM_SIZE1(flags); }
inline int Mat::type() const { return VS_MAT_TYPE(flags); }
inline int Mat::depth() const { return VS_MAT_DEPTH(flags); }
inline int Mat::channels() const { return VS_MAT_CN(flags); }
inline size_t Mat::step1(int i) const { return step.p[i]/elemSize1(); }
inline bool Mat::empty() const { return data == 0 || total() == 0; }
inline size_t Mat::total() const
{
    if( dims <= 2 )
        return (size_t)rows*cols;
    size_t p = 1;
    for( int i = 0; i < dims; i++ )
        p *= size[i];
    return p;
}

inline uchar* Mat::ptr(int y)
{
    VS_DbgAssert( y == 0 || (data && dims >= 1 && (unsigned)y < (unsigned)size.p[0]) );
    return data + step.p[0]*y;
}

inline const uchar* Mat::ptr(int y) const
{
    VS_DbgAssert( y == 0 || (data && dims >= 1 && (unsigned)y < (unsigned)size.p[0]) );
    return data + step.p[0]*y;
}

template<typename _Tp> inline _Tp* Mat::ptr(int y)
{
    VS_DbgAssert( y == 0 || (data && dims >= 1 && (unsigned)y < (unsigned)size.p[0]) );
    return (_Tp*)(data + step.p[0]*y);
}

template<typename _Tp> inline const _Tp* Mat::ptr(int y) const
{
    VS_DbgAssert( y == 0 || (data && dims >= 1 && (unsigned)y < (unsigned)size.p[0]) );
    return (const _Tp*)(data + step.p[0]*y);
}


inline uchar* Mat::ptr(int i0, int i1)
{
    VS_DbgAssert( dims >= 2 && data &&
                  (unsigned)i0 < (unsigned)size.p[0] &&
                  (unsigned)i1 < (unsigned)size.p[1] );
    return data + i0*step.p[0] + i1*step.p[1];
}

inline const uchar* Mat::ptr(int i0, int i1) const
{
    VS_DbgAssert( dims >= 2 && data &&
                 (unsigned)i0 < (unsigned)size.p[0] &&
                 (unsigned)i1 < (unsigned)size.p[1] );
    return data + i0*step.p[0] + i1*step.p[1];
}

template<typename _Tp> inline _Tp* Mat::ptr(int i0, int i1)
{
    VS_DbgAssert( dims >= 2 && data &&
                  (unsigned)i0 < (unsigned)size.p[0] &&
                  (unsigned)i1 < (unsigned)size.p[1] );
    return (_Tp*)(data + i0*step.p[0] + i1*step.p[1]);
}

template<typename _Tp> inline const _Tp* Mat::ptr(int i0, int i1) const
{
    VS_DbgAssert( dims >= 2 && data &&
                  (unsigned)i0 < (unsigned)size.p[0] &&
                  (unsigned)i1 < (unsigned)size.p[1] );
    return (const _Tp*)(data + i0*step.p[0] + i1*step.p[1]);
}

inline uchar* Mat::ptr(int i0, int i1, int i2)
{
    VS_DbgAssert( dims >= 3 && data &&
                  (unsigned)i0 < (unsigned)size.p[0] &&
                  (unsigned)i1 < (unsigned)size.p[1] &&
                  (unsigned)i2 < (unsigned)size.p[2] );
    return data + i0*step.p[0] + i1*step.p[1] + i2*step.p[2];
}

inline const uchar* Mat::ptr(int i0, int i1, int i2) const
{
    VS_DbgAssert( dims >= 3 && data &&
                  (unsigned)i0 < (unsigned)size.p[0] &&
                  (unsigned)i1 < (unsigned)size.p[1] &&
                  (unsigned)i2 < (unsigned)size.p[2] );
    return data + i0*step.p[0] + i1*step.p[1] + i2*step.p[2];
}

template<typename _Tp> inline _Tp* Mat::ptr(int i0, int i1, int i2)
{
    VS_DbgAssert( dims >= 3 && data &&
                  (unsigned)i0 < (unsigned)size.p[0] &&
                  (unsigned)i1 < (unsigned)size.p[1] &&
                  (unsigned)i2 < (unsigned)size.p[2] );
    return (_Tp*)(data + i0*step.p[0] + i1*step.p[1] + i2*step.p[2]);
}

template<typename _Tp> inline const _Tp* Mat::ptr(int i0, int i1, int i2) const
{
    VS_DbgAssert( dims >= 3 && data &&
                  (unsigned)i0 < (unsigned)size.p[0] &&
                  (unsigned)i1 < (unsigned)size.p[1] &&
                  (unsigned)i2 < (unsigned)size.p[2] );
    return (const _Tp*)(data + i0*step.p[0] + i1*step.p[1] + i2*step.p[2]);
}

inline uchar* Mat::ptr(const int* idx)
{
    int i, d = dims;
    uchar* p = data;
    VS_DbgAssert( d >= 1 && p );
    for( i = 0; i < d; i++ )
    {
        VS_DbgAssert( (unsigned)idx[i] < (unsigned)size.p[i] );
        p += idx[i]*step.p[i];
    }
    return p;
}

inline const uchar* Mat::ptr(const int* idx) const
{
    int i, d = dims;
    uchar* p = data;
    VS_DbgAssert( d >= 1 && p );
    for( i = 0; i < d; i++ )
    {
        VS_DbgAssert( (unsigned)idx[i] < (unsigned)size.p[i] );
        p += idx[i]*step.p[i];
    }
    return p;
}

template<typename _Tp> inline _Tp& Mat::at(int i0, int i1)
{
    VS_DbgAssert( dims <= 2 && data && (unsigned)i0 < (unsigned)size.p[0] &&
        (unsigned)(i1*DataType<_Tp>::channels) < (unsigned)(size.p[1]*channels()) &&
        VS_ELEM_SIZE1(DataType<_Tp>::depth) == elemSize1());
    return ((_Tp*)(data + step.p[0]*i0))[i1];
}

template<typename _Tp> inline const _Tp& Mat::at(int i0, int i1) const
{
    VS_DbgAssert( dims <= 2 && data && (unsigned)i0 < (unsigned)size.p[0] &&
        (unsigned)(i1*DataType<_Tp>::channels) < (unsigned)(size.p[1]*channels()) &&
        VS_ELEM_SIZE1(DataType<_Tp>::depth) == elemSize1());
    return ((const _Tp*)(data + step.p[0]*i0))[i1];
}

template<typename _Tp> inline _Tp& Mat::at(Point pt)
{
    VS_DbgAssert( dims <= 2 && data && (unsigned)pt.y < (unsigned)size.p[0] &&
        (unsigned)(pt.x*DataType<_Tp>::channels) < (unsigned)(size.p[1]*channels()) &&
        VS_ELEM_SIZE1(DataType<_Tp>::depth) == elemSize1());
    return ((_Tp*)(data + step.p[0]*pt.y))[pt.x];
}

template<typename _Tp> inline const _Tp& Mat::at(Point pt) const
{
    VS_DbgAssert( dims <= 2 && data && (unsigned)pt.y < (unsigned)size.p[0] &&
        (unsigned)(pt.x*DataType<_Tp>::channels) < (unsigned)(size.p[1]*channels()) &&
        VS_ELEM_SIZE1(DataType<_Tp>::depth) == elemSize1());
    return ((const _Tp*)(data + step.p[0]*pt.y))[pt.x];
}

template<typename _Tp> inline _Tp& Mat::at(int i0)
{
    VS_DbgAssert( dims <= 2 && data &&
                 (unsigned)i0 < (unsigned)(size.p[0]*size.p[1]) &&
                 elemSize() == VS_ELEM_SIZE(DataType<_Tp>::type) );
    if( isContinuous() || size.p[0] == 1 )
        return ((_Tp*)data)[i0];
    if( size.p[1] == 1 )
        return *(_Tp*)(data + step.p[0]*i0);
    int i = i0/cols, j = i0 - i*cols;
    return ((_Tp*)(data + step.p[0]*i))[j];
}

template<typename _Tp> inline const _Tp& Mat::at(int i0) const
{
    VS_DbgAssert( dims <= 2 && data &&
                 (unsigned)i0 < (unsigned)(size.p[0]*size.p[1]) &&
                 elemSize() == VS_ELEM_SIZE(DataType<_Tp>::type) );
    if( isContinuous() || size.p[0] == 1 )
        return ((const _Tp*)data)[i0];
    if( size.p[1] == 1 )
        return *(const _Tp*)(data + step.p[0]*i0);
    int i = i0/cols, j = i0 - i*cols;
    return ((const _Tp*)(data + step.p[0]*i))[j];
}

template<typename _Tp> inline _Tp& Mat::at(int i0, int i1, int i2)
{
    VS_DbgAssert( elemSize() == VS_ELEM_SIZE(DataType<_Tp>::type) );
    return *(_Tp*)ptr(i0, i1, i2);
}
template<typename _Tp> inline const _Tp& Mat::at(int i0, int i1, int i2) const
{
    VS_DbgAssert( elemSize() == VS_ELEM_SIZE(DataType<_Tp>::type) );
    return *(const _Tp*)ptr(i0, i1, i2);
}
template<typename _Tp> inline _Tp& Mat::at(const int* idx)
{
    VS_DbgAssert( elemSize() == VS_ELEM_SIZE(DataType<_Tp>::type) );
    return *(_Tp*)ptr(idx);
}
template<typename _Tp> inline const _Tp& Mat::at(const int* idx) const
{
    VS_DbgAssert( elemSize() == VS_ELEM_SIZE(DataType<_Tp>::type) );
    return *(const _Tp*)ptr(idx);
}
template<typename _Tp, int n> _Tp& Mat::at(const Vec<int, n>& idx)
{
    VS_DbgAssert( elemSize() == VS_ELEM_SIZE(DataType<_Tp>::type) );
    return *(_Tp*)ptr(idx.val);
}
template<typename _Tp, int n> inline const _Tp& Mat::at(const Vec<int, n>& idx) const
{
    VS_DbgAssert( elemSize() == VS_ELEM_SIZE(DataType<_Tp>::type) );
    return *(const _Tp*)ptr(idx.val);
}

template<typename _Tp> inline MatIterator_<_Tp> Mat::begin()
{
    VS_DbgAssert( elemSize() == sizeof(_Tp) );
    return MatIterator_<_Tp>((Mat_<_Tp>*)this);
}

template<typename _Tp> inline MatIterator_<_Tp> Mat::end()
{
    VS_DbgAssert( elemSize() == sizeof(_Tp) );
    MatIterator_<_Tp> it((Mat_<_Tp>*)this);
    it += total();
    return it;
}

template<typename _Tp> inline Mat::operator vector<_Tp>() const
{
    vector<_Tp> v;
    copyTo(v);
    return v;
}

template<typename _Tp, int n> inline Mat::operator Vec<_Tp, n>() const
{
    VS_Assert( data && dims <= 2 && (rows == 1 || cols == 1) &&
               rows + cols - 1 == n && channels() == 1 );

    if( isContinuous() && type() == DataType<_Tp>::type )
        return Vec<_Tp, n>((_Tp*)data);
    Vec<_Tp, n> v; Mat tmp(rows, cols, DataType<_Tp>::type, v.val);
    convertTo(tmp, tmp.type());
    return v;
}

template<typename _Tp, int m, int n> inline Mat::operator Matx<_Tp, m, n>() const
{
    VS_Assert( data && dims <= 2 && rows == m && cols == n && channels() == 1 );

    if( isContinuous() && type() == DataType<_Tp>::type )
        return Matx<_Tp, m, n>((_Tp*)data);
    Matx<_Tp, m, n> mtx; Mat tmp(rows, cols, DataType<_Tp>::type, mtx.val);
    convertTo(tmp, tmp.type());
    return mtx;
}


template<typename _Tp> inline void Mat::push_back(const _Tp& elem)
{
    if( !data )
    {
        VS_Assert((type()==0) || (DataType<_Tp>::type == type()));

        *this = Mat(1, 1, DataType<_Tp>::type, (void*)&elem).clone();
        return;
    }
    VS_Assert(DataType<_Tp>::type == type() && cols == 1
              /* && dims == 2 (cols == 1 implies dims == 2) */);
    uchar* tmp = dataend + step[0];
    if( !isSubmatrix() && isContinuous() && tmp <= datalimit )
    {
        *(_Tp*)(data + (size.p[0]++)*step.p[0]) = elem;
        dataend = tmp;
    }
    else
        push_back_(&elem);
}

template<typename _Tp> inline void Mat::push_back(const Mat_<_Tp>& m)
{
    push_back((const Mat&)m);
}

inline Mat::MSize::MSize(int* _p) : p(_p) {}
inline Size Mat::MSize::operator()() const
{
    VS_DbgAssert(p[-1] <= 2);
    return Size(p[1], p[0]);
}
inline const int& Mat::MSize::operator[](int i) const { return p[i]; }
inline int& Mat::MSize::operator[](int i) { return p[i]; }
inline Mat::MSize::operator const int*() const { return p; }

inline bool Mat::MSize::operator == (const MSize& sz) const
{
    int d = p[-1], dsz = sz.p[-1];
    if( d != dsz )
        return false;
    if( d == 2 )
        return p[0] == sz.p[0] && p[1] == sz.p[1];

    for( int i = 0; i < d; i++ )
        if( p[i] != sz.p[i] )
            return false;
    return true;
}

inline bool Mat::MSize::operator != (const MSize& sz) const
{
    return !(*this == sz);
}

inline Mat::MStep::MStep() { p = buf; p[0] = p[1] = 0; }
inline Mat::MStep::MStep(size_t s) { p = buf; p[0] = s; p[1] = 0; }
inline const size_t& Mat::MStep::operator[](int i) const { return p[i]; }
inline size_t& Mat::MStep::operator[](int i) { return p[i]; }
inline Mat::MStep::operator size_t() const
{
    VS_DbgAssert( p == buf );
    return buf[0];
}
inline Mat::MStep& Mat::MStep::operator = (size_t s)
{
    VS_DbgAssert( p == buf );
    buf[0] = s;
    return *this;
}

static inline Mat vsarrToMatND(const VsArr* arr, bool copyData=false, int coiMode=0)
{
    return vsarrToMat(arr, copyData, true, coiMode);
}

///////////////////////////////////////////// SVD //////////////////////////////////////////////////////

inline SVD::SVD() {}
inline SVD::SVD( InputArray m, int flags ) { operator ()(m, flags); }
inline void SVD::solveZ( InputArray m, OutputArray _dst )
{
    Mat mtx = m.getMat();
    SVD svd(mtx, (mtx.rows >= mtx.cols ? 0 : SVD::FULL_UV));
    _dst.create(svd.vt.cols, 1, svd.vt.type());
    Mat dst = _dst.getMat();
    svd.vt.row(svd.vt.rows-1).reshape(1,svd.vt.cols).copyTo(dst);
}

template<typename _Tp, int m, int n, int nm> inline void
    SVD::compute( const Matx<_Tp, m, n>& a, Matx<_Tp, nm, 1>& w, Matx<_Tp, m, nm>& u, Matx<_Tp, n, nm>& vt )
{
    assert( nm == MIN(m, n));
    Mat _a(a, false), _u(u, false), _w(w, false), _vt(vt, false);
    SVD::compute(_a, _w, _u, _vt);
    VS_Assert(_w.data == (uchar*)&w.val[0] && _u.data == (uchar*)&u.val[0] && _vt.data == (uchar*)&vt.val[0]);
}

template<typename _Tp, int m, int n, int nm> inline void
SVD::compute( const Matx<_Tp, m, n>& a, Matx<_Tp, nm, 1>& w )
{
    assert( nm == MIN(m, n));
    Mat _a(a, false), _w(w, false);
    SVD::compute(_a, _w);
    VS_Assert(_w.data == (uchar*)&w.val[0]);
}

template<typename _Tp, int m, int n, int nm, int nb> inline void
SVD::backSubst( const Matx<_Tp, nm, 1>& w, const Matx<_Tp, m, nm>& u,
                const Matx<_Tp, n, nm>& vt, const Matx<_Tp, m, nb>& rhs,
                Matx<_Tp, n, nb>& dst )
{
    assert( nm == MIN(m, n));
    Mat _u(u, false), _w(w, false), _vt(vt, false), _rhs(rhs, false), _dst(dst, false);
    SVD::backSubst(_w, _u, _vt, _rhs, _dst);
    VS_Assert(_dst.data == (uchar*)&dst.val[0]);
}

///////////////////////////////// Mat_<_Tp> ////////////////////////////////////

template<typename _Tp> inline Mat_<_Tp>::Mat_()
    : Mat() { flags = (flags & ~VS_MAT_TYPE_MASK) | DataType<_Tp>::type; }

template<typename _Tp> inline Mat_<_Tp>::Mat_(int _rows, int _cols)
    : Mat(_rows, _cols, DataType<_Tp>::type) {}

template<typename _Tp> inline Mat_<_Tp>::Mat_(int _rows, int _cols, const _Tp& value)
    : Mat(_rows, _cols, DataType<_Tp>::type) { *this = value; }

template<typename _Tp> inline Mat_<_Tp>::Mat_(Size _sz)
    : Mat(_sz.height, _sz.width, DataType<_Tp>::type) {}

template<typename _Tp> inline Mat_<_Tp>::Mat_(Size _sz, const _Tp& value)
    : Mat(_sz.height, _sz.width, DataType<_Tp>::type) { *this = value; }

template<typename _Tp> inline Mat_<_Tp>::Mat_(int _dims, const int* _sz)
    : Mat(_dims, _sz, DataType<_Tp>::type) {}

template<typename _Tp> inline Mat_<_Tp>::Mat_(int _dims, const int* _sz, const _Tp& _s)
    : Mat(_dims, _sz, DataType<_Tp>::type, Scalar(_s)) {}

template<typename _Tp> inline Mat_<_Tp>::Mat_(const Mat_<_Tp>& m, const Range* ranges)
    : Mat(m, ranges) {}

template<typename _Tp> inline Mat_<_Tp>::Mat_(const Mat& m)
    : Mat() { flags = (flags & ~VS_MAT_TYPE_MASK) | DataType<_Tp>::type; *this = m; }

template<typename _Tp> inline Mat_<_Tp>::Mat_(const Mat_& m)
    : Mat(m) {}

template<typename _Tp> inline Mat_<_Tp>::Mat_(int _rows, int _cols, _Tp* _data, size_t steps)
    : Mat(_rows, _cols, DataType<_Tp>::type, _data, steps) {}

template<typename _Tp> inline Mat_<_Tp>::Mat_(const Mat_& m, const Range& _rowRange, const Range& _colRange)
    : Mat(m, _rowRange, _colRange) {}

template<typename _Tp> inline Mat_<_Tp>::Mat_(const Mat_& m, const Rect& roi)
    : Mat(m, roi) {}

template<typename _Tp> template<int n> inline
    Mat_<_Tp>::Mat_(const Vec<typename DataType<_Tp>::channel_type, n>& vec, bool copyData)
    : Mat(n/DataType<_Tp>::channels, 1, DataType<_Tp>::type, (void*)&vec)
{
    VS_Assert(n%DataType<_Tp>::channels == 0);
    if( copyData )
        *this = clone();
}

template<typename _Tp> template<int m, int n> inline
    Mat_<_Tp>::Mat_(const Matx<typename DataType<_Tp>::channel_type,m,n>& M, bool copyData)
    : Mat(m, n/DataType<_Tp>::channels, DataType<_Tp>::type, (void*)&M)
{
    VS_Assert(n % DataType<_Tp>::channels == 0);
    if( copyData )
        *this = clone();
}

template<typename _Tp> inline Mat_<_Tp>::Mat_(const Point_<typename DataType<_Tp>::channel_type>& pt, bool copyData)
    : Mat(2/DataType<_Tp>::channels, 1, DataType<_Tp>::type, (void*)&pt)
{
    VS_Assert(2 % DataType<_Tp>::channels == 0);
    if( copyData )
        *this = clone();
}

template<typename _Tp> inline Mat_<_Tp>::Mat_(const Point3_<typename DataType<_Tp>::channel_type>& pt, bool copyData)
    : Mat(3/DataType<_Tp>::channels, 1, DataType<_Tp>::type, (void*)&pt)
{
    VS_Assert(3 % DataType<_Tp>::channels == 0);
    if( copyData )
        *this = clone();
}

template<typename _Tp> inline Mat_<_Tp>::Mat_(const MatCommaInitializer_<_Tp>& commaInitializer)
    : Mat(commaInitializer) {}

template<typename _Tp> inline Mat_<_Tp>::Mat_(const vector<_Tp>& vec, bool copyData)
    : Mat(vec, copyData) {}

template<typename _Tp> inline Mat_<_Tp>& Mat_<_Tp>::operator = (const Mat& m)
{
    if( DataType<_Tp>::type == m.type() )
    {
        Mat::operator = (m);
        return *this;
    }
    if( DataType<_Tp>::depth == m.depth() )
    {
        return (*this = m.reshape(DataType<_Tp>::channels, m.dims, 0));
    }
    VS_DbgAssert(DataType<_Tp>::channels == m.channels());
    m.convertTo(*this, type());
    return *this;
}

template<typename _Tp> inline Mat_<_Tp>& Mat_<_Tp>::operator = (const Mat_& m)
{
    Mat::operator=(m);
    return *this;
}

template<typename _Tp> inline Mat_<_Tp>& Mat_<_Tp>::operator = (const _Tp& s)
{
    typedef typename DataType<_Tp>::vec_type VT;
    Mat::operator=(Scalar((const VT&)s));
    return *this;
}

template<typename _Tp> inline void Mat_<_Tp>::create(int _rows, int _cols)
{
    Mat::create(_rows, _cols, DataType<_Tp>::type);
}

template<typename _Tp> inline void Mat_<_Tp>::create(Size _sz)
{
    Mat::create(_sz, DataType<_Tp>::type);
}

template<typename _Tp> inline void Mat_<_Tp>::create(int _dims, const int* _sz)
{
    Mat::create(_dims, _sz, DataType<_Tp>::type);
}


template<typename _Tp> inline Mat_<_Tp> Mat_<_Tp>::cross(const Mat_& m) const
{ return Mat_<_Tp>(Mat::cross(m)); }

template<typename _Tp> template<typename T2> inline Mat_<_Tp>::operator Mat_<T2>() const
{ return Mat_<T2>(*this); }

template<typename _Tp> inline Mat_<_Tp> Mat_<_Tp>::row(int y) const
{ return Mat_(*this, Range(y, y+1), Range::all()); }
template<typename _Tp> inline Mat_<_Tp> Mat_<_Tp>::col(int x) const
{ return Mat_(*this, Range::all(), Range(x, x+1)); }
template<typename _Tp> inline Mat_<_Tp> Mat_<_Tp>::diag(int d) const
{ return Mat_(Mat::diag(d)); }
template<typename _Tp> inline Mat_<_Tp> Mat_<_Tp>::clone() const
{ return Mat_(Mat::clone()); }

template<typename _Tp> inline size_t Mat_<_Tp>::elemSize() const
{
    VS_DbgAssert( Mat::elemSize() == sizeof(_Tp) );
    return sizeof(_Tp);
}

template<typename _Tp> inline size_t Mat_<_Tp>::elemSize1() const
{
    VS_DbgAssert( Mat::elemSize1() == sizeof(_Tp)/DataType<_Tp>::channels );
    return sizeof(_Tp)/DataType<_Tp>::channels;
}
template<typename _Tp> inline int Mat_<_Tp>::type() const
{
    VS_DbgAssert( Mat::type() == DataType<_Tp>::type );
    return DataType<_Tp>::type;
}
template<typename _Tp> inline int Mat_<_Tp>::depth() const
{
    VS_DbgAssert( Mat::depth() == DataType<_Tp>::depth );
    return DataType<_Tp>::depth;
}
template<typename _Tp> inline int Mat_<_Tp>::channels() const
{
    VS_DbgAssert( Mat::channels() == DataType<_Tp>::channels );
    return DataType<_Tp>::channels;
}
template<typename _Tp> inline size_t Mat_<_Tp>::stepT(int i) const { return step.p[i]/elemSize(); }
template<typename _Tp> inline size_t Mat_<_Tp>::step1(int i) const { return step.p[i]/elemSize1(); }

template<typename _Tp> inline Mat_<_Tp>& Mat_<_Tp>::adjustROI( int dtop, int dbottom, int dleft, int dright )
{ return (Mat_<_Tp>&)(Mat::adjustROI(dtop, dbottom, dleft, dright));  }

template<typename _Tp> inline Mat_<_Tp> Mat_<_Tp>::operator()( const Range& _rowRange, const Range& _colRange ) const
{ return Mat_<_Tp>(*this, _rowRange, _colRange); }

template<typename _Tp> inline Mat_<_Tp> Mat_<_Tp>::operator()( const Rect& roi ) const
{ return Mat_<_Tp>(*this, roi); }

template<typename _Tp> inline Mat_<_Tp> Mat_<_Tp>::operator()( const Range* ranges ) const
{ return Mat_<_Tp>(*this, ranges); }

template<typename _Tp> inline _Tp* Mat_<_Tp>::operator [](int y)
{ return (_Tp*)ptr(y); }
template<typename _Tp> inline const _Tp* Mat_<_Tp>::operator [](int y) const
{ return (const _Tp*)ptr(y); }

template<typename _Tp> inline _Tp& Mat_<_Tp>::operator ()(int i0, int i1)
{
    VS_DbgAssert( dims <= 2 && data &&
                  (unsigned)i0 < (unsigned)size.p[0] &&
                  (unsigned)i1 < (unsigned)size.p[1] &&
                  type() == DataType<_Tp>::type );
    return ((_Tp*)(data + step.p[0]*i0))[i1];
}

template<typename _Tp> inline const _Tp& Mat_<_Tp>::operator ()(int i0, int i1) const
{
    VS_DbgAssert( dims <= 2 && data &&
                  (unsigned)i0 < (unsigned)size.p[0] &&
                  (unsigned)i1 < (unsigned)size.p[1] &&
                  type() == DataType<_Tp>::type );
    return ((const _Tp*)(data + step.p[0]*i0))[i1];
}

template<typename _Tp> inline _Tp& Mat_<_Tp>::operator ()(Point pt)
{
    VS_DbgAssert( dims <= 2 && data &&
                  (unsigned)pt.y < (unsigned)size.p[0] &&
                  (unsigned)pt.x < (unsigned)size.p[1] &&
                  type() == DataType<_Tp>::type );
    return ((_Tp*)(data + step.p[0]*pt.y))[pt.x];
}

template<typename _Tp> inline const _Tp& Mat_<_Tp>::operator ()(Point pt) const
{
    VS_DbgAssert( dims <= 2 && data &&
                  (unsigned)pt.y < (unsigned)size.p[0] &&
                  (unsigned)pt.x < (unsigned)size.p[1] &&
                 type() == DataType<_Tp>::type );
    return ((const _Tp*)(data + step.p[0]*pt.y))[pt.x];
}

template<typename _Tp> inline _Tp& Mat_<_Tp>::operator ()(const int* idx)
{
    return Mat::at<_Tp>(idx);
}

template<typename _Tp> inline const _Tp& Mat_<_Tp>::operator ()(const int* idx) const
{
    return Mat::at<_Tp>(idx);
}

template<typename _Tp> template<int n> inline _Tp& Mat_<_Tp>::operator ()(const Vec<int, n>& idx)
{
    return Mat::at<_Tp>(idx);
}

template<typename _Tp> template<int n> inline const _Tp& Mat_<_Tp>::operator ()(const Vec<int, n>& idx) const
{
    return Mat::at<_Tp>(idx);
}

template<typename _Tp> inline _Tp& Mat_<_Tp>::operator ()(int i0)
{
    return this->at<_Tp>(i0);
}

template<typename _Tp> inline const _Tp& Mat_<_Tp>::operator ()(int i0) const
{
    return this->at<_Tp>(i0);
}

template<typename _Tp> inline _Tp& Mat_<_Tp>::operator ()(int i0, int i1, int i2)
{
    return this->at<_Tp>(i0, i1, i2);
}

template<typename _Tp> inline const _Tp& Mat_<_Tp>::operator ()(int i0, int i1, int i2) const
{
    return this->at<_Tp>(i0, i1, i2);
}


template<typename _Tp> inline Mat_<_Tp>::operator vector<_Tp>() const
{
    vector<_Tp> v;
    copyTo(v);
    return v;
}

template<typename _Tp> template<int n> inline Mat_<_Tp>::operator Vec<typename DataType<_Tp>::channel_type, n>() const
{
    VS_Assert(n % DataType<_Tp>::channels == 0);
    return this->Mat::operator Vec<typename DataType<_Tp>::channel_type, n>();
}

template<typename _Tp> template<int m, int n> inline Mat_<_Tp>::operator Matx<typename DataType<_Tp>::channel_type, m, n>() const
{
    VS_Assert(n % DataType<_Tp>::channels == 0);

    Matx<typename DataType<_Tp>::channel_type, m, n> res = this->Mat::operator Matx<typename DataType<_Tp>::channel_type, m, n>();
    return res;
}

template<typename T1, typename T2, typename Op> inline void
process( const Mat_<T1>& m1, Mat_<T2>& m2, Op op )
{
    int y, x, rows = m1.rows, cols = m1.cols;

    VS_DbgAssert( m1.size() == m2.size() );

    for( y = 0; y < rows; y++ )
    {
        const T1* src = m1[y];
        T2* dst = m2[y];

        for( x = 0; x < cols; x++ )
            dst[x] = op(src[x]);
    }
}

template<typename T1, typename T2, typename T3, typename Op> inline void
process( const Mat_<T1>& m1, const Mat_<T2>& m2, Mat_<T3>& m3, Op op )
{
    int y, x, rows = m1.rows, cols = m1.cols;

    VS_DbgAssert( m1.size() == m2.size() );

    for( y = 0; y < rows; y++ )
    {
        const T1* src1 = m1[y];
        const T2* src2 = m2[y];
        T3* dst = m3[y];

        for( x = 0; x < cols; x++ )
            dst[x] = op( src1[x], src2[x] );
    }
}


/////////////////////////////// Input/Output Arrays /////////////////////////////////

template<typename _Tp> inline _InputArray::_InputArray(const vector<_Tp>& vec)
    : flags(FIXED_TYPE + STD_VECTOR + DataType<_Tp>::type), obj((void*)&vec) {}

template<typename _Tp> inline _InputArray::_InputArray(const vector<vector<_Tp> >& vec)
    : flags(FIXED_TYPE + STD_VECTOR_VECTOR + DataType<_Tp>::type), obj((void*)&vec) {}

template<typename _Tp> inline _InputArray::_InputArray(const vector<Mat_<_Tp> >& vec)
    : flags(FIXED_TYPE + STD_VECTOR_MAT + DataType<_Tp>::type), obj((void*)&vec) {}

template<typename _Tp, int m, int n> inline _InputArray::_InputArray(const Matx<_Tp, m, n>& mtx)
    : flags(FIXED_TYPE + FIXED_SIZE + MATX + DataType<_Tp>::type), obj((void*)&mtx), sz(n, m) {}

template<typename _Tp> inline _InputArray::_InputArray(const _Tp* vec, int n)
    : flags(FIXED_TYPE + FIXED_SIZE + MATX + DataType<_Tp>::type), obj((void*)vec), sz(n, 1) {}

inline _InputArray::_InputArray(const Scalar& s)
    : flags(FIXED_TYPE + FIXED_SIZE + MATX + VS_64F), obj((void*)&s), sz(1, 4) {}

template<typename _Tp> inline _InputArray::_InputArray(const Mat_<_Tp>& m)
    : flags(FIXED_TYPE + MAT + DataType<_Tp>::type), obj((void*)&m) {}

template<typename _Tp> inline _OutputArray::_OutputArray(vector<_Tp>& vec)
    : _InputArray(vec) {}
template<typename _Tp> inline _OutputArray::_OutputArray(vector<vector<_Tp> >& vec)
    : _InputArray(vec) {}
template<typename _Tp> inline _OutputArray::_OutputArray(vector<Mat_<_Tp> >& vec)
    : _InputArray(vec) {}
template<typename _Tp> inline _OutputArray::_OutputArray(Mat_<_Tp>& m)
    : _InputArray(m) {}
template<typename _Tp, int m, int n> inline _OutputArray::_OutputArray(Matx<_Tp, m, n>& mtx)
    : _InputArray(mtx) {}
template<typename _Tp> inline _OutputArray::_OutputArray(_Tp* vec, int n)
    : _InputArray(vec, n) {}

template<typename _Tp> inline _OutputArray::_OutputArray(const vector<_Tp>& vec)
    : _InputArray(vec) {flags |= FIXED_SIZE;}
template<typename _Tp> inline _OutputArray::_OutputArray(const vector<vector<_Tp> >& vec)
    : _InputArray(vec) {flags |= FIXED_SIZE;}
template<typename _Tp> inline _OutputArray::_OutputArray(const vector<Mat_<_Tp> >& vec)
    : _InputArray(vec) {flags |= FIXED_SIZE;}

template<typename _Tp> inline _OutputArray::_OutputArray(const Mat_<_Tp>& m)
    : _InputArray(m) {flags |= FIXED_SIZE;}
template<typename _Tp, int m, int n> inline _OutputArray::_OutputArray(const Matx<_Tp, m, n>& mtx)
    : _InputArray(mtx) {}
template<typename _Tp> inline _OutputArray::_OutputArray(const _Tp* vec, int n)
    : _InputArray(vec, n) {}

//////////////////////////////////// Matrix Expressions /////////////////////////////////////////

class VS_EXPORTS MatOp
{
public:
    MatOp() {};
    virtual ~MatOp() {};

    virtual bool elementWise(const MatExpr& expr) const;
    virtual void assign(const MatExpr& expr, Mat& m, int type=-1) const = 0;
    virtual void roi(const MatExpr& expr, const Range& rowRange,
                     const Range& colRange, MatExpr& res) const;
    virtual void diag(const MatExpr& expr, int d, MatExpr& res) const;
    virtual void augAssignAdd(const MatExpr& expr, Mat& m) const;
    virtual void augAssignSubtract(const MatExpr& expr, Mat& m) const;
    virtual void augAssignMultiply(const MatExpr& expr, Mat& m) const;
    virtual void augAssignDivide(const MatExpr& expr, Mat& m) const;
    virtual void augAssignAnd(const MatExpr& expr, Mat& m) const;
    virtual void augAssignOr(const MatExpr& expr, Mat& m) const;
    virtual void augAssignXor(const MatExpr& expr, Mat& m) const;

    virtual void add(const MatExpr& expr1, const MatExpr& expr2, MatExpr& res) const;
    virtual void add(const MatExpr& expr1, const Scalar& s, MatExpr& res) const;

    virtual void subtract(const MatExpr& expr1, const MatExpr& expr2, MatExpr& res) const;
    virtual void subtract(const Scalar& s, const MatExpr& expr, MatExpr& res) const;

    virtual void multiply(const MatExpr& expr1, const MatExpr& expr2, MatExpr& res, double scale=1) const;
    virtual void multiply(const MatExpr& expr1, double s, MatExpr& res) const;

    virtual void divide(const MatExpr& expr1, const MatExpr& expr2, MatExpr& res, double scale=1) const;
    virtual void divide(double s, const MatExpr& expr, MatExpr& res) const;

    virtual void abs(const MatExpr& expr, MatExpr& res) const;

    virtual void transpose(const MatExpr& expr, MatExpr& res) const;
    virtual void matmul(const MatExpr& expr1, const MatExpr& expr2, MatExpr& res) const;
    virtual void invert(const MatExpr& expr, int method, MatExpr& res) const;

    virtual Size size(const MatExpr& expr) const;
    virtual int type(const MatExpr& expr) const;
};


class VS_EXPORTS MatExpr
{
public:
    MatExpr() : op(0), flags(0), a(Mat()), b(Mat()), c(Mat()), alpha(0), beta(0), s(Scalar()) {}
    MatExpr(const MatOp* _op, int _flags, const Mat& _a=Mat(), const Mat& _b=Mat(),
            const Mat& _c=Mat(), double _alpha=1, double _beta=1, const Scalar& _s=Scalar())
        : op(_op), flags(_flags), a(_a), b(_b), c(_c), alpha(_alpha), beta(_beta), s(_s) {}
    explicit MatExpr(const Mat& m);
    operator Mat() const
    {
        Mat m;
        op->assign(*this, m);
        return m;
    }

    template<typename _Tp> operator Mat_<_Tp>() const
    {
        Mat_<_Tp> m;
        op->assign(*this, m, DataType<_Tp>::type);
        return m;
    }

    MatExpr row(int y) const;
    MatExpr col(int x) const;
    MatExpr diag(int d=0) const;
    MatExpr operator()( const Range& rowRange, const Range& colRange ) const;
    MatExpr operator()( const Rect& roi ) const;

    Mat cross(const Mat& m) const;
    double dot(const Mat& m) const;

    MatExpr t() const;
    MatExpr inv(int method = DECOMP_LU) const;
    MatExpr mul(const MatExpr& e, double scale=1) const;
    MatExpr mul(const Mat& m, double scale=1) const;

    Size size() const;
    int type() const;

    const MatOp* op;
    int flags;

    Mat a, b, c;
    double alpha, beta;
    Scalar s;
};


VS_EXPORTS MatExpr operator + (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator + (const Mat& a, const Scalar& s);
VS_EXPORTS MatExpr operator + (const Scalar& s, const Mat& a);
VS_EXPORTS MatExpr operator + (const MatExpr& e, const Mat& m);
VS_EXPORTS MatExpr operator + (const Mat& m, const MatExpr& e);
VS_EXPORTS MatExpr operator + (const MatExpr& e, const Scalar& s);
VS_EXPORTS MatExpr operator + (const Scalar& s, const MatExpr& e);
VS_EXPORTS MatExpr operator + (const MatExpr& e1, const MatExpr& e2);

VS_EXPORTS MatExpr operator - (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator - (const Mat& a, const Scalar& s);
VS_EXPORTS MatExpr operator - (const Scalar& s, const Mat& a);
VS_EXPORTS MatExpr operator - (const MatExpr& e, const Mat& m);
VS_EXPORTS MatExpr operator - (const Mat& m, const MatExpr& e);
VS_EXPORTS MatExpr operator - (const MatExpr& e, const Scalar& s);
VS_EXPORTS MatExpr operator - (const Scalar& s, const MatExpr& e);
VS_EXPORTS MatExpr operator - (const MatExpr& e1, const MatExpr& e2);

VS_EXPORTS MatExpr operator - (const Mat& m);
VS_EXPORTS MatExpr operator - (const MatExpr& e);

VS_EXPORTS MatExpr operator * (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator * (const Mat& a, double s);
VS_EXPORTS MatExpr operator * (double s, const Mat& a);
VS_EXPORTS MatExpr operator * (const MatExpr& e, const Mat& m);
VS_EXPORTS MatExpr operator * (const Mat& m, const MatExpr& e);
VS_EXPORTS MatExpr operator * (const MatExpr& e, double s);
VS_EXPORTS MatExpr operator * (double s, const MatExpr& e);
VS_EXPORTS MatExpr operator * (const MatExpr& e1, const MatExpr& e2);

VS_EXPORTS MatExpr operator / (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator / (const Mat& a, double s);
VS_EXPORTS MatExpr operator / (double s, const Mat& a);
VS_EXPORTS MatExpr operator / (const MatExpr& e, const Mat& m);
VS_EXPORTS MatExpr operator / (const Mat& m, const MatExpr& e);
VS_EXPORTS MatExpr operator / (const MatExpr& e, double s);
VS_EXPORTS MatExpr operator / (double s, const MatExpr& e);
VS_EXPORTS MatExpr operator / (const MatExpr& e1, const MatExpr& e2);

VS_EXPORTS MatExpr operator < (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator < (const Mat& a, double s);
VS_EXPORTS MatExpr operator < (double s, const Mat& a);

VS_EXPORTS MatExpr operator <= (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator <= (const Mat& a, double s);
VS_EXPORTS MatExpr operator <= (double s, const Mat& a);

VS_EXPORTS MatExpr operator == (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator == (const Mat& a, double s);
VS_EXPORTS MatExpr operator == (double s, const Mat& a);

VS_EXPORTS MatExpr operator != (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator != (const Mat& a, double s);
VS_EXPORTS MatExpr operator != (double s, const Mat& a);

VS_EXPORTS MatExpr operator >= (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator >= (const Mat& a, double s);
VS_EXPORTS MatExpr operator >= (double s, const Mat& a);

VS_EXPORTS MatExpr operator > (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator > (const Mat& a, double s);
VS_EXPORTS MatExpr operator > (double s, const Mat& a);

VS_EXPORTS MatExpr min(const Mat& a, const Mat& b);
VS_EXPORTS MatExpr min(const Mat& a, double s);
VS_EXPORTS MatExpr min(double s, const Mat& a);

VS_EXPORTS MatExpr max(const Mat& a, const Mat& b);
VS_EXPORTS MatExpr max(const Mat& a, double s);
VS_EXPORTS MatExpr max(double s, const Mat& a);

template<typename _Tp> static inline MatExpr min(const Mat_<_Tp>& a, const Mat_<_Tp>& b)
{
    return vs::min((const Mat&)a, (const Mat&)b);
}

template<typename _Tp> static inline MatExpr min(const Mat_<_Tp>& a, double s)
{
    return vs::min((const Mat&)a, s);
}

template<typename _Tp> static inline MatExpr min(double s, const Mat_<_Tp>& a)
{
    return vs::min((const Mat&)a, s);
}

template<typename _Tp> static inline MatExpr max(const Mat_<_Tp>& a, const Mat_<_Tp>& b)
{
    return vs::max((const Mat&)a, (const Mat&)b);
}

template<typename _Tp> static inline MatExpr max(const Mat_<_Tp>& a, double s)
{
    return vs::max((const Mat&)a, s);
}

template<typename _Tp> static inline MatExpr max(double s, const Mat_<_Tp>& a)
{
    return vs::max((const Mat&)a, s);
}

template<typename _Tp> static inline void min(const Mat_<_Tp>& a, const Mat_<_Tp>& b, Mat_<_Tp>& c)
{
    vs::min((const Mat&)a, (const Mat&)b, (Mat&)c);
}

template<typename _Tp> static inline void min(const Mat_<_Tp>& a, double s, Mat_<_Tp>& c)
{
    vs::min((const Mat&)a, s, (Mat&)c);
}

template<typename _Tp> static inline void min(double s, const Mat_<_Tp>& a, Mat_<_Tp>& c)
{
    vs::min((const Mat&)a, s, (Mat&)c);
}

template<typename _Tp> static inline void max(const Mat_<_Tp>& a, const Mat_<_Tp>& b, Mat_<_Tp>& c)
{
    vs::max((const Mat&)a, (const Mat&)b, (Mat&)c);
}

template<typename _Tp> static inline void max(const Mat_<_Tp>& a, double s, Mat_<_Tp>& c)
{
    vs::max((const Mat&)a, s, (Mat&)c);
}

template<typename _Tp> static inline void max(double s, const Mat_<_Tp>& a, Mat_<_Tp>& c)
{
    vs::max((const Mat&)a, s, (Mat&)c);
}


VS_EXPORTS MatExpr operator & (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator & (const Mat& a, const Scalar& s);
VS_EXPORTS MatExpr operator & (const Scalar& s, const Mat& a);

VS_EXPORTS MatExpr operator | (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator | (const Mat& a, const Scalar& s);
VS_EXPORTS MatExpr operator | (const Scalar& s, const Mat& a);

VS_EXPORTS MatExpr operator ^ (const Mat& a, const Mat& b);
VS_EXPORTS MatExpr operator ^ (const Mat& a, const Scalar& s);
VS_EXPORTS MatExpr operator ^ (const Scalar& s, const Mat& a);

VS_EXPORTS MatExpr operator ~(const Mat& m);

VS_EXPORTS MatExpr abs(const Mat& m);
VS_EXPORTS MatExpr abs(const MatExpr& e);

template<typename _Tp> static inline MatExpr abs(const Mat_<_Tp>& m)
{
    return vs::abs((const Mat&)m);
}

////////////////////////////// Augmenting algebraic operations //////////////////////////////////

inline Mat& Mat::operator = (const MatExpr& e)
{
    e.op->assign(e, *this);
    return *this;
}

template<typename _Tp> inline Mat_<_Tp>::Mat_(const MatExpr& e)
{
    e.op->assign(e, *this, DataType<_Tp>::type);
}

template<typename _Tp> Mat_<_Tp>& Mat_<_Tp>::operator = (const MatExpr& e)
{
    e.op->assign(e, *this, DataType<_Tp>::type);
    return *this;
}

static inline Mat& operator += (const Mat& a, const Mat& b)
{
    add(a, b, (Mat&)a);
    return (Mat&)a;
}

static inline Mat& operator += (const Mat& a, const Scalar& s)
{
    add(a, s, (Mat&)a);
    return (Mat&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator += (const Mat_<_Tp>& a, const Mat_<_Tp>& b)
{
    add(a, b, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator += (const Mat_<_Tp>& a, const Scalar& s)
{
    add(a, s, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

static inline Mat& operator += (const Mat& a, const MatExpr& b)
{
    b.op->augAssignAdd(b, (Mat&)a);
    return (Mat&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator += (const Mat_<_Tp>& a, const MatExpr& b)
{
    b.op->augAssignAdd(b, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

static inline Mat& operator -= (const Mat& a, const Mat& b)
{
    subtract(a, b, (Mat&)a);
    return (Mat&)a;
}

static inline Mat& operator -= (const Mat& a, const Scalar& s)
{
    subtract(a, s, (Mat&)a);
    return (Mat&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator -= (const Mat_<_Tp>& a, const Mat_<_Tp>& b)
{
    subtract(a, b, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator -= (const Mat_<_Tp>& a, const Scalar& s)
{
    subtract(a, s, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

static inline Mat& operator -= (const Mat& a, const MatExpr& b)
{
    b.op->augAssignSubtract(b, (Mat&)a);
    return (Mat&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator -= (const Mat_<_Tp>& a, const MatExpr& b)
{
    b.op->augAssignSubtract(b, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

static inline Mat& operator *= (const Mat& a, const Mat& b)
{
    gemm(a, b, 1, Mat(), 0, (Mat&)a, 0);
    return (Mat&)a;
}

static inline Mat& operator *= (const Mat& a, double s)
{
    a.convertTo((Mat&)a, -1, s);
    return (Mat&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator *= (const Mat_<_Tp>& a, const Mat_<_Tp>& b)
{
    gemm(a, b, 1, Mat(), 0, (Mat&)a, 0);
    return (Mat_<_Tp>&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator *= (const Mat_<_Tp>& a, double s)
{
    a.convertTo((Mat&)a, -1, s);
    return (Mat_<_Tp>&)a;
}

static inline Mat& operator *= (const Mat& a, const MatExpr& b)
{
    b.op->augAssignMultiply(b, (Mat&)a);
    return (Mat&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator *= (const Mat_<_Tp>& a, const MatExpr& b)
{
    b.op->augAssignMultiply(b, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

static inline Mat& operator /= (const Mat& a, const Mat& b)
{
    divide(a, b, (Mat&)a);
    return (Mat&)a;
}

static inline Mat& operator /= (const Mat& a, double s)
{
    a.convertTo((Mat&)a, -1, 1./s);
    return (Mat&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator /= (const Mat_<_Tp>& a, const Mat_<_Tp>& b)
{
    divide(a, b, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator /= (const Mat_<_Tp>& a, double s)
{
    a.convertTo((Mat&)a, -1, 1./s);
    return (Mat_<_Tp>&)a;
}

static inline Mat& operator /= (const Mat& a, const MatExpr& b)
{
    b.op->augAssignDivide(b, (Mat&)a);
    return (Mat&)a;
}

template<typename _Tp> static inline
Mat_<_Tp>& operator /= (const Mat_<_Tp>& a, const MatExpr& b)
{
    b.op->augAssignDivide(b, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

////////////////////////////// Logical operations ///////////////////////////////

static inline Mat& operator &= (const Mat& a, const Mat& b)
{
    bitwise_and(a, b, (Mat&)a);
    return (Mat&)a;
}

static inline Mat& operator &= (const Mat& a, const Scalar& s)
{
    bitwise_and(a, s, (Mat&)a);
    return (Mat&)a;
}

template<typename _Tp> static inline Mat_<_Tp>&
operator &= (const Mat_<_Tp>& a, const Mat_<_Tp>& b)
{
    bitwise_and(a, b, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

template<typename _Tp> static inline Mat_<_Tp>&
operator &= (const Mat_<_Tp>& a, const Scalar& s)
{
    bitwise_and(a, s, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

static inline Mat& operator |= (const Mat& a, const Mat& b)
{
    bitwise_or(a, b, (Mat&)a);
    return (Mat&)a;
}

static inline Mat& operator |= (const Mat& a, const Scalar& s)
{
    bitwise_or(a, s, (Mat&)a);
    return (Mat&)a;
}

template<typename _Tp> static inline Mat_<_Tp>&
operator |= (const Mat_<_Tp>& a, const Mat_<_Tp>& b)
{
    bitwise_or(a, b, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

template<typename _Tp> static inline Mat_<_Tp>&
operator |= (const Mat_<_Tp>& a, const Scalar& s)
{
    bitwise_or(a, s, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

static inline Mat& operator ^= (const Mat& a, const Mat& b)
{
    bitwise_xor(a, b, (Mat&)a);
    return (Mat&)a;
}

static inline Mat& operator ^= (const Mat& a, const Scalar& s)
{
    bitwise_xor(a, s, (Mat&)a);
    return (Mat&)a;
}

template<typename _Tp> static inline Mat_<_Tp>&
operator ^= (const Mat_<_Tp>& a, const Mat_<_Tp>& b)
{
    bitwise_xor(a, b, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

template<typename _Tp> static inline Mat_<_Tp>&
operator ^= (const Mat_<_Tp>& a, const Scalar& s)
{
    bitwise_xor(a, s, (Mat&)a);
    return (Mat_<_Tp>&)a;
}

/////////////////////////////// Miscellaneous operations //////////////////////////////

template<typename _Tp> void split(const Mat& src, vector<Mat_<_Tp> >& mv)
{ split(src, (vector<Mat>&)mv ); }

//////////////////////////////////////////////////////////////

template<typename _Tp> inline MatExpr Mat_<_Tp>::zeros(int rows, int cols)
{
    return Mat::zeros(rows, cols, DataType<_Tp>::type);
}

template<typename _Tp> inline MatExpr Mat_<_Tp>::zeros(Size sz)
{
    return Mat::zeros(sz, DataType<_Tp>::type);
}

template<typename _Tp> inline MatExpr Mat_<_Tp>::ones(int rows, int cols)
{
    return Mat::ones(rows, cols, DataType<_Tp>::type);
}

template<typename _Tp> inline MatExpr Mat_<_Tp>::ones(Size sz)
{
    return Mat::ones(sz, DataType<_Tp>::type);
}

template<typename _Tp> inline MatExpr Mat_<_Tp>::eye(int rows, int cols)
{
    return Mat::eye(rows, cols, DataType<_Tp>::type);
}

template<typename _Tp> inline MatExpr Mat_<_Tp>::eye(Size sz)
{
    return Mat::eye(sz, DataType<_Tp>::type);
}

}

#endif
#endif
