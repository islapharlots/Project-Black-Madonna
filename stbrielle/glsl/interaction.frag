#version 330 core
uniform vec4 uProgramEnv[32];
out vec4 fragColor;

void main() {
    // Minimal visible light interaction for the standalone greybox build.
    // The full St. Brielle physically-lit interaction shader will replace
    // this after the engine/content separation is complete.
    vec3 lightColor = uProgramEnv[3].rgb;

    if (dot(abs(lightColor), vec3(1.0)) <= 0.001) {
        lightColor = vec3(0.18);
    }

    fragColor = vec4(lightColor * 0.65, 1.0);
}
