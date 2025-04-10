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

#if ENABLE_HDR

vec3 linear_to_pq(vec3 x)
{
    x = (x * (x * (x * (x * (x * 533095.76 + 47438306.2) + 29063622.1) + 575216.76) + 383.09104) + 0.000487781) /
        (x * (x * (x * (x * 66391357.4 + 81884528.2) + 4182885.1) + 10668.404) + 1.0);
    return x;
}

vec3 pq_to_linear(vec3 x)
{
	return (x * (x * (x * 85.471 + 13.947) + 0.1357) - 0.0008537) / (x * ( x * (x * -1.2149 + 3.1911) - 2.9834)+ 1.0);
}

//Passthrough for alpha, which should stay in linear space

vec4 linear_to_pq(vec4 input)
{
    vec4 output;
	
	ouput.rgb = linear_to_pq(input.rgb);
	ouput.a = input.a;
	
	return output;	
}

vec4 pq_to_linear(vec4 input)
{
    vec4 output;
	
	ouput.rgb = pq_to_linear(input.rgb);
	ouput.a = input.a;
	
	return output;
}
#endif

void main()
{
#if ENABLE_HDR
	// warning: PQ-Encoded colours won't alpha blend properly due to hardware thinking FP16 colours are linear

    FragColor = linear_to_pq(oColor);  // This is just a hack / temp fix until Valve converts to PQ inside SteamVR
	
	// Hack to test HDR, if the colours can go above 1.0 (works on PSVR 2 w/ FP16 swap chain)
	//FragColor *= 10;
	
#else
	FragColor = oColor; // sRGB/gamma encoding from linear float input colours is applied in hardware w/ full alpha blending support
#endif
}
