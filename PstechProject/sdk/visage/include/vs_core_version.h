/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright( C) 2000, Intel Corporation, all rights reserved.
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
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
//(including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort(including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/*
  definition of the current version of OpenCV
  Usefull to test in user programs
*/

#ifndef __VS_VERSION_HPP__
#define __VS_VERSION_HPP__

#define VS_VERSION_EPOCH    2
#define VS_VERSION_MAJOR    4
#define VS_VERSION_MINOR    11
#define VS_VERSION_REVISION 0

#define VS_AUX_STR_EXP(__A)  #__A
#define VS_AUX_STR(__A)      VS_AUX_STR_EXP(__A)

#define VS_AUX_STRW_EXP(__A)  L#__A
#define VS_AUX_STRW(__A)      VS_AUX_STRW_EXP(__A)

#if VS_VERSION_REVISION
#  define VS_VERSION        VS_AUX_STR(VS_VERSION_EPOCH) "." VS_AUX_STR(VS_VERSION_MAJOR) "." VS_AUX_STR(VS_VERSION_MINOR) "." VS_AUX_STR(VS_VERSION_REVISION)
#else
#  define VS_VERSION        VS_AUX_STR(VS_VERSION_EPOCH) "." VS_AUX_STR(VS_VERSION_MAJOR) "." VS_AUX_STR(VS_VERSION_MINOR)
#endif

/* old  style version constants*/
#define VS_MAJOR_VERSION    VS_VERSION_EPOCH
#define VS_MINOR_VERSION    VS_VERSION_MAJOR
#define VS_SUBMINOR_VERSION VS_VERSION_MINOR

#endif
