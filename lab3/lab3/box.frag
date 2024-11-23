#version 330 core

in vec3 color;
in vec3 worldPosition;
in vec3 worldNormal;

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform float reflectance = 0.78;

uniform sampler2D depthMap;
in vec2 TexCoords;

void main()
{
	vec3 lightDir = normalize(lightPosition-worldPosition);

	float cosTheta = dot(lightDir,worldNormal);

	 // Calculate the distance squared (r^2) between the light source and the fragment
    float distanceSquared = length(lightPosition - worldPosition) * length(lightPosition - worldPosition);

    // Implement the Lambertian illumination model using the formula
    vec3 irradiance = (reflectance / 3.14159) * cosTheta * (lightIntensity / (4.0 * 3.14159 * distanceSquared));


	finalColor = color* irradiance;

	// TODO: lighting, tone mapping, gamma correction
	finalColor = finalColor / (1.0 + finalColor);

    // Apply gamma correction for better display on screen
    finalColor = pow(finalColor, vec3(1.0/ 2.2));

	float depthValue = texture(depthMap, TexCoords).r;
	// finalColor /= vec3(depthValue);

	finalColor *= depthValue * depthValue;
}
