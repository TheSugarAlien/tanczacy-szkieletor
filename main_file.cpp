#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"
#include <assimp/Importer.hpp> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h> 
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <chrono>


float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;
float zoom = -8;
float rotation = 0.0f;

float lightPos[] = {
      -1.5f,  14.0f,  1.5f,
      1.5f,  14.0f,  1.5f,
      -1.5f,  14.0f,  -1.5f,
      1.5f,  14.0f,  -1.5f,
      10.0f,  14.0f,  0.0f,
      -10.0f,  14.0f,  0.0f
};

float lightColors[] = {
   1.0f, 0.0f, 1.0f, 1.0f,
   0.0f, 1.0f, 1.0f, 1.0f,
   0.0f, 0.0f, 1.0f, 1.0f,
   0.5f, 0.5f, 0.5f, 1.0f,
   1.0f, 0.0f, 0.0f, 1.0f,
   0.0f, 1.0f, 0.0f, 1.0f
};

bool enLight = true;

ShaderProgram* sp;
const aiScene* scene;

std::vector <glm::vec4> verts;
std::vector <glm::vec4> norms;
std::vector <glm::vec2> texCoords;
std::vector <unsigned int> indices;

std::vector <glm::vec4> vertsSkull;
std::vector <glm::vec4> normsSkull;
std::vector <glm::vec2> texCoordsSkull;
std::vector <unsigned int> indicesSkull;

std::vector<glm::vec4> vertsSkeleton;
std::vector<glm::vec4> normsSkeleton;
std::vector<glm::vec2> texCoordsSkeleton;
std::vector<unsigned int> indicesSkeleton;

std::vector<glm::vec4> vertsSkeleton2;
std::vector<glm::vec4> normsSkeleton2;
std::vector<glm::vec2> texCoordsSkeleton2;


std::vector<glm::vec4> vertsSkeleton3;
std::vector<glm::vec4> normsSkeleton3;
std::vector<glm::vec2> texCoordsSkeleton3;


std::vector<glm::vec4> vertsSkeleton4;
std::vector<glm::vec4> normsSkeleton4;
std::vector<glm::vec2> texCoordsSkeleton4;


std::vector<glm::vec4> vertsStage;
std::vector<glm::vec4> normsStage;
std::vector<glm::vec2> texCoordsStage;
std::vector<unsigned int> indicesStage;

std::vector<glm::vec4> vertsStageFloor;
std::vector<glm::vec4> normsStageFloor;
std::vector<glm::vec2> texCoordsStageFloor;
std::vector<unsigned int> indicesStageFloor;

GLuint texSkeleton;
GLuint texStageFloor;
GLuint texStageFloorSpec;
GLuint texStage;
GLuint texSkull;

struct Vertex {
    glm::vec4 vertex;
    glm::vec4 normal;     
    glm::vec2 texCoords; 
};


struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

std::vector<Mesh> meshes;

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
    fputs(description, stderr);
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_LEFT) speed_x = -PI;
        if (key == GLFW_KEY_RIGHT) speed_x = PI;
        if (key == GLFW_KEY_UP) speed_y = PI;
        if (key == GLFW_KEY_DOWN) speed_y = -PI;
        if (key == GLFW_KEY_1) {
            if (lightPos[1] == -100.0f) lightPos[1] = 14.0f;
            else lightPos[1] = -100.0f;
        }
        if (key == GLFW_KEY_2) {
            if (lightPos[4] == -100.0f) lightPos[4] = 14.0f;
            else lightPos[4] = -100.0f;
        }
        if (key == GLFW_KEY_3) {
            if (lightPos[7] == -100.0f) lightPos[7] = 14.0f;
            else lightPos[7] = -100.0f;
        }
        if (key == GLFW_KEY_4) {
            if (lightPos[10] == -100.0f) lightPos[10] = 14.0f;
            else lightPos[10] = -100.0f;
        }
        if (key == GLFW_KEY_5) {
            if (lightPos[13] == -100.0f) lightPos[13] = 14.0f;
            else lightPos[13] = -100.0f;
        }
        if (key == GLFW_KEY_6) {
            if (lightPos[16] == -100.0f) lightPos[16] = 14.0f;
            else lightPos[16] = -100.0f;
        }
        if (key == GLFW_KEY_X) {
            enLight = !enLight;
        }
        if (key == GLFW_KEY_SPACE) {
            enLight = true;
            for (int i = 0; i < 6; i++) {
                lightPos[1 + 3 * i] = 14.0f;
            }
        }
        if (key == GLFW_KEY_0) {
            enLight = false;
            for (int i = 0; i < 6; i++) {
                lightPos[1 + 3 * i] = -100.0f;
            }
        }
    }
    if (action == GLFW_REPEAT) {
        if (key == GLFW_KEY_UP && zoom < 8) zoom += 0.15;
        if (key == GLFW_KEY_DOWN && zoom > -8) zoom -= 0.15;
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT) speed_x = 0;
        if (key == GLFW_KEY_RIGHT) speed_x = 0;
        if (key == GLFW_KEY_UP) speed_y = 0;
        if (key == GLFW_KEY_DOWN) speed_y = 0;
    }
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (height == 0) return;
    aspectRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}

GLuint readTexture(const char* filename) {
    GLuint tex;
    glActiveTexture(GL_TEXTURE0);

    //Wczytanie do pamięci komputera
    std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
    unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
    //Wczytaj obrazek
    unsigned error = lodepng::decode(image, width, height, filename);

    //Import do pamięci karty graficznej
    glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
    glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
    //Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return tex;
}
Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
    Mesh myMesh;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        glm::vec4 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vector.w = 1.0f;
        vertex.vertex = vector;

        if (mesh->mNormals) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vector.w = 0.0f;
            vertex.normal = vector;
        }

        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }


        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

  
    myMesh.vertices = vertices;
    myMesh.indices = indices;

    return myMesh;
}


float getAnimationTime(float deltaTime, float duration) {
    static float currentTime = 0.0f;
    currentTime += deltaTime;
    if (currentTime > duration) {
        currentTime -= duration;
    }
    return currentTime;
}


void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, meshes);
    }
}

std::vector<Mesh> loadModel(const std::string& path) {
    Assimp::Importer importer;
    scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    std::vector<Mesh> meshes;

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << path << "\nERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return meshes;
    }

    processNode(scene->mRootNode, scene, meshes);


    return meshes;
}


void assignMeshData(const std::vector<Mesh>& meshes,
    std::vector<glm::vec4>& verts,
    std::vector<glm::vec4>& norms,
    std::vector<glm::vec2>& texCoords,
    std::vector<unsigned int>& indices) {
    unsigned int indexOffset = 0;

    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            verts.push_back(vertex.vertex);
            norms.push_back(vertex.normal);
            texCoords.push_back(vertex.texCoords);
        }

        for (const auto& index : mesh.indices) {
            indices.push_back(index + indexOffset);
        }

        indexOffset += mesh.vertices.size();
    }
}

std::vector<Mesh> meshesSkeleton;
std::vector<Mesh> meshesSkeleton2;
std::vector<Mesh> meshesSkeleton3;

std::vector <glm::vec4> framevVerts;
std::vector <glm::vec4> frameNorms;
std::vector <glm::vec2> frameTexCoords;

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");

    texSkeleton = readTexture("skeleton_tex_main.png");
    texStage = readTexture("gray.png");
    texStageFloor = readTexture("disco_tex2.png");
    texStageFloorSpec = readTexture("disco_tex_spec.png");
    texSkull = readTexture("skull_tex.png");


    std::vector<Mesh> meshesStageFloor = loadModel("stage_floor.obj");
    assignMeshData(meshesStageFloor, vertsStageFloor, normsStageFloor, texCoordsStageFloor, indicesStageFloor);

    std::vector<Mesh> meshesStage = loadModel("stage_objects.obj");
    assignMeshData(meshesStage, vertsStage, normsStage, texCoordsStage, indicesStage);

    std::vector<Mesh> meshesSkull = loadModel("skull.fbx");
    assignMeshData(meshesSkull, vertsSkull, normsSkull, texCoordsSkull, indicesSkull);

    meshesSkeleton = loadModel("szkielet_frame1.dae");
    assignMeshData(meshesSkeleton, vertsSkeleton, normsSkeleton, texCoordsSkeleton, indicesSkeleton);

    meshesSkeleton2 = loadModel("szkielet_frame2.dae");
    assignMeshData(meshesSkeleton2, vertsSkeleton2, normsSkeleton2, texCoordsSkeleton2, indicesSkeleton);

    meshesSkeleton3 = loadModel("szkielet_frame3.dae");
    assignMeshData(meshesSkeleton3, vertsSkeleton3, normsSkeleton3, texCoordsSkeleton3, indicesSkeleton);

    framevVerts = vertsSkeleton;
    frameNorms = normsSkeleton;
    frameTexCoords = texCoordsSkeleton;
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

    delete sp;

    glDeleteTextures(1, &texSkeleton);
    glDeleteTextures(1, &texStage);
    glDeleteTextures(1, &texStageFloor);
    glDeleteTextures(1, &texSkull);
}

float timeSinceFrame = 0.0f;
int currentFrame;
float frameDuration = 0.01f;



//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
    //************Tutaj umieszczaj kod rysujący obraz******************l
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec3 camPos = glm::vec3(0, 10, zoom);

    glm::mat4 V = glm::lookAt(
        camPos,
        glm::vec3(0, 4, 0),
        glm::vec3(0.0f, 1.0f, 0.0f));

    //V = glm::rotate(V, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 0.0f)); 

    glm::mat4 P = glm::perspective(100.0f * PI / 180.0f, aspectRatio, 0.01f, 80.0f); 

    glm::mat4 M = glm::mat4(1.0f);



    //M = glm::rotate(M, -angle_y, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
    //M = glm::rotate(M, -angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz modelu


    sp->use();//Aktywacja programu cieniującego
    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniform3fv(sp->u("camPos"), 1, glm::value_ptr(camPos));
    glUniform4fv(sp->u("lightColors"), 6, lightColors);
    glUniform3fv(sp->u("lightPos"), 6, lightPos);
    glUniform1i(sp->u("enLight"), enLight ? 1 : 0);

    //Podgłoga sceny
    glm::mat4 M_stage = glm::scale(M, glm::vec3(1.75f, 1.1f, 1.5));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_stage));


    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, vertsStageFloor.data());
    glEnableVertexAttribArray(sp->a("normal"));
    glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normsStageFloor.data());
    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoordsStageFloor.data());

    glUniform1i(sp->u("textureMap0"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texStageFloor);

    glUniform1i(sp->u("textureMap1"), 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texStageFloorSpec);

    glDrawElements(GL_TRIANGLES, indicesStageFloor.size(), GL_UNSIGNED_INT, indicesStageFloor.data());

    glDisableVertexAttribArray(sp->a("vertex"));
    glDisableVertexAttribArray(sp->a("normal"));
    glDisableVertexAttribArray(sp->a("texCoord0"));

    //Scena
    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, vertsStage.data());
    glEnableVertexAttribArray(sp->a("normal"));
    glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normsStage.data());
    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoordsStage.data());

    glUniform1i(sp->u("textureMap0"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texStage);

    glDrawElements(GL_TRIANGLES, indicesStage.size(), GL_UNSIGNED_INT, indicesStage.data());

    glDisableVertexAttribArray(sp->a("vertex"));
    glDisableVertexAttribArray(sp->a("normal"));
    glDisableVertexAttribArray(sp->a("texCoord0"));


    //Czaszki
    rotation += 6 * PI * glfwGetTime();

    glm::mat4 M_skull_1 = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 5.0f, 0.0f));
    M_skull_1 = glm::rotate(M_skull_1, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
    M_skull_1 = glm::rotate(M_skull_1, rotation, glm::vec3(0.0f, 0.0f, -1.0f));
    M_skull_1 = glm::scale(M_skull_1, glm::vec3(3.0f, 3.0f, 3.0f));



    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_skull_1));


    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, vertsSkull.data());
    glEnableVertexAttribArray(sp->a("normal"));
    glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normsSkull.data());
    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoordsSkull.data());

    glUniform1i(sp->u("textureMap0"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texSkull);


    glDrawElements(GL_TRIANGLES, indicesSkull.size(), GL_UNSIGNED_INT, indicesSkull.data());


    glm::mat4 M_skull_2 = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 5.0f, 0.0f));
    M_skull_2 = glm::rotate(M_skull_2, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
    M_skull_2 = glm::rotate(M_skull_2, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    M_skull_2 = glm::scale(M_skull_2, glm::vec3(3.0f, 3.0f, 3.0f));


    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_skull_2));
    glDrawElements(GL_TRIANGLES, indicesSkull.size(), GL_UNSIGNED_INT, indicesSkull.data());

    glDisableVertexAttribArray(sp->a("vertex"));
    glDisableVertexAttribArray(sp->a("normal"));
    glDisableVertexAttribArray(sp->a("texCoord0"));


    //Szkielet
    glm::mat4 M_skeleton = glm::translate(M, glm::vec3(0.0f, 4.7f, 0.0f));
    M_skeleton = glm::rotate(M_skeleton, -angle_x, glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_skeleton));

    glUniform1i(sp->u("textureMap0"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texSkeleton);

    timeSinceFrame += glfwGetTime();

    if (timeSinceFrame >= frameDuration)
    {
        if (currentFrame == 0)
        {
            framevVerts = vertsSkeleton;
            frameNorms = normsSkeleton;
            frameTexCoords = texCoordsSkeleton;
            currentFrame = 1;
        }
        else if (currentFrame == 1)
        {
            framevVerts = vertsSkeleton2;
            frameNorms = normsSkeleton2;
            frameTexCoords = texCoordsSkeleton2;
            currentFrame = 2;
        }
        else if (currentFrame == 2)
        {
            framevVerts = vertsSkeleton3;
            frameNorms = normsSkeleton3;
            frameTexCoords = texCoordsSkeleton3;
            currentFrame = 3;
        }
        else {
            framevVerts = vertsSkeleton2;
            frameNorms = normsSkeleton2;
            frameTexCoords = texCoordsSkeleton2;
            currentFrame = 0;
        }
        timeSinceFrame = 0.0f;

    }

    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, framevVerts.data());
    glEnableVertexAttribArray(sp->a("normal"));
    glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, frameNorms.data());
    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, frameTexCoords.data());

    glDrawElements(GL_TRIANGLES, indicesSkeleton.size(), GL_UNSIGNED_INT, indicesSkeleton.data());
    glDisableVertexAttribArray(sp->a("vertex"));
    glDisableVertexAttribArray(sp->a("normal"));
    glDisableVertexAttribArray(sp->a("texCoord0"));



    glfwSwapBuffers(window); //Przerzuć tylny bufor na przedni
}


int main(void)
{
    GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

    glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

    if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
        fprintf(stderr, "Nie można zainicjować GLFW.\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

    if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
    {
        fprintf(stderr, "Nie można utworzyć okna.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
    glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

    if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
        fprintf(stderr, "Nie można zainicjować GLEW.\n");
        exit(EXIT_FAILURE);
    }

    initOpenGLProgram(window); //Operacje inicjujące

    //Główna pętla
    float angle_x = 0; //Aktualny kąt obrotu obiektu
    float angle_y = 0; //Aktualny kąt obrotu obiektu
    glfwSetTime(0); //Zeruj timer



    while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
    {
        angle_x += speed_x * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
        angle_y += speed_y * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
        glfwSetTime(0); //Zeruj timer
        drawScene(window, angle_x, angle_y); //Wykonaj procedurę rysującą
        glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
    }

    freeOpenGLProgram(window);

    glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
    glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
    exit(EXIT_SUCCESS);
}