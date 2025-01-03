// Copyright (c) 2017-2024, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <exception>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include <time.h>
#include <string.h>

//
// Platform Headers
//
#ifdef XR_USE_PLATFORM_WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // !WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif  // !NOMINMAX

#include <windows.h>
#include <wrl/client.h>  // For Microsoft::WRL::ComPtr
#endif

#ifdef XR_USE_PLATFORM_ANDROID
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <jni.h>
#include <sys/system_properties.h>
#endif

#ifdef XR_USE_PLATFORM_WAYLAND
#include "wayland-client.h"
#endif

#ifdef XR_USE_PLATFORM_XLIB
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#ifdef XR_USE_PLATFORM_XCB
#include <xcb/xcb.h>
#endif

//
// Graphics Headers
//
#ifdef XR_USE_GRAPHICS_API_D3D11
#include <d3d11_4.h>
#endif

#ifdef XR_USE_GRAPHICS_API_D3D12
#include <d3d12.h>
#endif

#ifdef XR_USE_GRAPHICS_API_OPENGL
#if defined(XR_USE_PLATFORM_XLIB) || defined(XR_USE_PLATFORM_XCB)
#include <GL/glx.h>
#endif
#ifdef XR_USE_PLATFORM_XCB
#include <xcb/glx.h>
#endif
#ifdef XR_USE_PLATFORM_WIN32
#include <wingdi.h>  // For HGLRC
#include <GL/gl.h>
#endif
#endif

#ifdef XR_USE_GRAPHICS_API_OPENGL_ES
#include <EGL/egl.h>
#endif

#ifdef XR_USE_PLATFORM_EGL
#include <EGL/egl.h>
#endif  // XR_USE_PLATFORM_EGL

#ifdef XR_USE_GRAPHICS_API_VULKAN
#ifdef XR_USE_PLATFORM_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#ifdef XR_USE_PLATFORM_ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#endif
#include <vulkan/vulkan.h>
#endif

//
// OpenXR Headers
//
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#define ADD_EXTRA_CUBES 1

#ifndef ENABLE_ADVANCED_FEATURES
#define ENABLE_ADVANCED_FEATURES 1
#endif

#define THUMBSTICK_TURNING_SPEED_POWER 2.0f
#define RUNNING_GRIP_THRESHOLD 0.1f
#define SPINNING_GRIP_THRESHOLD 0.1f
#define VIBRATION_GRIP_THRESHOLD 0.9f

#if ENABLE_ADVANCED_FEATURES

#include <meta_openxr_preview/extx1_event_channel.h>
#include <meta_openxr_preview/meta_body_tracking_calibration.h>
#include <meta_openxr_preview/meta_body_tracking_fidelity.h>
#include <meta_openxr_preview/meta_body_tracking_full_body.h>
#include <meta_openxr_preview/meta_detached_controllers.h>
#include <meta_openxr_preview/meta_hand_tracking_wide_motion_mode.h>
#include <meta_openxr_preview/meta_simultaneous_hands_and_controllers.h>
#include <meta_openxr_preview/openxr_extension_helpers.h>

#ifdef XR_USE_PLATFORM_ANDROID
#define PLATFORM_ANDROID 1
#define PLATFORM_PC 0
#elif XR_USE_PLATFORM_WIN32
#define PLATFORM_ANDROID 0
#define PLATFORM_PC 1
#endif

#ifndef ENABLE_STREAMLINE
#define ENABLE_STREAMLINE (PLATFORM_PC && 0)
#endif

#define SL_MANUAL_HOOKING (ENABLE_STREAMLINE && 0)
#define ENABLE_STREAMLINE_WRAPPER (ENABLE_STREAMLINE && 0)

#define ENABLE_OPENXR_FB_REFRESH_RATE (PLATFORM_ANDROID && 1)
#define DEFAULT_REFRESH_RATE 72.0f
#define DESIRED_REFRESH_RATE 90.0f

#define ENABLE_OPENXR_FB_RENDER_MODEL 0

#define ENABLE_OPENXR_FB_SHARPENING (PLATFORM_ANDROID && 1) // Only works on standalone Android builds, not PC / Link
#define TOGGLE_SHARPENING_AT_RUNTIME_USING_RIGHT_GRIP (ENABLE_OPENXR_FB_SHARPENING && 0) // for debugging / comparison
#define ENABLE_OPENXR_FB_COMPOSITION_LAYER_SETTINGS ENABLE_OPENXR_FB_SHARPENING

// This is pointless, as Quest Pro supports LD as a platform-wide system setting. Just use that.
#define ENABLE_OPENXR_FB_LOCAL_DIMMING (PLATFORM_ANDROID && 0)

// Eye tracking only enabled on PC for now (needs permissions on Android, requires java calls. TODO)
#define ENABLE_OPENXR_FB_EYE_TRACKING_SOCIAL (PLATFORM_PC && 0) 
#define ENABLE_EXT_EYE_TRACKING (PLATFORM_PC && 1)
#define ENABLE_EYE_TRACKING (ENABLE_OPENXR_FB_EYE_TRACKING_SOCIAL || ENABLE_EXT_EYE_TRACKING)

#define DRAW_LOCAL_EYE_LASERS (ENABLE_EYE_TRACKING && 1)
#define DRAW_WORLD_EYE_LASERS (ENABLE_EYE_TRACKING && USE_THUMBSTICKS && 1)

#define ENABLE_OPENXR_META_FOVEATION_EYE_TRACKED (PLATFORM_PC && 0)

// Face tracking (not implemented yet)
#define ENABLE_OPENXR_FB_FACE_TRACKING 0
#define ENABLE_OPENXR_HAND_TRACKING 0
#define ENABLE_OPENXR_FB_BODY_TRACKING 1 // Hand tracking is redundant if you have body tracking, which includes all the same finger joints
#define ENABLE_OPENXR_FB_SIMULTANEOUS_HANDS_AND_CONTROLLERS ((ENABLE_OPENXR_FB_BODY_TRACKING || ENABLE_OPENXR_HAND_TRACKING) && 1)

// New inside-out body tracking, depends on FB Body Tracking
#define ENABLE_OPENXR_META_FULL_BODY_TRACKING (ENABLE_OPENXR_FB_BODY_TRACKING && 1)
#define ENABLE_OPENXR_META_BODY_TRACKING_FIDELITY (ENABLE_OPENXR_META_FULL_BODY_TRACKING && 1)

// HTC Vive Trackers / Ultimate Trackers support
#define ENABLE_VIVE_TRACKERS (PLATFORM_PC && 1) 
#define ADAPT_VIVE_TRACKER_POSES (ENABLE_VIVE_TRACKERS && 1)

#define ENABLE_VIVE_HANDHELD_OBJECTS (ENABLE_VIVE_TRACKERS && 0) 
#define ENABLE_VIVE_FEET (ENABLE_VIVE_TRACKERS && 1) 
#define ENABLE_VIVE_SHOULDERS (ENABLE_VIVE_TRACKERS && 1) 
#define ENABLE_VIVE_ELBOWS (ENABLE_VIVE_TRACKERS && 1) 
#define ENABLE_VIVE_KNEES (ENABLE_VIVE_TRACKERS && 1) 
#define ENABLE_VIVE_WRISTS (ENABLE_VIVE_TRACKERS && 0) 
#define ENABLE_VIVE_ANKLES (ENABLE_VIVE_TRACKERS && 0) 
#define ENABLE_VIVE_WAIST (ENABLE_VIVE_TRACKERS && 1) 
#define ENABLE_VIVE_CHEST (ENABLE_VIVE_TRACKERS && 1) 
#define ENABLE_VIVE_CAMERA (ENABLE_VIVE_TRACKERS && 0) // Unsupported for now
#define ENABLE_VIVE_KEYBOARD (ENABLE_VIVE_TRACKERS && 0) // Unsupported for now

#define ENABLE_BODY_TRACKING (ENABLE_OPENXR_FB_BODY_TRACKING || ENABLE_VIVE_TRACKERS)

#define ENABLE_QUAD_LAYER 0

#define USE_WAIST_ORIENTATION_FOR_STICK_DIRECTION (ENABLE_BODY_TRACKING && USE_THUMBSTICKS && 1)
#define SUPPORT_BACKWARDS_WAIST_ORIENTATION (USE_WAIST_ORIENTATION_FOR_STICK_DIRECTION && ENABLE_VIVE_TRACKERS && 1)
#define BACKWARDS_DOT_PRODUCT_THRESHOLD -0.5f


#define USE_DUAL_LAYERS 0

#define LOG_EYE_TRACKING_DATA (ENABLE_EYE_TRACKING && 0)
#define LOG_BODY_TRACKING_DATA (ENABLE_OPENXR_FB_BODY_TRACKING && 0)

#define USE_SDL_JOYSTICKS 0

#define USE_THUMBSTICKS 1
#define USE_THUMBSTICKS_FOR_MOVEMENT (USE_THUMBSTICKS && 1)

#define USE_THUMBSTICKS_FOR_MOVEMENT_X (USE_THUMBSTICKS_FOR_MOVEMENT && 1)
#define USE_THUMBSTICKS_FOR_MOVEMENT_Y (USE_THUMBSTICKS_FOR_MOVEMENT && 1)

#define USE_THUMBSTICKS_STRAFING_SPEED_POWER (USE_THUMBSTICKS_FOR_MOVEMENT_X && 1)
#define THUMBSTICK_STRAFING_SPEED_POWER 2.0f

#define SUPPORT_SMOOTH_TURNING (USE_THUMBSTICKS && 1)
#define PREFER_SMOOTH_TURNING (SUPPORT_SMOOTH_TURNING && 1)

#define SUPPORT_SNAP_TURNING (USE_THUMBSTICKS && 1)
#define PREFER_SNAP_TURNING (SUPPORT_SNAP_TURNING && !PREFER_SMOOTH_TURNING && 1)
#define SNAP_TURN_DEGREES_DEFAULT 30.0f

#define USE_THUMBSTICKS_FOR_TURNING ((SUPPORT_SMOOTH_TURNING || SUPPORT_SNAP_TURNING) && 1)

#define USE_THUMBSTICKS_TURNING_SPEED_POWER (USE_THUMBSTICKS_FOR_TURNING && 1)

#define SUPPORT_RUNNING_WITH_LEFT_GRIP (USE_THUMBSTICKS_FOR_MOVEMENT && 1)

#define SUPPORT_SPINNING_WITH_RIGHT_GRIP (USE_THUMBSTICKS_FOR_TURNING && 1)

#define WALKING_SPEED 0.02f
#define SMOOTH_TURNING_ROTATION_SPEED 1.0f

#define CONTROLLER_THUMBSTICK_DEADZONE_X 0.05f
#define CONTROLLER_THUMBSTICK_DEADZONE_Y 0.05f

#define ROTATION_DEADZONE 0.05f

#define ADD_GRIP_POSE 1
#define DRAW_GRIP_POSE (ADD_GRIP_POSE && 1)

#define ADD_AIM_POSE 1
#define DRAW_AIM_POSE (ADD_AIM_POSE && 1)
#define DRAW_ALL_VIVE_TRACKERS (ENABLE_VIVE_TRACKERS && 1)

#define DRAW_VISUALIZED_SPACES 0

#define SUPPORT_THIRD_PERSON_LOCO 1

// Draw / debug
#define DRAW_LOCAL_POSES (USE_THUMBSTICKS && 1)
#define DRAW_FIRST_PERSON_POSES (USE_THUMBSTICKS && 1)
#define DRAW_THIRD_PERSON_POSES (SUPPORT_THIRD_PERSON_LOCO && 1)

#define DRAW_BODY_JOINTS (ENABLE_BODY_TRACKING && 1)
#define DRAW_WAIST_DIRECTION (USE_WAIST_ORIENTATION_FOR_STICK_DIRECTION && 1)

#define LOCAL_WAIST_DIRECTION_OFFSET_Y 0.3f
#define LOCAL_WAIST_DIRECTION_OFFSET_Z 2.0f

#define BODY_CUBE_SIZE 0.02f
#define ADD_GROUND 0

#define SUPPORT_SCREENSHOTS 0
#define TAKE_SCREENSHOT_WITH_LEFT_GRAB (SUPPORT_SCREENSHOTS && 0)
#define ENABLE_LOCAL_DIMMING_WITH_RIGHT_GRAB (ENABLE_OPENXR_FB_LOCAL_DIMMING && 0)
#define LOG_IPD 0
#define LOG_FOV 0
#define LOG_MATRICES 0

#define ENABLE_BFI 0
#define ENABLE_ALTERNATE_EYE_RENDERING (!ENABLE_BFI && 0)
#define DEBUG_ALTERNATE_EYE_RENDERING (ENABLE_ALTERNATE_EYE_RENDERING && 0)
#define DEBUG_ALTERNATE_EYE_RENDERING_ALT (ENABLE_ALTERNATE_EYE_RENDERING && 0)

#define ENABLE_HDR_SWAPCHAIN 0

#include "utils.h"

#endif
