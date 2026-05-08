#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D texture1;
void main()
{
    vec4 texColor = texture(texture1, TexCoord);

    if(texColor.r > 0.95 && texColor.g > 0.95 && texColor.b > 0.95)
      discard;

    FragColor = texColor;
}
