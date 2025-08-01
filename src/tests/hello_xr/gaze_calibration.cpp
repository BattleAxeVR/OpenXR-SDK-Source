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

#include "stb_image.h"
#include "stb_image_write.h"

namespace BVR 
{
template<typename T> static inline T bvr_clamp(T v, T mn, T mx)
{
	return (v < mn) ? mn : (v > mx) ? mx : v;
}

GazeCalibration::GazeCalibration()
{
	reset_calibration();
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

			const int clamped_red   = bvr_clamp<int>((int)red_val, -127, 127);
			const int clamped_green = bvr_clamp<int>((int)green_val, -127, 127);

			const float euler_x_deg = (127 - clamped_red) * EYE_TRACKING_CALIBRATION_TOLERANCE_DEG / 127.0f;
			const float euler_y_deg = (127 - clamped_green) * EYE_TRACKING_CALIBRATION_TOLERANCE_DEG / 127.0f;

			CalibrationPoint& point = calibration_.points_[y_index][x_index];
			point.average_euler_deg_ = glm::vec3(euler_x_deg, euler_y_deg, 0.0f);
			point.calibrated_rotation_correction_ = glm::normalize(glm::fquat(deg2rad(point.average_euler_deg_)));
		}
	}

	stbi_image_free(read_buf);

	is_calibrated_ = true;
	return true;
}

bool GazeCalibration::save_calibration()
{
#if 0
	if (!is_calibrated_ || calibration_was_saved_)
	{
		return false;
	}
#endif
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
			const CalibrationPoint& point = calibration_.points_[y_index][x_index];

			const float normalized_red = point.average_euler_deg_.x / EYE_TRACKING_CALIBRATION_TOLERANCE_DEG;
			const float normalized_green = point.average_euler_deg_.y / EYE_TRACKING_CALIBRATION_TOLERANCE_DEG;

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

	const glm::fquat rotation = glm::rotation(glm::normalize(input), glm::normalize(output));
	const glm::vec3 euler_rad = glm::eulerAngles(rotation);
	const glm::vec3 euler_deg = rad2deg(euler_rad);
	const float euler_mag = glm::length(euler_deg);

	if (euler_mag < EYE_TRACKING_CALIBRATION_TOLERANCE_DEG)
	{
		CalibrationMapping& sample = samples_[num_samples_++];
		sample.input_ = input;
		sample.output_ = output;
		sample.euler_deg_ = euler_deg;

		if(num_samples_ == EYE_TRACKING_CALIBRATION_MAX_SAMPLES_PER_CELL)
		{
			compute_average_offset();
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

	average_euler_deg_ = zero_vec3;

	for(const CalibrationMapping& sample : samples_)
	{
		average_euler_deg_ += sample.euler_deg_;
	}

	average_euler_deg_ /= (float)EYE_TRACKING_CALIBRATION_MAX_SAMPLES_PER_CELL;
	calibrated_rotation_correction_ = glm::normalize(glm::fquat(deg2rad(average_euler_deg_)));

	is_calibrated_ = true;
	return true;
}

float GazeCalibration::get_x_position_from_index(const int x_index)
{
	const float x_position = (x_index * EYE_TRACKING_CALIBRATION_CELL_SIZE_X) - EYE_TRACKING_CALIBRATION_CENTER_X;
	return x_position;
}

int GazeCalibration::get_x_index_from_position(const float x_position)
{
	const float x_norm = (x_position + EYE_TRACKING_CALIBRATION_CENTER_X) / EYE_TRACKING_CALIBRATION_CELL_SIZE_X;
	const int x_index = bvr_clamp<int>((int)x_norm, 0, EYE_TRACKING_CALIBRATION_NUM_CELLS_X - 1);
	return x_index;
}

float GazeCalibration::get_y_position_from_index(const int y_index)
{
	const float y_position = ((EYE_TRACKING_CALIBRATION_NUM_CELLS_Y - 1 - y_index) * EYE_TRACKING_CALIBRATION_CELL_SIZE_Y) - EYE_TRACKING_CALIBRATION_CENTER_Y;
	return y_position;
}

int GazeCalibration::get_y_index_from_position(const float y_position)
{
	const float y_norm = (y_position + EYE_TRACKING_CALIBRATION_CENTER_Y) / EYE_TRACKING_CALIBRATION_CELL_SIZE_Y;
	const int y_index = bvr_clamp<int>(EYE_TRACKING_CALIBRATION_NUM_CELLS_Y - 1 - (int)y_norm, 0, EYE_TRACKING_CALIBRATION_NUM_CELLS_Y - 1);
	return y_index;
}

void GazeCalibration::reset_calibration()
{
	is_calibrating_ = false;
	is_calibrated_ = false;
	calibration_ = {};
	num_calibrated_ = 0;

	calibration_.points_.clear();
	calibration_.points_.resize(EYE_TRACKING_CALIBRATION_NUM_CELLS_Y);

	const glm::vec3 local_scale(EYE_TRACKING_CALIBRATION_CELL_SCALE_X, EYE_TRACKING_CALIBRATION_CELL_SCALE_Y, 0.0f);

	for(int y_index = 0; y_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_Y; y_index++)
	{
		calibration_.points_[y_index].resize(EYE_TRACKING_CALIBRATION_NUM_CELLS_X);

		for(int x_index = 0; x_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_X; x_index++)
		{
			CalibrationPoint& point = calibration_.points_[y_index][x_index];
			point.samples_.resize(EYE_TRACKING_CALIBRATION_MAX_SAMPLES_PER_CELL);

			point.local_pose_.translation_.x = get_x_position_from_index(x_index);
			point.local_pose_.translation_.y = get_y_position_from_index(y_index);
			point.local_pose_.scale_ = local_scale;
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
			const CalibrationPoint& point = calibration_.points_[y_index][x_index];
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
	return calibration_.points_[raster_y_][raster_x_];
}

const CalibrationPoint& GazeCalibration::get_raster_point() const
{
	return calibration_.points_[raster_y_][raster_x_];
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

	int x_index = get_x_index_from_position(raw_gaze_direction.x);
	int y_index = get_y_index_from_position(raw_gaze_direction.y);

	CalibrationPoint& point = calibration_.points_[y_index][x_index];

	if (point.is_calibrated_)
	{
		const glm::vec3 calibrated_gaze_direction = glm::normalize(point.calibrated_rotation_correction_ * raw_gaze_direction);
		return calibrated_gaze_direction;
	}

	return raw_gaze_direction;
}

GLMPose	GazeCalibration::get_calibration_cube() const
{
	const CalibrationPoint& point = calibration_.points_[raster_y_][raster_x_];
	return point.local_pose_;
}

} // BVH


#endif // ENABLE_GAZE_CALIBRATION
