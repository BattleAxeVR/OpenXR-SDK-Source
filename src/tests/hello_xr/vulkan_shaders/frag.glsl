// Copyright (c) 2017-2025 The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#pragma fragment

layout (location = 0) in vec4 oColor;

layout (location = 0) out vec4 FragColor;

#define ENABLE_HDR 0
// Only this part was written by BattleAxeVR:

//--------------------------------------------------------------------------------------
// Copyright (c) 2025 BattleAxeVR. All rights reserved.
//--------------------------------------------------------------------------------------

// Author: Bela Kampis
// Date: April 10, 2025

// A reference implementation of PQ (Perceptual Quantizer) <-> Linear conversion, used in HDR10 etc.
// Source: https://pub.smpte.org/latest/st2084/st2084-2014.pdf

// ShaderToy Link: https://www.shadertoy.com/view/wXjGRV

#if ENABLE_HDR
const float m1 = 0.1593017578125;
const float m2 = 78.84375;
const float c1 = 0.8359375;
const float c2 = 18.8515625;
const float c3 = 18.6875;

const float error_gain = 100.0;

float linear_to_pq(float L)
{
    float L_m1 = pow(L, m1);
    float numerator = c1 + c2 * L_m1;
    float denominator = 1.0 + c3 * L_m1;

    float N = pow(numerator / denominator, m2);
    return N;
}

vec3 linear_to_pq(vec3 L)
{
    vec3 N = vec3(linear_to_pq(L.r), linear_to_pq(L.g), linear_to_pq(L.b));
    return N;
}

vec4 linear_to_pq(vec4 L)
{
    vec4 N;
    N.a = 1.0; // Alpha passthrough stays linear
    
    N.rgb = linear_to_pq(L.rgb);
    
    return N;
}

float pq_to_linear(float N)
{
    float N_1_m2 = pow(N, 1.0/m2);
    float max_arg = (N_1_m2 - c1);
    float numerator = max(max_arg, 0.0);
    float denominator = c2 - c3 * N_1_m2;

    float L = pow(numerator / denominator, 1.0/m1);
    return L;
}

vec3 pq_to_linear(vec3 N)
{
    vec3 L = vec3(pq_to_linear(N.r), pq_to_linear(N.g), pq_to_linear(N.b));
    return L;
}

vec4 pq_to_linear(vec4 N)
{
    vec4 L;
    L.a = 1.0; // Alpha passthrough stays linear
    
    L.rgb = pq_to_linear(N.rgb);
    return L;
}
#endif

void main()
{
#if ENABLE_HDR
	FragColor = linear_to_pq(oColor);
    //FragColor = pq_to_linear(oColor);
#else
	FragColor = oColor; // sRGB/gamma encoding from linear float input colours is applied in hardware w/ full alpha blending support
    //FragColor.b = 0.0;
#endif
}
