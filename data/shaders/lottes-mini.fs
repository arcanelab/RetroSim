#version 330

// Uniforms for RayLib
uniform sampler2D texture0; // Texture sampler
uniform vec2 resolution; // Screen resolution

// Parameters
uniform float LOT_SHARP = 0.5;
uniform float LOT_CURV = 1.0;
uniform float LOT_SCAN = 0.3;
uniform float shadowMask = 2.0;
uniform float maskDark = 0.5;
uniform float maskLight = 1.5;

in vec2 fragTexCoord;
in vec2 scale;
out vec4 FragColor;

#define PI 3.14159265358979323846

vec4 Mask(vec2 pos) {
    vec3 mask = vec3(maskDark, maskDark, maskDark);

    if(shadowMask == 1.0) {
        float line = maskLight;
        float odd = 0.0;

        if(fract(pos.x * 0.166666666) < 0.5)
            odd = 1.0;
        if(fract((pos.y + odd) * 0.5) < 0.5)
            line = maskDark;

        pos.x = fract(pos.x * 0.333333333);

        if(pos.x < 0.333)
            mask.b = maskLight;
        else if(pos.x < 0.666)
            mask.g = maskLight;
        else
            mask.r = maskLight;
        mask *= line;
    } else if(shadowMask == 2.0) {
        pos.x = fract(pos.x * 0.333333333);

        if(pos.x < 0.333)
            mask.b = maskLight;
        else if(pos.x < 0.666)
            mask.g = maskLight;
        else
            mask.r = maskLight;
    } else if(shadowMask == 3.0) {
        pos.x += pos.y * 3.0;
        pos.x = fract(pos.x * 0.166666666);

        if(pos.x < 0.333)
            mask.b = maskLight;
        else if(pos.x < 0.666)
            mask.g = maskLight;
        else
            mask.r = maskLight;
    } else if(shadowMask == 4.0) {
        pos.xy = floor(pos.xy * vec2(1.0, 0.5));
        pos.x += pos.y * 3.0;
        pos.x = fract(pos.x * 0.166666666);

        if(pos.x < 0.333)
            mask.b = maskLight;
        else if(pos.x < 0.666)
            mask.g = maskLight;
        else
            mask.r = maskLight;
    } else {
        mask = vec3(1.0, 1.0, 1.0);
    }

    return vec4(mask, 1.0);
}

vec2 Warp(vec2 pos) {
    float warpx, warpy;
    if(LOT_CURV == 1.0) {
        warpx = 0.03;
        warpy = 0.02;
    } else if(LOT_CURV == 2.0) {
        warpx = 0.0;
        warpy = 0.05;
    } else {
        warpx = 0.0;
        warpy = 0.0;
    }

    pos = pos * 2.0 - 1.0;
    pos *= vec2(1.0 + (pos.y * pos.y) * warpx, 1.0 + (pos.x * pos.x) * warpy);

    return pos * 0.5 + 0.5;
}

#define one 1.384615
#define two 3.230769
#define w0 0.227027
#define w1 0.316216
#define w2 0.070270

void main() {
    vec2 pos = Warp(fragTexCoord * scale);
    vec2 corn = min(pos, 1.0 - pos); // This is used to mask the rounded corners later on
    corn.x = 0.0001 / corn.x; // corners cut
    pos /= scale;

    vec2 p = pos * resolution;
    vec2 i = floor(p) + 0.5;
    vec2 f = p - i;
    p = (i + 4.0 * f * f * f) / resolution;
    p.x = pos.x;

    vec4 final = texture(texture0, p);
    vec2 ps = vec2((1.0 - LOT_SHARP) / resolution.x, 0.0);

    final += texture(texture0, p + ps * one) * w1;
    final += texture(texture0, p - ps * one) * w1;
    final += texture(texture0, p - ps * two) * w2;
    final += texture(texture0, p + ps * two) * w2;

    final = sqrt(final) / 1.25;

    vec4 clean = final;
    float l = dot(vec3(0.2), final.rgb);
    final *= LOT_SCAN * sin((pos.y * resolution.y - 0.25) * PI * 2.0) + 1.0 - LOT_SCAN;
    final *= Mask(fragTexCoord * resolution * scale);
    final = mix(final, clean, l);

    if(LOT_CURV != 0.0) {
        if(corn.y <= corn.x || corn.x < 0.0001)
            final = vec4(0.0);
    }

    FragColor = final;
}
