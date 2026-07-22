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


#ifndef __MathMacros_h__
#define __MathMacros_h__

// C++
#include <cmath>

namespace VisageSDK
{

#define V_PI 3.1415926535897932384626433832795f // PI
#define V_EQF( a, b ) ( ( fabs( ( a ) - ( b ) ) ) < ( 0.00001f ) ) // returns true if 2 float numbers are equal
#define V_EQFT( a, b, t ) ( ( fabs( ( a ) - ( b ) ) ) < ( t ) ) // returns true if 2 float numbers are equal with specified tolerance
#define V_RAD2DEG(x) ( (x) * 180.f / V_PI ) // converts radians to degrees
#define V_DEG2RAD(x) ( (x) * V_PI / 180.f ) // converts degrees to radians
#define V_CHECK_BIT(var,pos) ((var) & (1<<(pos))) // checks if bit on given position on given variable is set
#define V_SQUARE(x) ( (x) * (x) ) // returns squared value of x

}

#endif // __MathMacros_h__
