#version 110

varying vec2 texCoord;
uniform sampler2D source;
uniform float trg_x;
uniform float trg_y;
uniform float trg_w;
uniform float trg_h;
uniform float scale;

// Emulated input resolution.

// float scale = 3.0;

vec2 res = vec2(470.0 * scale, 256.0 * scale);

// Hardness of scanline.
//  -8.0 = soft
// -16.0 = medium
float hardScan = -8.0;

// Hardness of pixels in scanline.
// -2.0 = soft
// -4.0 = hard
float hardPix = -2.0;

// Display warp.
// 0.0 = none
// 1.0/8.0 = extreme
// vec2 warp=vec2(1.0/64.0,1.0/48.0); 
vec2 warp = vec2(0.0, 0.0); 

// Amount of shadow mask.
float maskStrength = 0.1 * scale / 2.0;
float maskDark = 1.0 - maskStrength;
float maskLight = 1.0 + maskStrength;

//------------------------------------------------------------------------

// sRGB to Linear.
// Assuing using sRGB typed textures this should not be needed.
float ToLinear1(float c) {
    return (c <= 0.04045) ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}
vec3 ToLinear(vec3 c) {
    return vec3(ToLinear1(c.r), ToLinear1(c.g), ToLinear1(c.b));
}

// Linear to sRGB.
// Assuing using sRGB typed textures this should not be needed.
float ToSrgb1(float c) {
    return (c < 0.0031308 ? c * 12.92 : 1.055 * pow(c, 0.41666) - 0.055);
}
vec3 ToSrgb(vec3 c) {
    return vec3(ToSrgb1(c.r), ToSrgb1(c.g), ToSrgb1(c.b));
}

// Nearest emulated sample given floating point position and texel offset.
// Also zero's off screen.
vec3 Fetch(vec2 pos, vec2 off) {
    pos = (floor(pos * res + off) + vec2(0.5, 0.5)) / res;
    return ToLinear(1.2 * texture2D(source, pos.xy, -16.0).rgb);
}

// Distance in emulated pixels to nearest texel.
vec2 Dist(vec2 pos) {
    pos = pos * res;
    return -((pos - floor(pos)) - vec2(0.5));
}

// 1D Gaussian.
float Gaus(float pos, float scale) {
    return exp2(scale * pos * pos);
}

// 3-tap Gaussian filter along horz line.
vec3 Horz3(vec2 pos, float off) {
    vec3 b = Fetch(pos, vec2(-1.0, off));
    vec3 c = Fetch(pos, vec2(0.0, off));
    vec3 d = Fetch(pos, vec2(1.0, off));
    float dst = Dist(pos).x;
    // Convert distance to weight.
    float scale = hardPix;
    float wb = Gaus(dst - 1.0, scale);
    float wc = Gaus(dst + 0.0, scale);
    float wd = Gaus(dst + 1.0, scale);
    // Return filtered sample.
    return (b * wb + c * wc + d * wd) / (wb + wc + wd);
}

// 5-tap Gaussian filter along horz line.
vec3 Horz5(vec2 pos, float off) {
    vec3 a = Fetch(pos, vec2(-2.0, off));
    vec3 b = Fetch(pos, vec2(-1.0, off));
    vec3 c = Fetch(pos, vec2(0.0, off));
    vec3 d = Fetch(pos, vec2(1.0, off));
    vec3 e = Fetch(pos, vec2(2.0, off));
    float dst = Dist(pos).x;
    // Convert distance to weight.
    float scale = hardPix;
    float wa = Gaus(dst - 2.0, scale);
    float wb = Gaus(dst - 1.0, scale);
    float wc = Gaus(dst + 0.0, scale);
    float wd = Gaus(dst + 1.0, scale);
    float we = Gaus(dst + 2.0, scale);
    // Return filtered sample.
    return (a * wa + b * wb + c * wc + d * wd + e * we) / (wa + wb + wc + wd + we);
}

// Return scanline weight.
float Scan(vec2 pos, float off) {
    float dst = Dist(pos).y;
    return Gaus(dst + off, hardScan);
}

// Allow nearest three lines to effect pixel.
// vec3 Tri(vec2 pos){
//     vec3 a=Horz3(pos,-1.0);
//     vec3 b=Horz5(pos, 0.0);
//     vec3 c=Horz3(pos, 1.0);
//     float wa=Scan(pos,-1.0);
//     float wb=Scan(pos, 0.0);
//     float wc=Scan(pos, 1.0);
//     return a*wa+b*wb+c*wc;
// }

float sampleOffset = 1.0;

vec3 Tri(vec2 pos) {
    vec3 a = Horz3(pos, -sampleOffset);
    vec3 b = Horz5(pos, 0.0);
    vec3 c = Horz3(pos, sampleOffset);
    float wa = Scan(pos, -sampleOffset);
    float wb = Scan(pos, 0.0);
    float wc = Scan(pos, sampleOffset);
    return a * wa + b * wb + c * wc;
}

// Distortion of scanlines, and end of screen alpha.
vec2 Warp(vec2 pos) {
    pos = pos * 2.0 - 1.0;
    pos *= vec2(1.0 + (pos.y * pos.y) * warp.x, 1.0 + (pos.x * pos.x) * warp.y);
    return pos * 0.5 + 0.5;
}

// Shadow mask.
vec3 Mask(vec2 pos) {
    pos.x += pos.y * 3.0;
    vec3 mask = vec3(maskDark, maskDark, maskDark);
    pos.x = fract(pos.x / 6.0);
    if(pos.x < 0.333)
        mask.r = maskLight;
    else if(pos.x < 0.666)
        mask.g = maskLight;
    else
        mask.b = maskLight;
    return mask;
}

vec4 GetColor(vec2 pos) {
    vec4 l = texture2D(source, vec2(pos.x - 1.0 / res.x, pos.y));
    vec4 r = texture2D(source, vec2(pos.x + 1.0 / res.x, pos.y));
    // vec4 u = texture2D(source, vec2(pos.x, pos.y - 1.0 / res.y));
    // vec4 d = texture2D(source, vec2(pos.x, pos.y + 1.0 / res.y));
    vec4 c = texture2D(source, pos);

    // if scale == 1, then bias = 0, otherwise bias = 0.3
    float isScaleOne = step(0.9999, scale) * step(scale, 1.0001);
    float bias = (1.0 - isScaleOne) * 0.3;

    // float bias = 0.3; // how much effect the horizontal neighbor pixels have
    // blend between the current and the horizontal neighbors
    vec4 result = (l * bias + r * bias + c) / (1.0 + bias * 2.0); 

    return result;
}

void main() {
    hardScan = -12.0;
    //maskDark=maskLight;
    vec2 start = gl_FragCoord.xy - vec2(trg_x, trg_y);
    start.y = trg_h - start.y;

    // vec2 pos=(floor(pos*res+off)+vec2(0.5,0.5))/res;
    vec2 pos = Warp(start / vec2(trg_w, trg_h));
    // gl_FragColor.rgb=Tri(pos)*Mask(gl_FragCoord.xy);
    // gl_FragColor.rgb=texture2D(source,pos.xy, -16.0).rgb * Mask(gl_FragCoord.xy);

    vec2 scaledPos = vec2(gl_FragCoord.x * scale, gl_FragCoord.y * 1.0);
    // vec2 scaledPos = gl_FragCoord.xy * scale;

    gl_FragColor = vec4(GetColor(pos).rgb * Mask(scaledPos).bgr, 1.0);
}
