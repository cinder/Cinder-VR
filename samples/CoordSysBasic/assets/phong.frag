#version 150

uniform vec4 uLightPosition;
uniform vec4 uSkyDirection;

in vec4 vWorldPosition;
in vec3 vNormal;

out vec4 oColor;

void main( void )
{		
	const vec3 matDiffuse = vec3( 1 );
	const vec3 lightDiffuse = vec3( 1 );

	vec3 V = vWorldPosition.xyz;
	vec3 N = normalize( vNormal );
	vec3 L = normalize( uLightPosition.xyz - V );
	vec3 R = normalize( -reflect( L, N ) );

	// hemispherical ambient lighting
	float hemi = dot( N, uSkyDirection.xyz ) * 0.5 + 0.5;
	oColor.rgb = mix( vec3(0), vec3(0.1), hemi );

	// diffuse lighting
	float lambert = max( 0, dot( N, L ) );
	oColor.rgb += lambert * matDiffuse * lightDiffuse;

	// output gamma corrected color
	oColor.rgb = sqrt( oColor.rgb );
	oColor.a = 1;
}