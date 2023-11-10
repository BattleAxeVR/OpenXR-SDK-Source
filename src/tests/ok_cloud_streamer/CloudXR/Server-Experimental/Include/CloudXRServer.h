/*
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#if !defined(__CLOUDXR_SERVER_H__)
#define __CLOUDXR_SERVER_H__

#include "CloudXRCommon.h"
#include "CloudXRInputEvents.h"
#include <climits>
#include "stdarg.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


typedef enum
{
    cxrFeatureFlags_DisableSendAudio      = 0x0001, // Disable audio streaming from server to client
    cxrFeatureFlags_DisableReceiveAudio   = 0x0002, // Disable audio streaming from client to server
    cxrFeatureFlags_Disable2PassScaling   = 0x0010, // Use faster but lower quality scaling in foveation
    cxrFeatureFlags_DisablePosePrediction = 0x0100, // Disable pose prediction
    cxrFeatureFlags_DisableVVSync         = 0x0200, // Disable vvsync
    cxrFeatureFlags_DisableAuthorization  = 0x1000, // Disable authorization
} cxrFeatureFlags;

typedef enum
 {
    cxrFrameSurfaceFormat_RGBA,
    cxrFrameSurfaceFormat_SRGBA,
    cxrFrameSurfaceFormat_BGRA,
    cxrFrameSurfaceFormat_NV12
} cxrFrameSurfaceFormat;

typedef struct cxrFoveationAxisParams
{
    uint32_t unwarped_resolution;
    float centerScale;
    float centerPosition;
    float centerRadius;
} cxrFoveationAxisParams;

typedef struct cxrFrameFoveationParams
{
    cxrFoveationMode mode;
    cxrFoveationAxisParams axis[2]; // per-axis foveation parameters [x,y]
} cxrFrameFoveationParams;

typedef enum
{
    cxrServerSurfaceFormat_RGBA, ///< 32bpp packed R8B8G8A8 sRGB
    cxrServerSurfaceFormat_BGRA, ///< 32bpp packed B8G8R8A8 sRGB
    cxrServerSurfaceFormat_ABGR, ///< 32bpp packed A8B8G8R8 sRGB
    cxrServerSurfaceFormat_NV12  ///< 12bpp packed YUV 4:2:0 planar with interleaved chroma BT709
} cxrServerSurfaceFormat;

typedef struct cxrServerVideoStreamDesc
{
    uint32_t numStreams;
    cxrServerSurfaceFormat format;
} cxrServerVideoStreamsDesc;

typedef struct cxrServerInitParams
{
    uint32_t requestedVersion;

    cxrServerVideoStreamsDesc videoStreamsDesc;

    cxrGraphicsContext serverSharedContext;
    uint32_t numStreams;
    cxrFrameSurfaceFormat surfaceFormat;
    uint32_t featureFlags; // see cxrFeatureFlags
    uint32_t debugFlags; // see cxrDebugFlags
    int32_t logMaxSizeKB;
    int32_t logMaxAgeDays;
    char librarySearchPath[CXR_MAX_PATH];
    char appOutputPath[CXR_MAX_PATH];
} cxrServerInitParams;

typedef void(*cxrNotifyServerStatusChangeCallback)(void *context, cxrServerState newState);

typedef void(*cxrClientDescReceivedCallback)(void *context, cxrDeviceDesc* newDesc);

typedef void(*cxrSetVRPropertyCallback)(void *context, uint32_t prop, float value);

typedef void(*cxrSetVRChaperoneCallback)(void *context, const cxrChaperone* chaperone);

typedef void(*cxrSetLightInfoCallback)(void *context, const cxrLightProperties* chaperone);

typedef bool(*cxrPollHapticFeedbackCallback)(void *context, cxrHapticFeedback* feedback);

typedef bool(*cxrIsValidCallback)(void *context);

typedef void(*cxrInputEventCallback)(void *context, const cxrInputEvent *inputEvent);

// Server application is responsible for pacing received audio at realtime by waiting
// in this callback until it is ready for more data. If it is discarding data it should
// return false. 
// In order to enable receiving audio cxrFeatureFlags_DisableReceiveAudio must be cleared.
typedef bool(*cxrAudioCallback)(void *context, const cxrAudioFrame *audioFrame);

// Server application is responsible for checking that the authorization header matches
// what was provided to the client outside CXR and return true if so to continue
// connecting or false if not to disconnect.
// In order to enable authorization cxrFeatureFlags_DisableAuthorization must be cleared
// and an HTTPS certificate must be assigned to the server by creating an x509 rsa:2048
// cert.pem and key.pem and placing them in the server config directory.
typedef bool(*cxrAuthorizationHeaderCallback)(void *context, const char *header);

typedef void(*cxrPoseAvailableCallback)(void *context, const cxrVRTrackingState& trackingState);

typedef cxrError(*cxrAddControllerCallback)(void *data, const cxrControllerDesc* desc);
typedef cxrError(*cxrControllerPoseCallback)(void *data, uint64_t id, const cxrControllerTrackingState* state);
typedef cxrError(*cxrControllerEventsCallback)(void *data, uint64_t id, const cxrControllerEvent* events, uint32_t eventCount);
typedef cxrError(*cxrRemoveControllerCallback)(void *data, uint64_t id);

typedef struct cxrServerCallbacks
{
    cxrNotifyServerStatusChangeCallback pfnStatusChangeCallback;
    cxrClientDescReceivedCallback pfnClientDescReceivedCallback;
    cxrSetVRPropertyCallback pfnSetVRPropertyCallback;
    cxrSetVRChaperoneCallback pfnSetVRChaperoneCallback;
    cxrSetLightInfoCallback pfnSetLightInfoCallback;
    cxrPollHapticFeedbackCallback pfnPollHapticFeedbackCallback;
    cxrIsValidCallback pfnIsValidCallback;
    cxrInputEventCallback pfnInputEventCallback;
    cxrAudioCallback pfnAudioCallback;
    cxrAuthorizationHeaderCallback pfnAuthorizationHeaderCallback;
    cxrPoseAvailableCallback pfnPoseAvailableCallback;
    cxrMessageCallbackFunc messageCallback;
    cxrAddControllerCallback addControllerCallback;
    cxrControllerPoseCallback controllerPoseCallback;
    cxrControllerEventsCallback controllerEventsCallback;
    cxrRemoveControllerCallback removeControllerCallback;
    void* context;
} cxrServerCallbacks;

typedef struct cxrServer* cxrServerHandle;

typedef cxrError(*cxrCreateServerFunc)(cxrServerHandle* cloudXRServerHandle, const cxrServerInitParams* initParams,
                                        const cxrServerCallbacks *serverCallbacks);

typedef cxrError(*cxrGetDeviceDescFunc)(cxrServerHandle cloudXRServerHandle, cxrDeviceDesc* desc);

typedef cxrError(*cxrWaitForVVSyncFunc)(cxrServerHandle cloudXRServerHandle);

typedef cxrError(*cxrGetPoseFunc)(cxrServerHandle cloudXRServerHandle, cxrVRTrackingState* vrTrackingState, float avgServerRenderTime);

typedef cxrError(*cxrGetLightPropertiesFunc)(cxrServerHandle cloudXRServerHandle, cxrLightProperties* lightProperties);

typedef cxrError(*cxrSubmitFrameFunc)(cxrServerHandle cloudXRServerHandle, const cxrVideoFrame*  videoFrame, const cxrMatrix34* hmdMatrix, uint64_t poseID);

typedef cxrError(*cxrSubmitFoveatedFrameFunc)(cxrServerHandle cloudXRServerHandle, const cxrVideoFrame* videoFrame, const cxrMatrix34* hmdMatrix,
                                                const cxrFrameFoveationParams* fovParams, const uint64_t poseID);

typedef cxrError(*cxrSendUserDataFunc)(cxrServerHandle cloudXRServerHandle, const void* data, uint32_t dataSize);

typedef cxrError(*cxrSendAudioDataFunc)(cxrServerHandle handle, const cxrAudioFrame* audioFrame);

typedef cxrBool(*cxrServerIsStreamingFunc)(cxrServerHandle cloudXRServerHandle);

typedef cxrError(*cxrSetVVSyncTargetFpsFunc)(cxrServerHandle cloudXRServerHandle, float newFps, float queueTime);

typedef cxrError(*cxrVVSyncFunc)(cxrServerHandle cloudXRServerHandle, cxrBool active);

typedef cxrError(*cxrDestroyServerFunc)(cxrServerHandle cloudXRServerHandle);

typedef const char*(*cxrErrorStringFunc)(cxrError E);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif
