#version 130

uniform mat4 projection, view, model;

in vec3 position;
in vec3 normal;
out vec3 normalFrag;

void main()
{
    // Transform matrix to viewspace
    normalFrag = vec3(model * vec4(normal,0.0));
	// Transform position from pixel coordinates to clipping coordinates
	gl_Position = projection * view * model * vec4(position, 1.0);
}

