#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D diffuseTexture;

void main() {

    float ambientStrength = 0.2;
    float lightStrength = 0.9;
    vec3 lightDir = normalize(vec3(0.0,1.0,2.0));
    vec3 lightColor = normalize(vec3(0.72,0.7,0.8));


    //aien
    vec3 ambient = ambientStrength * lightColor;
    //diffuse
    float diff = max(dot(lightDir, fragNormal), 0.0);
    vec3 diffuse = diff * lightColor;

    vec4 text = texture(diffuseTexture, fragUv);
    vec3 light = diffuse*ambient;

    outColor = text*vec4(light,1.0);
}