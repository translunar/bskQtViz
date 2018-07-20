#version 120

// Input vertex data, different for all executions of this shader
attribute vec3 vertexPosition_modelSpace;
attribute vec3 vertexNormal_modelSpace;
attribute vec2 vertexUV;

// Constant values
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform vec3 lightPosition_worldSpace;

// Output data to be interpolated for each fragment
varying vec3 position_worldSpace;
varying vec3 normal_cameraSpace;
varying vec2 UV;
varying vec3 eyeDir_cameraSpace;
varying vec3 lightDir_cameraSpace;

void main()
{
    // Output position of the vertex in clip space
    gl_Position = projectionMatrix * viewMatrix * modelMatrix
                  * vec4(vertexPosition_modelSpace, 1.0);

    // Position of the vertex in world space
    position_worldSpace = vec4(modelMatrix
                               * vec4(vertexPosition_modelSpace, 1.0)).xyz;
    vec3 position_cameraSpace = vec4(viewMatrix * modelMatrix
                                     * vec4(vertexPosition_modelSpace, 1.0)).xyz;

    // Vector from vertex to camera in camera space
    // In camera space, the camera is at the origin
    vec3 vertexPosition_cameraSpace = vec4(viewMatrix * modelMatrix
                                           * vec4(vertexPosition_modelSpace, 1.0)).xyz;
    eyeDir_cameraSpace = vec3(0, 0, 0) - vertexPosition_cameraSpace;

    // Vector from vertex to the light in camera space
    vec3 lightPosition_cameraSpace = vec4(viewMatrix
                                          * vec4(lightPosition_worldSpace, 1.0)).xyz;
    lightDir_cameraSpace = lightPosition_cameraSpace - position_cameraSpace;

    // Normal of the vertex in camera space
    normal_cameraSpace = normalize(normalMatrix * vertexNormal_modelSpace);

    // UV of the vertex
    UV = vertexUV;
}
