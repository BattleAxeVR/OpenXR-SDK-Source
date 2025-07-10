//--------------------------------------------------------------------------------------
// Copyright (c) 2025 BattleAxeVR. All rights reserved.
//--------------------------------------------------------------------------------------

// Author: Bela Kampis
// Date: April 11, 2025

// MIT License
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pch.h"

#if ENABLE_PSVR2_EYE_TRACKING

#include "psvr2_eye_tracking.h"
#include "api.h"

#pragma comment(lib, "PSVR2ToolboxSDK.lib")
#pragma comment(lib, "SetupAPI.lib")

namespace BVR 
{
template<typename T> static inline T bvr_clamp(T v, T mn, T mx)
{
	return (v < mn) ? mn : (v > mx) ? mx : v;
}

PSVR2EyeTracker::PSVR2EyeTracker()
{
}

bool PSVR2EyeTracker::connect()
{
	if(!is_connected_)
	{
		Psvr2ToolboxError eResult = psvr2_toolbox_init();

		if(eResult == Psvr2ToolboxError_None)
		{
			is_connected_ = true;

#if ENABLE_PSVR2_EYE_TRACKING_AUTOMATICALLY
			set_enabled(true);
#endif
		}
	}

	return is_connected_;
}

void PSVR2EyeTracker::disconnect()
{
	if(is_connected_)
	{
		Psvr2ToolboxError eResult = psvr2_toolbox_shutdown();

		if(eResult != Psvr2ToolboxError_None)
		{
		}

		is_connected_ = false;
	}
}

bool PSVR2EyeTracker::update_gazes()
{
	if(!is_connected_)
	{
		return false;
	}

	Psvr2GazeState_t gaze_state = {};
	Psvr2ToolboxError eResult = psvr2_toolbox_get_gaze_state(&gaze_state);

	if(eResult == Psvr2ToolboxError_None)
	{
#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
		const GazeCombined_t& combined_gaze = gaze_state.packetData.combined;

		if(combined_gaze.bIsValid && combined_gaze.bNormalisedGazeValid)
		{
			combined_gaze_.local_gaze_direction_.x = combined_gaze.vNormalisedGaze.x;
			combined_gaze_.local_gaze_direction_.y = -combined_gaze.vNormalisedGaze.y;
			combined_gaze_.local_gaze_direction_.z = combined_gaze.vNormalisedGaze.z;

			combined_gaze_.is_valid_ = true;
		}
		else
		{
			combined_gaze_.is_valid_ = false;
		}
#endif

#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
		const EyeGaze_t& left_gaze = gaze_state.packetData.left;

		if(left_gaze.bGazeDirectionValid && !left_gaze.blink)
		{
			per_eye_gazes_[LEFT].local_gaze_direction_.x = left_gaze.vGazeDirection.x;
			per_eye_gazes_[LEFT].local_gaze_direction_.y = -left_gaze.vGazeDirection.y;
			per_eye_gazes_[LEFT].local_gaze_direction_.z = left_gaze.vGazeDirection.z;

			per_eye_gazes_[LEFT].is_valid_ = true;
		}
		else
		{
			per_eye_gazes_[LEFT].is_valid_ = false;
		}

		const EyeGaze_t& right_gaze = gaze_state.packetData.right;

		if(right_gaze.bGazeDirectionValid && !right_gaze.blink)
		{
			per_eye_gazes_[RIGHT].local_gaze_direction_.x = right_gaze.vGazeDirection.x;
			per_eye_gazes_[RIGHT].local_gaze_direction_.y = -right_gaze.vGazeDirection.y;
			per_eye_gazes_[RIGHT].local_gaze_direction_.z = right_gaze.vGazeDirection.z;

			per_eye_gazes_[RIGHT].is_valid_ = true;
		}
		else
		{
			per_eye_gazes_[RIGHT].is_valid_ = false;
		}
#endif

		return true;
	}

    return false;
}

#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
bool PSVR2EyeTracker::is_combined_gaze_available() const
{
	if(!is_connected() || !is_enabled())
	{
		return false;
	}

	return combined_gaze_.is_valid_;
}
#endif


#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
bool PSVR2EyeTracker::is_gaze_available(const int eye) const
{
	if(!is_connected() || !is_enabled())
	{
		return false;
	}

	return per_eye_gazes_[eye].is_valid_;
}
#endif

#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
bool PSVR2EyeTracker::get_combined_gaze(GazeVec3Type& combined_gaze_direction, GazeVec3Type* ref_gaze_direction_ptr)
{
#if !ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
	(void)ref_gaze_direction_ptr;
#endif

    if (combined_gaze_.is_valid_)
    {
#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
		if(apply_calibration_ || (is_combined_calibrating() && ref_gaze_direction_ptr))
		{
			const glm::vec3 combined_gaze_direction_glm = convert_to_glm(combined_gaze_.local_gaze_direction_);
			const glm::fquat gaze_orientation = glm::rotation(forward_gaze_dir, combined_gaze_direction_glm);
			const glm::vec3 gaze_euler_angles = glm::eulerAngles(gaze_orientation);

			const float x_deg = rad2deg(gaze_euler_angles.x) + 90.0f;
			const float y_deg = rad2deg(gaze_euler_angles.y) + 90.0f;

			const int x_index = (int)(bvr_clamp<int>((int)x_deg, 0, (int)(EYE_TRACKING_CALIBRATION_NUM_CELLS_X - 1)));
			const int y_index = (int)(bvr_clamp<int>((int)y_deg, 0, (int)(EYE_TRACKING_CALIBRATION_NUM_CELLS_Y - 1)));

			EyeTrackingCalibrationData& calibration_data = combined_calibration_;
			CalibrationPoint& calibration_point = calibration_data.points_[x_index][y_index];

			if(is_combined_calibrating() && !calibration_point.is_valid_ && ref_gaze_direction_ptr)
			{
				GazeVec3Type& ref_gaze_direction = *ref_gaze_direction_ptr;
				const glm::vec3 ref_gaze_dir_glm = convert_to_glm(ref_gaze_direction);
				const glm::fquat rotation_correction = glm::rotation(ref_gaze_dir_glm, combined_gaze_direction_glm);

				calibration_point.gaze_direction_ = combined_gaze_direction_glm;
				calibration_point.rotation_correction_ = rotation_correction;
				calibration_point.is_valid_ = true;

				calibration_data.valid_count_++;
			}

			if(calibration_data.valid_count_ == EYE_TRACKING_CALIBRATION_TOTAL_CELLS)
			{
				// Completed! Disable calibration now
				is_combined_calibrating_ = false;
				is_combined_calibrated_ = true;
				apply_calibration_ = true;
			}

			if(apply_calibration_ && calibration_point.is_valid_)
			{
				const glm::vec3 corrected_gaze_glm = glm::rotate(calibration_point.rotation_correction_, combined_gaze_direction_glm);
				combined_gaze_direction = convert_to_xr(corrected_gaze_glm);
				return true;
			}
		}
#endif

		combined_gaze_direction = combined_gaze_.local_gaze_direction_;
		return true;
    }

    return false;
}
#endif // ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE

#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
bool PSVR2EyeTracker::get_per_eye_gaze(const int eye, GazeVec3Type& per_eye_gaze_direction, GazeVec3Type* ref_gaze_direction_ptr)
{
#if !ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
	(void)ref_gaze_direction_ptr;
#endif

	if(per_eye_gazes_[eye].is_valid_)
	{
#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
		if (apply_calibration_ || (is_eye_calibrating(eye) && ref_gaze_direction_ptr))
		{
			const glm::vec3 per_eye_gaze_direction_glm = convert_to_glm(per_eye_gazes_[eye].local_gaze_direction_);
			const glm::fquat gaze_orientation = glm::rotation(forward_gaze_dir, per_eye_gaze_direction_glm);
			const glm::vec3 gaze_euler_angles = glm::eulerAngles(gaze_orientation);

			const float x_deg = rad2deg(gaze_euler_angles.x) + 90.0f;
			const float y_deg = rad2deg(gaze_euler_angles.y) + 90.0f;

			const int x_index = (int)(bvr_clamp<int>((int)x_deg, 0, (int)(EYE_TRACKING_CALIBRATION_NUM_CELLS_X - 1)));
			const int y_index = (int)(bvr_clamp<int>((int)y_deg, 0, (int)(EYE_TRACKING_CALIBRATION_NUM_CELLS_Y - 1)));

			EyeTrackingCalibrationData& calibration_data = per_eye_calibration_[eye];
			CalibrationPoint& calibration_point = calibration_data.points_[x_index][y_index];

			if(is_eye_calibrating(eye) && !calibration_point.is_valid_ && ref_gaze_direction_ptr)
			{
				GazeVec3Type& ref_gaze_direction = *ref_gaze_direction_ptr;
				const glm::vec3 ref_gaze_dir_glm = convert_to_glm(ref_gaze_direction);
				const glm::fquat rotation_correction = glm::rotation(ref_gaze_dir_glm, per_eye_gaze_direction_glm);

				calibration_point.gaze_direction_ = per_eye_gaze_direction_glm;
				calibration_point.rotation_correction_ = rotation_correction;
				calibration_point.is_valid_ = true;

				calibration_data.valid_count_++;
			}

			if(calibration_data.valid_count_ == EYE_TRACKING_CALIBRATION_TOTAL_CELLS)
			{
				// Completed! Disable calibration now
				calibrating_eye_index_ = INVALID_INDEX;
				is_eye_calibrated_[eye] = true;
				apply_calibration_ = true;
			}

			if(apply_calibration_ && calibration_point.is_valid_)
			{
				const glm::vec3 corrected_gaze_glm = glm::rotate(calibration_point.rotation_correction_, per_eye_gaze_direction_glm);
				per_eye_gaze_direction = convert_to_xr(corrected_gaze_glm);
				return true;
			}
		}
#endif
		
		per_eye_gaze_direction = per_eye_gazes_[eye].local_gaze_direction_;
		return true;
	}

    return false;
}
#endif // ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
void PSVR2EyeTracker::reset_calibrations()
{
#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
	is_combined_calibrating_ = false;
	is_combined_calibrated_ = false;
	combined_calibration_ = {};
#endif

#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
	calibrating_eye_index_ = INVALID_INDEX;
	is_eye_calibrated_[LEFT] = false;
	is_eye_calibrated_[RIGHT] = false;

	per_eye_calibration_[LEFT] = {};
	per_eye_calibration_[RIGHT] = {};
#endif

	apply_calibration_ = false;
}
#endif

} // BVH


#endif // ENABLE_PSVR2_EYE_TRACKING
