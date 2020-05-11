#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform float n1;
uniform float n2;
uniform float m;

vec3 normal;

void main()
{           
	float pi = 3.14159265;
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos); 
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 H = normalize(lightDir + viewDir); 

	normal = texture(normalMap, fs_in.TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
  	
    // diffuse 
    vec3 diffuse = lightColor * max(dot(lightDir, normal),0.0) * texture(diffuseMap, fs_in.TexCoords).rgb;
    
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
        
	float ambiantIntensity = 0.01;
	float diffuseIntensity = 1.0;                       
    vec3 lighting = ambiantIntensity * lightColor + diffuseIntensity * diffuse + specKoef * lightColor;    
    
    FragColor = vec4(lighting, 1.0);
}