#version 120

// Input vertex data, different for all executions of this shader
attribute vec4 vertex;
attribute vec2 vertexUV;

// Constant values
uniform mat4 projectionMatrix;

// Output data to be interpolated for each fragment
varying vec2 UV;

void main(void)
{
    gl_Position = projectionMatrix * vertex;
    UV = vertexUV;
}
