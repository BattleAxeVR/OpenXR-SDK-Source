// Copyright (c) 2017-2025 The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace Geometry {

struct Vertex {
    XrVector3f Position;
    XrVector3f Color;
};

constexpr XrVector3f Red{1, 0, 0};
constexpr XrVector3f DarkRed{0.25f, 0, 0};
constexpr XrVector3f Green{0, 1, 0};
constexpr XrVector3f DarkGreen{0, 0.25f, 0};
constexpr XrVector3f Blue{0, 0, 1};
constexpr XrVector3f DarkBlue{0, 0, 0.25f};
constexpr XrVector3f White{ 1.0f, 1.0f, 1.0f };
constexpr XrVector3f DarkGrey{ 0.15f, 0.15f, 0.15f };
constexpr XrVector3f Black{ 0.0f, 0.0f, 0.0f };

// Vertices for a 1x1x1 meter cube. (Left/Right, Top/Bottom, Front/Back)
constexpr XrVector3f LBB{-0.5f, -0.5f, -0.5f};
constexpr XrVector3f LBF{-0.5f, -0.5f, 0.5f};
constexpr XrVector3f LTB{-0.5f, 0.5f, -0.5f};
constexpr XrVector3f LTF{-0.5f, 0.5f, 0.5f};
constexpr XrVector3f RBB{0.5f, -0.5f, -0.5f};
constexpr XrVector3f RBF{0.5f, -0.5f, 0.5f};
constexpr XrVector3f RTB{0.5f, 0.5f, -0.5f};
constexpr XrVector3f RTF{0.5f, 0.5f, 0.5f};


#define USE_GRADIENT_CUBES 1
#define USE_WHITE_CUBES 0

#define CUBE_SIDE(V1, V2, V3, V4, V5, V6, COLOR) {V1, COLOR}, {V2, COLOR}, {V3, COLOR}, {V4, COLOR}, {V5, COLOR}, {V6, COLOR},
#define CUBE_SIDE_GADRIENT(V1, V2, V3, V4, V5, V6, C1, C2, C3, C4) {V1, C1}, {V2, C2}, {V3, C3}, {V4, C1}, {V5, C3}, {V6, C4},

#if USE_GRADIENT_CUBES
constexpr Vertex c_cubeVertices[] = {
    CUBE_SIDE_GADRIENT(LTB, LBF, LBB, LTB, LTF, LBF, Black, Black, Black, Black)   // -X
    CUBE_SIDE_GADRIENT(RTB, RBB, RBF, RTB, RBF, RTF, Red, Red, Black, Black)               // +X
    CUBE_SIDE_GADRIENT(LBB, LBF, RBF, LBB, RBF, RBB, Black, Black, Black, Black)   // -Y
    CUBE_SIDE_GADRIENT(LTB, RTB, RTF, LTB, RTF, LTF, Green, Green, Black, Black)               // +Y
    CUBE_SIDE_GADRIENT(LBB, RBB, RTB, LBB, RTB, LTB, Black, Black, Black, Black)   // -Z
    CUBE_SIDE_GADRIENT(LBF, LTF, RTF, LBF, RTF, RBF, Blue, Blue, Black, Black)               // +Z
};
#elif USE_WHITE_CUBES
constexpr Vertex c_cubeVertices[] = {
    CUBE_SIDE(LTB, LBF, LBB, LTB, LTF, LBF, White)  // -X
    CUBE_SIDE(RTB, RBB, RBF, RTB, RBF, RTF, White)  // +X
    CUBE_SIDE(LBB, LBF, RBF, LBB, RBF, RBB, White)  // -Y
    CUBE_SIDE(LTB, RTB, RTF, LTB, RTF, LTF, White)  // +Y
    CUBE_SIDE(LBB, RBB, RTB, LBB, RTB, LTB, White)  // -Z
    CUBE_SIDE(LBF, LTF, RTF, LBF, RTF, RBF, White)  // +Z
};
#else
constexpr Vertex c_cubeVertices[] = {
    CUBE_SIDE(LTB, LBF, LBB, LTB, LTF, LBF, DarkRed)    // -X
    CUBE_SIDE(RTB, RBB, RBF, RTB, RBF, RTF, Red)        // +X
    CUBE_SIDE(LBB, LBF, RBF, LBB, RBF, RBB, DarkGreen)  // -Y
    CUBE_SIDE(LTB, RTB, RTF, LTB, RTF, LTF, Green)      // +Y
    CUBE_SIDE(LBB, RBB, RTB, LBB, RTB, LTB, DarkBlue)   // -Z
    CUBE_SIDE(LBF, LTF, RTF, LBF, RTF, RBF, Blue)       // +Z
};
#endif

// Winding order is clockwise. Each side uses a different color.
constexpr unsigned short c_cubeIndices[] = {
    0,  1,  2,  3,  4,  5,   // -X
    6,  7,  8,  9,  10, 11,  // +X
    12, 13, 14, 15, 16, 17,  // -Y
    18, 19, 20, 21, 22, 23,  // +Y
    24, 25, 26, 27, 28, 29,  // -Z
    30, 31, 32, 33, 34, 35,  // +Z
};

}  // namespace Geometry
