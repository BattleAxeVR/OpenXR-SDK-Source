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
const glm::vec3 forward_gaze_dir(0.0f, 0.0f, 1.0f);
#endif

namespace BVR 
{
    typedef XrVector3f GazeVec3Type;

    struct GazeState
    {
        GazeVec3Type local_gaze_direction_ = {};
        bool is_valid_ = false;
    };

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
	struct CalibrationPoint
	{
        glm::vec3 gaze_direction_ = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::fquat rotation_correction_ = BVR::default_rotation;
		bool is_valid_ = false;
	};

	struct EyeTrackingCalibrationData
	{
        CalibrationPoint points_[EYE_TRACKING_CALIBRATION_NUM_CELLS_X][EYE_TRACKING_CALIBRATION_NUM_CELLS_Y];
	};
#endif

    class PSVR2EyeTracker
    {
    public:
        PSVR2EyeTracker();

        bool connect();
        void disconnect();
        bool update_gazes();

        bool is_combined_gaze_available() const;
        bool is_gaze_available(const int eye) const;
        
        bool get_combined_gaze(GazeVec3Type& combined_gaze_direction, GazeVec3Type* ref_gaze_direction_ptr);
        bool get_per_eye_gaze(const int eye, GazeVec3Type& per_eye_gaze_direction, GazeVec3Type* ref_gaze_direction_ptr);

        const bool is_connected() const { return is_connected_; }
        const bool is_enabled() const { return is_enabled_; }

        void set_enabled(const bool enabled)
        {
            is_enabled_ = is_connected_ && enabled;
        }

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
        void reset_calibrations();

		bool is_calibrating() const
		{
			return (is_combined_calibrating_ || (calibrating_eye_index_ != -1));
		}

        // Combined calibration
		bool is_combined_calibrated() const
		{
			return is_combined_calibrated_;
		}

		bool is_combined_calibrating() const
		{
			return is_combined_calibrating_;
		}

		void set_combined_calibration_enabled(const bool enable)
		{
            is_combined_calibrating_ = enable;
		}

        // Per eye calibration

        bool is_eye_calibrated(const int eye) const
        {
            return is_eye_calibrated_[eye];
        }

		bool is_eye_calibrating(const int eye) const
		{
			return (calibrating_eye_index_ == eye);
		}

		int get_calibrating_eye_index() const
		{
			return calibrating_eye_index_;
		}

        void start_eye_calibration(const int eye)
        {
            calibrating_eye_index_ = eye;
        }

		void stop_eye_calibration()
		{
            calibrating_eye_index_ = -1;
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

        GazeState combined_gaze_;
        GazeState per_eye_gazes_[NUM_EYES];

#if ENABLE_PSVR2_EYE_TRACKING_CALIBRATION
        bool is_combined_calibrating_ = false;
        bool is_combined_calibrated_ = false;
        EyeTrackingCalibrationData combined_calibration_;

        int calibrating_eye_index_ = -1;
        bool is_eye_calibrated_[NUM_EYES] = { false, false };
        EyeTrackingCalibrationData per_eye_calibration_[NUM_EYES];

        bool apply_calibration_ = false;
#endif
    };
}


#endif // ENABLE_PSVR2_EYE_TRACKING


#endif // PSVR2_EYE_TRACKING_H


