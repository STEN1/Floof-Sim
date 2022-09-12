#version 450

layout(location = 0) in vec3 pos;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform PushConstants {
    mat4 MVP;
    vec4 Color;
} pushConstants;

void main() {
    gl_Position = pushConstants.MVP * vec4(pos, 1.0);
    fragColor = pushConstants.Color;
}