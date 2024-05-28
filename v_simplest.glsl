#version 330 core


layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texCoord0;


out vec2 texCoord;
out vec3 Normal;
out vec3 crntPos;


uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main()
{
    crntPos = vec3(M * vertex);
    gl_Position = P * V * vec4(crntPos, 1.0);

    texCoord = texCoord0;
    Normal = normalize(mat3(transpose(inverse(M))) * vec3(normal));
}
