#version 330 core
layout(location = 0) in vec4 aPosition;
layout(location = 2) in vec3 aNormal;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;

out vec3 vNormal;
out float vDepth;

void main() {
    vec4 viewPos = uModelViewMatrix * aPosition;
    gl_Position = uProjectionMatrix * viewPos;

    // World brushes are unscaled in the Record Lab, so this is sufficient for
    // a stable diagnostic normal. We intentionally keep this shader simple.
    vNormal = normalize(aNormal);
    vDepth = max(0.0, -viewPos.z);
}
