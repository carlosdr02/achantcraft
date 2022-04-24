#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
} camera;

void main() {
    gl_Position = camera.projection * camera.view * vec4(inPosition, 1.0);
}
