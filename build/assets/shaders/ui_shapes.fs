#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform vec2 shapeSize;
uniform vec4 chamferSizes;
uniform int pointSide;
uniform float pointSize;
uniform float shapeSkew;
uniform int fadeDirection;
uniform float fadeStart;
uniform float fadeEnd;
uniform float lineWidth;
uniform float glowWidth;
uniform float shadowBlur;
uniform vec2 shadowOffset;
uniform vec4 fillColor;
uniform vec4 fillEndColor;
uniform vec4 topLineColor;
uniform vec4 bottomLineColor;
uniform vec4 glowColor;
uniform vec4 shadowColor;

float shapeDistance(vec2 point) {
    float halfWidth = shapeSize.x * 0.5;
    float halfHeight = shapeSize.y * 0.5;
    vec2 vertices[9];
    vertices[0] = vec2(-halfWidth, -halfHeight + chamferSizes.x);
    vertices[1] = vec2(-halfWidth + chamferSizes.x, -halfHeight);
    vertices[2] = vec2(halfWidth - chamferSizes.y, -halfHeight);
    vertices[3] = vec2(halfWidth, -halfHeight + chamferSizes.y);
    vertices[4] = vec2(halfWidth, halfHeight - chamferSizes.z);
    vertices[5] = vec2(halfWidth - chamferSizes.z, halfHeight);
    vertices[6] = vec2(-halfWidth + chamferSizes.w, halfHeight);
    vertices[7] = vec2(-halfWidth, halfHeight - chamferSizes.w);

    int count = 8;
    if (pointSide == 1) {
        for (int i = 8; i > 0; --i) vertices[i] = vertices[i - 1];
        vertices[0] = vec2(-halfWidth - pointSize, 0.0);
        count = 9;
    } else if (pointSide == 2) {
        for (int i = 8; i > 4; --i) vertices[i] = vertices[i - 1];
        vertices[4] = vec2(halfWidth + pointSize, 0.0);
        count = 9;
    }

    float distanceSquared = 1e20;
    bool inside = false;
    for (int i = 0; i < 9; ++i) {
        if (i >= count) break;
        vec2 a = vertices[i];
        vec2 b = vertices[(i + 1) % count];
        vec2 edge = b - a;
        float edgeLengthSquared = max(dot(edge, edge), 1e-6);
        float projection = clamp(dot(point - a, edge) / edgeLengthSquared, 0.0, 1.0);
        distanceSquared = min(distanceSquared, dot(point - (a + projection * edge),
                                                    point - (a + projection * edge)));
        if ((a.y > point.y) != (b.y > point.y) &&
            point.x < (b.x - a.x) * (point.y - a.y) / (b.y - a.y) + a.x)
            inside = !inside;
    }
    float distanceToEdge = sqrt(distanceSquared);
    return inside ? -distanceToEdge : distanceToEdge;
}

void main() {
    vec2 point = (fragTexCoord - vec2(0.5)) * shapeSize;
    point.x += shapeSkew * point.y;
    float distanceToEdge = shapeDistance(point);
    float aa = max(fwidth(distanceToEdge), 0.5);
    float silhouette = 1.0 - smoothstep(-aa, aa, distanceToEdge);

    float fade = 1.0;
    if (fadeDirection == 1)
        fade = 1.0 - smoothstep(fadeStart, fadeEnd, fragTexCoord.x);
    else if (fadeDirection == 2)
        fade = smoothstep(fadeStart, fadeEnd, fragTexCoord.x);

    vec4 color = fillEndColor.a > 0.0
        ? mix(fillColor, fillEndColor, fragTexCoord.x)
        : fillColor;
    float frame = distanceToEdge <= 0.0
        ? 1.0 - smoothstep(lineWidth - aa, lineWidth + aa, -distanceToEdge)
        : 0.0;
    float topEdge = 1.0 - smoothstep(lineWidth, lineWidth + aa,
                                     abs(point.y + shapeSize.y * 0.5));
    float bottomEdge = 1.0 - smoothstep(lineWidth, lineWidth + aa,
                                        abs(point.y - shapeSize.y * 0.5));
    vec4 edgeColor = point.y < 0.0 ? topLineColor : bottomLineColor;
    color = mix(color, edgeColor, max(frame, max(topEdge, bottomEdge)));

    if (glowWidth > 0.0) {
        float glow = exp(-abs(distanceToEdge) / max(glowWidth, 0.001));
        color.rgb = mix(color.rgb, glowColor.rgb, glow * glowColor.a);
        color.a = max(color.a, glow * glowColor.a);
    }

    vec4 shadowLayer = vec4(0.0);
    if (shadowColor.a > 0.0) {
        float shadowDistance = shapeDistance(point - shadowOffset);
        float shadow = 1.0 - smoothstep(0.0, max(shadowBlur, aa), shadowDistance);
        shadowLayer = vec4(shadowColor.rgb, shadowColor.a * shadow * fade);
    }

    color.a *= silhouette * fade;
    finalColor = color * fragColor + shadowLayer * (1.0 - color.a) * fragColor;
}
