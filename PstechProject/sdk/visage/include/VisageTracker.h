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


#ifndef __VisageTracker_h__
#define __VisageTracker_h__

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
#include "SmoothingFilter.h"
#include "VisageConfiguration.h"

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif /* !NOMINMAX */
#include <windows.h>
#include <process.h>
#endif

#ifdef ANDROID
extern "C" {
	VISAGE_DECLSPEC void _loadVisageVision();
}
#endif

#ifndef WIN32
#include <pthread.h>
#include <sys/types.h>
#define HANDLE pthread_t*
#endif

/** 
* @namespace VisageSDK
* Namespace
*/
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

#define TRACK_STAT_OFF 0
#define TRACK_STAT_OK 1
#define TRACK_STAT_RECOVERING 2
#define TRACK_STAT_INIT 3

#define VISAGE_CAMERA_UP 0
#define VISAGE_CAMERA_DOWN 1
#define VISAGE_CAMERA_LEFT 2
#define VISAGE_CAMERA_RIGHT 3

#define VISAGE_FRAMEGRABBER_FMT_RGB 0
#define VISAGE_FRAMEGRABBER_FMT_BGR 1
#define VISAGE_FRAMEGRABBER_FMT_LUMINANCE 2
#define VISAGE_FRAMEGRABBER_FMT_RGBA 3
#define VISAGE_FRAMEGRABBER_FMT_BGRA 4

#define VISAGE_FRAMEGRABBER_ORIGIN_TL 0
#define VISAGE_FRAMEGRABBER_ORIGIN_BL 1

class VisageTrackerInternal;
class Candide3Model;
class TrackerInternalInterface;
class TrackerGUIInterface;
class VisageDetector;
class ModelFitter;
class PoseEstimator;

/** VisageTracker is a face tracker capable of tracking the head pose, facial features and gaze for multiple faces in video coming from a
* video file, camera or other sources.
*
* Frames (images) need to be passed sequentially to the VisageTracker::track() method, which immediately returns results for the given frame.
*
* The tracker offers the following outputs as a part of FaceData object:
* - 3D head pose,
* - facial expression,
* - gaze information,
* - eye closure,
* - iris radius,
* - facial feature points,
* - full 3D face model, textured.
*
*
* The tracker requires a set of data and configuration files, available in <i>Samples/data</i> folder.
* For the list of required data files please consult VisageTracker() constructor description or simply copy the complete contents of this folder into your application's working folder.
*
* <h3>Configuring VisageTracker</h3>
* The tracker is fully configurable through comprehensive tracker configuration files and VisageConfiguration class allowing to customize the tracker in terms of performance,
* quality and other options.
* The configuration files are intended to be used for tracker initialization while the VisageConfiguration class allows the specific configuration parameter to change in runtime.
*
* visage|SDK contains optimal configurations for common uses such as head tracking, facial features tracking and ears tracking.
*
* The <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> (later in text referred to as <i>VTCM</i>) provides the list
* of available configurations and full detail on all available configuration options.
*
* Specific configuration parameters are used to enable features such as:
* - ear tracking,
* - pupil refinement, 
* \ifnot LINUX_NIR_DOXY
* - landmarks refinement,
* \endif
* - smoothing filter
*
* <h4>Ear tracking</h4>
* Ear tracking includes tracking of additional 24 points (12 points per ear).
* Detailed illustration of the points' location can be found in the description of the FaceData::featurePoints2D member.
* Ears' feature points are part of the group 10 (10.1 - 10.24).
* Tracking the ears' points require the 3D model with defined ears vertices,
* as well as corresponding points mapping file that includes definition for group 10. visage|SDK containes examples of such model files within <i>Samples/data/vft/fm</i> folder:
* <i>jk_300_wEars.wfm</i> and <i>jk_300_wEars.fdp</i>.
*
* For the list of model's vertices and triangles see chapter <i>2.3.2.1 The jk_300_wEars</i> of <a href="../VisageTracker Configuration Manual.pdf">VTCM</a>.
* 
* A set of three configuration parameters is used to configure ear tracking:
* - <i>refine_ears</i>,
* - <i>mesh_fitting_model</i> and <i>mesh_fitting_fdp</i> if fine 3D mesh is enabled, otherwise <i>pose_fitting_model</i> and <i>pose_fitting_fdp</i>
* - <i>smoothing_factors</i> 'ears' group (<i>smoothing_factors[7]</i>)
*
* visage|SDK provides optimal configuration for ear tracking. Please consult <i>Facial Features Tracker - With Ears.cfg</i> configuration file,
* located in <i>Samples/data</i>, for more details on the set configuration values.
*
* <h4>Pupil refinement</h4>
* Pupil refinement improves the precision of pupil center-point detection and provides iris radius information.
*
* See <em>process_eyes</em> in chapter <em>2.1. Configuration parameters</em> of <a href="../VisageTracker Configuration Manual.pdf">VTCM</a>.</p>
*
* \ifnot LINUX_NIR_DOXY
* <h4>Landmarks refinement</h4>
* Landmarks refinement improves tracking accuracy and robustness and minimizes tracking jitter at the cost of reduced tracking speed (performance).
*
* See <em>refine_landmarks</em> in chapter <em>2.1. Configuration parameters</em> of <a href="../VisageTracker Configuration Manual.pdf">VTCM</a>.</p>
* \endif
* <h4>Smoothing filter</h4>
* The tracker can apply a smoothing filter to tracking results to reduce the inevitable tracking noise.
* Smoothing factors are adjusted separately for different parts of the face.
* The smoothing settings in the supplied tracker configurations are adjusted conservatively to achieve optimal balance between
* smoothing and delay in tracking response for a general use case.
*
* See <i>smoothing_factors</i> in chapter <i>2.1. Configuration parameters</i> of <a href="../VisageTracker Configuration Manual.pdf">VTCM</a>.
*
*
* <h3>Tracking data files</h3>
* Features are determined by their own set of data files contained within <i>Samples/data</i> folder.
* In addition to the set values ​​of the configuration parameters, it is also necessary to ensure that the data of each feature is available.
* Default configuration files set the path to 3D models and data files within <i>Samples/data</i> folder as follows:
* - <i>vft</i> folder that contains tracking algorithm and features specific files. NOTE: The subfolder structure and names must be retained.
*
<table class="doxtable">
<tr>
<td width="100"><b>TRACKING ALGORITHM<br> DATA FILES</b> </td>
</tr>
<tr>
<td><div>ff/</div> 
\if (IOS_DOXY || ANDROID_DOXY)
<div style="text-indent: 2em;"> ff.tflite </div>
\elseif (WIN64_DOXY || LINUX_DOXY || REDHAT_DOXY)
<div style="text-indent: 2em;"> ff.vino.* </div>
\elseif (MACOSX_DOXY)
<div style="text-indent: 2em;"> ff.*</div>
\endif
<div>fa/</div> 
\if (IOS_DOXY || ANDROID_DOXY)
<div style="text-indent: 2em;"> d1qy.tflite </div>
\elseif (WIN64_DOXY || LINUX_DOXY || REDHAT_DOXY)
<div style="text-indent: 2em;">d1qy.vino.*</div>
\elseif (MACOSX_DOXY)
<div style="text-indent: 2em;">d1qy.*</div>
\endif
<div style="text-indent: 2em;">aux_file.bin</div> 
<div>fm/</div> 
<div style="text-indent: 2em;">*.fdp</div>
<div style="text-indent: 2em;">*.wfm</div>
<div style="text-indent: 2em;"></div> 
</td>
</tr>
</table>

</p>
<p>The list of tracking features available to use: </p><table class="doxtable">
<tr>
<td width="100"><b>TRACKER FEATURES</b> </td>
<td><b>REQUIRED DATA FILES</b> </td>
</tr>
<tr>
<td>Ear tracking </td><td><div>er/</div> 
<div style="text-indent: 2em;">efa.lbf</div> 
<div style="text-indent: 2em;">efc.lbf</div> 
</td>
</tr>
<tr>
<td>Pupil refinement </td><td><div>pr/</div>
\if (IOS_DOXY || ANDROID_DOXY)
<div style="text-indent: 2em;"> pr.tflite </div>
\elseif (WIN64_DOXY || LINUX_DOXY || REDHAT_DOXY)
<div style="text-indent: 2em;"> pr.vino.* </div>
\elseif (MACOSX_DOXY)
<div style="text-indent: 2em;"> pr.*</div>
\endif
</td>
</tr>
\ifnot LINUX_NIR_DOXY
<tr>
<td>Landmarks refinement </td>
<td><div>fa/</div>
\if (IOS_DOXY || ANDROID_DOXY)
<div style="text-indent: 2em;"> d2.tflite </div>
\elseif (WIN64_DOXY || LINUX_DOXY || REDHAT_DOXY)
<div style="text-indent: 2em;"> d2.vino.* </div>
\elseif (MACOSX_DOXY)
<div style="text-indent: 2em;"> d2.*</div>
\endif
</td>
</tr>
\endif
</table>
\if MACOSX_DOXY
* <sub><i>\*file extensions: <b>.tflite</b> (<b>arm64</b> architecture), <b>.vino.\*</b> (<b>x86_64</b> architecture)</i></sub>
<p>
\endif
*<br>
* NOTE: <i>.fdp</i> and <i>.wfm</i> files correspond to the 3D model (configuration parameters <b>*_fitting_model</b>, <b>*_fitting_fdp</b>).
*
* For further details on 3D models used in tracking and configuration parameters see 
* <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a>.
* \if IOS_DOXY and section on <a href="../../doc/creatingxc.html#config_selection">device-specific configuration selection</a>\endif
*
*/
class VISAGE_DECLSPEC VisageTracker
{
public:
	/** Constructor.
	*
	* Initializes tracker with specified configuration file.
	*
	* Configuration file is used to modify tracker behavior and set paths to the required data files.
	* Default configuration files are provided in the <i>Samples/data</i> folder.
	* 
	* \if ANDROID_DOXY
	* Please note that data files packaged within Android <i>apk</i> file cannot be read by our API so it is necessary to copy all required files to some other destination, e.g. application's data folder.
	* Example of how this is done can be found in all our samples. For example, you can take look at VisageTrackerDemo sample where copying is done in 
	* <a href="StartupActivity_8java_source.html">StartupActivity.java</a>, in method <i>copyAssets()</i>.
	* \endif 
	* 
	* @param trackerConfigFile the name of the tracker configuration file (.cfg), 
	* \if IOS_DOXY
	* relative to the Resources folder of the application bundle, e.g. "Facial Features Tracker.cfg" (assuming the file is located in the root of the Xcode project's Resources folder).
	* \else
	* relative to the current working directory, e.g. "Samples/data/Facial Features Tracker.cfg", considering <i>visageSDK</i> as a working directory.
	* \endif
	*/
	VisageTracker(const char* trackerConfigFile);

	/** Destructor.
	*/
	virtual ~VisageTracker();

	/**
	* \if LINUX_NIR_DOXY
	* Performs face tracking (one or more faces) in the given near-infrared (NIR) image and returns tracking results and status.
	* \else
	* Performs face tracking (one or more faces) in the given image and returns tracking results and status.
	* \endif
	* This function should be called repeatedly on a series of images in order to perform continuous tracking.
	*
	* If the tracker needs to be initialized, this will be done automatically before tracking is performed on the given image.
	* Initialization means loading the tracker configuration file, required data files and allocating various data buffers to the given image size.
	* This operation may take several seconds.
	* This happens in the following cases:
	*   - In the first frame (first call to track() function).
	*   - When frameWidth or frameHeight are changed, i.e. when they are different from the ones used in the last call to track() function.
	*   - If setTrackerConfiguration() function was called after the last call to track() function.
	*   - When maxFaces is changed, i.e. when it its different from the one used in the last call to track() function.
	*
	* The tracker results are returned in an array of FaceData objects, one FaceData object for each tracked face.
	* Contents of each FaceData element depend on the corresponding tracker status (tracker statuses are returned as return value from the function).
	*
	* The tracking of multiple faces is performed in parallel ( using \if IOS_DOXY Dispatch Framework \else OpenMP \endif) and performance (speed) may vary depending on the number of CPU cores,
	* the number of faces in the current image and the value of maxFaces argument.
	*
	* @param frameWidth Width of the frame
	* @param frameHeight Height of the frame
	* @param p_imageData Pointer to image pixel data; size of the array must correspond to frameWidth and frameHeight
	* @param facedata Array of FaceData instances that will receive the tracking results. No tracking results will be returned if NULL pointer is passed.
	* On first call of this function, the memory for the required member variables of the passed array of FaceData objects will be allocated and initialized
	* automatically. The FacaData array must have the size equal to maxFaces argument.
	* @param format Format of input images passed in p_imageData. It can not change during tracking. Format can be one of the following:
	* \ifnot LINUX_NIR_DOXY
	* - VISAGE_FRAMEGRABBER_FMT_RGB: each pixel of the image is represented by three bytes representing red, green and blue channels, respectively.
	* - VISAGE_FRAMEGRABBER_FMT_BGR: each pixel of the image is represented by three bytes representing blue, green and red channels, respectively.
	* - VISAGE_FRAMEGRABBER_FMT_RGBA: each pixel of the image is represented by four bytes representing red, green, blue and alpha (ignored) channels, respectively.
	* - VISAGE_FRAMEGRABBER_FMT_BGRA: each pixel of the image is represented by four bytes representing blue, green, red and alpha (ignored) channels, respectively.
	* \endif
	* - VISAGE_FRAMEGRABBER_FMT_LUMINANCE: each pixel of the image is represented by one byte representing the luminance (gray level) of the image.
	* @param origin No longer used, therefore, passed value will not have an effect on this function. However, the parameter is left to avoid API changes. 
	* @param widthStep Width of the image data buffer, in bytes.
	* @param timeStamp The timestamp of the the input frame in milliseconds. The passed value will be returned with the tracking data for that frame (FaceData::timeStamp). Alternatively, the value of -1 can be passed, in which case the tracker will return time, in milliseconds, measured from the moment when tracking started.
	* @param maxFaces The maximum number of faces that will be tracked (tracker currently supports up to 20 faces).
	* @returns array of tracking statuses for each of the tracked faces (TRACK_STAT_OFF, TRACK_STAT_OK, TRACK_STAT_RECOVERING and TRACK_STAT_INIT, see @ref FaceData for more details).
	*
	* @see FaceData
	*/
	virtual int* track(int frameWidth, int frameHeight, const char* p_imageData, FaceData* facedata, int format = VISAGE_FRAMEGRABBER_FMT_RGB, int origin = VISAGE_FRAMEGRABBER_ORIGIN_TL, int widthStep = 0, long timeStamp = -1, int maxFaces = 1);

	/** Sets tracking configuration.
	*
	* The tracker configuration file name and other configuration parameters are set and will be used for the next tracking session (i.e. when track() is called).
	* Default configuration files (.cfg) are provided in Samples/data folder.
	* Please refer to the  <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for further details
	* on using the configuration files and all configurable options. \if IOS_DOXY Also, please read the section on automatic device-specific configuration selection.\endif
	* @param trackerConfigFile the name of the tracker configuration file.
	* @param au_fitting_disabled disables the use of the 3D model used to estimate action units (au_fitting_model configuration parameter). If set to true, estimation of action units shall not be performed, and action units related data in the returned FaceData structure will not be available (FaceData::ActionUnits etc.). Disabling will result in a small performance gain.
	* @param mesh_fitting_disabled disables the use of the fine 3D mesh (mesh_fitting_model configuration parameter). If set to true, the 3D mesh shall not be fitted and the related information shall not be available in the returned FaceData structure (FaceData::FaceModelVertices etc.). Disabling will result in a small performance gain.
	*/
	void setTrackerConfiguration(const char* trackerConfigFile, bool au_fitting_disabled = false, bool mesh_fitting_disabled = false);

	/** Sets the inter pupillary distance
	*
	* Inter pupillary distance (IPD) is used by the tracker to estimate the distance of the face from the camera.
	* By default, IPD is set to 0.065 (65 millimeters) which is considered average. If the actual IPD of the tracked person is known,
	* this function can be used to set this IPD. As a result, the calculated distance from the camera will be accurate (as long as the 
	* camera focal lenght is also set correctly).
	* This is important for applications that require accurate distance. For example, in Augmented Reality applications objects such as 
	* virtual eyeglasses can be rendered at appropriate distance and will thus appear in the image with real-life scale.
	*
	* IMPORTANT NOTE: In case of multitracking, same IPD will be set for all tracked faces!
	*
	* @param value the inter pupillary distance (IPD) in meters.
	* @see getIPD()
	*/
	void setIPD(float value);

	/** Returns the current inter pupillary distance (IPD) setting.
	* IPD setting is used by the tracker to estimate the distance of the face from the camera. See setIPD() for further details.
	*
	* @return current setting of inter pupillary distance (IPD) in meters.
	* @see setIPD()
	*/
	float getIPD();

	/**
	*
	*  \deprecated Stops the tracker.
	*
	*/
	void stop();

	/** Reset tracking
	*
	* Resets the tracker. Tracker will reinitialise.
	*
	*/
	void reset();

	/** Returns visageSDK version and revision number.
	*
	*/
	const char* getSDKVersion();

	/** @brief Returns tracking configuration.
	*
	* @return VisageConfiguration object with the values currently used by tracker.
	*/
	VisageConfiguration getTrackerConfiguration();

	/** @brief Sets tracking configuration.
	*
	* The tracker configuration object is set and will be used for the next tracking session (i.e. when track() is called).
	*
	* @param configuration object obtained by calling getTrackerConfiguration() function.
	*/
	void setTrackerConfiguration(VisageConfiguration &configuration);

#ifdef IOS
	/** \if IOS_DOXY\ Set data bundle
	 *
	 * Used to set bundle from which data files will be read. Default is main bundle.
	 *
	 * \endif
	 */
	void setDataBundle(NSBundle *bundle);
#endif
private:
	int Init(void);
	void Finish(void);
	bool shouldReset(int resetFlag);

	static void processEyesClosure(const VsImage* frame, FDP* fdp, float* t, float* r, float* out);
	static void GetPupilCoords(FDP* points, VsMat* eyes_coords, int w, int h);

	TrackerGUIInterface *guiIface;
	TrackerInternalInterface *internalIface;

	float recovery_timeout; /* This value is used only in automatic initialisation mode. It is used when the tracker looses the face and can not detect any face in the frame. This value tells the tracker how long it should wait before considering that the current user is gone and initialising the full re-initialisation procedure.  If the face is detected before this time elapses, the tracker considers that it is the same person and it recovers, i.e. continues tracking it using the previous settings. The time is expressed in milliseconds. */

	friend class VisageFeaturesDetector;
	friend class VisageTrackerInternal;

	VsImage* frame_gray; // Current video frame converted to grayscale; all processing is done on grayscale images
	VsImage* previous_frame_gray; // Previous video frame converted to grayscale; all processing is done on grayscale images

protected:
	VsMat *smoothing_factors;
	unsigned int enable_smoothing;
	unsigned int temporally_denoise_input;

	VsImage* frame_input; // Current video frame, input; may be color or grayscale

	void terminateMT();

	int frameCount; // frame count from beginning of tracking
	double frameTime; // duration of one frame in milliseconds
	long pts; // presentation time stamp
	int pts_frame;

	unsigned long initialTime;
	unsigned long last_times[10];
	int cnt;

	volatile bool active;

private:
	VisageConfiguration* m_Configuration;
	VisageConfiguration* m_ConfigurationIntermediary;

	long getCurrentTimeMs(bool init);
	void deNoiseFrameGray();
	bool grabFrame(bool init);

	static void AugmentFDP(FDP* src, FDP* dst);
	static void unProjectStuff(FDP* fp2D, FDP* fp3D, int w, int h, float f);
	static void removeTransform(FDP* fp3D, FDP* fp3DR, const float* r, const float* t);
	static void setPose(float pose[6], float t[3], float r[3]);
	static float getAvgQuality(const FDP &fdp);

	bool init_successful;

	std::string configuration_filename; //tracker configuration file

#ifdef IOS
	NSBundle *dataBundle;
#endif

	volatile bool tracking_ok; //tracking status OK
	float trackingFrameRate; // measured tracking frame rate

	int m_width; //frame width
	int m_height; //frame height
	int m_format; //frame format for the frame image (RGB, BGR or LUMINANCE)

	void initDetector();
	void initModels();
	void initTrackerInternal();

	bool loadModel(Candide3Model **model, int modelType);

	float m_IPD;

#if defined(IOS) || defined(ANDROID) || defined(MAC_OS_X) || defined(LINUX)
	pthread_mutex_t track_mutex;
	pthread_mutex_t config_mutex;
#endif

#ifdef WIN32
	CRITICAL_SECTION track_mutex;
	CRITICAL_SECTION config_mutex;
#endif

	std::string vfd_data_path;
	std::string vft_data_path;
	std::string pr_data_path;
	std::string er_data_path;
#ifdef EMSCRIPTEN
	public: VisageDetector* m_DetectorBDFS;
	private:
#else
	VisageDetector* m_DetectorBDFS;
#endif

	// pose estimator model
	Candide3Model* model;
	// au fitting model
	Candide3Model* au_model;
	// mesh fitting model
	Candide3Model* mesh_model;

	bool use_single_model;
	bool use_pose_model;
	bool use_au_model;
	bool use_mesh_model;

	bool au_fitting_disable;
	bool mesh_fitting_disable;

	bool inited;

	long fg_pts;

	int configChanged;

	char m_trackCfgDataPath[1024];

	FILE *log_file; //log file - used for debugging, set via log_filename configuration parameter

	int m_maxFaces;

	VisageTrackerInternal **tracker;
	int* trackStatus;
};
}

#endif // __VisageTracker_h__

