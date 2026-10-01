#version 330 core
out vec4 fragColor;

void main() {
    // ST. BRIELLE standalone greybox: deterministic neutral surface.
    // Do not inherit Skin Deep program-env color state during bootstrap.
    fragColor = vec4(0.22, 0.23, 0.26, 1.0);
}
