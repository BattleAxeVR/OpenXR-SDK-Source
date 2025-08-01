//--------------------------------------------------------------------------------------
// Copyright (c) 2025 BattleAxeVR. All rights reserved.
//--------------------------------------------------------------------------------------

// Author: Bela Kampis
// Date: July 21st, 2025

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

#ifndef GAZE_CALIBRATION_H
#define GAZE_CALIBRATION_H

#include "defines.h"

#if ENABLE_GAZE_CALIBRATION
#include "glm/gtx/quaternion.hpp"
const glm::vec3 forward_gaze_dir(0.0f, 0.0f, -1.0f);

namespace BVR 
{
	const glm::vec3 base_cube_position_ = { 0.0f, 0.0f, 1.0f };

	struct CalibrationMapping
	{
		glm::vec3 input_ = { 0.0f, 0.0f, 0.0f };
		glm::vec3 output_ = { 0.0f, 0.0f, 0.0f };
		glm::vec3 delta_ = { 0.0f, 0.0f, 0.0f };
		float error_ = EYE_TRACKING_CALIBRATION_TOLERANCE_MAX_ERROR;
	};

	struct CalibrationPoint
	{
		GLMPose local_pose_;
		std::vector<CalibrationMapping> samples_;
		int num_samples_ = 0;
		glm::vec3 average_delta_ = { 0.0f, 0.0f, 0.0f };
		float average_error_ = EYE_TRACKING_CALIBRATION_TOLERANCE_MAX_ERROR;

#if !EYE_TRACKING_CALIBRATION_IN_CARTESIAN_COORDS
		glm::fquat calibrated_rotation_correction_ = default_rotation;
#endif
		bool is_calibrated_ = false;

		bool add_sample(const glm::vec3& input);// , const glm::vec3& output);
		bool compute_average_offset();
	};

	struct EyeTrackingCalibrationData
	{
		std::vector<std::vector<CalibrationPoint>> points_;
        int completed_count_ = 0;
	};

    class GazeCalibration
    {
    public:
        GazeCalibration();

		bool load_calibration();
		bool save_calibration();
		
		bool is_calibrating() const
		{
			return is_calibrating_;
		}
		
		bool is_calibrated() const
		{
			return is_calibrated_;
		}
		
		void start_calibration();
		void stop_calibration();

		float get_x_position_from_index(const int x_index);
		int get_x_index_from_position(const float x_position);

		float get_y_position_from_index(const int y_index);
		int get_y_index_from_position(const float y_position);
		
		void reset_calibration();
		bool compute_calibration();
		
		int get_raster_x() const
		{
			return raster_x_;
		}

		int get_raster_y() const
		{
			return raster_y_;
		}

		void increment_raster();
		
		CalibrationPoint& get_raster_point();
		const CalibrationPoint& get_raster_point() const;

		bool is_current_raster_cell_calibrated() const;

		glm::vec3 apply_calibration(const glm::vec3& raw_gaze_direction);

		GLMPose	get_calibration_cube() const;

		int num_calibrated_ = 0;

    private:
        
		bool is_calibrating_ = false;
		bool is_calibrated_ = false;
		bool calibration_was_saved_ = false;
		EyeTrackingCalibrationData calibration_;

		int raster_x_ = 0;// EYE_TRACKING_CALIBRATION_CELL_CENTER_X;
		int raster_y_ = 0;;// EYE_TRACKING_CALIBRATION_CELL_CENTER_Y;
    };
}


#endif // ENABLE_GAZE_CALIBRATION

#endif // GAZE_CALIBRATION_H


