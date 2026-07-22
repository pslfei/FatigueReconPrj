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

#ifndef __VisageConfiguration_h__
#define __VisageConfiguration_h__

#ifdef IOS
#import <Foundation/Foundation.h>
#endif

#ifdef ANDROID
#include <android/log.h>
#define  LOG_TAG    "VisageConfiguration"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

#ifdef VISAGE_STATIC
#define VISAGE_DECLSPEC
#else

#ifdef VISAGE_EXPORTS
#define VISAGE_DECLSPEC __declspec(dllexport)
#else
#define VISAGE_DECLSPEC __declspec(dllimport)
#endif

#endif

#pragma once

#include <string>
#include "Common.h"

enum class ConfigOwner
{
	VisageTracker, VisageDetector
};


/** This is a templated structure that can store an array of values of varying types, including:
* - short
* - int
* - float
* - double
* - string
* 
* It is exclusively used in conjunction with VisageConfiguration class as means for obtaining and manipulation of array type configuration 
* parameters (such as <i>smoothing_factors</i> or <i>au_names</i>).
*
* Allocated memory is managed automatically by a destructor that deallocates its internal arrays at the end of its lifetime.
*
* Following code examples demonstrate:
* - initialization of VsCfgArr,
* - accessing and reading values stored within VsCfgAr,
* - iterating and changing values stored in a VsCfgArr
*
* Initialization of VsCfgArr
* ----------------------------------------------
* A new VsCfgArr may be created as follows: 
*
* \code{.cpp}
* VsCfgArr<int> newArray(6);
* \endcode
*
* This would create a VsCfgArr that allocates space for 6 values of the integer type. The parameters supplied to the VsCfgArr constructor should be suited to the type and number of items that you wish to store:
* - <i>Type</i> - short, integer, float, double or string
* - <i>Size</i> - the number of elements to allocate storage space for
*
* It may also be created from a pre-existing array: 
*
* \code{.cpp}
* float values[5] = {1, 2, 3, 4, 5};
* VsCfgArr<float> newArray (values, 5);
* \endcode
*
* A VsCfgArr's destructor automatically deallocates its internal arrays at the end of its lifetime.
*
*
* Accessing and reading values stored in a VsCfgArr
* -------------------------------------------------------
* The values stored in a VsCfgArr which contains floating-point data may be accessed as follows: 
*
* \code{.cpp}
* m_Tracker = new VisageTracker(defaultConfigFile);
* VsCfgArr<float> smoothingFactors =  m_Tracker->getTrackerConfiguration()->getSmoothingFactors();
* float sf_eyebrows = smoothingFactors[0];
* \endcode
*
* Similarly for a VsCfgArr containing an array of integer values: 
*
* \code{.cpp}
* VsCfgArr<int> pose_fitting_au_use = m_Tracker->getTrackerConfiguration()->getPoseFittingAuUse();
* int au1state = pose_fitting_au_use[0];
* \endcode
*
* For information on whether a specific parameter is stored as a floating-point or integer data type, please refer to the respective setter or getter function for that parameter.
*
* Iterating and changing values stored in a VsCfgArr
* -------------------------------------------------------
*
* The size/number of elements stored in a VsCfgArr can be obtained by the size member of VsCfgArr. Thus, for example, an iteration over a VsCfgArr array could be done in the following manner: 
*
* \code{.cpp}
* VsCfgArr<float> poseFittingAuSens = m_Tracker->getTrackerConfiguration()->getPoseFittingAuSensitivity();
* unsigned int numOfEntries = poseFittingAuSens.size;
* for (int i = 0; i < numOfEntries; i++)
* {
*     poseFittingAuSens[i] = 1; //Setting all members to 1
* }
* poseFittingAuSens[0] = 5.5; //Setting the first member to 5.5
* \endcode
*
* As shown in the above example values can be set directly via standard [] bracket operator. 
*/
template <class T>
struct VISAGE_DECLSPEC VsCfgArr
{
	int size;

	T* ptr;

	VsCfgArr(int size);

	VsCfgArr(VsCfgArr<T>&& origin) noexcept;

	VsCfgArr(T* ptr, int size);

	VsCfgArr(const VsCfgArr<T>& origin);

	VsCfgArr<T>& operator=(const VsCfgArr<T>& origin);

	VsCfgArr<T>& operator=(VsCfgArr<T>&& origin) noexcept;

	T& operator[](int index);

	const T& operator[](int index) const;

	~VsCfgArr();
};

namespace VisageSDK
{
	class ModelConfiguration;

	/* FittingAlgorithms namespace contains values that specify fitting algorithm used to estimate 3D head pose and 3D model deformations that include shape units and action units.
	*/
	namespace FittingAlgorithms
	{
		/* Enables 3D face fitting.
		*/
		static const char* EIF_POSE_SU_AU = "eif_pose_su_au";
		/* Disables 3D face fitting.
		*/
		static const char* NONE = "none";

		static const char* const valid_list[] = {
			EIF_POSE_SU_AU,
			NONE
		};
	}

	/**
	* VisageConfiguration is a class used to change, apply, load and save configuration parameters used by VisageTracker. 
	*
	* The idea behind this interface is for a user to:
	*	- obtain a copy of the VisageConfiguration object from VisageTracker,
	*	- apply the desired changes on the VisageConfiguration object and,
	*	- send the modified object back to the VisageTracker
	*
	* As a direct result of this implementation, the VisageConfiguration default constructor is private and it is expected and encouraged 
	* that the initial copy is obtained from the VisageTracker. Alternatively, VisageConfiguration can also be constructed by passing a 
	* path to the valid .cfg file using VisageConfiguration::VisageConfiguration(const char* trackerConfigFile).
	*
	* Setting and getting of the VisageConfiguration object is thread-safe, i.e. methods can be invoked from another thread.
	*
	* Obtaining a copy of the VC object
	* ----------------------------------------------
	*
	* A copy of the VisageTracker internal member can be obtained by calling VisageTracker::getTrackerConfiguration()
	* method as demonstrated in the code example below:
	*
	* \code{.cpp}
	* VisageTracker* m_Tracker = new VisageTracker(defaultConfigFile);
	* // Obtain a copy of internal VisageConfiguration member
	* VisageConfiguration m_Configuration = m_Tracker->getTrackerConfiguration();
	* \endcode
	*
	* Applying changes on the VC object
	* ----------------------------------------------
	*
	* The interface of the VisageConfiguration class is designed so the parameters cannot be accessed directly, 
	* instead, each parameter has its own getter and setter function. After a local copy is obtained, a setter function can be called.
	* Setter functions perform input validation to verify that the value passed to the function is:
	* a) within a certain range, 
	* b) not empty, or 
	* c) otherwise valid.
	* Thus, the return parameter of the setter function is a boolean and if the value passed as an argument is invalid, the setter function will 
	* return false. Otherwise, if the value of the parameter was successfully changed, the function will return true.
	*
	* A detailed description of each particular configuration parameter, including valid ranges and other specifics, are available 
	* in the <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a>.
	*
	*
	* Parameters of the VisageConfiguration class can be divided into two categories:
	*	- primitive type parameters (booleans, integers, floats, strings)
	*	- arrays of different types (integers, floats, strings)
	*
	* Following code example demonstrates a modification of <b>primitive type parameters</b> by modifying the <i>camera_focus</i> configuration parameter:
	*
	* \code{.cpp}
	* // Obtain a copy of internal VisageConfiguration member
	* VisageConfiguration m_Configuration = m_Tracker->getTrackerConfiguration();
	* bool success = m_Configuration.setCameraFocus(5.0f);
	* if (success)
	*     //The parameter was changed successfully
	* else
	*     //The operation failed
	* \endcode
	*
	* A new template struct, VsCfgArr, has been introduced for storing <b>array-type parameters</b>. This structure can hold arrays of various types including short, integer, float, double and string. VsCfgArr is used exclusively in conjunction with VisageConfiguration interface.
	*
	* Following code example demonstrates a modification of <b>array-type parameters</b> by modifying the <i>smoothing_factors</i> configuration parameter:
	*
	* \code{.cpp}
	* // Obtain a copy of internal VisageConfiguration member
	* VisageConfiguration m_Configuration = m_Tracker->getTrackerConfiguration();
	* VsCfgArr<float> smoothingFactors = m_Configuration->getSmoothingFactors();
	* smoothingFactors[0] = 2.0f;
	* bool success = m_Configuration.setSmoothingFactors(smoothingFactors);
	* if (success)
	*     //The parameter was changed successfully
	* else
	*     //The operation failed
	* \endcode
	*
	* See the detailed page of the VsCfgArr structure for code examples demonstrating:
	* - creating the structure,
	* - accessing and reading values stored within VsCfgArr object,
	* - modifying values stored in a VsCfgArr object
	*
	* Once the changes have been applied, the local copy needs to be applied to the tracker for the changes to take effect.
	*
	*
	* Sending the object to VisageTracker
	* ----------------------------------------------
	*
	* VisageTracker provides an overloaded method VisageTracker::setTrackerConfiguration(VisageConfiguration& vc). A reference of the local object is
	* passed to the VisageTracker and copied to the internal member. Usage is demonstrated in the code example below:
	*
	* \code{.cpp}
	* // Assuming m_Configuration object has been obtained as in example above
	* m_Tracker->setTrackerConfiguration(m_Configuration);
	*\endcode
	*
	* The parameters' values of the sent configuration object will be used in the next tracking session (i.e. when VisageTracker::track() is called).
	*
	*/

	class VISAGE_DECLSPEC VisageConfiguration
	{
	public:
		/** @brief Constructor. Creates a new instance of the VisageConfiguration class by reading a specified configuration (*.cfg) file.
		*
		*  @param trackerConfigFile Path to a specified configuration (*.cfg) file.
		*/
		VisageConfiguration(const char* trackerConfigFile);

		/** @brief Copy constructor. Creates a new instance of the VisageConfiguration class by copying an already existing instance.
		*
		*  @param configuration Reference to pre-existing VisageConfiguration object.
		*/
		VisageConfiguration(const VisageConfiguration& configuration);

		~VisageConfiguration();

		/** @brief Returns the value of the camera_focus parameter.
		*
		*  @return Float value of the camera_focus parameter
		*/
		float getCameraFocus() const;

		/** @brief Returns the value of the recovery_timeout parameter.
		*
		*  @return Float value of the recovery_timeout parameter.
		*/
		float getRecoveryTimeout() const;

		/** @brief Returns the value of the smoothing_factors parameter.
		*
		*  @return VsCfgArr containing float values of the smoothing_factors parameter.
		*/
		VsCfgArr<float> getSmoothingFactors() const;

		/** @brief Returns the value of the enable_smoothing parameter.
		*
		*  @return Unsigned integer value of the enable_smoothing parameter.
		*/
		unsigned int getEnableSmoothing() const;

		/** @brief Returns the value of the temporally_denoise_input parameter.
		*
		*  @return Unsigned integer value of the temporally_denoise_input parameter.
		*/
		unsigned int getTemporallyDenoiseInput() const;

		/** @brief Returns the value of the vft_data_path parameter.
		*
		*  @return String containing the path of the vft_data_path parameter.
		*/
		const std::string& getVftDataPath() const;

		/** @brief Returns the value of the vfd_data_path parameter.
		*
		*  @return String containing the path of the vfd_data_path parameter.
		*/
		const std::string& getVfdDataPath() const;

		/** @brief Returns the value of the pr_data_path parameter.
		*
		*  @return String containing the path of the pr_data_path parameter.
		*/
		const std::string& getPrDataPath() const;

		/** @brief Returns the value of the er_data_path parameter.
		*
		*  @return String containing the path of the er_data_path parameter.
		*/
		const std::string& getErDataPath() const;

		/** @brief Returns the value of the process_eyes parameter.
		*
		*  @return Unsigned integer value of the process_eyes parameter.
		*/
		unsigned int getProcessEyes() const;

		/** \ifnot LINUX_NIR_DOXY
		@brief Returns the value of the refine_landmarks parameter.
		*
		*  @return Unsigned integer value of the refine_landmarks parameter.
		* \endif
		*/
		unsigned int getRefineLandmarks() const;

		/** @brief Returns the value of the refine_ears parameter.
		*
		*  @return Unsigned integer value of the refine_ears parameter.
		*/
		unsigned int getRefineEars() const;

		/** @brief Returns the value of the min_face_scale parameter.
		*
		*  @return Float value of the min_face_scale parameter.
		*/
		float getMinFaceScale() const;

		/** @brief Returns the value of the max_face_scale parameter.
		*
		*  @return Float value of the max_face_scale parameter.
		*/
		float getMaxFaceScale() const;

		/** @brief Returns the value of the face_detector_sensitivity parameter.
		*
		*  @return Float value of the face_detector_sensitivity parameter.
		*/
		float getFaceDetectorSensitivity() const;

		/* @brief Returns the value of the fitting_algorithm parameter.
		*
		*  @return String containing value of the fitting_algorithm parameter.
		*/
		const std::string& getFittingAlgorithm() const;

		/** @brief Returns the value of the pose_fitting_model_configuration parameter.
		*
		*  @return String containing path of the pose_fitting_model_configuration parameter.
		*/
		const std::string& getPoseFittingModelConfiguration() const;

		/** @brief Returns the value of the au_fitting_model_configuration parameter.
		*
		*  @return String containing path of the au_fitting_model_configuration parameter.
		*/
		const std::string& getAuFittingModelConfiguration() const;

		/** @brief Returns the value of the mesh_fitting_model_configuration parameter.
		*
		*  @return String containing path of the mesh_fitting_model_configuration parameter.
		*/
		const std::string& getMeshFittingModelConfiguration() const;

		/** @brief Sets the value of the camera_focus parameter.
		*
		*  Value must be greater than zero.
		*
		*  @param camera_focus Float value
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setCameraFocus(float camera_focus);

		/** @brief Sets the value of the recovery_timeout parameter.
		*
		*  Value must be greater than or equal to zero and not a NaN value.
		*
		*  @param recovery_timeout Float value
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setRecoveryTimeout(float recovery_timeout);

		/** @brief Sets the values of the smoothing_factors parameter.
		*
		*  Values must not be a NaN value.
		*
		*  @param smoothing_factors Reference to a VsCfgArr containing float values
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setSmoothingFactors(const VsCfgArr<float>& smoothing_factors);

		/** @brief Sets the values of the enable_smoothing parameter.
		*
		*  Value can be 0 or 1.
		*
		*  @param enable_smoothing Integer value
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setEnableSmoothing(unsigned int enable_smoothing);

		/** @brief Sets the values of the temporally_denoise_input parameter.
		*
		*  Value can be 0 or 1.
		*
		*  @param temporally_denoise_input Integer value
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setTemporallyDenoiseInput(unsigned int temporally_denoise_input);

		/** @brief Sets the value of the vft_data_path parameter.
		*
		*  The path must be relative to the configuration file used for tracker initialization and not empty.
		*
		*  @param vft_data_path String path to the folder containing tracking algorithm data files required by tracker
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setVftDataPath(std::string vft_data_path);

		/**@brief Sets the value of the vfd_data_path parameter.
		*
		*  The path must be relative to the configuration file used for tracker initialization and not empty.
		*
		*  @param vfd_data_path String path to the folder containing algorithm data file required by tracker and detector
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setVfdDataPath(std::string vfd_data_path);

		/**@brief Sets the value of the pr_data_path parameter.
		*
		*  The path must be relative to the configuration file used for tracker initialization and not empty.
		*
		*  @param pr_data_path String path to the folder containing pupils' refinement data files
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setPrDataPath(std::string pr_data_path);

		/**@brief Sets the value of the er_data_path parameter.
		*
		*  The path must be relative to the configuration file used for tracker initialization and not empty.
		*
		*  @param er_data_path String path to the folder containing ears' refinement data files
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setErDataPath(std::string er_data_path);

		/** @brief Sets the value of the process_eyes parameter.
		*
		*  Value can only be 0, 1, 2, or 3.
		*
		*  @param process_eyes Integer value
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setProcessEyes(unsigned int process_eyes);

		/** \ifnot LINUX_NIR_DOXY
		* @brief Sets the value of the refine_landmarks parameter.
		*
		*  Value can be 0 or 1.
		*
		*  @param refine_landmarks Integer value
		*  @return True if the parameter was changed, false if the input was invalid
		* \endif
		*/
		bool setRefineLandmarks(unsigned int refine_landmarks);

		/** @brief Sets the value of the refine_ears parameter.
		*
		*  Value can only be 0 or 1.
		*
		*  @param refine_ears Integer value
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setRefineEars(unsigned int refine_ears);

		/** @brief Sets the value of the min_face_scale parameter.
		*
		*  Value must be greater than 0.0 and less than or equal to 1.0.
		*
		*  @param min_face_scale Float value
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setMinFaceScale(float min_face_scale);

		/** @brief Sets the value of the max_face_scale parameter.
		*
		*  Value must be greater than 0.0 and less than or equal to 1.0.
		*
		*  @param max_face_scale Float value
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setMaxFaceScale(float max_face_scale);

		/** @brief Sets the value of the face_detector_sensitivity parameter.
		*
		*  Value must be greater than 0.0 and less than or equal to 1.0.
		*
		*  @param face_detector_sensitivity Float value
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setFaceDetectorSensitivity(float face_detector_sensitivity);

		/* @brief Sets the value of the fitting_algorithm parameter.
		*
		*  Valid parameter values are listed in section 2.1. of <a href="../VisageTracker Configuration Manual.pdf">VisageTracker Configuration Manual</a>. Values are also defined within FittingAlgorithms namespace.
		*  The fitting will be disabled if the passed string is empty ("") or "none".
		*
		*  @param fitting_algorithm String that specifies which fitting algorithm to use
		*  @return True if the parameter was changed, false if the input was invalid
		*/
		bool setFittingAlgorithm(std::string fitting_algorithm);

		/** @brief Sets the value of the pose_fitting_model_configuration parameter.
		*
		*  This function sets the .cfg (pose_fitting_model_configuration) file used for pose fitting. 
		*  The path to the model configuration must be relative to the configuration file used for tracker initialization.
		*  The model will be disabled if the passed string is empty ("") or "none".
		*
		*  NOTE: Changing the model configuration may affect the sizes of arrays in the FaceData object populated with values from the model file.
		*
		*  @param pose_fitting_model_configuration String path to the model file
		*  @return True if the parameter was changed, false if the input file does not exist
		*/
		bool setPoseFittingModelConfiguration(std::string pose_fitting_model_configuration);

		/** @brief Sets the value of the au_fitting_model_configuration parameter.
		*
		*  This function sets the .cfg (au_fitting_model_configuration) file used for action unit fitting.
		*  The path to the model configuration must be relative to the configuration file used for tracker initialization.
		*  The model will be disabled if the passed string is empty ("") or "none".
		*
		*  NOTE: Changing the model configuration may affect the sizes of arrays in the FaceData object populated with values from the model file.
		*
		*  @param au_fitting_model_configuration String path to the model file
		*  @return True if the parameter was changed, false if the input file does not exist
		*/
		bool setAuFittingModelConfiguration(std::string au_fitting_model_configuration);

		/** @brief Sets the value of the mesh_fitting_model_configuration parameter.
		*
		*  This function sets the .cfg (mesh_fitting_model_configuration) file used for mesh fitting.
		*  The path to the model configuration must be relative to the configuration file used for tracker initialization.
		*  The model will be disabled if the passed string is empty ("") or "none".
		*
		*  NOTE: Changing the model configuration may affect the sizes of arrays in the FaceData object populated with values from the model file.
		*
		*  @param mesh_fitting_model_configuration String path to the model file
		*  @return True if the parameter was changed, false if the input file does not exist
		*/
		bool setMeshFittingModelConfiguration(std::string mesh_fitting_model_configuration);

		/** @brief Enables the use of the 3D model used to estimate action units (au_fitting_model_configuration configuration parameter).
		*
		*/
		void enableAuModel();

		/** @brief Enables the use of the fine 3D mesh (mesh_fitting_model_configuration configuration parameter). 
		*
		*/
		void enableMeshModel();

		/** @brief Disables the use of the 3D model used to estimate action units (au_fitting_model_configuration configuration parameter).
		*
		*/
		void disableAuModel();

		/** @brief Disables the use of the fine 3D mesh (mesh_fitting_model_configuration configuration parameter).
		*
		*/
		void disableMeshModel();
		
		/** @brief Reads and loads values from a specified configuration (*.cfg) file.
		*
		*  @param trackerConfigFile Path to a configuration (*.cfg) file
		*/
		bool setConfigurationFile(const char* trackerConfigFile);

		/** @brief Copy assignment operator. Copies the values of one VisageConfiguration object to another.
		*
		*  @param configuration VisageConfiguration object to copy values from
		*/
		VisageConfiguration& operator=(VisageConfiguration configuration);
	private:
		VisageConfiguration();

		#ifdef IOS
		NSBundle* mainBundle;
		#endif

		friend class VisageTracker;
		friend class VisageFeaturesDetector;
		friend class Candide3Model;

		int changes;
		
		std::string m_vft_data_path;
		std::string m_vfd_data_path;
		std::string m_pr_data_path;
		std::string m_er_data_path;
		std::string m_fitting_algorithm;
		std::string m_pose_fitting_model_configuration;
		std::string m_au_fitting_model_configuration;
		std::string m_mesh_fitting_model_configuration;
		
		VsMat* m_smoothing_factors;

		float m_camera_focus;
		float m_recovery_timeout;
		float m_min_face_scale;
		float m_max_face_scale;
		float m_face_detector_sensitivity;

		unsigned int m_enable_smoothing;
		unsigned int m_temporally_denoise_input;
		unsigned int m_process_eyes;
		unsigned int m_refine_landmarks;
		unsigned int m_refine_ears;

		bool m_disable_au_model;
		bool m_disable_mesh_model;

		const ModelConfiguration* m_pose_fitting_model_cfg;
		const ModelConfiguration* m_au_fitting_model_cfg;
		const ModelConfiguration* m_mesh_fitting_model_cfg;

		std::string m_log_filename;
		std::string m_cfg_dir_string;

		/* Release memory of VsMat objects and arrays.
		*/
		void clean();

		/* Initialize class members.
		*/
		void init();

		/* Return changes of the class members.
		*  Changes are represented with integer variable. Each type of change set the particular bit of the integer variable to 1.
		*  Once the function is called integer variable is set to 0 (reset).
		*/
		int getChanges();

		void swap(VisageConfiguration& first, VisageConfiguration& second);
	};
}
#endif
