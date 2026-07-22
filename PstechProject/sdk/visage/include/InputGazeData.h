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

#ifndef INPUTGAZEDATA_H
#define INPUTGAZEDATA_H

#include "FDP.h"
#include "FaceData.h"

namespace VisageSDK
{
struct InputGazeData
{
	FDP featurePoints2D;
	float faceRotation[3];
	float faceTranslation[3];
	float gazeDirection[2];
	float eyeClosure[2];
	int width, height, index;
	bool isDefined;
	bool isCalib;
	long timestamp;
	float frameRate;
	int usedEye;
	float trackingQuality;

	InputGazeData();
	~InputGazeData();

	void setData(int index, bool isDefined, FaceData* data, bool isCalib, long timestamp, VsImage* frame);
};
}
#endif