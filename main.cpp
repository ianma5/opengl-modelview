// template based on material from learnopengl.com
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <fstream>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "importobj.h"
#include <vector>
#include <iostream>
#include <sstream>

std::string importShaders(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open " << filename << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();

}

void normalizeVertices(std::vector<float>& vertices) {
    float minX = vertices[0], maxX = vertices[0];
    float minY = vertices[1], maxY = vertices[1];
    float minZ = vertices[2], maxZ = vertices[2];

    for (size_t i = 0; i < vertices.size(); i += 3) {
        float x = vertices[i];
        float y = vertices[i + 1];
        float z = vertices[i + 2];

        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
        if (z < minZ) minZ = z;
        if (z > maxZ) maxZ = z;
    }

    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;
    float centerZ = (minZ + maxZ) / 2.0f;

    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;

    float maxSize = sizeX;
    if (sizeY > maxSize) maxSize = sizeY;
    if (sizeZ > maxSize) maxSize = sizeZ;

    float scale = 1.8f / maxSize;

    for (size_t i = 0; i < vertices.size(); i += 3) {
        vertices[i]     = (vertices[i]     - centerX) * scale;
        vertices[i + 1] = (vertices[i + 1] - centerY) * scale;
        vertices[i + 2] = (vertices[i + 2] - centerZ) * scale;
    }
}

struct Object {
    std::vector<float> vertices;
    unsigned int VBO = 0;
    unsigned int VAO = 0;
    unsigned int numVertices = 0;
    glm::vec3 center = glm::vec3(0.0f);
};

void setObj(Object& obj) {
    obj.numVertices = obj.vertices.size() / 3;

    glGenVertexArrays(1, &obj.VAO);
    glGenBuffers(1, &obj.VBO);

    glBindVertexArray(obj.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, obj.VBO);
    glBufferData(GL_ARRAY_BUFFER, obj.vertices.size() * sizeof(float), obj.vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// edit vertex and fragment shader to account for the new colors
std::string vs = importShaders("../source.vs");
std::string fs = importShaders("../source.fs");

const char* vertexShaderSource = vs.c_str();
const char* fragmentShaderSource = fs.c_str();

// part d
float posx = 0.0f, posy = 0.0f, posz = 0.0f;
float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;
float global_scale = 1.0f;

// animation variables
bool isOrbiting = false;
bool spacePressed = false;
float orbitAngle = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "viewGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // // glew: load all OpenGL function pointers
    glewInit();
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------   ------------
    Object obj1;
    Object obj2;

    //std::vector<float> vertices;


    if (!loadFile("../f-16.obj", obj1.vertices)) {
        return -1;
    }
    if (!loadFile("../pawn.obj", obj2.vertices)) {
        return -1;
    }
    normalizeVertices(obj1.vertices);
    normalizeVertices(obj2.vertices);
    obj1.center = glm::vec3(-0.8f, 0.0f, 0.0f);
    obj2.center = glm::vec3(0.8f,0.0f,0.0f);

    setObj(obj1);
    setObj(obj2);
    //unsigned int numVertices = vertices.size() / 3;

    // unsigned int VBO, VAO;
    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    // // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    // glBindVertexArray(VAO);
    //
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // // for gpu
    // //glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    // // cpu
    // //glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // change to 6 to account for color
    // glEnableVertexAttribArray(0);
    //
    //
    // // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float))); // change to 6 to account for color
    // // glEnableVertexAttribArray(1);
    //
    // // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    //
    // // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);


    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // get uniform location
    int gpuMatrix = glGetUniformLocation(shaderProgram, "modelmatrix");

    //std::vector<float> newVs; cpu
    // render loop
    // -----------
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // animation
        if (isOrbiting) {
            orbitAngle += 0.01f;
        }

        // Calculate the model view
        glm::mat4 modelView = glm::mat4(1.0f);
        modelView = glm::translate(modelView, glm::vec3(posx, posy, posz));
        modelView = glm::rotate(modelView, rotX, glm::vec3(1.0f, 0.0f, 0.0f));
        modelView = glm::rotate(modelView, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
        modelView = glm::rotate(modelView, rotZ, glm::vec3(0.0f, 0.0f, 1.0f));
        modelView = glm::scale(modelView, glm::vec3(global_scale, global_scale, global_scale));

        glm::vec3 midpoint = (obj1.center + obj2.center) * 0.5f;
        glm::vec3 axis = glm::normalize(obj2.center - obj1.center);

        glm::mat4 orbit = glm::mat4(1.0f);
        orbit = glm::translate(orbit, midpoint);
        orbit = glm::rotate(orbit, orbitAngle, axis);
        orbit = glm::translate(orbit, -midpoint);

        // apply orbit then user controls
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, obj1.center);
        model1 = orbit * model1;
        model1 = modelView * model1;

        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, obj2.center);
        model2 = orbit * model2;
        model2 = modelView * model2;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Make sure Depth Buffer is cleared too!

        glUseProgram(shaderProgram);

        // Draw first obj
        glBindVertexArray(obj1.VAO);
        glUniformMatrix4fv(gpuMatrix, 1, GL_FALSE, glm::value_ptr(model1));
        glDrawArrays(GL_TRIANGLES, 0, obj1.numVertices);

        // Draw second obj
        glBindVertexArray(obj2.VAO);
        glUniformMatrix4fv(gpuMatrix, 1, GL_FALSE, glm::value_ptr(model2));
        glDrawArrays(GL_TRIANGLES, 0, obj2.numVertices);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // translation with WASD
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) posx -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) posx += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) posy += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) posy -= 0.01f;

    // rotation with arrow keys
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  rotY += 0.02f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rotY -= 0.02f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    rotX += 0.02f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  rotX -= 0.02f;

    // z-axis rotation
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) rotZ += 0.02f;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) rotZ -= 0.02f;

    // scaling
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) global_scale -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) global_scale += 0.01f;

    if (global_scale < 0.05f) global_scale = 0.05f;

    // animation toggle
    bool space = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (spacePressed && !space) {
        isOrbiting = !isOrbiting;
    }
    spacePressed = space;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}