#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

// Hardcoded light and material properties
const vec3 lightPos = vec3(0.0, -4.0, -2.0);
const vec3 lightColor = vec3(1.0);        // White light
const float ambientStrength = 0.3;      // Ambient light

void main() {
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);

    // Ambient
    vec3 ambient = ambientStrength * fragColor;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse);
    outColor = vec4(result, 1.0);
}