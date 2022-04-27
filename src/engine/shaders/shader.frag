#version 450

layout(location = 0) flat in uint inColor;

layout(location = 0) out vec4 outColor;

const vec3 colors[] = {
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 0.0)
};

void main() {
    outColor = vec4(colors[inColor], 1.0);
}
