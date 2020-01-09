#version 410

uniform vec3 ConstantColor;
in vec4 QuadColor;
out vec4 FragColor;
void main()
{
    FragColor = QuadColor + vec4(ConstantColor, 0.0);
}
