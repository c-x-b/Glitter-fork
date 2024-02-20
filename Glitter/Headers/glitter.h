// Preprocessor Directives
#ifndef GLITTER_H
#define GLITTER_H

// System Headers
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <btBulletDynamicsCommon.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include <camera.h>

#include <iostream>
#include <cstring>

// Define Some Constants
float deltaTime, lastFrame = 0.0f;
bool firstMouse = true;
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
Camera camera(glm::vec3(0.0f, 0.0f, 150.0f));

const float earthRadius = 6360e3f;
const float atmosphereRadius = earthRadius * 1.015f;

const float scale = 1e5f;
const float earthScale = earthRadius / scale;
const float atmosphereScale = atmosphereRadius / scale;
const float atmosphereThicknessScale = atmosphereScale - earthScale;

bool mode = true;

bool mouses[16];
bool keys[1024];
bool keysPressed[1024];
// Moves/alters the camera positions based on user input
void Do_Movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_Q]) {
        camera.Position = glm::normalize(camera.Position) * (6362000.0f / scale);
    }
    if (keys[GLFW_KEY_E]) {
        std::cout << glm::length(camera.Position) << " " << earthRadius << std::endl;
    }
    if (keys[GLFW_KEY_R]) {
        camera.Up = glm::normalize(camera.Position);
    }
    if (keys[GLFW_KEY_T]) {
        camera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (keys[GLFW_KEY_1]) {
        mode = true;
    }
    if (keys[GLFW_KEY_2]) {
        mode = false;
    }
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key <= 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
        {
            keys[key] = false;
            keysPressed[key] = false;
        }
    }
}

// Moves/alters the camera positions based on user input
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    if (mouses[GLFW_MOUSE_BUTTON_RIGHT])
        camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button>=0 && button<=GLFW_MOUSE_BUTTON_LAST) 
    {
        if (action == GLFW_PRESS) 
        {
            mouses[button] = true;
        }
        else 
        {
            mouses[button] = false;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// unsigned int loadTexture(const char *path)
// {
//     std::string actualPath = "../../Glitter/Resources/"+ std::string(path);
//     unsigned int textureID;
//     glGenTextures(1, &textureID);
    
//     int width, height, nrComponents;
//     unsigned char *data = stbi_load(actualPath.c_str(), &width, &height, &nrComponents, 0);
//     if (data)
//     {
//         GLenum format;
//         if (nrComponents == 1)
//             format = GL_RED;
//         else if (nrComponents == 3)
//             format = GL_RGB;
//         else if (nrComponents == 4)
//             format = GL_RGBA;

//         glBindTexture(GL_TEXTURE_2D, textureID);
//         glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//         glGenerateMipmap(GL_TEXTURE_2D);

//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//         stbi_image_free(data);
//     }
//     else
//     {
//         std::cout << "Texture failed to load at path: " << actualPath << std::endl;
//         stbi_image_free(data);
//     }

//     return textureID;
// }

#endif //~ Glitter Header
