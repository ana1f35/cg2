#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D explosionTexture;

void main()
{    
    FragColor = texture(explosionTexture, TexCoords);
}