#version 400 core
out vec4 fragColor;

in vec3 normal;  
in vec3 fragPos;  
  
in vec2 TexCoords;

uniform sampler2D texture_diffuse;

uniform vec3 lightPos; 
uniform vec3 lightColor;

void main()
{
    vec3 lightDir = normalize(lightPos - fragPos);
  	
    // diffuse 
	float angle = max(dot(lightDir, normal), 0.0);
	if(angle > 0.75){
		angle = 1.0;
	}else if(angle > 0.5){
		angle = 0.75;
	}else if(angle > 0.25){
		angle = 0.5;
	}else if(angle > 0.1){
		angle = 0.25;
	}else{
		angle = 0.0;
	}
    vec3 diffuse = lightColor * angle * texture(texture_diffuse, TexCoords).rgb;
    
	vec4 result = vec4(diffuse, 1.0);
    fragColor = result;
} 