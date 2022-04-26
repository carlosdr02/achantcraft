#version 450

layout(location = 0) in vec4 inPosition;

layout(location = 0) out float outHeight;

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
} camera;

void main() {
    gl_Position = camera.projection * camera.view * inPosition;
    outHeight = inPosition.y;
}
