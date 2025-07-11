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

#ifndef PSVR2_EYE_TRACKING_H
#define PSVR2_EYE_TRACKING_H

#include "defines.h"

#if ENABLE_PSVR2_EYE_TRACKING

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
#include "utils.h"
#include "glm/gtx/quaternion.hpp"
const glm::vec3 forward_gaze_dir(0.0f, 0.0f, -1.0f);
#endif

namespace BVR 
{
    struct GazeState
    {
		glm::vec3 local_gaze_direction_ = {};
        bool is_valid_ = false;
    };

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
	struct CalibrationPoint
	{
		glm::vec3 final_correction_ = {0.0f, 0.0f, 0.0f};
		std::array<glm::vec3, EYE_TRACKING_CALIBRATION_SAMPLES_PER_CELL> corrections_;
		int sample_count_ = 0;
	};

	struct EyeTrackingCalibrationData
	{
        CalibrationPoint points_[EYE_TRACKING_CALIBRATION_NUM_CELLS_X][EYE_TRACKING_CALIBRATION_NUM_CELLS_Y];
        int completed_count_ = 0;
	};
#endif

    class PSVR2EyeTracker
    {
    public:
        PSVR2EyeTracker();

        bool connect();
        void disconnect();
        bool update_gazes();

		const bool is_connected() const { return is_connected_; }
		const bool is_enabled() const { return is_enabled_; }

		void set_enabled(const bool enabled)
		{
			is_enabled_ = is_connected_ && enabled;
		}

#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
        bool is_combined_gaze_available() const;
        bool get_combined_gaze(glm::vec3& combined_gaze_direction, glm::vec3* ref_gaze_direction_ptr);

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
		bool is_combined_calibrated() const
		{
			return is_combined_calibrated_;
		}

		bool is_combined_calibrating() const
		{
			if(is_combined_calibrated())
			{
				return false;
			}

			return is_combined_calibrating_;
		}

		void set_combined_calibration_enabled(const bool enable)
		{
			if(!is_combined_calibrated())
			{
				is_combined_calibrating_ = enable;
			}
			else
			{
				is_combined_calibrating_ = false;
			}
		}
#endif // ENABLE_PSVR2_EYE_TRACKING_CALIBRATION

#endif // ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
        
#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
        bool is_gaze_available(const int eye) const;
        bool get_per_eye_gaze(const int eye, glm::vec3& per_eye_gaze_direction, glm::vec3* ref_gaze_direction_ptr);

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
		bool is_eye_calibrated(const int eye) const
		{
			return is_eye_calibrated_[eye];
		}

		bool is_eye_calibrating(const int eye) const
		{
			if(is_eye_calibrated(eye))
			{
				return false;
			}

			return (calibrating_eye_index_ == eye);
		}

		int get_calibrating_eye_index() const
		{
			return calibrating_eye_index_;
		}

		void start_eye_calibration(const int eye)
		{
			if(!is_eye_calibrated(eye))
			{
				calibrating_eye_index_ = eye;
			}
			else
			{
				calibrating_eye_index_ = INVALID_INDEX;
			}
		}

		void stop_eye_calibration()
		{
			calibrating_eye_index_ = INVALID_INDEX;
		}
#endif // ENABLE_PSVR2_EYE_TRACKING_CALIBRATION

#endif // ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
        void reset_calibrations();

		bool is_calibrating() const
		{
#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
			if (is_combined_calibrated_)
			{
				return false;
			}

			if(is_combined_calibrating_)
			{
				return true;
			}
#endif

#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
			if(calibrating_eye_index_ != INVALID_INDEX)
			{
				if (is_eye_calibrated_[calibrating_eye_index_])
				{
					return false;
				}

				return true;
			}
#endif
			return false;
		}

		void set_apply_calibration(const bool enabled)
		{
            apply_calibration_ = enabled;
		}

		void toggle_apply_calibration()
		{
			apply_calibration_ = !apply_calibration_;
		}
#endif

    private:
        bool is_connected_ = false;
        bool is_enabled_ = false;

#if ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE
		GazeState combined_gaze_;

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
		bool is_combined_calibrating_ = false;
		bool is_combined_calibrated_ = false;
		EyeTrackingCalibrationData combined_calibration_;
#endif // ENABLE_PSVR2_EYE_TRACKING_CALIBRATION

#endif // ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE

#if ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES
        GazeState per_eye_gazes_[NUM_EYES];

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
        int calibrating_eye_index_ = INVALID_INDEX;
        bool is_eye_calibrated_[NUM_EYES] = { false, false };
        EyeTrackingCalibrationData per_eye_calibration_[NUM_EYES];
#endif // ENABLE_PSVR2_EYE_TRACKING_CALIBRATION

#endif // ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
        bool apply_calibration_ = true;
#endif
    };
}


#endif // ENABLE_PSVR2_EYE_TRACKING


#endif // PSVR2_EYE_TRACKING_H


