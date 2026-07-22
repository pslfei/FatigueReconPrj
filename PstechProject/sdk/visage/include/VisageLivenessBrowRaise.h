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
#ifndef VSDK_VISAGELIVENESSBROWRAISE_H
#define VSDK_VISAGELIVENESSBROWRAISE_H

#ifdef VISAGE_STATIC
#define VISAGE_DECLSPEC
#else

#ifdef VISAGE_EXPORTS
#define VISAGE_DECLSPEC __declspec(dllexport)
#else
#define VISAGE_DECLSPEC __declspec(dllimport)
#endif

#endif


#include "VisageLivenessAction.h"
#include "FaceData.h"

namespace VisageSDK
{
	/**
	* VisageLivenessBrowRaise is a class that detects whether the person on the live video stream has raised his/hers eyebrows.
	* In order for action to be detected accurately, it is required that the face is frontal and facial expression is neutral for 30 frames
	* before performing eyebrows raise.\n During eyebrows raise verification, face data from latest 30 frames ("face data sequence"), i.e. last 30 calls of update() is used to verify head stability (STATE_WAIT_FOR_STABLE).
	* Each time the action is reset, face data sequence will be cleared and the action will need to go through initializing state (STATE_ACTION_INITIALIZING)
	* to store initial face data sequence.
	* Additionally, face data sequence is used as a baseline for neutral facial expression when detecting if eyebrows raise was performed.
	* Algorithm presumes that the face will be neutral before performing eyebrows raise.
	*
	* Action should be updated for each frame in order to update its internal state.
	* The action can be in one of the following states (states are sequentially verified in each call of update()):
	*	1. <b>STATE_WAIT_FOR_FRONTAL</b> Face is not frontal. Action is reset.
	*	2. <b>STATE_ACTION_INITIALIZING</b> Action is storing first set of face data sequence. After each reset, action needs to reinitialize.
	*	3. <b>STATE_WAIT_FOR_STABLE</b> Head is not stable. In order for action to be verified successfully, the user should keep his head still.
	*	4. <b>STATE_WAIT_FOR_ACTION</b> Data is processed in order to verify eyebrows raise.
	*	5. <b>STATE_ACTION_VERIFIED</b> Action is verified.
	*	.
	*
	*/

	class VISAGE_DECLSPEC VisageLivenessBrowRaise : public VisageLivenessAction {
	public:

		/**
		* Constructor
		*/
		VisageLivenessBrowRaise();

		/**
		* Destructor
		*/
		~VisageLivenessBrowRaise();

		/**
		* Method that should be called for each frame until the action is verified. It should only be called when tracking status is TRACK_STAT_OK.
		* Returns action's current state so appropriate message and/or image can be displayed if user interaction is required (e.g. warning for user or instruction on how to perform eyebrows raise).
		* @param faceData face data obtained from the tracker
		* @param frame optional image to display
		* @return action's current state. State can be one of the following: \n
		* - STATE_WAIT_FOR_FRONTAL, STATE_ACTION_INITIALIZING, STATE_WAIT_FOR_STABLE, STATE_WAIT_FOR_ACTION, STATE_ACTION_VERIFIED.
		*/
		int update(const FaceData *faceData, VsImage*);

		/**
		* Resets action. Clears face data sequence.
		*/
		void reset();

		/**
		* Describes action's state where face in the frame is not frontal (head rotations larger than 0.3 rad, ~17 degrees).
		* If this state is detected, the action will be reset.
		*/
		static const int STATE_WAIT_FOR_FRONTAL = 1;

		/**
		* Describes action's state while the action is initializing, i.e. storing initial face data sequence.
		* After each reset, action needs to reinitialize.
		*/
		static const int STATE_ACTION_INITIALIZING = 2;

		/**
		* Describes action's state when user is not keeping his head still.
		*/
		static const int STATE_WAIT_FOR_STABLE = 3;

		/**
		* Describes action's state when eyebrows raise is trying to be verified.
		*/
		static const int STATE_WAIT_FOR_ACTION = 4;
		

	private:

		void resetInternal();

		int counterForInit;

		bool faceNotFrontal;

		bool HeadStable;

		bool actionDone;

		SignalProcessor *SPLeft;

		SignalProcessor *SPRight;

	};
}

#endif //VSDK_VISAGELIVENESSBROWRAISE_H