#version 450

layout(location = 0) in float inHeight;

layout(location = 0) out vec4 outColor;

vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
vec4 write = vec4(1.0, 1.0, 1.0, 1.0);

void main() {
    float w = (inHeight + 25) / (25 + 25);
    outColor = mix(green, write, w);
}
