#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectAmbiantColor;
uniform vec3 objectSpecularColor;
uniform vec3 objectDiffuseColor;
uniform vec3 lightColor;

uniform float n1;
uniform float n2;
uniform float m;

vec3 normal;

float ShadowCalculation(vec4 fragPosLightSpace)
{

	//récupération de la projection du point de vue de la lumière
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // récupération de la distance stockée à partir de la texture de profondeur et de la projection lumière
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // récupération de la profondeur du fragment actuel
    float currentDepth = projCoords.z;
    
	// calcul d'un biais qui viens de la depthmap et de l'inclinaison
    vec3 normale = normalize(normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normale, lightDir)), 0.005);

	// test d'ombre : profondeur du fragment corrigé par le biais > à la profondeur de la map. Si oui pas d'ombre
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
	//correction dans le cas ou l'on dépasse le frustrum
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{           
	float pi = 3.14159265;
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos); 
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 H = normalize(lightDir + viewDir); 

	normal = texture(normalMap, fs_in.TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);

    // ambiant
    vec3 ambient = objectAmbiantColor * lightColor;
  	
    // diffuse 
    vec3 diffuse = (vec4(objectDiffuseColor * lightColor * max(dot(lightDir, normal),0.0), 1.0) * texture(diffuseMap, fs_in.TexCoords)).xyz;
    
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
	vec3 specular = objectSpecularColor * lightColor;
        
	float ambiantIntensity = 0.1;
	float diffuseIntensity = 1.0;  
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = ambiantIntensity * ambient + (1.0 - shadow) * (diffuseIntensity * diffuse + specKoef * specular);    
    
    FragColor = vec4(lighting, 1.0);
}