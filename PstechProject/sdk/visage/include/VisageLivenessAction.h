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

#ifndef __VisageLivenessAction_h__
#define __VisageLivenessAction_h__

#ifdef VISAGE_STATIC
#define VISAGE_DECLSPEC
#else

	#ifdef VISAGE_EXPORTS
	#define VISAGE_DECLSPEC __declspec(dllexport)
	#else
	#define VISAGE_DECLSPEC __declspec(dllimport)
	#endif

#endif

#include "FaceData.h"

namespace VisageSDK
{	
	class SignalProcessor;

	/**
	* VisageLivenessAction is an abstract class that can be used to implement algorithm which searches for a specific pattern in live video stream in order to verify liveness.
	* On each call of update() method, action should process face data obtained by
	* VisageTracker::track() and/or video frame and return action's current state. If user interaction is required, appropriate message and/or image can be displayed to user.
	*
	* The action can go through different states while it is trying to be verified. VisageLivenessAction defines only final state where action is performed and verified (STATE_ACTION_VERIFIED). 
	*
	* Following actions are implemented :
	* - VisageLivenessBrowRaise - verifies that both eyebrows have been raised
	* - VisageLivenessSmile - verifies that the person has smiled
	* - VisageLivenessBlink - verifies that a blink has occurred
	* 
	*/
	class VISAGE_DECLSPEC VisageLivenessAction {

	public:

		virtual ~VisageLivenessAction() {};

		/**
		* Method used to process input data in order to confirm that the action has occured.
		* This method is supposed to be called for each frame until the action is verified. 
		* If action uses \p faceData, it should only be called when tracking status is TRACK_STAT_OK.
		* Method should return action's current state and in case user interaction is required, appropriate message and/or image can be displayed.
		* @param faceData face data of the current frame.
		* @param frame current frame that needs to be processed by action.
		* @return action's current state
		*/
		virtual int update(const FaceData *faceData, VsImage *frame = 0) = 0;

		/**
		* Resets action state.
		*/
		virtual void reset() = 0;

		/**
		* Describes action's state where the action was performed and verified.
		*/
		static const int STATE_ACTION_VERIFIED = 0;

	};

}

#endif // __VisageLivenessAction_h__