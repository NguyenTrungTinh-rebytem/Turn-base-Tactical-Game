#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 texelSize;
uniform float blurRadius;
uniform int blurQuality;

void main() {
    vec2 stepSize = texelSize * blurRadius;
    vec4 color = texture(texture0, fragTexCoord) * 0.28;
    color += texture(texture0, fragTexCoord + vec2(stepSize.x, 0.0)) * 0.16;
    color += texture(texture0, fragTexCoord - vec2(stepSize.x, 0.0)) * 0.16;
    color += texture(texture0, fragTexCoord + vec2(0.0, stepSize.y)) * 0.16;
    color += texture(texture0, fragTexCoord - vec2(0.0, stepSize.y)) * 0.16;
    if (blurQuality == 1) {
        color += texture(texture0, fragTexCoord + stepSize) * 0.02;
        color += texture(texture0, fragTexCoord - stepSize) * 0.02;
        color += texture(texture0, fragTexCoord + vec2(stepSize.x, -stepSize.y)) * 0.02;
        color += texture(texture0, fragTexCoord + vec2(-stepSize.x, stepSize.y)) * 0.02;
    }
    finalColor = color * fragColor;
}
