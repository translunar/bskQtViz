#version 120

// Interpolated values from the vertex shader
varying vec3 position_worldSpace;
varying vec3 normal_cameraSpace;
varying vec2 UV;
varying vec3 eyeDir_cameraSpace;
varying vec3 lightDir_cameraSpace;

// Constant values
uniform sampler2D textureId;
uniform vec3 lightPosition_worldSpace;
uniform vec3 lightIntensity;

uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;
uniform float shininess;

void main()
{
    // Normal of the computed fragment in camera space
    vec3 normal = normalize(normal_cameraSpace);
    // Direction of the light (From the fragment to the light)
    vec3 lightDir = normalize(lightDir_cameraSpace);
    // Cosine of the angle between the normal and the light direction
    // clamped above 0
    float cosTheta = clamp(dot(normal, lightDir), 0, 1);
    vec4 cosThetaVec = vec4(cosTheta, cosTheta, cosTheta, 1.0);

    // Eye vector (toward the camera) in camera space
    vec3 eyeDir = normalize(eyeDir_cameraSpace);
    // Direction in which the the triangle reflects the light
    vec3 r = reflect(-lightDir, normal);
    // Cosine of the angle between the eye and reflect vectors
    float cosAlpha = clamp(dot(eyeDir, r), 0, 1);
    float powCosAlpha = pow(cosAlpha, shininess);
    vec4 cosAlphaVec = vec4(powCosAlpha, powCosAlpha, powCosAlpha, 1.0);

    vec4 textureColor = texture2D(textureId, UV);

    gl_FragColor = ambientColor * textureColor
                + diffuseColor * textureColor * vec4(lightIntensity, 1.0) * cosThetaVec
                + specularColor * vec4(lightIntensity, 1.0) * cosAlphaVec;
}
