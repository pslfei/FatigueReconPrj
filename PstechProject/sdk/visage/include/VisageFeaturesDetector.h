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


#ifndef __VisageFeaturesDetector_h__
#define __VisageFeaturesDetector_h__

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

namespace VNN
{
	class INNRunner;
}

namespace VisageSDK
{

#ifndef ANDROID
	int VISAGE_DECLSPEC initializeLicenseManager(const char *licenseKeyFileFolder);
#endif

/** Returns visageSDK version and revision number.
*
* @return visageSDK version and revision number as char array
*/
const VISAGE_DECLSPEC char* getVisageSDKVersion(void);

class VisageDetector;
class Candide3Model;
class PupilRefiner;
class EarsRefiner;
class DeepAlignmentRuntime;
class VisageConfiguration;
class VNNFaceFinder;

/** Faces and facial features detector implementation.
* 
* This class can be used to detect one or more faces and their facial features in an image. The input is an image bitmap or an image file in one of the supported file formats: JPEG, PNG, BMP or PPM.
*
* VisageFeaturesDetector uses VsImage structure as a container for the input image. VsImage is the image storage class similar to IplImage from OpenCV; it has the same structure and members so it can be used like IplImage.  
* Please refer to OpenCV documentation for details of accessing IplImage data members; the basic members are the size of the image (frame->width, frame->height) and the pointer to the actual pixel data of the 
* image (frame->imageData). VisageFeaturesDetector expects input image to be in the RGB, RGBA or grayscale color format with bit depth set to 8 and with origin in the top left pixel of the image. 
* \n\n
* There are two ways to create input image:
* 1. Create IplImage and set image data using OpenCV. Image should be converted to VsImage before passing it to VisageFeaturesDetector. Please note that 
* OpenCV loads image (imread, cvLoadImage) by default in BGR format. In that case, the input image should be converted before passing it to detector.

* 2. visage|SDK provides methods similar to the ones from OpenCV which are used to create input image:
* \code
* VsImage *inputImage;
* inputImage = vsCreateImageHeader(vsSize(frameWidth, frameHeight), 8, 3);
* vsSetData(image, (void*)frameData, image->widthStep);
* vsReleaseImageHeader(&inputImage); //release memory
* \endcode
*
* To use the detector, it must first be initialized by calling the function @ref Initialize().
* The detector requires a set of data and a configuration file. For more information about required data please consult @ref Initialize() function description.
*
* <h4>Methods of detection</h4>
* Function @ref detectFaces() performs a fast face detection on the image. The result is, for each detected face, a VsRect object containing facial bounding box. VsRect is similar to CvRect from OpenCV; it has the 
* same structure and members.
*
* Function @ref detectFacialFeatures() performs a face and facial features detection on the image. The results is, for each detected face, FaceData object containing 3D head pose, coordinates of facial feature points 
* (e.g. pupils, nose tip, lip corners etc.), 3D face model fitted to the face and more. Please refer to the FaceData documentation for detailed description of returned data.
*
*
* Implemented in libVisageVision.lib
*
* \if IOS_DOXY
* Demonstrated in <a href="../facedetect.html">FaceDetector</a> sample project.
* \elseif MACOSX_DOXY
* Demonstrated in <a href="../facedetect.html">FaceDetector</a> sample project.
* \elseif LINUX_DOXY
* Demonstrated in <a href="../../Samples/Linux/build/VisageDetectorDemo/doc/index.html">FaceDetector</a> sample project.
* \elseif ANDROID_DOXY
* Demonstrated in <a href="../../../doc/facedetect.html">FaceDetector</a> sample project.
* \elseif REDHAT_DOXY
* Demonstrated in <a href="../../Samples/Linux/build/VisageDetectorDemo/doc/index.html">FaceDetector</a> sample project.
* \endif
*
*/
class VISAGE_DECLSPEC VisageFeaturesDetector {

public:

	/** Constructor.
	*
	* Creates detector object with specified configuration file.
	* 
	* Configuration file is used to modify detector behavior and set paths to the required data files.
	* Default detector configuration file <b>Face Detector.cfg</b> is provided in the <i>Samples/data</i> folder.
	*
	* To successfully initialize VisageFeaturesDetector, provide path to the configuration file that is located in folder with the following folder structure and data files:
	* - <i>vft</i> folder that contains algorithm and features specific files. <b>NOTE:</b> The subfolder structure and names must be retained.
	*     - Required folders and algorithm data files:
	*       - <i>ff/ *</i>
	*       - <i>fa/ *</i>
	*         - <b>NOTE:</b> If landmarks refinement (configuration parameter <b>refine_landmarks</b> [0]) is disabled, 
	*                \if (IOS_DOXY || ANDROID_DOXY) <i>d2.tflite</i> is not needed.
	*                \elseif (WIN64_DOXY || LINUX_DOXY || REDHAT_DOXY) <i>d2.vino.*</i> is not needed.
	*                \elseif (MACOSX_DOXY) <i>d2.vino.*</i> and <i>d2.tflite</i> are not needed.
	*                \endif
	*       - <i>fm/</i> containing <i>.fdp</i> and <i>.wfm</i> file corresponding to the 3D model (configuration parameters <b>*_fitting_model</b>, <b>*_fitting_fdp</b>)
	* 
	* \if MACOSX_DOXY
	* <sub><i>\*Data files with extension <b>.tflite</b> required when targetting <b>arm64</b> architecture, and with <b>.vino.\*</b> extension when targetting
	* <b>x86_64</b> architecture</i></sub>
	* \endif
	*
	* \if ANDROID_DOXY
	* Please note that data files packaged within Android <i>apk</i> file cannot be read by our API so it is necessary to copy all required files to some other destination, e.g. application's data folder.
	* Example of how this is done can be found in all our samples. For example, you can take look at FaceDetectDemo sample where copying is done in
	* <a href="StartupActivity_8java_source.html">StartupActivity.java</a>, in method <i>copyAssets()</i>.
	* \endif
	*
	* @param detectorConfigFile the name of the detector configuration file (.cfg),
	* \if IOS_DOXY
	* relative to the Resources folder of the application bundle, e.g. "Face Detector.cfg" (assuming the file is located in the root of the Xcode project's Resources folder).
	* \else
	* relative to the current working directory, e.g. "Samples/data/Face Detector.cfg", considering visageSDK as a working directory.
	* \endif
	*
	*/
	VisageFeaturesDetector(const char* detectorConfigFile);

	/** Destructor.
	*/
	~VisageFeaturesDetector();

	/** Initializes the features detector.
	*
	* @return true if successful
	*/
	bool Initialize();

	/**
	* Performs face detection in a still image.
	*
	* The algorithm detects one or more faces. For each detected face a square facial bounding box is returned.
	*
	* The results are returned in form of VsRect objects. An array of VsRect objects passed to this method as output parameter should be allocated to maxFaces size.
	* For example:
	*
	* \code
	* VsRect* faces = new VsRect[maxFaces];
	*
	* n = this->m_Detector->detectFaces(image, faces, maxFaces);
	* \endcode
	*
	* After this call, n contains the number of detected faces. The first n members of the faces array are filled with resulting bounding boxes for each detected face.
	* If maxFaces is smaller than the number of faces actually detected in the image, the function will return only first maxFaces detected faces.
	*
	* VsImage is the image storage class similar to IplImage from OpenCV, it has the same structure and members so it can be used like IplImage. Please refer to
	* OpenCV documentation for details of accessing IplImage data members; the basic members are the size of the image (frame->width, frame->height) and the pointer to the actual pixel data of the image (frame->imageData).
	* 
	* VisageFeaturesDetector expects input image to be in the RGB, RGBA or grayscale color space with bit depth set to 8 and with origin in the top left pixel of the image.
	*
	* Note that the input image is internally converted to grayscale.
	*
	* @param frame the input image.
	* @param faces pointer to an array of VsRect objects in which the results will be returned.
	* @param maxFaces maximum number of faces to be detected
	* @param minFaceScale scale of smallest face to be searched for, defined as decimal fraction [0.0 - 1.0] of input image size (min(width, height))
	* @param maxFaceScale scale of largest face to be searched for, defined as decimal fraction [0.0 - 1.0] of input image size (min(width, height))
	* @param useRefinementStep if set to true, additional refinement algorithm will be used resulting with more precise facial bounding boxes and lower FPR, but higher detection time 
	* @return number of detected faces (0 or more)
	*/
	int detectFaces(VsImage* frame, VsRect* faces, int maxFaces = 1, float minFaceScale = 0.1f, float maxFaceScale = 1.0f, bool useRefinementStep = true);

	/**
	* Performs faces and facial features detection in a still image.
	* 
	* The algorithm detects one or more faces and their features. The results are, for each detected face, the 3D head pose, gaze direction, eye closure, the coordinates of facial feature points 
	* (e.g. chin tip, nose tip, lip corners etc.) and 3D face model fitted to the face. If outputOnly2DFeatures is set, only the 2D feature points will be returned.
	*
	* The results are returned in form of FaceData objects. An array of FaceData objects passed to this method as output parameter should be allocated to maxFaces size. 
	* For example:
	* 
	* \code
	* FaceData* data = new FaceData[maxFaces];
	*
	* n = this->m_Detector->detectFacialFeatures(image, data, maxFaces, minFaceScale);
	* \endcode
	*
	* After this call, n contains the number of faces actually detected. The first n members of the data array are filled with resulting data for each detected face.
	* Please refer to the FaceData documentation for detailed description of returned parameters. If maxFaces is smaller than the number of faces actually present in the image, the function will return only first maxFaces detected faces.
	*
	* VsImage is the image storage class similar to IplImage from OpenCV, it has the same structure and members so it can be used like IplImage. Please refer to 
	* OpenCV documentation for details of accessing IplImage data members; the basic members are the size of the image (frame->width, frame->height) and the pointer to the actual pixel data of the image (frame->imageData).
	*
	* VisageFeaturesDetector expects input image to be in the RGB, RGBA or grayscale color space with bit depth set to 8 and with origin in the top left pixel of the image.
	*
	* Note that the input image is internally converted to grayscale.
	*
	* @param frame the input image. 
	* @param output pointer to an array of FaceData objects in which the results will be returned.
	* @param maxFaces maximum number of faces to be detected
	* @param minFaceScale scale of smallest face to be searched for, defined as decimal fraction [0.0 - 1.0] of input image size (min(width, height))
	* @param maxFaceScale scale of largest face to be searched for, defined as decimal fraction [0.0 - 1.0] of input image size (min(width, height))
	* @param outputOnly2DFeatures if set, detection time will be reduced and only FeaturePoints2D will be returned
	* @return number of detected faces (0 or more)
	*
	* @see FaceData
	*/
	int detectFacialFeatures(VsImage* frame, FaceData* output, int maxFaces = 1, float minFaceScale = 0.1f, float maxFaceScale = 1.0f, bool outputOnly2DFeatures = false);

	bool fitModelToFaceD(FDP *input, FaceData *output, VsImage *frame, float pose[6], bool freezeRot = false, bool freezePos = false);

	bool fitModelToFace(FDP* input, FaceData* output, VsImage * frame, float initYaw);

	Candide3Model* m_model;

private:

	void calculateFDP(FDP* f, int w, int h, VsMat* vert, bool _3D);
	void setFDPIndices(FDP* f);

	bool configureDetector();

	bool loadModel(Candide3Model **model_, int modelType);

	VisageDetector* m_detector;
	PupilRefiner* m_pupilRefiner;
	EarsRefiner* m_earsRefiner;
	DeepAlignmentRuntime* m_deepAlignmentRuntime;
	VisageConfiguration* m_Configuration;
	VNNFaceFinder* m_faceFinder;

	Candide3Model* m_au_model;
	Candide3Model* m_mesh_model;
	bool use_pose_model;
	bool use_au_model;
	bool use_mesh_model;

	bool m_initialised;

	char m_detectCfgDirPath[1024];
};

}
#endif // __VisageFeaturesDetector_h__

