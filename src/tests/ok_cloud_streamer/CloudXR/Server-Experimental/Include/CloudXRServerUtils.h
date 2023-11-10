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

#if !defined(__CLOUDXR_SERVER_UTILS_H__)
#define __CLOUDXR_SERVER_UTILS_H__

#include "CloudXRServer.h"
#ifndef LOG_TAG
#define LOG_TAG  "CloudXRServer"
#endif
#include "CloudXRLog.h"

#ifdef _WIN32
#if !defined _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
struct IUnknown;
#include <Windows.h>
#include <Shlobj.h>
#include <wchar.h>
#include <pathcch.h>
#include <stdio.h>
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Pathcch.lib")
#else
#include <dlfcn.h>
#endif
#include <string>

// Fallback for loading using the legacy library name
#ifdef _WIN32
#define CLOUDXR_SERVER_DLL "driver_CloudXRRemoteHMD.dll"
#else
#define CLOUDXR_SERVER_DLL "driver_CloudXRRemoteHMD.so"
#endif

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#if defined(_WIN32)
#define cxr_dlopen(name) LoadLibraryA(name)
#define cxr_dlclose(handle) FreeLibrary((HMODULE) handle)
#define cxr_dlsym(handle, name) GetProcAddress((HMODULE) handle, name)
    typedef HINSTANCE cxr_dlHandle;
#elif __linux__
#define cxr_dlopen(name) dlopen(name, RTLD_NOW | RTLD_LOCAL)
#define cxr_dlclose(handle) dlclose(handle)
#define cxr_dlsym(handle, name) dlsym(handle, name)
    typedef void* cxr_dlHandle;
#endif

typedef struct cxrServerModule
{
    cxrCreateServerFunc CreateCloudXRServer;
    cxrGetDeviceDescFunc GetDeviceDesc;
    cxrWaitForVVSyncFunc WaitForVVSync;
    cxrGetPoseFunc GetPose;
    cxrSubmitFrameFunc SubmitFrame;
    cxrSendUserDataFunc SendUserData;
    cxrSendAudioDataFunc SendAudioData;
    cxrServerIsStreamingFunc ServerIsStreaming;
    cxrSetVVSyncTargetFpsFunc SetVVSyncTargetFps;
    cxrDestroyServerFunc DestroyCloudXRServer;
    cxrSubmitFoveatedFrameFunc SubmitFoveatedFrame;
    cxrErrorStringFunc ErrorString;
    cxr_dlHandle dllHandle;
} cxrServerModule;


// lets's provide a basic console-logger that we can use internally here
// and can be used by apps prior to loading the library or after it is unloaded.
#ifndef rawConsoleLog
#define rawConsoleLog(fmt, ...)  do {  printf(fmt "\n", ## __VA_ARGS__); } while (0)
#endif

static inline cxrError cxrLoadServerModule(cxrServerModule *serverModule, const char *searchPath)
{
    bool error = false;

    if (!serverModule)
    {
        CXR_LOGE("The serverModule parameter is required");
        return cxrError_Required_Parameter;
    }

    serverModule->dllHandle = nullptr;
    char libraryPath[CXR_MAX_PATH] = { 0 };
    if (searchPath)
    {
#ifdef _WIN32
        snprintf(libraryPath, CXR_MAX_PATH, "%s\\CloudXRServer_%d_%d.dll", searchPath, CLOUDXR_VERSION_MAJOR, CLOUDXR_VERSION_MINOR);
        serverModule->dllHandle = LoadLibraryExA(libraryPath, nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
#elif __linux__
        snprintf(libraryPath, CXR_MAX_PATH, "%s/libCloudXRServer_%d_%d.so", searchPath, CLOUDXR_VERSION_MAJOR, CLOUDXR_VERSION_MINOR);
        serverModule->dllHandle = cxr_dlopen(libraryPath);
#endif
    }
    else
    {
#ifdef _WIN32
        snprintf(libraryPath, CXR_MAX_PATH, "CloudXRServer_%d_%d.dll", CLOUDXR_VERSION_MAJOR, CLOUDXR_VERSION_MINOR);
#elif __linux__
        snprintf(libraryPath, CXR_MAX_PATH, "libCloudXRServer_%d_%d.so", CLOUDXR_VERSION_MAJOR, CLOUDXR_VERSION_MINOR);
#endif
        serverModule->dllHandle = cxr_dlopen(libraryPath);
    }

    // Fallback to loading using the legacy library name
    if (!serverModule->dllHandle)
    {
        CXR_LOGW("Attempting to load the server library using legacy name %s.", CLOUDXR_SERVER_DLL);
        serverModule->dllHandle = cxr_dlopen(CLOUDXR_SERVER_DLL);
    }

    if (!serverModule->dllHandle)
    {
        CXR_LOGE("CloudXR server library could not be loaded from %s.",
            searchPath == nullptr ? "the default OS search paths" : searchPath);
        return cxrError_Module_Load_Failed;
    }


    serverModule->CreateCloudXRServer = (cxrCreateServerFunc)cxr_dlsym(serverModule->dllHandle, "cxrCreateServer");
    if (!serverModule->CreateCloudXRServer)
    {
        CXR_LOGE("Couldn't find cxrCreateServer().");
        error = true;
    }

    serverModule->GetDeviceDesc = (cxrGetDeviceDescFunc)cxr_dlsym(serverModule->dllHandle, "cxrGetDeviceDesc");
    if (!serverModule->GetDeviceDesc)
    {
        CXR_LOGE("Couldn't find cxrGetDeviceDesc().");
        error = true;
    }

    serverModule->WaitForVVSync = (cxrWaitForVVSyncFunc)cxr_dlsym(serverModule->dllHandle, "cxrWaitForVVSync");
    if (!serverModule->WaitForVVSync)
    {
        CXR_LOGE("Couldn't find cxrWaitForVVSync().");
        error = true;
    }

    serverModule->GetPose = (cxrGetPoseFunc)cxr_dlsym(serverModule->dllHandle, "cxrGetPose");
    if (!serverModule->GetPose)
    {
        CXR_LOGE("Couldn't find cxrGetPose().");
        error = true;
    }

    serverModule->SubmitFrame = (cxrSubmitFrameFunc)cxr_dlsym(serverModule->dllHandle, "cxrSubmitFrame");
    if (!serverModule->SubmitFrame)
    {
        CXR_LOGE("Couldn't find cxrSubmitFrame().");
        error = true;
    }

    serverModule->SendUserData = (cxrSendUserDataFunc)cxr_dlsym(serverModule->dllHandle, "cxrSendUserData");
    if (!serverModule->SendUserData)
    {
        CXR_LOGE("Couldn't find cxrSendUserData().");
        error = true;
    }

    serverModule->SendAudioData = (cxrSendAudioDataFunc)cxr_dlsym(serverModule->dllHandle, "cxrSendAudioData");
    if (!serverModule->SendAudioData)
    {
        CXR_LOGE("Couldn't find cxrSendAudioData().");
        error = true;
    }

    serverModule->ServerIsStreaming = (cxrServerIsStreamingFunc)cxr_dlsym(serverModule->dllHandle, "cxrServerIsStreaming");
    if (!serverModule->ServerIsStreaming)
    {
        CXR_LOGE("Couldn't find cxrServerIsStreaming().");
        error = true;
    }

    serverModule->SetVVSyncTargetFps = (cxrSetVVSyncTargetFpsFunc)cxr_dlsym(serverModule->dllHandle, "cxrSetVVSyncTargetFps");
    if (!serverModule->SetVVSyncTargetFps)
    {
        CXR_LOGE("Couldn't find cxrSetVVSyncTargetFps().");
        error = true;
    }

    serverModule->DestroyCloudXRServer = (cxrDestroyServerFunc)cxr_dlsym(serverModule->dllHandle, "cxrDestroyServer");
    if (!serverModule->DestroyCloudXRServer)
    {
        CXR_LOGE("Couldn't find cxrDestroyServer().");
        error = true;
    }

    serverModule->SubmitFoveatedFrame = (cxrSubmitFoveatedFrameFunc)cxr_dlsym(serverModule->dllHandle, "cxrSubmitFoveatedFrame");
    if (!serverModule->SubmitFoveatedFrame)
    {
        CXR_LOGE("Couldn't find cxrSubmitFoveatedFrame().");
        error = true;
    }

    serverModule->ErrorString = (cxrErrorStringFunc)cxr_dlsym(serverModule->dllHandle, "cxrErrorString");
    if (!serverModule->ErrorString)
    {
        CXR_LOGE("Couldn't find cxrErrorString().");
        error = true;
    }

    if (error)
    {
        cxr_dlclose(serverModule->dllHandle);
        serverModule->dllHandle = NULL;
        CXR_LOGE("CloudXR server library missing symbols.");
        return cxrError_Module_Load_Failed;
    }
    else
    {
        CXR_LOGI("Successfully loaded CloudXR server library.");
        return cxrError_Success;
    }
}

static inline void cxrUnloadServerModule(cxrServerModule *serverModule)
{
    if (!serverModule || !serverModule->dllHandle)
        return;

    CXR_LOGI("Unloading CloudXR server library.");
    cxr_dlclose(serverModule->dllHandle);
    serverModule->dllHandle = NULL;
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif
