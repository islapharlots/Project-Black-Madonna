#version 330 core
in vec3 vNormal;
in float vDepth;

out vec4 fragColor;

void main() {
    vec3 n = abs(normalize(vNormal));

    // High-contrast St. Brielle greybox palette by surface orientation:
    // X walls = warmer dark stone, Y walls = cooler stone,
    // horizontal surfaces = lighter slate.
    vec3 xFace = vec3(0.22, 0.20, 0.21);
    vec3 yFace = vec3(0.18, 0.21, 0.23);
    vec3 zFace = vec3(0.30, 0.31, 0.33);

    vec3 color = xFace * n.x + yFace * n.y + zFace * n.z;

    // Mild depth falloff gives the room readable perspective without relying
    // on the missing Skin Deep lighting/post-processing package.
    float depthShade = clamp(1.0 - vDepth / 1800.0, 0.72, 1.0);
    color *= depthShade;

    fragColor = vec4(color, 1.0);
}
