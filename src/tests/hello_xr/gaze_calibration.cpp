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

#include "pch.h"

#if ENABLE_GAZE_CALIBRATION

#include "gaze_calibration.h"

namespace BVR 
{
template<typename T> static inline T bvr_clamp(T v, T mn, T mx)
{
	return (v < mn) ? mn : (v > mx) ? mx : v;
}

GazeCalibration::GazeCalibration()
{
}

void GazeCalibration::reset_calibration()
{
	is_calibrating_ = false;
	is_calibrated_ = false;
	calibration_ = {};

	for(int y_index = 0; y_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_Y; y_index++)
	{
		for(int x_index = 0; x_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_X; x_index++)
		{
			CalibrationPoint point;

			if (x_index != EYE_TRACKING_CALIBRATION_CELL_X_CENTER)
			{
				const int x_delta = EYE_TRACKING_CALIBRATION_CELL_X_CENTER - x_index;
				const float x_frac = x_delta / (float)EYE_TRACKING_CALIBRATION_CELL_X_CENTER;
				const float x_degrees = x_frac * EYE_TRACKING_CALIBRATION_MAX_DEGREES;
				point.euler_angles_deg_.x = x_degrees;
			}

			if(y_index != EYE_TRACKING_CALIBRATION_CELL_Y_CENTER)
			{
				const int y_delta = EYE_TRACKING_CALIBRATION_CELL_Y_CENTER - y_index;
				const float y_frac = y_delta / (float)EYE_TRACKING_CALIBRATION_CELL_Y_CENTER;
				const float y_degrees = y_frac * EYE_TRACKING_CALIBRATION_MAX_DEGREES;
				point.euler_angles_deg_.y = y_degrees;
			}

			point.euler_angles_rad_ = deg2rad(point.euler_angles_deg_);
			point.rotation_ = glm::fquat(point.euler_angles_rad_);
			point.local_position_ = glm::rotate(point.rotation_, base_cube_position_);

			calibration_.points_[x_index][y_index] = point;
		}
	}
}

void GazeCalibration::increment_raster()
{
	if (raster_x_ == EYE_TRACKING_CALIBRATION_NUM_CELLS_X - 1)
	{
		raster_x_ = 0;

		if(raster_y_ == EYE_TRACKING_CALIBRATION_NUM_CELLS_Y - 1)
		{
			raster_y_ = 0;
		}
		else
		{
			raster_y_++;
		}
	}
	else
	{
		raster_x_++;
	}
}

const glm::vec3 GazeCalibration::get_raster_position()
{
	const CalibrationPoint& point = get_raster_point();
	return point.local_position_with_offset_;
}

CalibrationPoint& GazeCalibration::get_raster_point()
{
	return calibration_.points_[raster_x_][raster_y_];
}

const CalibrationPoint& GazeCalibration::get_raster_point() const
{
	return calibration_.points_[raster_x_][raster_y_];
}

bool GazeCalibration::is_current_raster_cell_calibrated() const
{
	const CalibrationPoint& point = get_raster_point();
	return point.is_calibrated_;
}

glm::vec3 GazeCalibration::apply_calibration(const glm::vec3& raw_gaze_direction)
{
	if(!is_calibrated_)
	{
		return raw_gaze_direction;
	}
	
	//Passthrough for now until implementation
	glm::vec3 calibrated_gaze_direction = raw_gaze_direction;
	return calibrated_gaze_direction;
}

} // BVH


#endif // ENABLE_GAZE_CALIBRATION
