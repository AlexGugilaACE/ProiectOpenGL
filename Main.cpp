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

//CAMERA
// pozitie si orientare
glm::vec3 cameraPos = glm::vec3(0.0f, 40.0f, 90.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// unghiuri
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f, lastY = 600.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow* window) {
    float cameraSpeed = 20.0f * deltaTime; // viteza de deplasare

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // inainte/inapoi
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;

    // stanga/dreapta
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // sus/jos
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

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
    float terrainSize = 215.0f;

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
    unsigned int asphaltTex = loadTexture("asphalt.jpg");
    unsigned int buildingTex = loadTexture("building.jpg");
    unsigned int leavesTex = loadTexture("leaves.jpg");

    // geometrie circuit
    std::vector<float> circleVertices;
    std::vector<unsigned int> circleIndices;
    int segments = 60;
    float innerRadius = 25.0f;
    float outerRadius = 33.0f;

    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159f * (float)i / (float)segments;
        float xCos = cos(angle);
        float zSin = sin(angle);

        // Punct interior
        circleVertices.push_back(innerRadius * xCos);
        circleVertices.push_back(0.0f);
        circleVertices.push_back(innerRadius * zSin);
        circleVertices.push_back((float)i / segments * 10.0f); // Tex U
        circleVertices.push_back(0.0f);                        // Tex V

        // Punct exterior
        circleVertices.push_back(outerRadius * xCos);
        circleVertices.push_back(0.0f);
        circleVertices.push_back(outerRadius * zSin);
        circleVertices.push_back((float)i / segments * 10.0f); // Tex U
        circleVertices.push_back(1.0f);                        // Tex V
    }

    for (int i = 0; i < segments; i++) {
        circleIndices.push_back(2 * i);
        circleIndices.push_back(2 * i + 1);
        circleIndices.push_back(2 * i + 2);
        circleIndices.push_back(2 * i + 1);
        circleIndices.push_back(2 * i + 3);
        circleIndices.push_back(2 * i + 2);
    }

    unsigned int roadVAO, roadVBO, roadEBO;
    glGenVertexArrays(1, &roadVAO);
    glGenBuffers(1, &roadVBO);
    glGenBuffers(1, &roadEBO);

    glBindVertexArray(roadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roadVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), &circleVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, roadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, circleIndices.size() * sizeof(unsigned int), &circleIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float cubeVertices[] = {
        // Pozitii            // Textura
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // while loop principal
    while (!glfwWindowShouldClose(window)) {
        // calcul pentru viteza camerei
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // procesare input
        processInput(window);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // desenare teren
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTex);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (unsigned int)terrainIndices.size(), GL_UNSIGNED_INT, 0);

        // desenare munti
        glBindVertexArray(mntVAO);
        glBindTexture(GL_TEXTURE_2D, rockTex);

        // muntele 1
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-40.0f, 1.2f, -40.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.2f, 1.2f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 12);

        // muntele 2
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.4f, -45.0f));
        model = glm::scale(model, glm::vec3(1.1f, 1.6f, 1.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 12);

        // muntele 3
        model = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 1.2f, -40.0f));
        model = glm::scale(model, glm::vec3(0.8f, 1.3f, 1.4f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 12);

        // desenare circuit
        glBindVertexArray(roadVAO);
        glBindTexture(GL_TEXTURE_2D, asphaltTex);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.1f, 15.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, circleIndices.size(), GL_UNSIGNED_INT, 0);

        // desenare blocuri
        glBindVertexArray(cubeVAO);
        glBindTexture(GL_TEXTURE_2D, buildingTex);
        for (int i = 0; i < 5; i++) {
            float angle = i * (2.0f * 3.14159f / 5.0f);
            float dist = 20.0f;
            float x = cos(angle) * dist;
            float z = sin(angle) * dist + 15.0f;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, 4.0f, z));
            model = glm::scale(model, glm::vec3(5.0f, 12.0f, 5.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // desenare copaci
        glBindVertexArray(mntVAO);
        glBindTexture(GL_TEXTURE_2D, leavesTex);
        for (int i = 0; i < 5; i++) {
            float angle = i * (2.0f * 3.14159f / 5.0f) + 0.5f;
            float dist = 40.0f;
            float x = cos(angle) * dist;
            float z = sin(angle) * dist + 15.0f;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, 2.0f, z));
            model = glm::scale(model, glm::vec3(0.15f, 0.25f, 0.15f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 12);
        }

        // desenare skybox
        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxShader);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 skyView = glm::mat4(glm::mat3(view));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, glm::value_ptr(skyView));

        glBindVertexArray(skyVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyTex);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &skyVAO);
    glfwTerminate();
    return 0;
}