//--------------------------------------------------------------------------------------
// Copyright (c) 2025 BattleAxeVR. All rights reserved.
//--------------------------------------------------------------------------------------

// Author: Bela Kampis
// Date: April 11, 2025

// MIT License
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
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

#if ENABLE_PSVR2_EYE_TRACKING

// Temp hack until the code upstream is published, to keep this file private / offline
#include "ipc_client.cpp"
#include "psvr2_gazes.h"
#include "psvr2_eye_tracking.h"

namespace BVR 
{
PSVR2EyeTracker::PSVR2EyeTracker()
{
}


bool PSVR2EyeTracker::connect()
{
    if (!is_connected_ && ipc_client_.Connect()) 
    {
        is_connected_ = true;
        return true;
    }

    return false;
}

void PSVR2EyeTracker::disconnect()
{
    // No need or even support for this yet
    if (is_connected_) 
    {
        //is_connected_ = false;
    }
}

bool PSVR2EyeTracker::are_gazes_available() const 
{
#if 0
    if (!is_connected_)
    {
        return false;
    }

    ipc::Request_t request_gaze_packet(ipc::RequestHandshake);
    ipc::Response_t response = ipc_client_.SendBlocking(request_gaze_packet);

    if (response.type == ipc::ResponseHandshake) 
    {
        return true;
    }

    return false;
#else
    return true;
#endif
}

bool PSVR2EyeTracker::update_gazes()
{
	connect();

	if(!is_connected_)
	{
		return false;
	}

	ipc::Request_t request_gaze_packet(ipc::RequestUpdateGazeState);
	ipc::Response_t response = ipc_client_.SendBlocking(request_gaze_packet);

	if(response.type == ipc::ResponseSuccess)
	{
		Psvr2GazeState_t* pLocalGazeState = (Psvr2GazeState_t*)&response.gazeState;

		if(pLocalGazeState)
		{
			const GazeCombined_t& combined_gaze = pLocalGazeState->packetData.combined;

			if(combined_gaze.bIsValid && combined_gaze.bNormalisedGazeValid)
			{
				valid_combined_gaze_vector_ = DirectX::XMVectorSet(combined_gaze.vNormalisedGaze.x, combined_gaze.vNormalisedGaze.x, combined_gaze.vNormalisedGaze.x, 1);
				previous_combined_gaze_valid_ = true;
			}

			const EyeGaze_t& left_gaze = pLocalGazeState->packetData.left;

			if(left_gaze.bGazeDirectionValid)// !per_eye_gaze.blink)
			{
				valid_per_eye_gaze_vectors_[LEFT] = DirectX::XMVectorSet(left_gaze.vGazeDirection.x, left_gaze.vGazeDirection.x, left_gaze.vGazeDirection.x, 1);
				previous_per_eye_gazes_valid_[LEFT] = true;
			}

			const EyeGaze_t& right_gaze = pLocalGazeState->packetData.right;

			if(right_gaze.bGazeDirectionValid)// !per_eye_gaze.blink)
			{
				valid_per_eye_gaze_vectors_[RIGHT] = DirectX::XMVectorSet(right_gaze.vGazeDirection.x, right_gaze.vGazeDirection.x, right_gaze.vGazeDirection.x, 1);
				previous_per_eye_gazes_valid_[RIGHT] = true;
			}
		}
	}

    return false;
}

bool PSVR2EyeTracker::get_combined_gaze(DirectX::XMVECTOR& combined_gaze_vector) 
{
    update_gazes();

    if (previous_combined_gaze_valid_)
    {
		combined_gaze_vector = valid_combined_gaze_vector_;
		return true;
    }

    return false;
}

bool PSVR2EyeTracker::get_per_eye_gaze(const int eye, DirectX::XMVECTOR& gaze_vector) 
{
    if (eye == LEFT)
    {
        update_gazes();
    }

	if(previous_per_eye_gazes_valid_[eye])
	{
		gaze_vector = valid_per_eye_gaze_vectors_[eye];
		return true;
	}

    return false;
}

} // BVH


#endif // ENABLE_PSVR2_EYE_TRACKING