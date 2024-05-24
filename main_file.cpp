/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

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
#include <assimp/Importer.hpp> //wczytuje plik
#include <assimp/scene.h> //obiekt reprezentujacy calosc pliku
#include <assimp/postprocess.h> //dodatkowe rzeczy ktore mozna robicz modelem
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cassert>



float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;
float zoom = -8;
ShaderProgram* sp;

std::vector <glm::vec4> verts;
std::vector <glm::vec4> norms;
std::vector <glm::vec2> texCoords;
std::vector <unsigned int> indices;

std::vector<glm::vec4> vertsSkeleton;
std::vector<glm::vec4> normsSkeleton;
std::vector<glm::vec2> texCoordsSkeleton;
std::vector<unsigned int> indicesSkeleton;

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
GLuint tex0;
GLuint tex1;


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


#pragma region ModelLoading


/*  // TUTAJ TO POPRZEDNIE MAPOWANIE
struct BoneInfo {
    glm::mat4 boneOffset;
    glm::mat4 finalTransformation;
};

struct VertexBoneData {
    unsigned int IDs[4] = { 0 };
    float weights[4] = { 0.0f };

    void addBoneData(unsigned int boneID, float weight) {
        for (unsigned int i = 0; i < 4; i++) {
            if (weights[i] == 0.0f) {
                IDs[i] = boneID;
                weights[i] = weight;
                return;
            }
        }
    }
};

std::vector<VertexBoneData> bones;
std::map<std::string, unsigned int> boneMapping;
std::vector<BoneInfo> boneInfo;
unsigned int numBones = 0;


void importujMesh(aiMesh* mesh, const glm::mat4& transform, unsigned int& vertexOffset) {
    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        aiVector3D vertex = mesh->mVertices[i];
        glm::vec4 transformedVertex = transform * glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);
        verts.push_back(transformedVertex);

        aiVector3D normal = mesh->mNormals[i];
        glm::vec4 transformedNormal = glm::transpose(glm::inverse(transform)) * glm::vec4(normal.x, normal.y, normal.z, 0.0f);
        norms.push_back(transformedNormal);

        if (mesh->mTextureCoords[0]) {
            aiVector3D texCoord = mesh->mTextureCoords[0][i];
            texCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
        }
        else {
            texCoords.push_back(glm::vec2(0.0f, 0.0f));
        }

        bones.push_back(VertexBoneData()); // Initialize bone data for each vertex
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(vertexOffset + face.mIndices[j]);
        }
    }

    // Load bones
    for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
        aiBone* bone = mesh->mBones[i];
        unsigned int boneIndex = 0;
        std::string boneName(bone->mName.data);

        if (boneMapping.find(boneName) == boneMapping.end()) {
            boneIndex = numBones;
            numBones++;
            BoneInfo bi;
            bi.boneOffset = glm::transpose(glm::make_mat4(&bone->mOffsetMatrix.a1));
            boneInfo.push_back(bi);
            boneMapping[boneName] = boneIndex;
        }
        else {
            boneIndex = boneMapping[boneName];
        }

        for (unsigned int j = 0; j < bone->mNumWeights; ++j) {
            aiVertexWeight& vw = bone->mWeights[j];
            unsigned int vertexID = vertexOffset + vw.mVertexId;
            bones[vertexID].addBoneData(boneIndex, vw.mWeight);
        }
    }

    // Update vertex offset
    vertexOffset += mesh->mNumVertices;
}


void processNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform, unsigned int& vertexOffset) {
    glm::mat4 nodeTransform = parentTransform * glm::transpose(glm::make_mat4(&node->mTransformation.a1));

    // Process all meshes assigned to this node
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        importujMesh(mesh, nodeTransform, vertexOffset);
    }

    // Recursively process each child node
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, nodeTransform, vertexOffset);
    }
}
*/

//TO JEST MAPOWANIE Z TEGO FILMIKU TEGO ŁYSEGO ZIOMKA. WYDAJE SIĘ DOBRZE DZIAŁAĆ
#define MAX_NUM_BONES_PER_VERTEX 4

struct VertexBoneData
{
    unsigned int BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 };
    float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };

    VertexBoneData()
    {
    }

    void AddBoneData(unsigned int BoneID, float Weight)
    {
        for (unsigned int i = 0; i < MAX_NUM_BONES_PER_VERTEX; i++) {
            if (Weights[i] == 0.0f) {
                BoneIDs[i] = BoneID;
                Weights[i] = Weight;
                //printf("Adding bone %d weight %f at index %i\n", BoneID, Weight, i);
                return;
            }
        }

        // should never get here - more bones than we have space for
        assert(0);
    }
};

std::vector<VertexBoneData> vertex_to_bones;
std::vector<int> mesh_base_vertex;
std::map<std::string, unsigned int> bone_name_to_index_map;


static int space_count = 0;

void print_space()
{
    for (int i = 0; i < space_count; i++) {
        printf(" ");
    }
}

void print_assimp_matrix(const aiMatrix4x4& m)
{
    print_space(); printf("%f %f %f %f\n", m.a1, m.a2, m.a3, m.a4);
    print_space(); printf("%f %f %f %f\n", m.b1, m.b2, m.b3, m.b4);
    print_space(); printf("%f %f %f %f\n", m.c1, m.c2, m.c3, m.c4);
    print_space(); printf("%f %f %f %f\n", m.d1, m.d2, m.d3, m.d4);
}

int get_bone_id(const aiBone* pBone)
{
    int bone_id = 0;
    std::string bone_name(pBone->mName.C_Str());

    if (bone_name_to_index_map.find(bone_name) == bone_name_to_index_map.end()) {
        // Allocate an index for a new bone
        bone_id = (int)bone_name_to_index_map.size();
        bone_name_to_index_map[bone_name] = bone_id;
    }
    else {
        bone_id = bone_name_to_index_map[bone_name];
    }

    return bone_id;
}

void parse_single_bone(int mesh_index, const aiBone* pBone)
{
    printf("      Bone '%s': num vertices affected by this bone: %d\n", pBone->mName.C_Str(), pBone->mNumWeights);

    int bone_id = get_bone_id(pBone);

    print_assimp_matrix(pBone->mOffsetMatrix);

    for (unsigned int i = 0; i < pBone->mNumWeights; i++) {
        const aiVertexWeight& vw = pBone->mWeights[i];

        unsigned int global_vertex_id = mesh_base_vertex[mesh_index] + vw.mVertexId;

        assert(global_vertex_id < vertex_to_bones.size());
        vertex_to_bones[global_vertex_id].AddBoneData(bone_id, vw.mWeight);
    }

    printf("\n");
}

void parse_mesh_bones(int mesh_index, const aiMesh* pMesh)
{
    for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
        parse_single_bone(mesh_index, pMesh->mBones[i]);
    }
}

void parse_meshes(const aiScene* pScene)
{
    printf("*******************************************************\n");
    printf("Parsing %d meshes\n\n", pScene->mNumMeshes);

    int total_vertices = 0;
    int total_indices = 0;
    int total_bones = 0;

    mesh_base_vertex.resize(pScene->mNumMeshes);

    for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
        const aiMesh* pMesh = pScene->mMeshes[i];
        int num_vertices = pMesh->mNumVertices;
        int num_indices = pMesh->mNumFaces * 3;
        int num_bones = pMesh->mNumBones;
        mesh_base_vertex[i] = total_vertices;
        printf("  Mesh %d '%s': vertices %d indices %d bones %d\n\n", i, pMesh->mName.C_Str(), num_vertices, num_indices, num_bones);
        total_vertices += num_vertices;
        total_indices += num_indices;
        total_bones += num_bones;

        vertex_to_bones.resize(total_vertices);

        for (unsigned int j = 0; j < pMesh->mNumVertices; j++) {
            const aiVector3D& pos = pMesh->mVertices[j];
            verts.emplace_back(pos.x, pos.y, pos.z, 1.0f);

            if (pMesh->HasNormals()) {
                const aiVector3D& normal = pMesh->mNormals[j];
                norms.emplace_back(normal.x, normal.y, normal.z, 0.0f);
            }

            if (pMesh->HasTextureCoords(0)) {
                const aiVector3D& texCoord = pMesh->mTextureCoords[0][j];
                texCoords.emplace_back(texCoord.x, texCoord.y);
            }
        }

        for (unsigned int j = 0; j < pMesh->mNumFaces; j++) {
            const aiFace& face = pMesh->mFaces[j];
            assert(face.mNumIndices == 3);
            indices.push_back(mesh_base_vertex[i] + face.mIndices[0]);
            indices.push_back(mesh_base_vertex[i] + face.mIndices[1]);
            indices.push_back(mesh_base_vertex[i] + face.mIndices[2]);
        }

        if (pMesh->HasBones()) {
            parse_mesh_bones(i, pMesh);
        }

        printf("\n");
    }

    printf("\nTotal vertices %d total indices %d total bones %d\n", total_vertices, total_indices, total_bones);
}

void parse_node(const aiNode* pNode)
{
    print_space(); printf("Node name: '%s' num children %d num meshes %d\n", pNode->mName.C_Str(), pNode->mNumChildren, pNode->mNumMeshes);
    print_space(); printf("Node transformation:\n");
    print_assimp_matrix(pNode->mTransformation);

    space_count += 4;

    for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
        printf("\n");
        print_space(); printf("--- %d ---\n", i);
        parse_node(pNode->mChildren[i]);
    }

    space_count -= 4;
}

void parse_hierarchy(const aiScene* pScene)
{
    printf("\n*******************************************************\n");
    printf("Parsing the node hierarchy\n");

    parse_node(pScene->mRootNode);
}

void parse_scene(const aiScene* pScene)
{
    parse_meshes(pScene);

    parse_hierarchy(pScene);
}

void loadModel(const std::string& plik) {
    using namespace std;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(plik,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
        return;
    }

    // Clear previous data
    verts.clear();
    norms.clear();
    texCoords.clear();
    indices.clear();

    unsigned int vertexOffset = 0;

    // Process the root node
    parse_scene(scene);
}
#pragma endregion


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


    loadModel(std::string("szkielet2.fbx"));
    vertsSkeleton = verts;
    normsSkeleton = norms;
    texCoordsSkeleton = texCoords;
    indicesSkeleton = indices;

    loadModel(std::string("stage_floor.obj"));
    vertsStageFloor = verts;
    normsStageFloor = norms;
    texCoordsStageFloor = texCoords;
    indicesStageFloor = indices;

    loadModel(std::string("stage_objects.obj"));
    vertsStage = verts;
    normsStage = norms;
    texCoordsStage = texCoords;
    indicesStage = indices;
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

    delete sp;

    glDeleteTextures(1, &tex0);
}




//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
    //************Tutaj umieszczaj kod rysujący obraz******************l
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float lightPos[] = {
        -1.5f,  14.0f,  1.5f,
        1.5f,  14.0f,  1.5f,
        -1.5f,  14.0f,  -1.5f,
        1.5f,  14.0f,  -1.5f
    };

    float lightColors[] = {
       1.0f, 0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f, 1.0f,
       0.0f, 0.0f, 1.0f, 1.0f,
       0.5f, 0.5f, 0.5f, 1.0f
    };

    glm::vec3 camPos = glm::vec3(0, 10, zoom);

    glm::mat4 V = glm::lookAt(
        camPos,
        glm::vec3(0, 4, 0),
        glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz widoku

    //V = glm::rotate(V, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 0.0f)); 

    glm::mat4 P = glm::perspective(100.0f * PI / 180.0f, aspectRatio, 0.01f, 80.0f); //Wylicz macierz rzutowania

    glm::mat4 M = glm::mat4(1.0f);

   // M = glm::rotate(M, -angle_y, glm::vec3(1.0f, 0.0f, 0.0f)); //Wylicz macierz modelu
   // M = glm::rotate(M, -angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz modelu


    sp->use();//Aktywacja programu cieniującego
    //Przeslij parametry programu cieniującego do karty graficznej
    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniform3fv(sp->u("camPos"), 1, glm::value_ptr(camPos));
    glUniform4fv(sp->u("lightColors"), 4, lightColors);
    glUniform3fv(sp->u("lightPos"), 4, lightPos);

    
    //Rysuj podgłoge sceny
    glm::mat4 M_stage = glm::scale(M, glm::vec3(1.75f, 1.1f, 1.5)); //Przeskaluj scene jeśli potrzeba
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
    
   //Rysuj scene
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

    
    // Rysuj szkielet
    glm::mat4 M_skeleton = glm::translate(M, glm::vec3(0.0f, 4.7f, 0.0f));
    M_skeleton = glm::rotate(M_skeleton, -angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz modelu

    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_skeleton));

    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, vertsSkeleton.data());
    glEnableVertexAttribArray(sp->a("normal"));
    glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normsSkeleton.data());
    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoordsSkeleton.data());

    glUniform1i(sp->u("textureMap0"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texSkeleton);

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