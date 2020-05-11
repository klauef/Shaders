#version 400 core
out vec4 fragColor;

in vec3 normal;  
in vec3 fragPos;  
  
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 objectAmbiantColor;
uniform vec3 objectSpecularColor;
uniform vec3 objectDiffuseColor;
uniform vec3 lightColor;

uniform float n1;
uniform float n2;
uniform float m;

void main()
{
	float pi = 3.14159265;
    vec3 viewDir = normalize(viewPos - fragPos); 
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 H = normalize(lightDir + viewDir); 

    // ambiant
    vec4 ambiant = vec4(objectAmbiantColor * lightColor, 1.0);
  	
    // diffuse 
    vec4 diffuse = vec4(lightColor * objectDiffuseColor * max(dot(lightDir, normal),0.0), 1.0) * texture(texture_diffuse1, TexCoords);
    
    // specular
	float cosAlpha = dot(normal,H);
	float tangenteCarre = (1.0 - pow(cosAlpha, 2.0)) / (pow(cosAlpha, 2.0) * pow(m, 2.0));
	float D = exp(-tangenteCarre) / (pi * pow(m, 2.0) * pow(cosAlpha, 4.0));
	  
	float R0 = (n1 - n2) / (n1 + n2);
	float F = R0 + (1.0 - R0) * pow((1.0 - max(dot(normal, viewDir),0.0)), 5.0);
	
	float G1 = (2.0 * max(dot(H,normal),0.0) * max(dot(viewDir, normal),0.0)) / max(dot(viewDir, H),0.0);
	float G2 = (2.0 * max(dot(H,normal),0.0) * max(dot(lightDir, normal),0.0)) / max(dot(viewDir, H),0.0);
	float G = min(1, min(G1, G2));
	float specKoef = (D * F * G) / (4.0 * max(dot(viewDir, normal),0.0) * max(dot(normal, lightDir),0.0));
	vec4 specular = vec4(objectSpecularColor * lightColor, 1.0);
        
	float ambiantIntensity = 0.1;
	float diffuseIntensity = 1.0;
    vec4 result = ambiantIntensity * ambiant + diffuseIntensity * diffuse + specKoef * specular;
    fragColor = result;
} 