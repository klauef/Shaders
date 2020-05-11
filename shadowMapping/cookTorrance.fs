#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
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
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normale = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normale, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
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
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{           
	float pi = 3.14159265;
    vec3 viewDir = normalize(viewPos - fs_in.FragPos); 
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 H = normalize(lightDir + viewDir); 


    // ambiant
    vec3 ambient = objectAmbiantColor * lightColor;
  	
    // diffuse 
    vec3 diffuse = (vec4(objectDiffuseColor * lightColor * max(dot(lightDir, fs_in.Normal),0.0), 1.0) * texture(diffuseMap, fs_in.TexCoords)).xyz;
    
    // specular
	float cosAlpha = dot(fs_in.Normal,H);
	float tangenteCarre = (1.0 - pow(cosAlpha, 2.0)) / (pow(cosAlpha, 2.0) * pow(m, 2.0));
	float D = exp(-tangenteCarre) / (pi * pow(m, 2.0) * pow(cosAlpha, 4.0));
	  
	float R0 = (n1 - n2) / (n1 + n2);
	float F = R0 + (1.0 - R0) * pow((1.0 - max(dot(fs_in.Normal, viewDir),0.0)), 5.0);
	
	float G1 = (2.0 * max(dot(H,fs_in.Normal),0.0) * max(dot(viewDir, fs_in.Normal),0.0)) / max(dot(viewDir, H),0.0);
	float G2 = (2.0 * max(dot(H,fs_in.Normal),0.0) * max(dot(lightDir, fs_in.Normal),0.0)) / max(dot(viewDir, H),0.0);
	float G = min(1, min(G1, G2));
	float specKoef = (D * F * G) / (4.0 * max(dot(viewDir, fs_in.Normal),0.0) * max(dot(fs_in.Normal, lightDir),0.0));
	vec3 specular = objectSpecularColor * lightColor;
        
	float ambiantIntensity = 0.1;
	float diffuseIntensity = 1.0;  
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = ambiantIntensity * ambient + (1.0 - shadow) * (diffuseIntensity * diffuse + specKoef * specular);    
    
    FragColor = vec4(lighting, 1.0);
}