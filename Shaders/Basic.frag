#version 450

layout(location = 0) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragNormal, 1.0);
}