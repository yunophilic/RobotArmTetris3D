#version 130

in vec4 vPosition;
in vec4 vColor;
out vec4 color;
uniform mat4 MVPmat;

void main() 
{
	gl_Position = MVPmat * vPosition;
	color = vColor;	
}