#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 17) out;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fColor;
out vec3 normal;

void main() {    	
	fColor = vec3(1.0, 0.0, 1.0); 
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4(-5.0, 0.0,  5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();   
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( 5.0, 0.0,  5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4(-5.0, 10.0, 5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( 5.0, 10.0, 5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4(-5.0, 10.0, -5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( 5.0, 10.0, -5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( 5.0, 10.0, 5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( 5.0, 0.0, -5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( 5.0, 0.0, 5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( -5.0, 0.0, -5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( -5.0, 0.0, 5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( -5.0, 10.0, -5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( -5.0, 10.0, 5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( -5.0, 10.0, -5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( 5.0, 10.0, -5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( -5.0, 0.0, 5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + projection * view * model * vec4( 5.0, 0.0, 5.0, 0.0);
	normal = gl_in[0].gl_Position.xyz + gl_Position.xyz;
    EmitVertex();
    EndPrimitive();
}