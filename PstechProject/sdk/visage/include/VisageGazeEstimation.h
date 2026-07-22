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

#ifndef VISAGEGAZEESTIMATION_H
#define VISAGEGAZEESTIMATION_H

#include "ScreenSpaceGazeMapping.h"

namespace VisageSDK
{

class GazeEstimationPredictor
{
	public:
		virtual float Predict(std::vector<VsMat*> sample) = 0;
		virtual ~GazeEstimationPredictor() {};
};

class GazeEstimationCalibrator
{
	public:
		virtual GazeEstimationPredictor* Calibrate(std::vector<VsMat*> samples, const VsMat* observations, GaussianProcessMapping& mapping) = 0;
		virtual ~GazeEstimationCalibrator() {};
};

class GaussianProcessPredictor : public GazeEstimationPredictor
{
	public:

		GaussianProcessPredictor(const GaussianProcessMapping& mapping, VsMat* coeficient);

		GaussianProcessPredictor(const GaussianProcessPredictor& predictor);

		GaussianProcessPredictor& operator=(const GaussianProcessPredictor& predictor);

		virtual ~GaussianProcessPredictor();

		float Predict(std::vector<VsMat*> sample);
	
		private:
			GaussianProcessMapping mapping;
			VsMat* coeficient;
};

class GaussianProcessCalibrator : public GazeEstimationCalibrator
{
	public:
		GazeEstimationPredictor* Calibrate(std::vector<VsMat*> samples, const VsMat* observations, GaussianProcessMapping& mapping);
};

class GeometricPredictor : GazeEstimationPredictor
{

};

class GeometricCalibrator : GazeEstimationCalibrator
{

};


}
#endif