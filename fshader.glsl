#version 130

in  vec4 color;
out vec4  fColor;

void main() 
{ 
	if(color.w == 0.0) //for transparency
		discard;
	fColor = color;
}