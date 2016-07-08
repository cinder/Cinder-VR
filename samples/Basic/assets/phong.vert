#version 150

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelMatrix;
uniform mat3 ciNormalMatrix;

in vec4 ciPosition;
in vec3 ciNormal;

out vec4 vWorldPosition;
out vec3 vNormal;

void main( void )
{
	vWorldPosition = ciModelMatrix * ciPosition;
	vNormal = ciNormalMatrix * ciNormal;
	gl_Position = ciModelViewProjection * ciPosition;
}