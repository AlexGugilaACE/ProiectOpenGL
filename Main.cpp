#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb/stb_image.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

std::string readFile(const char* filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open()) {
        std::cerr << "Eroare: Nu s-a putut deschide fisierul: " << filePath << std::endl;
        return "";
    }
    std::stringstream sstr;
    sstr << fileStream.rdbuf();
    content = sstr.str();
    fileStream.close();
    return content;
}

unsigned int loadTexture(char const* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 3);
    if (data) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap failure at: " << faces[i] << std::endl;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proiect OpenGL - P1", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);

    // shadere
    unsigned int shaderProgram = glCreateProgram();
    {
        std::string v = readFile("ground.vert"), f = readFile("ground.frag");
        const char* vs = v.c_str(), * fs = f.c_str();
        unsigned int vS = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vS, 1, &vs, NULL); glCompileShader(vS);
        unsigned int fS = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fS, 1, &fs, NULL); glCompileShader(fS);
        glAttachShader(shaderProgram, vS); glAttachShader(shaderProgram, fS); glLinkProgram(shaderProgram);
        glDeleteShader(vS); glDeleteShader(fS);
    }

    unsigned int skyboxShader = glCreateProgram();
    {
        std::string v = readFile("skybox.vert"), f = readFile("skybox.frag");
        const char* vs = v.c_str(), * fs = f.c_str();
        unsigned int vS = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vS, 1, &vs, NULL); glCompileShader(vS);
        unsigned int fS = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fS, 1, &fs, NULL); glCompileShader(fS);
        glAttachShader(skyboxShader, vS); glAttachShader(skyboxShader, fS); glLinkProgram(skyboxShader);
        glDeleteShader(vS); glDeleteShader(fS);
    }

    // generare teren
    std::vector<float> terrainVertices;
    int resolution = 100; // 100x100 patratele
    float terrainSize = 100.0f;

    for (int i = 0; i <= resolution; i++) {
        for (int j = 0; j <= resolution; j++) {
            float x = -terrainSize / 2.0f + (float)j / resolution * terrainSize;
            float z = -terrainSize / 2.0f + (float)i / resolution * terrainSize;
            
            // formula relief: Sin si Cos creeaza damburi line
            float y = sin(x * 0.2f) * cos(z * 0.2f) * 2.0f;

            // pozitie
            terrainVertices.push_back(x);
            terrainVertices.push_back(y);
            terrainVertices.push_back(z);
            // coordonate textura (de 10 ori)
            terrainVertices.push_back((float)j / resolution * 10.0f);
            terrainVertices.push_back((float)i / resolution * 10.0f);
        }
    }

    std::vector<unsigned int> terrainIndices;
    for (int i = 0; i < resolution; i++) {
        for (int j = 0; j < resolution; j++) {
            int row1 = i * (resolution + 1);
            int row2 = (i + 1) * (resolution + 1);
            terrainIndices.push_back(row1 + j);
            terrainIndices.push_back(row1 + j + 1);
            terrainIndices.push_back(row2 + j);
            terrainIndices.push_back(row1 + j + 1);
            terrainIndices.push_back(row2 + j + 1);
            terrainIndices.push_back(row2 + j);
        }
    }

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, terrainVertices.size() * sizeof(float), &terrainVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrainIndices.size() * sizeof(unsigned int), &terrainIndices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);

    // geometrie cer
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };
    unsigned int skyVAO, skyVBO;
    glGenVertexArrays(1, &skyVAO); glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO); glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);

    // munti
    float mountainVertices[] = {
         0.0f,  15.0f,  0.0f, 0.5f, 1.0f, -15.0f, 0.0f, 15.0f, 0.0f, 0.0f, 15.0f, 0.0f, 15.0f, 1.0f, 0.0f,
         0.0f,  15.0f,  0.0f, 0.5f, 1.0f, 15.0f, 0.0f, 15.0f, 0.0f, 0.0f, 15.0f, 0.0f, -15.0f, 1.0f, 0.0f,
         0.0f,  15.0f,  0.0f, 0.5f, 1.0f, 15.0f, 0.0f, -15.0f, 0.0f, 0.0f, -15.0f, 0.0f, -15.0f, 1.0f, 0.0f,
         0.0f,  15.0f,  0.0f, 0.5f, 1.0f, -15.0f, 0.0f, -15.0f, 0.0f, 0.0f, -15.0f, 0.0f, 15.0f, 1.0f, 0.0f
    };
    unsigned int mntVAO, mntVBO;
    glGenVertexArrays(1, &mntVAO); glGenBuffers(1, &mntVBO);
    glBindVertexArray(mntVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mntVBO); glBufferData(GL_ARRAY_BUFFER, sizeof(mountainVertices), mountainVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);

    // texturi
    unsigned int grassTex = loadTexture("grass.jpg");
    std::vector<std::string> faces{ "sky_rt.tga", "sky_lf.tga", "sky_up.tga", "sky_dn.tga", "sky_ft.tga", "sky_bk.tga" };
    unsigned int rockTex = loadTexture("rock.jpg");
    unsigned int skyTex = loadCubemap(faces);

    // while loop principal
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 15.0f, 45.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // desenare teren cu relief
        glUseProgram(shaderProgram);
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTex);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (unsigned int)terrainIndices.size(), GL_UNSIGNED_INT, 0);

        // desenare munti
        glBindVertexArray(mntVAO);
        glBindTexture(GL_TEXTURE_2D, rockTex);

        // muntele 1
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-35.0f, 1.2f, -30.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.2f, 1.2f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 12);

        // muntele 2
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.4f, -20.0f));
        model = glm::scale(model, glm::vec3(1.1f, 1.6f, 1.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 12);

        // muntele 3
        model = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 1.2f, -40.0f));
        model = glm::scale(model, glm::vec3(0.8f, 1.3f, 1.4f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 12);

        // desenare sykbox
        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxShader);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glBindVertexArray(skyVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyTex);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO); glDeleteVertexArrays(1, &skyVAO);
    glfwTerminate();
    return 0;
}