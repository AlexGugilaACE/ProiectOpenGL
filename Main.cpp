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

// setam fereastra
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// functie ca sa citim codul din fisiere
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

// functie pentru incarcarea texturii
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
        std::cout << "Textura incarcata cu succes: " << path << " (" << width << "x" << height << ")" << std::endl;
    }
    else {
        std::cerr << "Eroare: Textura nu a fost gasita la calea: " << path << std::endl;
    }
    return textureID;
}

int main() {
    // initializare glfw
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proiect OpenGL - P1 Sol", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // initializare glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // test de adancime
    glEnable(GL_DEPTH_TEST);

    // citim si compilam shader-ele
    std::string vertSourceStr = readFile("ground.vert");
    std::string fragSourceStr = readFile("ground.frag");
    const char* vertSource = vertSourceStr.c_str();
    const char* fragSource = fragSourceStr.c_str();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // geometrie sol
    float vertices[] = {
        // pozitii (x, y, z)    // coordonate textura
        -50.0f, 0.0f, -50.0f,   0.0f,  0.0f,  // stanga spate
         50.0f, 0.0f, -50.0f,  20.0f,  0.0f,  // dreapta spate
         50.0f, 0.0f,  50.0f,  20.0f, 20.0f,  // dreapta fata
        -50.0f, 0.0f,  50.0f,   0.0f, 20.0f   // stanga fata
    };
    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // atribut pozitie
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // atribut textura
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // incarcare textura pentru iarba
    unsigned int grassTex = loadTexture("grass.jpg");

    // randare
    while (!glfwWindowShouldClose(window)) {
        // input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // fundal gri inchis
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // folosim shader-ul
        glUseProgram(shaderProgram);

        // --- MATRICI MATEMATICE (GLM) ---
        // Projection: Unghi 45 grade, aspect ratio corect, clipping distat
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);

        // View: camera sta la (0, 10, 20) si priveste spre originea (0, 0, 0)
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 10.0f, 25.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        // Model: obiectul nu este mutat sau rotit
        glm::mat4 model = glm::mat4(1.0f);

        // Trimitem matricile catre Shader
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // --- RANDARE PROPRIU-ZISA ---
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTex);
        // Sincronizam numele uniformei "texture_diffuse" din ground.frag cu unitatea 0
        glUniform1i(glGetUniformLocation(shaderProgram, "texture_diffuse"), 0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap buffers si evenimente
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // curatarea resurselor
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();

    return 0;
}