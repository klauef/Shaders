#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 objectAmbiantColor;
uniform vec3 objectSpecularColor;
uniform vec3 objectDiffuseColor;
uniform vec3 lightColor;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform float specIndice;


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
    vec3 normale = normalize(fs_in.Normal);
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
        // ambiant
    vec3 ambiant = objectAmbiantColor * lightColor;
  	
    // diffuse 
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 diffuse = (vec4(lightColor * objectDiffuseColor * max(dot(lightDir, fs_in.Normal),0.0), 1.0) * texture(diffuseTexture, fs_in.TexCoords)).xyz;
    
    // specular 
    vec3 viewDir = normalize(viewPos - fs_in.FragPos); 
    vec3 H = normalize(lightDir + viewDir); 
	vec3 specular = lightColor * objectSpecularColor * max(pow(dot(H, fs_in.Normal), specIndice),0.0);
        
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = ambiant + (1.0 - shadow) * (diffuse + specular);    
    
    FragColor = vec4(lighting, 1.0);
}