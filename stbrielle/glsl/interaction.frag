#version 330 core
out vec4 fragColor;

void main() {
    // Disable inherited interaction contribution while validating the
    // standalone St. Brielle geometry/camera path.
    fragColor = vec4(0.0, 0.0, 0.0, 0.0);
}
