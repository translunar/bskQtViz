#version 120

// Interpolated values from the vertex shader
varying vec2 UV;

// Constant values
uniform sampler2D textureId;
uniform vec4 color;

void main(void)
{
    vec4 textureColor = texture2D(textureId, UV);
    gl_FragColor = color * textureColor;
}
