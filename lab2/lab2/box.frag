#version 330 core

in vec3 color;

// TODO: To add UV input to this fragment shader
in vec2 uv;

// TODO: To add the texture sampler
uniform sampler2D textureSampler;

out vec3 finalColor;

void main()
{
	// finalColor = color;

	// TODO: texture lookup.
	finalColor = texture(textureSampler,uv).rgb;

}
