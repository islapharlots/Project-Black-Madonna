#version 330 core
out vec4 fragColor;

void main() {
    // ST. BRIELLE greybox lighting pass.
    // The previous bootstrap returned zero here, which made otherwise-valid
    // world/light interactions render completely black.
    fragColor = vec4(0.24, 0.27, 0.33, 1.0);
}
