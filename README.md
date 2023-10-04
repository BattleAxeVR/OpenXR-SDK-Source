# OpenXR™ Software Development Kit (SDK) Sources Project

<!--
Copyright (c) 2017-2023, The Khronos Group Inc.

SPDX-License-Identifier: CC-BY-4.0
-->

BRANCH: QUEST CLIENT USING OCULUS INSTRUCTIONS FROM HERE:

https://developer.oculus.com/documentation/native/android/mobile-build-run-hello-xr-app/

QUEST PRO EYE TRACKING sample on PC VR via Link / AirLink (enable Dev mode + eye-tracking sliders in Oculus PC app):


NB This same exact code should work on Quest Pro standalone mode, but I didn't add the request permissions Java code for eye-tracking (which needs to be accepted for it to work). There is no java code in this project so it will probably have to be done via JNI / Android Native somehow. I'll try to add that soon.

Body tracking extension works fine on Standalone but gives a runtime error on PC via Link.


![image](https://user-images.githubusercontent.com/11604039/200270625-e627a78b-5d4e-409f-80da-79bebe81bb63.png)

I also implemented waist-oriented locomotion, where available. This works on Quest 1, 2, and Pro, both on Android and Link/AirLink on PC. 

I posted a hello_xr.exe test app and an OpenGL ES / Vulkan pre-built APK for SideLoading / ADB pushing to try it out quickly.

Just added support for XR_METAX1_simultaneous_hands_controllers_management, which allows Quest Pro and Quest 3 to use hands and controllers at the same time (many use cases for this, such as single 2-handed weapon games w/ 1 free hand to do other stuff with.

It works on standalone (you need experimental features added in manifest as well as the adb command to enable experimental features) as well as PC VR over Link with developer account and public alpha build of Oculus Home.

https://github.com/BattleAxeVR/OpenXR-SDK-Source/assets/11604039/d1cb467e-bfeb-42ad-81f3-162baa7d94f9

