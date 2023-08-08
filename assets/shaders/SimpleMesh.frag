#version 330 core

in vec2 TexCoord;

out vec4 color;


uniform sampler2D ourTexture1;
uniform vec4 objectColor;

void main() {
	color = objectColor * texture(ourTexture1, TexCoord);
}
