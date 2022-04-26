#version 450

#define AMPLITUDE 100

layout(location = 0) in float inHeight;

layout(location = 0) out vec4 outColor;

vec4 orange = vec4(1.0, 0.27, 0.0, 1.0);
vec4 purple = vec4(0.27, 0.0, 0.5, 1.0);

void main() {
    float w = (inHeight + AMPLITUDE) / (AMPLITUDE + AMPLITUDE);
    outColor = mix(orange, purple, w);
}
