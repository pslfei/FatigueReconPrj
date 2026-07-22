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

#ifndef __FaceData_h__
#define __FaceData_h__

#ifdef VISAGE_STATIC
    #define VISAGE_DECLSPEC
#else

    #ifdef VISAGE_EXPORTS
        #define VISAGE_DECLSPEC __declspec(dllexport)
    #else
        #define VISAGE_DECLSPEC __declspec(dllimport)
    #endif

#endif

#include "vs_main.h"
#include "FDP.h"
#include "TrackerGazeCalibrator.h"

namespace VisageSDK
{
/**
 * Face data structure, used as container for all face tracking and detection results.
 This structure is passed as parameter to the VisageTracker::track() or VisageFeaturesDetector::detectFacialFeatures() method. Any of these methods copies latest tracking or detection results into it.

When filling the structure with data some members are filled while some are left undefined depending on tracking/detection status.

<h2>Obtaining tracking data</h2>

The tracker returns these main classes of data:
 - 3D head pose
 - facial expression
 - gaze direction
 - eye closure
 - facial feature points
 - full 3D face model, textured
 - screen space gaze position (if calibrated)

The tracker status is the return value of the VisageTracker::track() functions.
The following table describes the possible states of the tracker, and lists active member variables (those that are filled with data) for each status when 3D face fitting is enabled.
 <table>
 <tr><td width="100"><b>TRACKER STATUS</b></td><td><b>DESCRIPTION</b></td><td><b>ACTIVE VARIABLES</b></td></tr>
 <tr><td>TRACK_STAT_OFF</td><td>Tracker is not active, i.e. it has not yet been started, or it has been stopped.</td>
 <td>N/A</td></tr>
 <tr><td>TRACK_STAT_OK</td><td>Tracker is tracking normally.</td>
 <td>
 frameRate,
 cameraFocus,
 faceScale,
 faceBoundingBox,
 faceTranslation,
 faceRotation,
 faceRotationApparent,
 actionUnitCount,
 actionUnitsUsed,
 actionUnits,
 actionUnitsNames,
 featurePoints3D,
 featurePoints3DRelative,
 featurePoints2D,
 faceModelVertexCount,
 faceModelVertices,
 faceModelVerticesProjected,
 faceModelTriangleCount,
 faceModelTriangles,
 faceModelTextureCoords
</td></tr>
 <tr><td>TRACK_STAT_RECOVERING</td><td>Tracker has lost the face and is attempting to recover and continue tracking. If it can not recover within the time defined by the parameter recovery_timeout in the <a href="../VisageTracker Configuration Manual.pdf">tracker configuration file</a>, the tracker will fully re-initialize (i.e. it will assume that a new user may be present).</td>
 <td>
 frameRate,
 cameraFocus</td></tr>
 <tr><td>TRACK_STAT_INIT</td><td>Tracker is initializing. The tracker enters this state immediately when it is started, or when it has lost the face and failed to recover (see TRACK_STAT_RECOVERING above). The initialization process is configurable through a number of parameters in the <a href="../VisageTracker Configuration Manual.pdf">tracker configuration file.</a></td>
 <td>
 frameRate,
 cameraFocus
</td></tr>
 </table>

The following table describes the possible states of the tracker, and lists active member variables (those that are filled with data) for each status when 3D face fitting is disabled.
 <table>
 <tr><td width="100"><b>TRACKER STATUS</b></td><td><b>DESCRIPTION</b></td><td><b>ACTIVE VARIABLES</b></td></tr>
 <tr><td>TRACK_STAT_OFF</td><td>Tracker is not active, i.e. it has not yet been started, or it has been stopped.</td>
 <td>N/A</td></tr>
 <tr><td>TRACK_STAT_OK</td><td>Tracker is tracking normally.</td>
 <td>
 frameRate,
 faceScale,
 faceBoundingBox,
 featurePoints2D
</td></tr>
 <tr><td>TRACK_STAT_RECOVERING</td><td>Tracker has lost the face and is attempting to recover and continue tracking. If it can not recover within the time defined by the parameter recovery_timeout in the <a href="../VisageTracker Configuration Manual.pdf">tracker configuration file</a>, the tracker will fully re-initialize (i.e. it will assume that a new user may be present).</td>
 <td>
 frameRate</td></tr>
 <tr><td>TRACK_STAT_INIT</td><td>Tracker is initializing. The tracker enters this state immediately when it is started, or when it has lost the face and failed to recover (see TRACK_STAT_RECOVERING above). The initialization process is configurable through a number of parameters in the <a href="../VisageTracker Configuration Manual.pdf">tracker configuration file.</a></td>
 <td>
 frameRate
</td></tr>
 </table>

 <h3>Smoothing</h3>

The tracker can apply a smoothing filter to tracking results to reduce the inevitable tracking noise. Smoothing factors are adjusted separately for different parts of the face. The smoothing settings in the supplied tracker configurations are adjusted conservatively to avoid delay in tracking response, yet provide reasonable smoothing. For further details please see the smoothing_factors parameter array in the
<a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a>.

 <h2>Obtaining detection data</h2>

The detector returns these main classes of data for each detected face:
 - 3D head pose
 - gaze direction
 - eye closure
 - facial feature points
 - full 3D face model, textured.

Detection result is returned from VisageFeaturesDetector::detectFacialFeatures() function.
The following table describes possible output from the detector and the list of active variables (those that are filled with data). All other variables are left undefined.
<table>
<tr><td width="100"><b>DETECTION RESULT</b></td><td><b>DESCRIPTION</b></td><td><b>ACTIVE VARIABLES</b></td></tr>
<tr><td> 0 </td><td>Detector did not find any faces in the image</td>
<td>N/A</td></tr>
<tr><td> N > 0 </td><td>Detector detected N faces in the image.</td>
<td>
<b>For first N FaceData objects in the array:</b>
 cameraFocus,
 faceTranslation,
 faceRotation,
 faceRotationApparent,
 featurePoints3D,
 featurePoints3DRelative,
 featurePoints2D,
 faceModelVertexCount,
 faceModelVertices,
 faceModelVerticesProjected,
 faceModelTriangleCount,
 faceModelTriangles,
 faceModelTextureCoords
 <br>
 <b>For other FaceData objects in the array:</b>
 N/A
 </td></tr>
</table>

<h2>Returned data</h2>

 The following sections give an overview of main classes of data that may be returned in FaceData by the tracker or the detector, and pointers to specific data members.

 <h3>3D head pose</h3>

The 3D head pose consists of head translation and rotation. Absolute pose of the head with respect to the camera is available.

 The following member variables return the head pose:
 - #faceTranslation
 - #faceRotation
 - #faceRotationApparent

 Both face tracker and face detector return the 3D head pose.

 <h3>Facial expression</h3>

 Facial expression is available in a form of Action Units (AUs), which describe the basic motions of the face, such as mouth opening, eyebrow raising etc.


 AUs are <a href="../VisageTracker Configuration Manual.pdf">fully configurable</a> in the tracker configuration files. For the full list of default AUs, and for other information about the use and configuration of AUs, please refer to the <a href="../VisageTracker Configuration Manual.pdf">Tracker Configuration Manual</a>.

 Please note that the AUs are similar, but not identical to the FACS Action Units.

 The following member variables return Action Units data:
 - #actionUnitCount
 - #actionUnitsUsed
 - #actionUnits
 - #actionUnitsNames

 Only face tracker returns the facial expression; face detector leaves these variables undefined. Furthermore, it is returned only if the au_fitting_model parameter is defined in the configuration file. An example of such file is Facial Features Tracker.cfg. Please see the <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for further details.

 <h3>Gaze direction and eye closure</h3>

 Gaze direction is available in local coordinate system of the person's face or global coordinate system of the camera. Eye closure is available as binary information (OPEN/CLOSED).

 The following member variables return gaze direction and eye closure:
 - #gazeDirection
 - #gazeDirectionGlobal
 - #eyeClosure

 Both face tracker and face detector return gaze direction and eye closure.

 <h3>Facial feature points</h3>

2D or 3D coordinates of facial feature points, including the ones defined by the <a href="../MPEG-4 FBA Overview.pdf">MPEG-4 FBA standard</a> and some additional points are available.

3D coordinates are available in global coordinate system or relative to the origin of the face (i.e. the point in the center between the eyes in the input image).

 Facial features are available through the following member variables:
 - #featurePoints3D
 - #featurePoints3DRelative
 - #featurePoints2D

 Both face tracker and face detector return facial feature points.

 <h3>3D face model</h3>

 The 3D face model is fitted in 3D to the face in the current image/video frame. The model is a single textured 3D triangle mesh.
 The texture of the model is the current image/video frame.
 This means that, when the model is drawn using the correct perspective it exactly recreates the facial part of the image. The correct perspective is defined by camera focal length (#cameraFocus),
 width and height of the input image or the video frame, model rotation (#faceRotation) and translation (#faceTranslation).

 The 3D face model is fully configurable and can even be replaced by a custom model; it can also be disabled for performance reasons if not required. Please see the <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for further details.
 The default model is illustrated in the following image:

 \image html "coord3.png" "3D face model"

 There are multiple potential uses for the face model. Some ideas include, but are not limited to:
 - Draw textured model to achieve face paint or mask effect.
 - Draw the 3D face model into the Z buffer to achieve correct occlusion of virtual objects by the head in AR applications.
 - Use texture coordinates to cut out the face from the image.
 - Draw the 3D face model from a different perspective than the one in the actual video.
 - Insert the 3D face model into another video or 3D scene.

 \if IOS_DOXY
 The sample projects <a href="../tracker.html">VisageTrackerDemo</a> and <a href="../unity_ar.html">VisageTrackerUnityDemo</a> both demonstrate how to access the 3D model information and display the model correctly aligned with the face image.
 \endif
 \if ANDROID_DOXY
 The sample projects <a href="../samples.html">VisageTrackerDemo</a> and <a href="../unity_ar.html">VisageTrackerUnityDemo</a> both demonstrate how to access the 3D model information and display the model correctly aligned with the face image.
 \endif

 Note that the vertices of the face model may not always exactly correspond to the facial feature points obtained from tracking/detection (featurePoints3D). For applications where the precise positioning of the facial feature points is recommended (e.g. virtual make-up), it is important to use the featurePoints3D and not the face model.

 The 3D face model is contained in the following members:
 - #faceModelVertexCount
 - #faceModelVertices
 - #faceModelTriangleCount
 - #faceModelTriangles
 - #faceModelTextureCoords

 Both face tracker and face detector return the 3D face model, if the mesh_fitting_model parameter in the configuration file is set (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).

  <h3>Screen space gaze position</h3>
  Screen space gaze position is available if the tracker was provided with calibration repository and screen space gaze estimator is working in real time mode.
 Otherwise tracker returns default screen space gaze data. Default gaze position is center of screen. Default estimator state is off (ScreenSpaceGazeData::inState == 0). Please refer to VisageGazeTracker documentation for instructions on usage of screen space gaze estimator.

  Screen space gaze position is contained in member #gazeData. The session level gaze tracking quality is contained in member gazeQuality.

  Only face tracker returns screen space gaze position.

*/
struct VISAGE_DECLSPEC FaceData
{
    //data while tracking

    /* Estimated probability of wearing a protective face mask.
    * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
    *
    * The probability is a value between 0.0 and 1.0.
    */
    float hasMask;

    /** Tracking quality level.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
     *
     * Estimated tracking quality level for the current frame. The value is between 0 and 1.
     */
    float trackingQuality;

    float trackingQualityBdts;

    /** The frame rate of the tracker, in frames per second, measured over last 10 frames.
     * <i>This variable is set while tracker is running., i.e. while tracking status is not TRACK_STAT_OFF. Face detector leaves this variable undefined.</i>
     */
    float frameRate;

    /** Timestamp of the current video frame.
     * <i>This variable is set while tracker is running., i.e. while tracking status is not TRACK_STAT_OFF.
     * Face detector leaves this variable undefined.</i>
     *
     * It returns the value passed to timeStamp argument in VisageTracker::track() method if it is different than -1, otherwise it returns time, in milliseconds, measured from the moment when tracking started.
     *
     */
    long timeStamp;

    // translation and rotation data

    /** Translation of the head from the camera.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
     *
     *
     * Translation is expressed with three coordinates x, y, z.
     * The coordinate system is such that when looking towards the camera, the direction of x is to the
     * left, y iz up, and z points towards the viewer - see illustration below. The global origin (0,0,0) is placed at the camera. The reference point on the head is in the center between the eyes.
     *
     * \image html "coord-camera.png" "Coordinate system"
     * \image latex coord-camera.png "Coordinate system" width=10cm
     *
     * If the value set for the camera focal length in the <a href="../VisageTracker Configuration Manual.pdf">tracker/detector configuration</a> file
     * corresponds to the real camera used, the returned coordinates shall be in meters; otherwise the scale of the translation values is not known, but the relative values are still correct (i.e. moving towards the camera results in smaller values of z coordinate).
     *
     * <b>Aligning 3D objects with the face</b>
     *
     * The translation, rotation and the camera focus value together form the 3D coordinate system of the head in its current position
     * and they can be used to align 3D rendered objects with the head for AR or similar applications.
     * This \if WIN_DOXY <a href="doc/ar-notes.cpp">\else <a href="../ar-notes.cpp">\endif example code</a> shows how to do this using OpenGL.
     *
     * The relative facial feature coordinates (featurePoints3DRelative)
     * can then be used to align rendered 3D objects to the specific features of the face, like putting virtual eyeglasses on the eyes. Samples projects demonstrate how to do this, including full source code.
     *
     * @see faceRotation
     */
    float faceTranslation[3];

    /* Translation of the head, compensated.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK).</i>
     *
     * Translation is expressed with three coordinates x, y, z.
     * The coordinate system is such that when looking towards the camera, the direction of x is to the
     * left, y iz up, and z points towards the viewer.  The global origin (0,0,0) is placed at the camera.
     *
     * Translation of the head relative to its position in the first video frame.
     * This is compensated translation estimated by the tracker.
     * Compensated translation values take into account the fact that the tracker
     * uses a relatively flat face model for tracking, so the center of rotation of this model
     * is in the front area of the head, while the anatomical center
     * of rotation is behind, in the base of the neck. Therefore, when the rotation
     * is applied to a 3D head model with anatomically correct center of rotation, the
     * face naturally translates as well. When this translation is compounded with
     * the translation values obtained from the tracker, the total resulting translation
     * is exaggerated. To avoid this exaggerated translation of the animated head,
     * the translation can be compensated. The compensation algorithm
     * estimates how much the translation would be exaggerated, and makes it that much
     * smaller. The compensated translation can directly be applied to animated head
     * models that use the neck base as the center of rotation, and is expected to give
     * better results than the uncompensated translation.
     *
     * @see faceTranslation, faceRotation
     */
    float faceTranslationCompensated[3];

    /** Rotation of the head.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
     *
     * This is the estimated rotation of the head, in radians.
     * Rotation is expressed with three values determining the rotations
     * around the three axes x, y and z, in radians. This means that the values represent
     * the pitch, yaw and roll of the head, respectively. The zero rotation
     * (values 0, 0, 0) corresponds to the face looking straight ahead along the camera axis.
     * Positive values for pitch correspond to head turning down.
     * Positive values for yaw correspond to head turning right in the input image.
     * Positive values for roll correspond to head rolling to the left in the input image, see illustration below.
     * The values are in radians.
     *
     * Note: The order to properly apply these rotations is y-x-z.
     *
     * \image html "coord-rotation.png" "Rotations"
     * \image latex coord-rotation.png "Rotations" width=10cm
     *
     * @see faceTranslation
     */
    float faceRotation[3];

    /** Rotation of the head from the camera viewpoint
    * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
    *
    * This is the estimated apparent rotation of the head, in radians.
    * Rotation is expressed with three values determining the rotations
    * around the three axes x, y and z, in radians. This means that the values represent
    * the pitch, yaw and roll of the head, respectively. The zero apparent rotation
    * (values 0, 0, 0) corresponds to the face looking straight into the camera i.e. a frontal face.
    * Positive values for pitch correspond to head turning down.
    * Positive values for yaw correspond to head turning right in the input image.
    * Positive values for roll correspond to head rolling to the left in the input image.
    * The values are in radians.
    * 
    * Note: The order to properly apply these rotations is y-x-z.
    */
    float faceRotationApparent[3];

     /** Gaze direction.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
     *
     * This is the current estimated gaze direction relative to the person's head.
     * Direction is expressed with two values x and y, in radians. Values (0, 0) correspond to person looking straight.
     * X is the horizontal rotation with positive values corresponding to person looking to his/her left.
     * Y is the vertical rotation with positive values corresponding to person looking down.
     *
     * @see gazeDirectionGlobal
     */
    float gazeDirection[2];

    /** Global gaze direction, taking into account both head pose and eye rotation.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
     *
     * This is the current estimated gaze direction relative to the camera axis.
     * Direction is expressed with three values determining the rotations
     * around the three axes x, y and z, i.e. pitch, yaw and roll. Values (0, 0, 0) correspond to the gaze direction parallel to the camera axis.
     * Positive values for pitch correspond to gaze turning down.
     * Positive values for yaw correspond to gaze turning right in the input image.
     * Positive values for roll correspond to face rolling to the left in the input image, see illustration below.
     *
     * The values are in radians.
     *
     * \image html "coord-rotation-eye.png" "Rotations"
     * \image latex coord-rotation-eye.png "Rotations" width=10cm
     *
     * The global gaze direction can be combined with eye locations to determine the line(s) of sight in the real-world coordinate system with the origin at the camera.
     * To get eye positions use #featurePoints3D and FDP::getFP() function, e.g.:
     *
     * \code
     * FeaturePoint *left_eye_fp = const_cast<FeaturePoint*>( &featurePoints3D->getFP(3,5) );
     * FeaturePoint *right_eye_fp = const_cast<FeaturePoint*>( &featurePoints3D->getFP(3,6) );
     *
     * float left_eye_pos[3], right_eye_pos[3];
     *
     * left_eye_pos[0]  = left_eye_fp->pos[0];  // x
     * left_eye_pos[1]  = left_eye_fp->pos[1];  // y
     * left_eye_pos[2]  = left_eye_fp->pos[2];  // z
     * right_eye_pos[0] = right_eye_fp->pos[0]; // x
     * right_eye_pos[1] = right_eye_fp->pos[1]; // y
     * right_eye_pos[2] = right_eye_fp->pos[2]; // z
     * \endcode
     *
     * @see gazeDirection, featurePoints3D
     */
    float gazeDirectionGlobal[3];

    /** Discrete eye closure value.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
     *
     * Index 0 represents closure of left eye. Index 1 represents closure of right eye.
     * Value of 1 represents open eye. Value of 0 represents closed eye.
     */
    float eyeClosure[2];

     /** Iris radius values in px.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
     *
     * The value with index 0 represents the iris radius of the left eye. The value with index 1 represents the iris radius of the right eye.
     * If iris is not detected, the value is set to -1.
     */
    float irisRadius[2];

    // shape units data
    /** Number of facial Shape Units.
     * <i>This variable is set while tracker is running., i.e. while tracking status is not TRACK_STAT_OFF or if the detector has detected a face and only if au_fitting_model parameter in the configuration file is set (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details)</i>.
     *
     * Number of shape units that are defined for current face model.
     *
     * @see shapeUnits
     */
    int shapeUnitCount;

    /** List of current values for facial Shape Units, one value for each shape unit.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face and only if au_fitting_model parameter in the configuration file is set (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).</i>.
     *
     * Shape units can be described as static parameters of the face that are specific for each individual (e.g. shape of the nose).

     *
     * The shape units used by the tracker and detector are defined in the
     * 3D face model file, specified by the au_fitting_model in the configuration file (see the <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).
     *
     * @see shapeUnitCount
     *
     */
    float *shapeUnits;

    // action units data
    /** Number of facial Action Units.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) and only if au_fitting_model parameter in the configuration file is set (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).
     * Face detector leaves this variable undefined.</i>
     *
     * Number of action units that are defined for current face model.
     *
     * @see actionUnits, actionUnitsUsed, actionUnitsNames
     */
    int actionUnitCount;

    /** Used facial Action Units.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) and only if au_fitting_model parameter in the configuration file is set (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).
     * Face detector leaves this variable undefined.</i>
     *
     * List of values, one for each action unit, to determine if specific action unit is actually used in the current tracker configuration.
     * Values are as follows: 1 if action unit is used, 0 if action unit is not used.
     *
     * @see actionUnits, actionUnitCount, actionUnitsNames
     */
    int *actionUnitsUsed;

    /** List of current values for facial Action Units, one value for each Action Unit. Action Units are the basic motions of the face, such as mouth opening, eyebrow raising etc.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) and only if au_fitting_model parameter in the configuration file is set (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).
     * Face detector leaves this variable undefined.</i>
     *
     * The Action Units used by the tracker are defined in the
     * 3D face model file specified by au_fitting_model parameter in the configuration file (see the <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).
     * Furthermore, the tracker configuration file defines the names of Action Units and these names can be accessed through actionUnitsNames.
     * Please refer to section or <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for full list of Action Units.
     *
     * @see actionUnitsUsed, actionUnitCount, actionUnitsNames
     *
     */
    float *actionUnits;

    /**
    * List of facial Action Units names.
    *
    * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) and only if au_fitting_model parameter in the configuration file is set (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).
    * Face detector leaves this variable undefined.</i>
    *
    * @see actionUnitsUsed, actionUnitCount, actionUnits
    */
    const char **actionUnitsNames;

    // feature points data
    /** Facial feature points (global 3D coordinates).
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
     *
     * The coordinate system is such that when looking towards the camera, the direction of x is to the
     * left, y iz up, and z points towards the viewer.  The global origin (0,0,0) is placed at the camera, see illustration.
     *
     * \image html "coord-camera.png" "Coordinate system"
     * \image latex coord-camera.png "Coordinate system" width=10cm
     *
     * If the value set for the camera focal length in the <a href="../VisageTracker Configuration Manual.pdf">tracker/detector configuration</a> file
     * corresponds to the real camera used, the returned coordinates shall be in meters; otherwise the scale is not known, but the relative values are still correct (i.e. moving towards the camera results in smaller values of z coordinate).
     *
     * The feature points are identified
     * according to the MPEG-4 standard (with extension for additional points), so each feature point is identified by its group and index. For example, the tip of the chin
     * belongs to group 2 and its index is 1, so this point is identified as point 2.1. The identification of all feature points is
     * illustrated in the following image:
     *
     * \image html "mpeg-4_fba.png"
     * \image html "half_profile_physical_2d.png" "Visible/Physical contour"
     *
     * Certain feature points, like the ones on the tongue and teeth, can not be reliably detected so they are not returned
     * and their coordinates are always set to zero. These points are:
     * 6.1, 6.2, 6.3, 6.4, 9.8, 9.9, 9.10, 9.11, 11.4, 11.5, 11.6.
     *
     * Several other points are estimated, rather than accurately detected, due to their specific locations. These points are:
     * 2.10, 2.11, 2.12, 2.13, 2.14, 5.1, 5.2, 5.3, 5.4, 7.1, 9.1, 9.2, 9.6, 9.7, 9.12, 9.13, 9.14, 11.1,
     * 11.2, 11.3, 12.1.
     *
     * Ears' points - group 10 (points 10.1 - 10.24) can be either set to zero, accurately detected or estimated:
     * - zero: 
     *   - <i>refine_ears</i> configuration parameter turned off, 3D model with ears vertices and points mapping file for group 10 NOT provided
     *   - <i>refine_ears</i> configuration parameter turned on, 3D model with ears vertices and points mapping file for group 10 NOT provided
     * - detected: 
     *   - <i>refine_ears</i> configuration parameter turned on, 3D model with ears vertices and points mapping file for group 10 provided
     * - estimated: 
     *   - <i>refine_ears</i> configuration parameter turned off, 3D model with ears vertices and points mapping file for group 10 provided
     *
     * <br/>
     * Face contour - group 13 and group 15. Face contour is available in two versions: the visible contour (points 13.1 - 13.17) and the physical contour (points 15.1 - 15.17). For more details regarding face contour please refer to the documentation of FDP class.
     *
     * Nose contour - group 14, points: 14.21, 14.22, 14.23, 14.24, 14.25.
     *
     * The resulting feature point coordinates are returned in form of an FDP object. This is a container class used for storage of MPEG-4 feature points.
     * It provides functions to access each feature point by its group and index and to read its coordinates.
     *
     * @see featurePoints3DRelative, featurePoints2D, FDP
     *
     */
    FDP *featurePoints3D;

    /** Facial feature points (3D coordinates relative to the face origin, placed at the center between eyes).
     *
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
     *
     * The coordinates are in the local coordinate system of the face, with the origin (0,0,0) placed at the center between the eyes.
     * The x-axis points laterally towards the side of the face, y-xis points up and z-axis points into the face - see illustration below.
     *
     * \image html "coord3.png" "Coordinate system"
     * \image latex coord3.png "Coordinate system" width=10cm
     *
     * The feature points are identified
     * according to the MPEG-4 standard (with extension for additional points), so each feature point is identified by its group and index. For example, the tip of the chin
     * belongs to group 2 and its index is 1, so this point is identified as point 2.1. The identification of all feature points is
     * illustrated in the following image:
     *
     * \image html "mpeg-4_fba.png"
     * \image html "half_profile_physical_2d.png" "Visible/Physical contour"
     *
     * Certain feature points, like the ones on the tongue and teeth, can not be reliably detected so they are not returned
     * and their coordinates are always set to zero. These points are:
     * 6.1, 6.2, 6.3, 6.4, 9.8, 9.9, 9.10, 9.11, 11.4, 11.5, 11.6.
     *
     * Several other points are estimated, rather than accurately detected, due to their specific locations. These points are:
     * 2.10, 2.11, 2.12, 2.13, 2.14, 5.1, 5.2, 5.3, 5.4, 7.1, 9.1, 9.2, 9.6, 9.7, 9.12, 9.13, 9.14, 11.1,
     * 11.2, 11.3, 12.1.
     *
     * Ears' points - group 10 (points 10.1 - 10.24) can be either set to zero, accurately detected or estimated:
     * - zero: 
     *   - <i>refine_ears</i> configuration parameter turned off, 3D model with ears vertices and points mapping file for group 10 NOT provided
     *   - <i>refine_ears</i> configuration parameter turned on, 3D model with ears vertices and points mapping file for group 10 NOT provided
     * - detected: 
     *   - <i>refine_ears</i> configuration parameter turned on, 3D model with ears vertices and points mapping file for group 10 provided
     * - estimated: 
     *   - <i>refine_ears</i> configuration parameter turned off, 3D model with ears vertices and points mapping file for group 10 provided
     *
     * <br/>
     * Face contour - group 13 and group 15. Face contour is available in two versions: the visible contour (points 13.1 - 13.17) and the physical contour (points 15.1 - 15.17). For more details regarding face contour please refer to the documentation of FDP class.
     *
     * Nose contour - group 14, points: 14.21, 14.22, 14.23, 14.24, 14.25.
     *
     * The resulting feature point coordinates are returned in form of an FDP object. This is a container class used for storage of MPEG-4 feature points.
     * It provides functions to access each feature point by its group and index and to read its coordinates.
     *
     * @see featurePoints3D featurePoints2D, FDP
     */
    FDP *featurePoints3DRelative;

    /** Facial feature points (2D coordinates).
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face.</i>
     *
     * The 2D feature point coordinates are normalised to image size so that the lower left corner of the image has coordinates 0,0 and upper right corner 1,1.
     *
     * The feature points are identified
     * according to the MPEG-4 standard (with extension for additional points), so each feature point is identified by its group and index. For example, the tip of the chin
     * belongs to group 2 and its index is 1, so this point is identified as point 2.1. The identification of all feature points is
     * illustrated in the following image:
     *
     * \image html "mpeg-4_fba.png"
     * \image html "half_profile_physical_2d.png" "Visible/Physical contour"
     *
     * Certain feature points, like the ones on the tongue and teeth, can not be reliably detected so they are not returned
     * and their coordinates are always set to zero. These points are:
     * 6.1, 6.2, 6.3, 6.4, 9.8, 9.9, 9.10, 9.11, 11.4, 11.5, 11.6.
     *
     * Several other points are estimated, rather than accurately detected, due to their specific locations. These points are:
     * 2.10, 2.11, 2.12, 2.13, 2.14, 5.1, 5.2, 5.3, 5.4, 7.1, 9.1, 9.2, 9.6, 9.7, 9.12, 9.13, 9.14, 11.1,
     * 11.2, 11.3, 12.1.
     *
     * Ears' points - group 10 (points 10.1 - 10.24) can be either set to zero, accurately detected or estimated:
     * - zero: 
     *   - <i>refine_ears</i> configuration parameter turned off, 3D model with ears vertices and points mapping file for group 10 NOT provided
     *   - <i>refine_ears</i> configuration parameter turned on, 3D model with ears vertices and points mapping file for group 10 NOT provided
     * - detected: 
     *   - <i>refine_ears</i> configuration parameter turned on, 3D model with ears vertices and points mapping file for group 10 provided
     * - estimated: 
     *   - <i>refine_ears</i> configuration parameter turned off, 3D model with ears vertices and points mapping file for group 10 provided
     *
     * <br/>
     * Face contour - group 13 and group 15. Face contour is available in two versions: the visible contour (points 13.1 - 13.17) and the physical contour (points 15.1 - 15.17). For more details regarding face contour please refer to the documentation of FDP class.
     *
     * Nose contour - group 14, points: 14.21, 14.22, 14.23, 14.24, 14.25.
     *
     * The resulting feature point coordinates are returned in form of an FDP object. This is a container class used for storage of MPEG-4 feature points.
     * It provides functions to access each feature point by its group and index and to read its coordinates. Note that FDP stores 3D points and in the case of 2D feature points only the x and y coordinates of each point are used.
     *
     * @see featurePoints3D, featurePoints3DRelative, FDP
     */
    FDP *featurePoints2D;

    // face model mesh data
    /** Number of vertices in the 3D face model.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face, providing that the mesh_fitting_model parameter is set in the configuration file (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).</i>
     *
     * @see faceModelVertices, faceModelVerticesProjected, faceModelTriangleCount, faceModelTriangles, faceModelTextureCoords
     */
    int faceModelVertexCount;

    /** List of vertex coordinates of the 3D face model.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face, providing that the mesh_fitting_model parameter is set in the configuration file (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).</i>
     *
     * The format of the list is x, y, z coordinate for each vertex.
     *
     * The coordinates are in the local coordinate system of the face, with the origin (0,0,0) placed at the center between the eyes.
     * The x-axis points laterally  towards the side of the face, y-axis points up and z-axis points into the face - see illustration below.
     *
     * \image html "coord3.png" "Coordinate system"
     * \image latex coord3.png "Coordinate system" width=5cm
     *
     * To transform the coordinates into the coordinate system of the camera, use faceTranslation and faceRotation.
     *
     * If the value set for the camera focal length in the <a href="../VisageTracker Configuration Manual.pdf">tracker/detector configuration</a> file
     * corresponds to the real camera used, the scale of the coordinates shall be in meters; otherwise the scale is not known.
     *
     *
     * @see faceModelVertexCount, faceModelVerticesProjected, faceModelTriangleCount, faceModelTriangles, faceModelTextureCoords
     */
    float* faceModelVertices;

    /** List of projected (image space) vertex coordinates of the 3D face model.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face, providing that the mesh_fitting_model parameter is set in the configuration file (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).</i>
     *
     * The format of the list is x, y coordinate for each vertex.
     * The 2D coordinates are normalised to image size so that the lower left corner of the image has coordinates 0,0 and upper right corner 1,1.
     *
     *
     * @see faceModelVertexCount, faceModelVertices, faceModelTriangleCount, faceModelTriangles,faceModelTextureCoords
     */
    float* faceModelVerticesProjected;

    /** Number of triangles in the 3D face model.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face, providing that the mesh_fitting_model parameter is set in the configuration file (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).</i>
     *
     * @see faceModelVertexCount, faceModelVertices, faceModelVerticesProjected, faceModelTriangles, faceModelTextureCoords
     */
    int faceModelTriangleCount;

    /** Triangles list for the 3D face model.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK), or if the detector has detected a face, providing that the mesh_fitting_model parameter is set in the configuration file (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).</i>
     *
     * Each triangle is described by three indices into the list of vertices @ref faceModelVertices (counter-clockwise convention is used for normals direction).
     *
     * @see faceModelVertexCount, faceModelVertices, faceModelVerticesProjected, faceModelTriangleCount, faceModelTextureCoords
     */
    int* faceModelTriangles;

    /** Texture coordinates for the 3D face model.
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face, providing that the mesh_fitting_model parameter is set in the configuration file (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).</i>
     *
     * A pair of u, v coordinates for each vertex. When FaceData is obtained from the tracker, the texture image is the current video frame.
     * When FaceData is obtained from detector, the texture image is the input image of the detector.
     *
     * @see faceModelVertexCount, faceModelVertices, faceModelVerticesProjected, faceModelTriangleCount, faceModelTriangles
     */
    float* faceModelTextureCoords;

     /** Static texture coordinates of the mesh from the configuration file parameter mesh_fitting_model.
     * They can be used to apply textures based on the texture template for the unwrapped mesh to the face model. The texture template for these coordinates is provided in jk_300_textureTemplate.png
     * <i>This variable is set only while tracker is tracking (TRACK_STAT_OK) or if the detector has detected a face, providing that the mesh_fitting_model parameter is set in the configuration file (see <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a> for details).</i>
     *
     * A pair of u, v coordinates for each vertex.
     *
     * @see faceModelVertexCount, faceModelVertices, faceModelVerticesProjected, faceModelTriangleCount, faceModelTriangles
     */
    float* faceModelTextureCoordsStatic;

    /** Scale of facial bounding box expressed in pixels.*/
    int faceScale;

    /** Face bounding box.
    * The bounding box is rectangular determined by the x and y coordinates of the upper-left corner and the width and height of the rectangle.
    * Values are expressed in pixels.
    */
    VsRect faceBoundingBox;

    /** Focal distance of the camera, as configured in the <a href="../VisageTracker Configuration Manual.pdf">tracker/detector configuration</a> file.
     * <i>This variable is set while tracker is running (any status other than TRACK_STAT_OFF), or if the detector has detected a face.</i>
     *
     * Focal length of a pinhole camera model used as approximation for the camera used to capture the video in which tracking is performed. The value is defined as
     * distance from the camera (pinhole) to an imaginary projection plane where the smaller dimension of the projection plane is defined as 2, and the other dimension
     * is defined by the input image aspect ratio. Thus, for example, for a landscape input image with aspect ratio of 1.33 the imaginary projection plane has height 2
     * and width 2.66.
     *
     * This value is used for 3D scene set-up and accurate interpretation of tracking data.
     *
     * Corresponding FoV (field of view) can be calculated as follows:
     *
     * fov = 2 * atan( size / (2*cameraFocus) ), where size is 2 if width is larger than height and 2*height/width otherwise.
     *
     * This member corresponds to the camera_focus parameter in the <a href="../VisageTracker Configuration Manual.pdf">tracker/detector configuration</a> file.
     */
    float cameraFocus;

    bool isDataInitialized; // true if tracking data is initialized, false otherwise

    // data range, currently unused
    int* dataRange;

    /** Structure holding screen space gaze position and quality for current frame.
    *<i>This variable is set only while tracker is tracking (TRACK_STAT_OK).
    * Face detector leaves this variable undefined.</i>
    *
    * Position values are dependent on estimator state. Please refer to VisageGazeTracker and ScreenSpaceGazeData documentation for more details.
    *
    **/
    ScreenSpaceGazeData gazeData;

    /**
    * The session level gaze tracking quality in online mode.
    * Quality is returned as a value from 0 to 1, where 0 is the worst and 1 is the best quality. The quality is 0 also when the gaze tracking is off or calibrating.
    */
    float gazeQuality;

    //float screenSpaceGazeQuality[2];

    FaceData();
    FaceData(const FaceData& faceData);
    ~FaceData();

    FaceData& operator=(FaceData faceData);
    void reinit();

private:
    void swap(FaceData& first, FaceData& second);
};
}

#endif
