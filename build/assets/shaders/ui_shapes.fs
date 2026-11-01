#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform vec2 shapeSize;
uniform float cornerRadius;
uniform float borderWidth;
uniform vec4 fillColor;
uniform vec4 borderColor;

float roundedBoxDistance(vec2 point, vec2 halfSize, float radius) {
    vec2 q = abs(point) - halfSize + vec2(radius);
    return length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - radius;
}

void main() {
    vec2 point = (fragTexCoord - vec2(0.5)) * shapeSize;
    float distanceToEdge = roundedBoxDistance(point, shapeSize * 0.5, cornerRadius);
    float aa = max(fwidth(distanceToEdge), 0.75);
    float edge = 1.0 - smoothstep(-aa, aa, distanceToEdge);
    float innerEdge = 1.0 - smoothstep(-aa, aa, distanceToEdge + borderWidth);
    vec4 color = mix(borderColor, fillColor, innerEdge);
    color.a *= edge;
    finalColor = color * fragColor;
}
