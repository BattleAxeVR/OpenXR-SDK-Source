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

#include "openxr/openxr.h"
#include "common.h"
#include <common/xr_linear.h>

#if ENABLE_PSVR2_EYE_TRACKING

namespace BVR 
{
    class PSVR2EyeTracker
    {
    public:
        PSVR2EyeTracker();

        bool connect();
        void disconnect();
        bool are_gazes_available() const;
        bool update_gazes();
        bool get_combined_gaze(XrVector3f& combined_gaze_vector);
        bool get_per_eye_gaze(const int eye, XrVector3f& per_eye_gaze_vector);

        const bool is_connected() const { return is_connected_; }
        const bool is_enabled() const { return is_enabled_; }

        void set_enabled(const bool enabled)
        {
            is_enabled_ = is_connected_ && enabled;
        }

    private:
        bool is_connected_ = false;
        bool is_enabled_ = false;

        XrVector3f valid_combined_gaze_vector_ = {};
        bool previous_combined_gaze_valid_ = false;

        XrVector3f valid_per_eye_gaze_vectors_[NUM_EYES] = {};
        bool previous_per_eye_gazes_valid_[NUM_EYES] = { false, false };
    };

}


#endif // ENABLE_PSVR2_EYE_TRACKING


#endif // PSVR2_EYE_TRACKING_H


