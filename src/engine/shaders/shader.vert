#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in uint inColor;
layout(location = 2) in vec4 inNormal;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

layout(set = 0, binding = 0) uniform Camera {
    mat4 viewProjection;
    mat4 projection;
} camera;

const vec3 colors[] = {
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 1.0, 1.0),
    vec3(0.5, 0.5, 0.5),
    vec3(0.0, 1.0, 0.0),
    vec3(1.0, 1.0, 0.25)
};

void main() {
    outColor = colors[inColor];
    outNormal = inNormal.xyz;
    gl_Position = camera.viewProjection * vec4(inPosition, 1.0);
}
