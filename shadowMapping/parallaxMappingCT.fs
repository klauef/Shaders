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
uniform sampler2D dispMap;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform float n1;
uniform float n2;
uniform float m;
uniform float heightScale;

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

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // nombre de couche que l'on veut afficher avec la texture
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    
	// calcul de l'espace entre chaque couche
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;

	// montant que l'on calcule pour la profondeur de layer
    vec2 P = viewDir.xy / viewDir.z * heightScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(dispMap, currentTexCoords).r;
      
	// tant que l'on a pas le layer correspondant à l'intersection voulue entre le vecteur oeil/frament
    while(currentLayerDepth < currentDepthMapValue)
    {
        // on décale la coordonnée de la texture par rapport à la vue
        currentTexCoords -= deltaTexCoords;
		// on récupère la valeur de profondeur de la texture actuelle
        currentDepthMapValue = texture(dispMap, currentTexCoords).r;  
        // récupération de la profondeur
        currentLayerDepth += layerDepth;  
    }

	// coordonnée avant collision
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	//récupération des deux point avant et après colision pour l'interpolation linéaire ce qui donne la position
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(dispMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // récupération de la position visible à partir de l'interpolation
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    
    return currentTexCoords;
}

void main()
{           
	float pi = 3.14159265;
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos); 
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 H = normalize(lightDir + viewDir); 

	//Parallax
	float height = texture(dispMap, fs_in.TexCoords).r * heightScale;
	vec2 texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);   
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

	//Normalmap
	normal = texture(normalMap, texCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	//diffuseMap
	vec3 diffuse = texture(diffuseMap, texCoords).rgb;

    // ambiant
    vec3 ambientColor = 0.1 * diffuse * lightColor;
  	
    // diffuse 
    vec3 diffuseColor = lightColor * max(dot(lightDir, normal),0.0) * diffuse;
    
    // specular
	float cosAlpha = dot(normal,H);
	float tangenteCarre = (1.0 - pow(cosAlpha, 2.0)) / (pow(cosAlpha, 2.0) * pow(m, 2.0));
	float D = exp(-tangenteCarre) / (pi * pow(m, 2.0) * pow(cosAlpha, 4.0));
	  
	float R0 = (n1 - n2) / (n1 + n2);
	float F = R0 + (1.0 - R0) * pow((1.0 - max(dot(normal, viewDir),0.0)), 5.0);
	
	float G1 = (2.0 * max(dot(H,normal),0.0) * max(dot(viewDir, normal),0.0)) / max(dot(viewDir, H),0.0);
	float G2 = (2.0 * max(dot(H,normal),0.0) * max(dot(lightDir, normal),0.0)) / max(dot(viewDir, H),0.0);
	float G = min(1, min(G1, G2));
	float specCoef = (D * F * G) / (4.0 * max(dot(viewDir, normal),0.0) * max(dot(normal, lightDir),0.0));
        
	float ambiantIntensity = 0.1;
	float diffuseIntensity = 1.0;  
    // calcul ombre
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);   
	
	// calcul final
    vec3 lighting = ambiantIntensity * ambientColor + (1.0 - shadow) * (diffuseIntensity * diffuseColor + specCoef);    
    
    FragColor = vec4(lighting, 1.0);
}