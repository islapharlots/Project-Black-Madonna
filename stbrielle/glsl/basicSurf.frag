#version 330 core
uniform vec4 uProgramEnv[32];
out vec4 fragColor;

void main() {
    vec4 c = uProgramEnv[19];

    // Standalone St. Brielle greybox fallback:
    // if the inherited renderer does not populate the expected material color
    // slot, use a guaranteed visible neutral surface instead of black.
    if (c.a <= 0.001 || dot(abs(c.rgb), vec3(1.0)) <= 0.001) {
        c = vec4(0.22, 0.23, 0.26, 1.0);
    }

    fragColor = c;
}
