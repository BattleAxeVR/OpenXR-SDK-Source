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

#include "openxr/openxr.h"

namespace BVR 
{
    typedef XrVector3f GazeVec3Type;

    struct GazeState
    {
        GazeVec3Type local_gaze_direction_ = {};
        bool is_valid_ = false;
    };

    class PSVR2EyeTracker
    {
    public:
        PSVR2EyeTracker();

        bool connect();
        void disconnect();
        bool update_gazes();

        bool is_combined_gaze_available() const;
        bool is_gaze_available(const int eye) const;
        
        bool get_combined_gaze(GazeVec3Type& combined_gaze_direction);
        bool get_per_eye_gaze(const int eye, GazeVec3Type& per_eye_gaze_direction);

        const bool is_connected() const { return is_connected_; }
        const bool is_enabled() const { return is_enabled_; }

        void set_enabled(const bool enabled)
        {
            is_enabled_ = is_connected_ && enabled;
        }

    private:
        bool is_connected_ = false;
        bool is_enabled_ = false;

        GazeState combined_gaze_;
        GazeState per_eye_gazes_[NUM_EYES];
    };
}


#endif // ENABLE_PSVR2_EYE_TRACKING


#endif // PSVR2_EYE_TRACKING_H


