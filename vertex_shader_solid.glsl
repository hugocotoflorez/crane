#version 330 core

layout (location = 0) in vec3 aPos;  // Posición del vértice

uniform mat4 model;   // Matriz de modelo
uniform mat4 view;    // Matriz de vista (cámara)
uniform mat4 projection; // Matriz de proyección

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);  // Transformación de vértices
}

