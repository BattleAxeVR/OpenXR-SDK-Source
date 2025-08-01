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

			CalibrationPoint& point = calibration_.points_[y_index][x_index];

			const float x_delta = (127 - clamped_red) * EYE_TRACKING_CALIBRATION_TOLERANCE_DIST / 127.0f;
			const float y_delta = (127 - clamped_green) * EYE_TRACKING_CALIBRATION_TOLERANCE_DIST / 127.0f;
			point.average_delta_ = glm::vec3(x_delta, y_delta, 0.0f);
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

			const CalibrationPoint& point = calibration_.points_[y_index][x_index];

#if 1
			const float normalized_red = point.average_delta_.x / EYE_TRACKING_CALIBRATION_TOLERANCE_DIST;
			const float normalized_green = point.average_delta_.y / EYE_TRACKING_CALIBRATION_TOLERANCE_DIST;
			const float normalized_blue = point.average_error_ / EYE_TRACKING_CALIBRATION_TOLERANCE_MAX_ERROR;

			const int red_int = bvr_clamp<int>(int(normalized_red * 127.0f), -127, 127) + 127;
			const int green_int = bvr_clamp<int>(int(normalized_green * 127.0f), -127, 127) + 127;
			const int blue_int = bvr_clamp<int>(int(normalized_blue * 127.0f), -127, 127) + 127;

			red_val = (stbi_uc)red_int;
			green_val = (stbi_uc)green_int;
			blue_val = (stbi_uc)blue_int;
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

bool CalibrationPoint::add_sample(const glm::vec3& input)
{
	if(is_calibrated_)
	{
		return false;
	}

	glm::vec3 output = local_pose_.translation_;
	output.z = -DISTANCE_TO_VIEW_FRONT_METERS;
	output = glm::normalize(output);

	const glm::vec3 delta = (output - input);
	const glm::vec2 delta_xy = glm::vec2(delta.x, delta.y);
	const float error = glm::length(delta_xy);

	if (error < EYE_TRACKING_CALIBRATION_TOLERANCE_DIST)
	{
		CalibrationMapping& sample = samples_[num_samples_++];
		sample.input_ = input;
		sample.output_ = output;
		sample.delta_ = delta;
		sample.error_ = error;

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

	average_delta_ = zero_vec3;
	average_error_ = 0;

	for(const CalibrationMapping& sample : samples_)
	{
		average_delta_ += sample.delta_;
		average_error_ += sample.error_;
	}

	average_delta_ /= (float)EYE_TRACKING_CALIBRATION_MAX_SAMPLES_PER_CELL;
	average_error_ /= (float)EYE_TRACKING_CALIBRATION_MAX_SAMPLES_PER_CELL;

	is_calibrated_ = true;
	return true;
}

void GazeCalibration::start_calibration()
{
	if(!is_calibrated())
	{
		is_calibrating_ = true;
	}
	else
	{
		is_calibrating_ = false;
	}
}

void GazeCalibration::stop_calibration()
{
	if (is_calibrating_)
	{
		is_calibrating_ = false;

#if AUTO_INCREMENT_ON_STOP_CALIBRATION
		increment_raster();
#endif
	}
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

#if 0
	for(int y_index = 0; y_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_Y; y_index++)
	{
		for(int x_index = 0; x_index < EYE_TRACKING_CALIBRATION_NUM_CELLS_X; x_index++)
		{
			const CalibrationPoint& point = calibration_.points_[y_index][x_index];
		}
	}
#endif
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

#if AUTO_CALIBRATE
	if ((raster_x_ == 0) && (raster_y_ == 0) && (num_calibrated_ == EYE_TRACKING_CALIBRATION_NUM_CELLS))
	{
		// We're done!
		compute_calibration();
		stop_calibration();

#if AUTO_SAVE_CALIBRATION_WHEN_DONE
		save_calibration();
#endif

#if AUTO_QUIT_APP_WHEN_DONE
		exit(0);
#endif
	}
#endif
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
		const glm::vec3 calibrated_gaze_direction = glm::normalize(point.average_delta_ + raw_gaze_direction);
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
