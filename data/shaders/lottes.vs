// PUBLIC DOMAIN CRT STYLED SCAN-LINE SHADER
//
//   by Timothy Lottes
//
// This is more along the style of a really good CGA arcade monitor.
// With RGB inputs instead of NTSC.
// The shadow mask example has the mask rotated 90 degrees for less chromatic aberration.
//
// Left it unoptimized to show the theory behind the algorithm.
//
// It is an example what I personally would want as a display option for pixel art games.
// Please take and use, change, or whatever.

// Parameter lines go here:
// #pragma parameter hardScan "hardScan" -8.0 -20.0 0.0 1.0
// #pragma parameter hardPix "hardPix" -3.0 -20.0 0.0 1.0
// #pragma parameter warpX "warpX" 0.031 0.0 0.125 0.01
// #pragma parameter warpY "warpY" 0.041 0.0 0.125 0.01
// #pragma parameter maskDark "maskDark" 0.5 0.0 2.0 0.1
// #pragma parameter maskLight "maskLight" 1.5 0.0 2.0 0.1
// #pragma parameter scaleInLinearGamma "scaleInLinearGamma" 1.0 0.0 1.0 1.0
// #pragma parameter shadowMask "shadowMask" 3.0 0.0 4.0 1.0
// #pragma parameter brightBoost "brightness boost" 1.0 0.0 2.0 0.05
// #pragma parameter hardBloomPix "bloom-x soft" -1.5 -2.0 -0.5 0.1
// #pragma parameter hardBloomScan "bloom-y soft" -2.0 -4.0 -1.0 0.1
// #pragma parameter bloomAmount "bloom ammount" 0.15 0.0 1.0 0.05
// #pragma parameter shape "filter kernel shape" 2.0 0.0 10.0 0.05

#version 330

in vec4 vertexPosition;
in vec4 COLOR;
in vec4 vertexTexCoord;
out vec4 COL0;
out vec4 fragTexCoord;

uniform mat4 mvp;
uniform int FrameDirection;
uniform int FrameCount;
uniform vec2 OutputSize;
uniform vec2 TextureSize;
uniform vec2 InputSize;

// vertex compatibility #defines
// #define vTexCoord TEX0.xy
// #define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
// #define outsize vec4(OutputSize, 1.0 / OutputSize)

void main() {
    gl_Position = mvp * vertexPosition;
    fragTexCoord.xy = vertexTexCoord.xy;
}
