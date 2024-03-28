# Copyright (c) 2017-2023, The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")
file(GLOB VULKAN_SHADERS "vulkan_shaders/*.glsl")

# For including compiled shaders
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(ANDROID)
    add_library(ok_cloud_streamer MODULE
        ${LOCAL_SOURCE}
        ${LOCAL_HEADERS}
        ${VULKAN_SHADERS}
        $<TARGET_OBJECTS:android_native_app_glue>)
    target_link_libraries(ok_cloud_streamer ${ANDROID_LIBRARY} ${ANDROID_LOG_LIBRARY})

    # Only for Android because it lacks a command line
    set(HELLOXR_DEFAULT_GRAPHICS_PLUGIN "OpenGLES" CACHE STRING "Which graphics plugin should be used by default for artifacts?")
    # These are the recognized options: provide them as a helper in the CMake GUI
    set_property(CACHE HELLOXR_DEFAULT_GRAPHICS_PLUGIN PROPERTY STRINGS "OpenGLES" "Vulkan")

    # Now handle the value, normally provided by Gradle
    if(HELLOXR_DEFAULT_GRAPHICS_PLUGIN STREQUAL "OpenGLES")
        message(STATUS "ok_cloud_streamer will default to OpenGL ES graphics plugin")
        target_compile_definitions(ok_cloud_streamer PRIVATE DEFAULT_GRAPHICS_PLUGIN_OPENGLES)
    elseif(HELLOXR_DEFAULT_GRAPHICS_PLUGIN STREQUAL "Vulkan")
        message(STATUS "ok_cloud_streamer will default to Vulkan graphics plugin")
        target_compile_definitions(ok_cloud_streamer PRIVATE DEFAULT_GRAPHICS_PLUGIN_VULKAN)
    endif()

else()
    add_executable(ok_cloud_streamer
        ${LOCAL_SOURCE}
        ${LOCAL_HEADERS}
        ${VULKAN_SHADERS})
endif()
set_target_properties(ok_cloud_streamer PROPERTIES FOLDER ${SAMPLES_FOLDER})

#function(set_cxxstd target cpp_version)
#    set_property(TARGET ${target} PROPERTY CXX_STANDARD ${cpp_version})
#    set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED ON)
#endfunction()

#set_cxxstd(ok_cloud_streamer 17)

source_group("Headers" FILES ${LOCAL_HEADERS})
source_group("Shaders" FILES ${VULKAN_SHADERS})

compile_glsl(run_ok_cloud_streamer_glsl_compiles ${VULKAN_SHADERS})

add_dependencies(ok_cloud_streamer
    generate_openxr_header
    run_ok_cloud_streamer_glsl_compiles
)

target_include_directories(ok_cloud_streamer
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/common
    ${PROJECT_SOURCE_DIR}/src/tests/ok_cloud_streamer/vulkan_shaders

    # for helper headers
    ${PROJECT_SOURCE_DIR}/external/include
)

if(GLSLANG_VALIDATOR AND NOT GLSL_COMPILER)
    target_compile_definitions(ok_cloud_streamer PRIVATE USE_GLSLANGVALIDATOR)
endif()

if(Vulkan_FOUND)
    target_include_directories(ok_cloud_streamer
        PRIVATE
        ${Vulkan_INCLUDE_DIRS}
    )
endif()

option(ENABLE_META_OPENXR_SDK "Enable Meta OpenXR SDK" ON)

if(ENABLE_META_OPENXR_SDK)
    
    target_include_directories(ok_cloud_streamer PUBLIC $ENV{OCULUS_OPENXR_MOBILE_SDK}/OpenXR/Include)
    
    if(ANDROID)
        add_library(meta_openxr_loader SHARED IMPORTED)

        set_target_properties(meta_openxr_loader
                PROPERTIES
                IMPORTED_LOCATION $ENV{OCULUS_OPENXR_MOBILE_SDK}/OpenXR/Libs/Android/arm64-v8a/Release/libopenxr_loader.so
                )

        #target_link_libraries(ok_cloud_streamer meta_openxr_loader)
    endif()
    
else()

endif()

target_link_libraries(ok_cloud_streamer OpenXR::openxr_loader)

if(TARGET openxr-gfxwrapper)
    target_link_libraries(ok_cloud_streamer openxr-gfxwrapper)
endif()
if(WIN32)
    if(MSVC)
        target_compile_definitions(ok_cloud_streamer PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_options(ok_cloud_streamer PRIVATE /Zc:wchar_t /Zc:forScope /W4)
        
        if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            # If actually msvc and not clang-cl
            target_compile_options(openxr_c_compile_test PRIVATE /WX)
        endif()
    endif()
    target_link_libraries(ok_cloud_streamer ole32)
    if(MSVC)
        # Right now can't build this on MinGW because of directxcolors, directxmath, etc.
        target_link_libraries(ok_cloud_streamer d3d11 d3d12 d3dcompiler dxgi)
    endif()
endif()

if(Vulkan_LIBRARY)
    target_link_libraries(ok_cloud_streamer ${Vulkan_LIBRARY})
endif()

target_include_directories(ok_cloud_streamer PUBLIC ${PROJECT_SOURCE_DIR}/src/external/glm)
target_include_directories(ok_cloud_streamer PUBLIC ${PROJECT_SOURCE_DIR}/src/external/SDL/include)

option(ENABLE_CLOUDXR_SDK "Enable CloudXR SDK" OFF)

if(ENABLE_CLOUDXR_SDK)
    add_library(CloudXRClient SHARED IMPORTED)
    target_include_directories(ok_cloud_streamer PRIVATE ${CLOUDXR_ROOT}/include)
    target_compile_definitions(ok_cloud_streamer PRIVATE ENABLE_CLOUDXR=1)

    target_compile_options(ok_cloud_streamer PRIVATE -Wno-unused-parameter)
    target_sources(ok_cloud_streamer PRIVATE ${CLOUDXR_ROOT}/include/CloudXRController.cpp)

    target_compile_definitions(ok_cloud_streamer PRIVATE ENABLE_CLOUDXR_LOGGING_STUB=1)
    
    target_include_directories(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/jsoncpp)

    target_sources(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/jsoncpp/json_reader.cpp)
    target_sources(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/jsoncpp/json_writer.cpp)
    target_sources(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/jsoncpp/json_value.cpp)
    
    target_include_directories(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR})

    target_sources(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/GLMPose.cpp)
    target_sources(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/OKDigitalButton.cpp)
    target_sources(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/OKAnalogAxis.cpp)
    target_sources(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/OKPlayerState.cpp)
    target_sources(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/OKController.cpp)
    target_sources(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/OKConfig.cpp)
    target_sources(ok_cloud_streamer PRIVATE ${OK_CLOUD_STREAMER_DIR}/OKCloudClient.cpp)

    target_compile_definitions(hello_xr PRIVATE USE_ADVANCED_IMAGE_READER_DECODER=0)
    target_compile_definitions(hello_xr PRIVATE USE_OK_CALLBACKS=1)

    if(ANDROID)
        set_target_properties(CloudXRClient
                PROPERTIES
                IMPORTED_LOCATION ${CLOUDXR_ROOT}/jni/${ANDROID_ABI}/libCloudXRClient.so
                )

        target_link_libraries(ok_cloud_streamer CloudXRClient)
    else()
        message(FATAL_ERROR "Missing CloudXR")
    endif()
else()
    target_compile_definitions(ok_cloud_streamer PRIVATE ENABLE_CLOUDXR=0)
endif()


if(NOT ANDROID)
    install(TARGETS ok_cloud_streamer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT ok_cloud_streamer)
    if(NOT WIN32)
        install(FILES ok_cloud_streamer.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/ COMPONENT ManPages)
    endif()
endif()
        
