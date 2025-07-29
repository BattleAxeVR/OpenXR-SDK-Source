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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image_write.c"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image.c"


namespace BVR 
{
template<typename T> static inline T bvr_clamp(T v, T mn, T mx)
{
	return (v < mn) ? mn : (v > mx) ? mx : v;
}

GazeCalibration::GazeCalibration()
{
}


bool GazeCalibration::load_calibration()
{
	const std::string calibration_filename = EYE_TRACKING_CALIBRATION_PNG_FILENAME;

	int width = EYE_TRACKING_CALIBRATION_NUM_CELLS_X;
	int height = EYE_TRACKING_CALIBRATION_NUM_CELLS_Y;
	int num_components = 3;  // 3 = RGB or 4 = RGBA
	int req_comp = num_components;

	stbi_uc* read_buf = stbi_load(calibration_filename.c_str(), &width, &height, &num_components, req_comp);

	if(!read_buf)
	{
		return false;
	}

	reset_calibration();

	for(int y_index = 0; y_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_Y; y_index++)
	{
		for(int x_index = 0; x_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_X; x_index++)
		{
			const int current_index = (y_index * EYE_TRACKING_CALIBRATION_NUM_CELLS_X) + x_index;

			const int byte_offset = current_index * num_components;

			const stbi_uc& red_val   = read_buf[byte_offset];
			const stbi_uc& green_val = read_buf[byte_offset + 1];

			const float x_offset = (127 - (int)red_val) / EYE_TRACKING_CALIBRATION_CELL_RANGE_X;
			const float y_offset = (127 - (int)green_val) / EYE_TRACKING_CALIBRATION_CELL_RANGE_Y;

			CalibrationPoint& point = calibration_.points_[x_index][y_index];
			point.local_offset_ = glm::vec3(x_offset, y_offset, 0.0f);
			point.local_position_with_offset_ = point.local_position_ + point.local_offset_;
		}
	}

	stbi_image_free(read_buf);

	is_calibrated_ = true;
	return true;
}

bool GazeCalibration::save_calibration()
{
	if (!is_calibrated_ || calibration_was_saved_)
	{
		return false;
	}

	int width = EYE_TRACKING_CALIBRATION_NUM_CELLS_X;
	int height = EYE_TRACKING_CALIBRATION_NUM_CELLS_Y;
	int num_components = 3;  // 3 = RGB or 4 = RGBA

	size_t num_bytes = (size_t)(width * height * num_components);
	stbi_uc* write_buf = (stbi_uc*)malloc(num_bytes);

	if(!write_buf)
	{
		return false;
	}

	for(int y_index = 0; y_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_Y; y_index++)
	{
		for(int x_index = 0; x_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_X; x_index++)
		{
			const int current_index = (y_index * EYE_TRACKING_CALIBRATION_NUM_CELLS_X) + x_index;
			
			const int byte_offset = current_index * num_components;

			stbi_uc& red_val = write_buf[byte_offset];
			stbi_uc& green_val = write_buf[byte_offset + 1];
			stbi_uc& blue_val = write_buf[byte_offset + 2];

#if 0
			const CalibrationPoint& point = calibration_.points_[x_index][y_index];
			red_val = point.local_position_with_offset_;
			green_val = 0;
			blue_val = 0;
#else
			red_val = 127;
			green_val = 0;
			blue_val = 0;
#endif
		}
	}

	const std::string calibration_filename = EYE_TRACKING_CALIBRATION_PNG_FILENAME;

	int write_status = stbi_write_png(calibration_filename.c_str(), width, height, num_components, write_buf, 0);

	free(write_buf);

	calibration_was_saved_ = (write_status == 1);

	return calibration_was_saved_;
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
