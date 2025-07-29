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

			const int clamped_red = bvr_clamp<int>((int)red_val, -127, 127);
			const int clamped_green = bvr_clamp<int>((int)green_val, -127, 127);

			const float x_offset = (127 - clamped_red) * EYE_TRACKING_CALIBRATION_CELL_RANGE_X / 127.0f;
			const float y_offset = (127 - clamped_green) * EYE_TRACKING_CALIBRATION_CELL_RANGE_Y / 127.0f;

			CalibrationPoint& point = calibration_.points_[x_index][y_index];
			point.average_offset_ = glm::vec3(x_offset, y_offset, 0.0f);
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

			const float normalized_red = point.average_offset_.x / EYE_TRACKING_CALIBRATION_CELL_RANGE_X;
			const float normalized_green = point.average_offset_.y / EYE_TRACKING_CALIBRATION_CELL_RANGE_Y;

			const int red_int   = bvr_clamp<int>(int(normalized_red * 127.0f), -127, 127) + 127;
			const int green_int = bvr_clamp<int>(int(normalized_green * 127.0f), -127, 127) + 127;

			red_val = (stbi_uc)red_int;
			green_val = (stbi_uc)green_int;
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

bool CalibrationPoint::add_sample(const glm::vec3& input, const glm::vec3& output)
{
	if(is_calibrated_)
	{
		return false;
	}

	const glm::vec3 delta = output - input;
	const float delta_mag = delta.length();

	if (delta_mag < EYE_TRACKING_CALIBRATION_TOLERANCE)
	{
		CalibrationMapping& sample = samples_[num_samples_++];
		sample.input_ = input;
		sample.output_ = output;
		sample.delta_ = delta;

		if(num_samples_ == EYE_TRACKING_CALIBRATION_MAX_SAMPLES_PER_CELL)
		{
			is_calibrated_ = true;
		}

		return true;
	}

	return false;
}

bool CalibrationPoint::compute_average_offset()
{
	if (is_calibrated_ || (num_samples_ < EYE_TRACKING_CALIBRATION_MAX_SAMPLES_PER_CELL))
	{
		return false;
	}

	glm::vec3 average_offset = zero_vec3;

	for(const CalibrationMapping& sample : samples_)
	{
		average_offset += sample.delta_;
	}

	average_offset /= (float)EYE_TRACKING_CALIBRATION_MAX_SAMPLES_PER_CELL;

	is_calibrated_ = true;

	return true;
}


void GazeCalibration::reset_calibration()
{
	is_calibrating_ = false;
	is_calibrated_ = false;
	calibration_ = {};
	num_calibrated_ = 0;

	const float x_cell_offset = EYE_TRACKING_CALIBRATION_CELL_RANGE_X / (float)EYE_TRACKING_CALIBRATION_NUM_CELLS_X;
	const float y_cell_offset = EYE_TRACKING_CALIBRATION_CELL_RANGE_Y / (float)EYE_TRACKING_CALIBRATION_NUM_CELLS_Y;

	const float x_scale = x_cell_offset * EYE_TRACKING_CALIBRATION_CELL_SCALE_X;
	const float y_scale = y_cell_offset * EYE_TRACKING_CALIBRATION_CELL_SCALE_Y;

	const glm::vec3 local_scale(x_scale, y_scale, 0.0f);

	for(int y_index = 0; y_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_Y; y_index++)
	{
		for(int x_index = 0; x_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_X; x_index++)
		{
			CalibrationPoint& point = calibration_.points_[x_index][y_index];
			point.local_pose_.scale_ = local_scale;

			if (x_index != EYE_TRACKING_CALIBRATION_CELL_X_CENTER)
			{
				const int x_delta = EYE_TRACKING_CALIBRATION_CELL_X_CENTER - x_index;
				const float x_frac = x_delta / (float)EYE_TRACKING_CALIBRATION_CELL_X_CENTER;
				point.local_pose_.translation_.x = x_frac * x_cell_offset;
			}

			if(y_index != EYE_TRACKING_CALIBRATION_CELL_Y_CENTER)
			{
				const int y_delta = EYE_TRACKING_CALIBRATION_CELL_Y_CENTER - y_index;
				const float y_frac = y_delta / (float)EYE_TRACKING_CALIBRATION_CELL_Y_CENTER;
				point.local_pose_.translation_.y = y_frac * y_cell_offset;
			}
		}
	}
}

bool GazeCalibration::compute_calibration()
{
	if(is_calibrated_ || (num_calibrated_ < EYE_TRACKING_CALIBRATION_NUM_CELLS))
	{
		return false;
	}

	for(int y_index = 0; y_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_Y; y_index++)
	{
		for(int x_index = 0; x_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_X; x_index++)
		{
			const CalibrationPoint& point = calibration_.points_[x_index][y_index];
		}
	}

	is_calibrated_ = true;

	return true;
}

void GazeCalibration::increment_raster()
{
	if (is_calibrated_)
	{
		return;
	}

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

GLMPose	GazeCalibration::get_next_calibration_cube()
{
	increment_raster();
	const CalibrationPoint& point = calibration_.points_[raster_x_][raster_y_];
	return point.local_pose_;
}

} // BVH


#endif // ENABLE_GAZE_CALIBRATION
