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

		if(combined_gaze.bNormalisedGazeValid)
		{
			combined_gaze_.local_gaze_direction_.x = -combined_gaze.vNormalisedGaze.x;
			combined_gaze_.local_gaze_direction_.y = combined_gaze.vNormalisedGaze.y;
			combined_gaze_.local_gaze_direction_.z = -combined_gaze.vNormalisedGaze.z;

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
			per_eye_gazes_[LEFT].local_gaze_direction_.x = -left_gaze.vGazeDirection.x;
			per_eye_gazes_[LEFT].local_gaze_direction_.y = left_gaze.vGazeDirection.y;
			per_eye_gazes_[LEFT].local_gaze_direction_.z = -left_gaze.vGazeDirection.z;

			per_eye_gazes_[LEFT].is_valid_ = true;
		}
		else
		{
			per_eye_gazes_[LEFT].is_valid_ = false;
		}

		const EyeGaze_t& right_gaze = gaze_state.packetData.right;

		if(right_gaze.bGazeDirectionValid && !right_gaze.blink)
		{
			per_eye_gazes_[RIGHT].local_gaze_direction_.x = -right_gaze.vGazeDirection.x;
			per_eye_gazes_[RIGHT].local_gaze_direction_.y = right_gaze.vGazeDirection.y;
			per_eye_gazes_[RIGHT].local_gaze_direction_.z = -right_gaze.vGazeDirection.z;

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
bool PSVR2EyeTracker::get_combined_gaze(glm::vec3& combined_gaze_direction, glm::vec3* ref_gaze_direction_ptr)
{
#if !ENABLE_GAZE_CALIBRATION
	(void)ref_gaze_direction_ptr;
#endif

    if (combined_gaze_.is_valid_)
    {
#if ENABLE_GAZE_CALIBRATION
		if(apply_calibration_ && calibrations_[2].is_calibrated())
		{
			combined_gaze_direction = calibrations_[2].apply_calibration(combined_gaze_.local_gaze_direction_);
			return true;
		}
#endif

		combined_gaze_direction = combined_gaze_.local_gaze_direction_;
		return true;
	}

    return false;
}
#endif // ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE

#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
bool PSVR2EyeTracker::get_per_eye_gaze(const int eye, glm::vec3& per_eye_gaze_direction, glm::vec3* ref_gaze_direction_ptr)
{
#if !ENABLE_GAZE_CALIBRATION
	(void)ref_gaze_direction_ptr;
#endif

	if(per_eye_gazes_[eye].is_valid_)
	{
#if ENABLE_GAZE_CALIBRATION
		if (calibrations_[eye].is_calibrating())
		{
			CalibrationPoint& point = calibrations_[eye].get_raster_point();

			if (point.is_calibrated_)
			{
#if (!ANIMATE_CALIBRATION_CUBES && !DRAW_ALL_CALIBRATION_CUBES)
				increment_raster();
#endif
			}
			else if (ref_gaze_direction_ptr)
			{
				point.add_sample(per_eye_gazes_[eye].local_gaze_direction_, *ref_gaze_direction_ptr);
			}
		}

		if (apply_calibration_ && calibrations_[eye].is_calibrated())
		{
			per_eye_gaze_direction = calibrations_[eye].apply_calibration(per_eye_gazes_[eye].local_gaze_direction_);
			return true;
		}
#endif
		
		per_eye_gaze_direction = per_eye_gazes_[eye].local_gaze_direction_;
		return true;
	}

    return false;
}
#endif // ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES

#if ENABLE_GAZE_CALIBRATION
void PSVR2EyeTracker::reset_calibrations()
{
#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
	calibrating_eye_index_ = INVALID_INDEX;
	calibrations_[LEFT_CALIBRATION_INDEX].reset_calibration();
	calibrations_[RIGHT_CALIBRATION_INDEX].reset_calibration();
#endif

#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
	calibrations_[COMBINED_CALIBRATION_INDEX].reset_calibration();
#endif
}

bool PSVR2EyeTracker::load_calibrations()
{
	bool success = true;

#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
	success &= calibrations_[LEFT_CALIBRATION_INDEX].load_calibration();
	success &= calibrations_[RIGHT_CALIBRATION_INDEX].load_calibration();
#endif

#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
	success &= calibrations_[COMBINED_CALIBRATION_INDEX].reset_calibration();
#endif

	return success;
}

bool PSVR2EyeTracker::save_calibrations()
{
	bool success = true;

#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
	success &= calibrations_[LEFT_CALIBRATION_INDEX].save_calibration();
	success &= calibrations_[RIGHT_CALIBRATION_INDEX].save_calibration();
#endif

#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
	success &= calibrations_[COMBINED_CALIBRATION_INDEX].save_calibration();
#endif

	return success;
}

bool PSVR2EyeTracker::is_calibrating() const
{
#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
	if (calibrating_eye_index_ != INVALID_INDEX)
	{
		if(calibrations_[calibrating_eye_index_].is_calibrating())
		{
			return true;
		}
	}
#endif

#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
	if(calibrations_[COMBINED_CALIBRATION_INDEX].is_calibrating())
	{
		return true;
	}
#endif

	return false;
}

void PSVR2EyeTracker::increment_raster()
{
#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
	if(calibrating_eye_index_ != INVALID_INDEX)
	{
		if(calibrations_[calibrating_eye_index_].is_calibrating())
		{
			return calibrations_[calibrating_eye_index_].increment_raster();
		}
	}
#endif

#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
	if(calibrations_[COMBINED_CALIBRATION_INDEX].is_calibrating())
	{
		return calibrations_[COMBINED_CALIBRATION_INDEX].increment_raster();
	}
#endif
}

GLMPose PSVR2EyeTracker::get_calibration_cube() const
{
#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
	if(calibrating_eye_index_ != INVALID_INDEX)
	{
		if(calibrations_[calibrating_eye_index_].is_calibrating())
		{
			return calibrations_[calibrating_eye_index_].get_calibration_cube();
		}
	}
#endif

#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
	if(calibrations_[COMBINED_CALIBRATION_INDEX].is_calibrating())
	{
		return calibrations_[COMBINED_CALIBRATION_INDEX].get_calibration_cube();
	}
#endif

	return {};
}

#endif

} // BVH


#endif // ENABLE_PSVR2_EYE_TRACKING
