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


#ifndef __TrackerGUIInterface_h__
#define __TrackerGUIInterface_h__

#ifdef VISAGE_STATIC
	#define VISAGE_DECLSPEC
#else

	#ifdef VISAGE_EXPORTS
		#define VISAGE_DECLSPEC __declspec(dllexport)
	#else
		#define VISAGE_DECLSPEC __declspec(dllimport)
	#endif

#endif

#ifdef D_EXPORT_TESTING
#define TESTING_DECLSPEC VISAGE_DECLSPEC
#else
#define TESTING_DECLSPEC
#endif


#include <cstdlib>

namespace VisageSDK
{

class VisageTracker;
class FDP;

/* Optional GUI Interface for VisageTracker.
*
* When VisageTracker is configured for semi-automatic operation it
* requires some interaction with the user through a Graphical User Interface (GUI).
* For this purpose, the application that uses the tracker in semi-automatic modes must
* implement an instance of the TrackerGUIInterface abstract class and provide
* the necessary functions to the tracker by implementing the functions
* in TrackerGUIInterface. 
*
* All functions in this class are virtual and must be implemented in the 
* inherited class.
*
* \if IOS_DOXY
* A sample implementation of TracerGUIInterface is currently not available on iOS.
* \elseif ANDROID_DOXY
* A sample implementation of TracerGUIInterface is currently not available on Android.
* \else
* The full implementation of an TrackerGUIInterface is
* provided, (with source code) in the FaceTracker2 sample, where the
* class COpenGLWnd is an implementation of the TrackerGUIInterface.
*
* It is highly recommneded to run the FaceTracker2 sample and get familiar with all its
* functions by reading the tutorial and watching the tutorial videos in order
* to get a practical understanding of what various functions of TrackerGUIInterface do.
* \endif
*/
class TESTING_DECLSPEC TrackerGUIInterface
{
public:

	/** A virtual function for displaying a text message.
	*
	* VisageTracker calls this function when it needs to display a simple
	* text prompt to the user. The function displays the message and
	* lets the user click "OK" to continue. Essentiylly it is
	* the same as Windows MessageBox function.
	*
	* @param userMessage message to be displayed
	* @param caption the caption to be displayed on the message window
	* @param type type of message
	*/
	virtual void displayMessage(const char *userMessage,const char *caption,int type) = 0;
};

}
#endif // __TrackerGUIInterface_h__

