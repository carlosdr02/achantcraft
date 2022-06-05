#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform Camera {
    mat4 viewProjection;
    mat4 inverseView;
} camera;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, 1.0, 1.0));
const float AMBIENT = 0.02;

void main() {
    vec3 norm = normalize(inNormal);

    float lightIntensity = AMBIENT + max(dot(norm, DIRECTION_TO_LIGHT), 0);

    vec3 viewPos = camera.inverseView[3].xyz;
    vec3 viewDir = normalize(viewPos - inPosition);
    vec3 reflector = reflect(-DIRECTION_TO_LIGHT, norm);
    float spec = pow(max(dot(viewDir, reflector), 0.0), 32.0);
    vec3 specular = spec * vec3(1.0);

    outColor = vec4(inColor * (lightIntensity + specular), 1.0);
}
