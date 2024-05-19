#version 330

// Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

// Atrybuty
in vec4 vertex; // współrzędne wierzchołka w przestrzeni modelu
in vec4 color; // kolor związany z wierzchołkiem
in vec4 normal; // wektor normalny w przestrzeni modelu
in vec2 texCoord0;

// Zmienne interpolowane
out vec4 ic;
out vec4 l;
out vec4 n;
out vec4 v;
out vec2 iTexCoord0;

void main(void) {
    vec4 lp = vec4(0, 3, 5, 1); // pozycja światła, przestrzeń świata
    vec4 worldPosition = M * vertex; // Pozycja wierzchołka w przestrzeni świata
    l = normalize(lp - worldPosition); // wektor do światła w przestrzeni świata
    v = normalize(-worldPosition); // wektor do obserwatora w przestrzeni świata
    n = normalize(M * normal); // wektor normalny w przestrzeni świata

    iTexCoord0 = texCoord0;
    ic = color;

    gl_Position = P * V * worldPosition;
}
