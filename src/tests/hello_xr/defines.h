//--------------------------------------------------------------------------------------
// Copyright (c) 2025 BattleAxeVR. All rights reserved.
//--------------------------------------------------------------------------------------

#ifndef DEFINES_H__
#define DEFINES_H__

namespace BVR
{
	const int LEFT = 0;
	const int RIGHT = 1;
	const int NUM_EYES = 2;
}

#define INVALID_INDEX -1
#define DRAW_FLOOR_AND_CEILING 0

#define SUPPORT_THIRD_PERSON 0
#define PREFER_THIRD_PERSON_AUTO (SUPPORT_THIRD_PERSON && 0)

#ifndef ENABLE_ADVANCED_FEATURES
#define ENABLE_ADVANCED_FEATURES 1
#endif

#define CEILING_HEIGHT_METERS 5.0f

#define THUMBSTICK_TURNING_SPEED_POWER 2.0f
#define GRIP_THRESHOLD 0.1f
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
#define ENABLE_OPENXR_FB_EYE_TRACKING_SOCIAL (PLATFORM_PC && 0)  // An OpenXR API layer plugin will be written to expose PSVR 2 independent gazes through this API.
#define ENABLE_EXT_EYE_TRACKING (PLATFORM_PC && 0) // Quest Pro + API plugin via Meta Link, or PSVR 2 via same plugin -> SteamVR, or directly via SteamVR exposing ET internally

#define ENABLE_PSVR2_TOOLKIT (PLATFORM_PC && 1)

#define ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE (ENABLE_PSVR2_TOOLKIT && 0)
#define ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES (ENABLE_PSVR2_TOOLKIT && 1)

#define ENABLE_PSVR2_EYE_TRACKING ((ENABLE_PSVR2_EYE_TRACKING_COMBINED_GAZE || ENABLE_PSVR2_EYE_TRACKING_PER_EYE_GAZES) && 1)

#define ENABLE_GAZE_CALIBRATION (ENABLE_PSVR2_EYE_TRACKING && 1) // Only for PSVR 2 for now
#define TOGGLE_APPLY_CALIBRATION (ENABLE_GAZE_CALIBRATION && 1)

#define EYE_TRACKING_CALIBRATION_MAX_DEGREES 45.0f

#define EYE_TRACKING_CALIBRATION_CELL_RANGE_X 5.0f
#define EYE_TRACKING_CALIBRATION_CELL_RANGE_Y 5.0f

#define EYE_TRACKING_CALIBRATION_CELL_SCALE_X 0.5f
#define EYE_TRACKING_CALIBRATION_CELL_SCALE_Y 0.5f

#define EYE_TRACKING_CALIBRATION_NUM_CELLS_X 3
#define EYE_TRACKING_CALIBRATION_NUM_CELLS_Y 3
#define EYE_TRACKING_CALIBRATION_NUM_CELLS (EYE_TRACKING_CALIBRATION_NUM_CELLS_X * EYE_TRACKING_CALIBRATION_NUM_CELLS_Y)

#define EYE_TRACKING_CALIBRATION_CELL_X_CENTER ((EYE_TRACKING_CALIBRATION_NUM_CELLS_X - 1) / 2)
#define EYE_TRACKING_CALIBRATION_CELL_Y_CENTER ((EYE_TRACKING_CALIBRATION_NUM_CELLS_Y - 1) / 2)

#define EYE_TRACKING_CALIBRATION_HARDCODED_DEG_SCALE 1.0f
#define EYE_TRACKING_CALIBRATION_HARDCODED_Y_DEG_SCALE 1.0f

#define EYE_TRACKING_CALIBRATION_SAMPLES_PER_CELL 10
#define EYE_TRACKING_CALIBRATION_DISTANCE_THRESHOLD 1.0f 

#define EYE_TRACKING_CALIBRATION_TOTAL_CELLS (EYE_TRACKING_CALIBRATION_NUM_CELLS_X * EYE_TRACKING_CALIBRATION_NUM_CELLS_Y)

#define CALIBRATION_CUBE_SIZE 0.01f

#define NUM_CALIBRATION_GRANULARITIES 1

#define ENABLE_PSVR2_EYE_TRACKING_AUTOMATICALLY (ENABLE_PSVR2_EYE_TRACKING && 1)
#define ENABLE_EYE_TRACKING (ENABLE_OPENXR_FB_EYE_TRACKING_SOCIAL || ENABLE_EXT_EYE_TRACKING || ENABLE_PSVR2_EYE_TRACKING)

#define DRAW_EYE_LASERS (ENABLE_EYE_TRACKING && 1)
#define DRAW_FIRST_PERSON_EYE_LASERS (DRAW_EYE_LASERS && USE_THUMBSTICKS && 1)
#define DRAW_THIRD_PERSON_EYE_LASERS (DRAW_EYE_LASERS && SUPPORT_THIRD_PERSON && USE_THUMBSTICKS && 1)

#define EYE_LASER_WIDTH 0.005f
#define EYE_LASER_DISTANCE_OFFSET 0.3f
#define USE_CUBE_PER_EYE_RELEVANCE (ENABLE_EYE_TRACKING && 1)
#define BOTH_EYE_RELEVANCE -1

#define ENABLE_OPENXR_META_FOVEATION_EYE_TRACKED (PLATFORM_PC && 0)

// Face tracking (not implemented yet)
#define ENABLE_OPENXR_FB_FACE_TRACKING 0
#define ENABLE_OPENXR_HAND_TRACKING 0
#define ENABLE_OPENXR_FB_BODY_TRACKING 0 // Hand tracking is redundant if you have body tracking, which includes all the same finger joints
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
#define USE_THUMBSTICKS_FOR_MOVEMENT (USE_THUMBSTICKS && 0)

#define USE_THUMBSTICKS_FOR_MOVEMENT_X (USE_THUMBSTICKS_FOR_MOVEMENT && 1)
#define USE_THUMBSTICKS_FOR_MOVEMENT_Y (USE_THUMBSTICKS_FOR_MOVEMENT && 1)

#define USE_THUMBSTICKS_STRAFING_SPEED_POWER (USE_THUMBSTICKS_FOR_MOVEMENT_X && 1)
#define THUMBSTICK_STRAFING_SPEED_POWER 2.0f

#define SUPPORT_SMOOTH_TURNING (USE_THUMBSTICKS && 1)

#define SUPPORT_SNAP_TURNING (USE_THUMBSTICKS && 1)
#define PREFER_SNAP_TURNING (SUPPORT_SNAP_TURNING && 1)
#define SNAP_TURN_DEGREES_DEFAULT 30.0f
#define SNAP_TURN_EXTRA_FAST 90.0f

#define USE_THUMBSTICKS_FOR_TURNING 1
#define USE_THUMBSTICKS_TURNING_SPEED_POWER (USE_THUMBSTICKS_FOR_TURNING && 1)

#define SUPPORT_RUNNING_WITH_LEFT_GRIP (USE_THUMBSTICKS_FOR_MOVEMENT && 0)
#define SUPPORT_SPINNING_WITH_RIGHT_GRIP (USE_THUMBSTICKS_FOR_TURNING && 0)

#define WALKING_SPEED 0.02f
#define RUNNING_SPEED_BOOST 0.08f

#define SMOOTH_TURNING_ROTATION_SPEED 1.0f
#define ROTATION_SPEED_EXTRA 8.0f

#define CONTROLLER_THUMBSTICK_DEADZONE_X 0.05f
#define CONTROLLER_THUMBSTICK_DEADZONE_Y 0.05f

#define ROTATION_DEADZONE 0.05f

#define ADD_GRIP_POSE 1
#define DRAW_GRIP_POSE (ADD_GRIP_POSE && 1)

#define ADD_AIM_POSE 0
#define DRAW_AIM_POSE (ADD_AIM_POSE && 1)
#define DRAW_ALL_VIVE_TRACKERS (ENABLE_VIVE_TRACKERS && 1)

#define USE_BUTTONS_TRIGGERS 1

#define DRAW_VISUALIZED_SPACES 0

#define DRAW_VIEW_SPACE 1
#define DRAW_EXTRA_VIEW_CUBES 1
#define ANIMATE_VIEW_RASTER_CUBES 1

// Draw / debug
#define DRAW_LOCAL_POSES (USE_THUMBSTICKS && 0)
#define DRAW_FIRST_PERSON_POSES (USE_THUMBSTICKS && 0)
#define DRAW_THIRD_PERSON_POSES (SUPPORT_THIRD_PERSON && 0)

#define DRAW_BODY_JOINTS (ENABLE_BODY_TRACKING && 1)
#define DRAW_WAIST_DIRECTION (USE_WAIST_ORIENTATION_FOR_STICK_DIRECTION && 1)

#define LOCAL_WAIST_DIRECTION_OFFSET_Y 0.3f
#define LOCAL_WAIST_DIRECTION_OFFSET_Z 2.0f

#define GRIP_CUBE_WIDTH 0.01f
#define GRIP_CUBE_LENGTH 1.0f
#define APPLY_GRIP_OFFSET 1

#define AIM_CUBE_WIDTH 0.01f
#define AIM_CUBE_LENGTH 0.1f
#define APPLY_AIM_OFFSET 1

#define BODY_CUBE_SIZE 0.02f
#define ADD_GROUND 0

#define SUPPORT_SCREENSHOTS 0
#define ENABLE_LOCAL_DIMMING_WITH_RIGHT_GRAB (ENABLE_OPENXR_FB_LOCAL_DIMMING && 0)
#define LOG_IPD 0
#define LOG_FOV 0
#define LOG_MATRICES 0

#define ENABLE_BFI 0
#define ENABLE_ALTERNATE_EYE_RENDERING (!ENABLE_BFI && 0)
#define DEBUG_ALTERNATE_EYE_RENDERING (ENABLE_ALTERNATE_EYE_RENDERING && 0)
#define DEBUG_ALTERNATE_EYE_RENDERING_ALT (ENABLE_ALTERNATE_EYE_RENDERING && 0)

#define ENABLE_HDR_SWAPCHAIN 1
#define HDR_SHOULD_CONVERT_TO_PQ_ENCODING (ENABLE_HDR_SWAPCHAIN && 0)
#define HDR_BASE_INTENSITY 0.1f
#define HDR_INTENSITY_RANGE (10000.0f - HDR_BASE_INTENSITY)

#define TOGGLE_3RD_PERSON_AUTO_LEFT_STICK_CLICK (SUPPORT_THIRD_PERSON && 0)
#define TOGGLE_SNAP_TURNING_RIGHT_STICK_CLICK (SUPPORT_SNAP_TURNING && 1)

#define AUTO_HIDE_OTHER_BODY (SUPPORT_THIRD_PERSON && 1)

#define ENABLE_TINT 1
#define ENABLE_BLENDING (ENABLE_TINT && 1)
#define ENABLE_CONTROLLER_MOTION_BLUR (ENABLE_BLENDING && 1)
#define MAX_MOTION_BLUR_STEPS 20
#define MODULATE_BLUR_STEPS_WITH_GRIP_VALUE 1
#define MODULATE_ALPHA_BASE_WITH_GRIP_VALUE 0
#define DEBUG_LOG_ALPHA_VALUES 0
#define ALPHA_BASE 0.95f
#define ALPHA_MULT 0.2f

#endif // ENABLE_ADVANCED_FEATURES

#endif // DEFINES_H__
