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



#ifndef _FACEANALYSER_H
#define _FACEANALYSER_H


#ifdef VISAGE_STATIC
	#define VISAGE_ANALYSER_DECLSPEC
#else

	#ifdef VISAGE_ANALYSER_EXPORTS
		#define VISAGE_ANALYSER_DECLSPEC __declspec(dllexport)
	#else
		#define VISAGE_ANALYSER_DECLSPEC __declspec(dllimport)
	#endif

#endif

#define VFA_MAX_FACES 20

#include "FaceData.h"
#include <array>
#include <deque>

namespace VNN
{
class INNFrontend;
class INNRunner;
}

namespace VisageSDK
{
class ScoreFilter;
class SmoothingFilter;
class VFAConcatenatedInstance;

const int VFA_EMOTIONS_COUNT = 7;

/** \brief VFAEmotionIndices enumeration facilitates easier access to a specific emotion, particularly when iterating through arrays.
*/
enum VFAEmotionIndices {
	Anger = 0, ///< Emotion index 0
	Disgust = 1, ///< Emotion index 1
	Fear = 2, ///< Emotion index 2
	Happiness = 3, ///< Emotion index 3
	Sadness = 4, ///< Emotion index 4
	Surprise = 5, ///< Emotion index 5
	Neutral = 6 ///< Emotion index 6
};

/** \brief VFAFlags are used to describe whether age, gender and/or emotion data have been loaded succesfully or used to select which face analysis operations will be performed.
*
* Flags are returned as a bitwise combination in function init() and need to be passed as options by functions analyseImage() and analyseStream().
*/
enum VFAFlags {
	VFA_AGE = 1, ///< enable age estimation or check whether age data files have been properly initialized
	VFA_GENDER = 2, ///< enable gender estimation or check whether gender data files have been properly initialized
	VFA_EMOTION = 4 ///< enable emotion estimation or check whether emotion data files have been properly initialized
};

/** \brief Enumeration describing potential outcomes of the VisageFaceAnalyser::analyseImage() and VisageFaceAnalyser::analyseStream() methods.
*/
enum class VFAReturnCode {
	NoError, ///< the function has executed successfully
	InvalidFrame, ///<  the image pointer or the related data may be null
	InvalidFaceData, ///<  the FaceData has not been initialized by a previous tracking or detection operation
	InvalidLicense, ///<  the license may be missing, invalid file name passed, expired or another related error
	DataUninitialized ///<  VisageFaceAnalyser was not properly initialized by a call to Init() method, or required data files were not found on supplied location
};

/** \brief Analysis data structure, used as a container for all face analysis results.
*
* This structure is passed as a parameter to the VisageFaceAnalyser::analyseImage() or VisageFaceAnalyser::analyseStream() method. Both methods store analysis results into the structure.
*
* Members ageValid, genderValid and emotionsValid may be used to verify that analysis for those features was performed successfully, before checking the results.
* If an operation was not performed (turned off via options) or failed, the respective validity boolean will return false and estimations will contain the default value.
*
* If the validity boolean for the respective operation is true, then the estimation succeeded and the results may be used. An example:
* \code
* if (analysisData.ageValid)
*      printf("Age: %f", analysisData.age);
* if (analysisData.genderValid)
*      printf("Gender: %d", analysisData.gender);
* if (analysisData.emotionsValid)
* {
*      printf("Anger: %f", analysisData.emotionProbabilities[Anger]);
*      printf("Disgust: %f", analysisData.emotionProbabilities[Disgust]);
*      printf("Fear: %f", analysisData.emotionProbabilities[Fear]);
*      printf("Happiness: %f", analysisData.emotionProbabilities[Happiness]);
*      printf("Sadness: %f", analysisData.emotionProbabilities[Sadness]);
*      printf("Surprise: %f", analysisData.emotionProbabilities[Surprise]);
*      printf("Neutral: %f", analysisData.emotionProbabilities[Neutral]);
* }
* \endcode
*/
struct VISAGE_ANALYSER_DECLSPEC AnalysisData
{
	float                                 age = -1.0f; ///< result of age estimation
	bool                                  ageValid = false; ///< boolean flag returning whether age estimation has been successful or not
	int                                   gender = -1; ///< result of gender estimation which can be 1 in case of MALE gender and 0 for FEMALE gender
	bool                                  genderValid = false; ///< boolean flag returning whether gender estimation has been successful or not
	std::array<float, VFA_EMOTIONS_COUNT> emotionProbabilities = { 0.0 }; ///< result of emotion estimation, see: #VFAEmotionIndices
	bool                                  emotionsValid = false; ///< boolean flag returning whether emotion estimation has been successful or not
};

#ifndef ANDROID
int VISAGE_DECLSPEC initializeLicenseManager(const char *licenseKeyFileFolder);
#endif

/** Returns visageSDK version and revision number.
*
* @return visageSDK version and revision number as char array
*/
const VISAGE_DECLSPEC char* getVisageSDKVersion(void);

/** VisageFaceAnalyser contains face analysis algorithms capable of estimating age, gender and emotion from facial images.
*
* Before using VisageFaceAnalyser, it must be initialized using the function init(). After the initialization the following types of analysis can be used:
* 
* <ul>
* <li>Age estimation</li>
* <li>Gender estimation</li>
* <li>Emotion estimation</li>
* </ul>
* 
* Face analysis may be performed on an individual image, or on a sequential stream of images (such as a video or camera feed). The two corresponding functions are:
* 
* <table>
* <tr><td>analyseImage()</td><td>Perform face analysis on a single image</td></tr>
* <tr><td>analyseStream()</td><td>Perform face analysis on a sequential stream (utilizes score-based selection of ideal candidate frames to deliver a high-precision averaged result)</td></tr>
* </table>
* 
* VisageFaceAnalyser uses VsImage structure as a container for the input image. Please refer to the VsImage structure documentation for details of accessing its data members; the basic members are the size of the image (frame->width, frame->height) and the pointer to the actual pixel data of the 
* image (frame->imageData). All of the estimation methods expect grayscale, RGB or RGBA image. Please note that for the most efficient usage, the grayscale image format should be used. Grayscale format is a default image 
* format for the estimation methods and conversion from other valid image formats is done internally.
* \n\n
* The following is an example of how to create an input image using methods provided by visage|SDK:
* \code
* VsImage *inputImage;
* const int nChannels = 3;
* inputImage = vsCreateImageHeader(vsSize(frameWidth, frameHeight), VS_DEPTH_8U, nChannels);
* int frameWidthStep = frameWidth * nChannels; //If no padding, otherwise the widthStep should include the padding
* vsSetImageData(image, frameData, frameWidthStep);
* vsReleaseImageHeader(&inputImage); //release memory
* \endcode
*\n
*/
class VISAGE_ANALYSER_DECLSPEC VisageFaceAnalyser
{

public:
	/** Constructor.
	*/
	VisageFaceAnalyser();
	/** Destructor.
	*/
	~VisageFaceAnalyser();

	/** Initialise VisageFaceAnalyser. This function must be called before using VisageFaceAnalyser by passing it a path to the folder containing the VisageFaceAnalyser 
	* data files. Within the visage|SDK package, this folder is	<i>Samples/data/vfa</i><b>*</b>. When implementing applications, 
	* developers may copy this folder as part of their application. The path is given relative to the current working directory at the time of calling the init() function. 
	* 
	* \if ANDROID_DOXY
	* Please note that data files packaged within Android <i>apk</i> file cannot be read by our API so it is necessary to copy all required files to some other destination, e.g. application's data folder.
	* Example of how this is done can be found in all our samples. For example, you can take look at ShowcaseDemo sample where copying is done in 
	* <a href="MainActivity_8java_source.html">MainActivity.java</a>, in method <i>copyAssets()</i>.
	* \endif 
	* 
	* The VisageFaceAnalyser data folder contains the following subfolders corresponding to the various types of analysis that can be performed:
	*
	<ul>
	\if MACOSX_DOXY
		<li>ad: age estimation*</li>
		<li>gd: gender estimation*</li>
	\else
		<li>ad: age estimation</li>
		<li>gd: gender estimation</li>
	\endif
		<li>ed: emotion estimation</li>
	</ul>
	*
	* \if MACOSX_DOXY
	* <sub><i>\*Data files with extension <b>.tflite</b> required when targetting <b>arm64</b> architecture, and with <b>.vino.\*</b> extension when targetting
	* <b>x86_64</b> architecture</i></sub>
	* \endif
	* 
	* Note that it is not neccessary for all subfolders to be present, only the ones needed by the types of analysis that will be performed. For example, if only gender estimation will be performed, 
	* then only the gd folder is required.
	*
	*
	* The return value of the init() function indicates which types of analysis have been successfully initialised. Subsequent attempt to use a type of analysis that was not initialized shall fail; for example, if only gd folder was present at initialization, attempt to estimate emotions shall fail.
	* 
	* The return value is a bitwise combination of flags indicating which types of analysis are successfully initialized.
	*
	* 
	* @param dataPath relative path from the working directory to the directory that contains VisageFaceAnalyser data files.
	* @return The return value of 0 indicates that no data files are found and no analysis will be possible. In such case, the placement of data files should be verified. A non-zero value indicates that one or more types of analysis are successfully initialized. The specific types of analysis that have been initialized are indicated by a bitwise combination of the flags from #VFAFlags.
	*/
	int init(const char* dataPath);

	/** Performs face analysis tasks specified by \p options parameter on a given image and outputs results to an AnalysisData object instance.
	* 
	* This function is primarily intended for performing face analysis on a single image, or consecutive unrelated images.
	* As such, it outputs raw, unfiltered estimation data without smoothing or averaging.
	*
	* \note Prior to using this function it is necessary to process the facial image using VisageTracker or VisageFeaturesDetector and pass the frame and obtained data to this function.
	* When using data from VisageTracker, only data obtained when tracking status was TRACK_STAT_OK should be passed.
	*
	* An example of usage with OpenCV, estimate gender and age on all the faces detected in an image file:
	* \code
	* VisageFeaturesDetector* detector = new VisageFeaturesDetector("Face Detector.cfg");
	* bool init = detector->Initialize();
	*
	* VisageFaceAnalyser* analyser = new VisageFaceAnalyser();
	* const char* dataPath = "vfa";
	* int isInitialized = analyser->init(dataPath);
	* 
	* IplImage* cvImg = cvLoadImage(filePath, CV_LOAD_IMAGE_GRAYSCALE);
	* VsImage* inputImage = vsCreateImageHeader(vsSize(cvImg->width, cvImg->height), 8, 1);
	* vsSetData(inputImage, cvImg->imageData, cvImg->widthStep);
	*
	* const int MAX_FACES = 4;
	*
	* const int options = VFA_AGE | VFA_GENDER | VFA_EMOTION;
	*
	* if ((isInitialized & options) == options)
	* {
	*     FaceData* data = new FaceData[MAX_FACES];
	*
	*     int nFaces = detector->detectFacialFeatures(inputImage, data, MAX_FACES);
	*
	*     AnalysisData results[MAX_FACES];
	*     VFAReturnCode status;
	*
	*     for (int i = 0; i < nFaces; i++)
	*     {
	*         status = analyser->analyseImage(reinterpret_cast<VsImage*>(cvImg), data[i], options, results[i]);
	*         if (status == VFAReturnCode::NoError)
	*         {
	*             if (results[i].ageValid)
	*                 // use age
	*             if (results[i].genderValid)
	*                 // use gender
	*             if (results[i].emotionsValid)
	*                 // use emotion
	*         }
	*     }
	*
	*     delete[] data;
	* }
	* vsReleaseImageHeader(&inputImage); //release memory
	* cvReleaseImage(&cvImg); //release memory
	* delete detector;
	* \endcode
	*
	* @param image VsImage container that contains the input grayscale, RGB or RGBA image.
	* @param faceData FaceData obtained by a previous successful tracking (TRACK_STAT_OK) or detection operation with VisageTracker or VisageFeaturesDetector.
	* @param options Bitwise combination of #VFAFlags which determines the analysis operations to be performed.
	* @param results AnalysisData struct containing success flags for individual operations and their assorted results.
	* @return #VFAReturnCode value indicating the status of the performed analysis.
	*/
	VFAReturnCode analyseImage(const VsImage* image, const FaceData& faceData, const int options, AnalysisData& results);

	/** Performs face analysis tasks specified by \p options parameter on a given frame and outputs results to an AnalysisData object instance.
	*
	* This function is primarily intended for performing face analysis on a continuous stream of related frames containing the same person, such as a video or camera feed.
	* Sampling face analysis data from multiple frames can increase estimation accuracy by averaging the result over multiple frames.
	* Internally, the suitability of frames chosen for analysis is continually evaluted based on head pose and overall tracking quality.
	* This guarantees that the analysis buffer is always working with the best available frames, ensuring highest possible estimation accuracy.
	*
	* **Important notes:**<br>
	* This method should only be called with FaceData obtained from a successful tracking operation that returned TRACK_STAT_OK tracking status.
	*
	* If \p options parameter is changed between subsequent calls to analyseStream(), the internal state will be reset and previously collected analysis data will be lost.
	* For optimal results \p options parameter should remain constant during single stream analysis session.
	*
	* If a new person in the continuous stream replaces the old one, it is neccessary to call resetStreamAnalysis() method otherwise results will not be correct as 
	* analyseStream() method has no capability of differentiating faces.
	*
	* \note Prior to using this function it is necessary to process the facial image using VisageTracker and pass the frame and obtained data to this function.
	*
	* An example of usage with OpenCV, estimate gender and age on all the faces tracked in a video file:
	* \code
	* VisageTracker* tracker = new VisageTracker("Facial Features Tracker.cfg");
	* 
	* VisageFaceAnalyser* analyser = new VisageFaceAnalyser();
	* const char *dataPath = "vfa";
	* int isInitialized = analyser->init(dataPath);
	*
	* const int MAX_FACES = 1;
	* FaceData data;
	*
	* const int options = VFA_AGE | VFA_GENDER | VFA_EMOTION;
	* AnalysisData analysisData;
	*
	* cap = cvCaptureFromFile(filePath);
	*
	* IplImage* videoFrame;
	*
	* if(isInitialized & options)
	* {
	*     for (int i = 0; i < 30; i++)
	*     {
	*         videoFrame = cvQueryFrame(cap);
	*
	*         int width = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH);
	*         int height = cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT);
	*         int timeStamp = cvGetCaptureProperty(cap, CV_CAP_PROP_POS_MSEC);
	*
	*         int* trackerStatus = tracker->track(width, height, videoFrame->imageData, &data, VISAGE_FRAMEGRABBER_FMT_BGR, VISAGE_FRAMEGRABBER_ORIGIN_TL, videoFrame->widthStep, timeStamp, MAX_FACES);
	*
	*         if (trackerStatus[0] == TRACK_STAT_OK)
	*         {
	*             VsImage* frameRGB = vsCreateImage(vsSize(width, height), VS_DEPTH_8U, 3);
	*             vsCvtColor(videoFrame, frameRGB, VS_BGR2RGB);
	*
	*             VFAReturnCode status = analyser->analyseStream(reinterpret_cast<VsImage*>(frameRGB), data, options, analysisData);
	*
	*             if (status == VFAReturnCode::NoError)
	*             {
	*                 if (results[i].ageValid)
	*                     // use age
	*                 if (results[i].genderValid)
	*                     // use gender
	*                 if (results[i].emotionsValid)
	*                     // use emotion
	*             }
	*             vsReleaseImage(&frameRGB);
	*         }
	*     }
	* }
	* delete tracker;
	* delete analyser;
	* \endcode
	*
	* @param frame VsImage container that contains the input grayscale, RGB or RGBA frame.
	* @param faceData FaceData obtained by a previous successful tracking operation with VisageTracker (TRACK_STAT_OK).
	* @param options Bitwise combination of #VFAFlags which determines the analysis operations to be performed.
	* @param results AnalysisData struct containing success flags for individual operations and their assorted results.
	* @param faceIndex Index of the face for which analysis should be performed.
	* @return #VFAReturnCode value indicating the status of the performed analysis.
	*/
	VFAReturnCode analyseStream(const VsImage* frame, const FaceData& faceData, const int options, AnalysisData& results, const int faceIndex = 0);
	
	/** Resets collected face analysis data.
	*
	* Erases age, gender and emotion data collected up to this point.
	*
	* This is intended to be used in cases when a new person replaces the old one in the same continous input stream.
	*
	* If an index parameter is specified, only data for that specific face is erased. 
	* If no parameter is specified, data for all faces is erased.
	* 
	* @param faceIndex Index of the face for which analysis data should be reset
	*/
	void resetStreamAnalysis(const int faceIndex = -1);

private:
	int m_dataLoaded;
	std::array<int, VFA_MAX_FACES> m_analyserOptions;

	VFAConcatenatedInstance* m_emotionRuntime;

	VNN::INNFrontend* m_ageFrontend;
	VNN::INNRunner* m_ageRunner;

	VNN::INNFrontend* m_genderFrontend;
	VNN::INNRunner* m_genderRunner;

	std::array<ScoreFilter*, VFA_MAX_FACES> m_scoreFilter;
	std::array<std::deque<std::array<float, VFA_EMOTIONS_COUNT>>, VFA_MAX_FACES> m_emotionFilter;

	void estimateAgeAndGender(const VsImage* frame, const FaceData &facedata, const int options, AnalysisData& results);
	void estimateEmotion(const VsImage* frame, const FaceData &facedata, AnalysisData& results);

	std::array<float, VFA_EMOTIONS_COUNT> getAverageEmotions(const int faceIndex);

	const int EMOTION_WINDOW = 500; //ms
};
}

#endif //_FACEANALYSER_H
