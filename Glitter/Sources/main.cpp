// Local Headers
#include "glitter.h"
#include "shader.h"
#include "sphere.h"
#include "atmosphere.h"
#include "textureManager.h"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <iostream>

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

//glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::fvec3 lightDir(0.0f, 0.0f, 1.0f);
glm::fvec3 lightColor(1.0f, 1.0f, 1.0f);
TextureManager &textureManager = TextureManager::GetInstance();

const float const3Divide16PI = 3.0f / (16.0f * PI);

float quadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f
};

void setAtmosphereShader(Shader &shader, glm::mat4 &projection, glm::mat4 &view) {
    double cameraHeight = glm::length(camera.Position) * scale;
    shader.use();

    shader.setVec3("cameraPos", camera.Position * scale);
    shader.setFloat("cameraHeight", cameraHeight);
    shader.setFloat("cameraHeight2", cameraHeight * cameraHeight);
    shader.setVec3("sunLightDir", lightDir);
    shader.setFloat("atmosphereRadius", atmosphereRadius);
    shader.setFloat("atmosphereRadius2", atmosphereRadius * atmosphereRadius);
    shader.setFloat("earthRadius", earthRadius);
    shader.setFloat("const3Divide16PI", const3Divide16PI);
    shader.setFloat("PI", PI);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    shader.setVec3("lightColor", lightColor * 10.0f);
    shader.setBool("mode", mode);
}

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    //glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    glfwSetKeyCallback(mWindow, key_callback);
    glfwSetCursorPosCallback(mWindow, cursor_pos_callback);
    glfwSetMouseButtonCallback(mWindow, mouse_button_callback);
    glfwSetScrollCallback(mWindow, scroll_callback);

    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetInputMode(mWindow, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glCheckError();
    glCheckError();

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glEnable(GL_MULTISAMPLE);

    std::vector<std::pair<std::string, std::string>> textures;

    Shader shader("EarthVertex.vert", "EarthFragment.frag");
    Shader outAtmosphereShader("AtmosphereVertex.vert", "OutAtmosphereFragment.frag");
    Shader insideAtmosphereShader("AtmosphereVertex.vert", "InsideAtmosphereFragment.frag");
    // Shader atmosphereShader("NewAtmosphereVertex.vert", "NewAtmosphereFragment.frag");
    Shader testShader("BasicVertex.vert", "BasicFragment.frag");

    textureManager.LoadTexture2D("Textures/earth_day_8k.jpg", "EarthDay");
    textures.clear();
    textures.push_back(std::make_pair("EarthDay", "sphereTexture"));

    Sphere *earth = new Sphere(earthRadius, 50, 50);
    earth->generateMesh();
    earth->setPosition(glm::fvec3(0.0f, 0.0f, 0.0f));
    earth->setScale(glm::fvec3(earthScale, earthScale, earthScale));
    //earth->setDiffuse(glm::fvec3(1.0f, 0.941f, 0.898f));
    earth->setTextures(textures);
    earth->initBuffer(shader);

    atmosphereSphere *atmosphere = new atmosphereSphere(atmosphereRadius, 100, 100);
    atmosphere->generateMesh();
    atmosphere->setPosition(glm::fvec3(0.0f, 0.0f, 0.0f));
    atmosphere->setScale(glm::fvec3(atmosphereScale, atmosphereScale, atmosphereScale));
    atmosphere->setEarthRadius(earthRadius);
    atmosphere->calcLookUpTable();
    atmosphere->generateLUTTexture(textureManager);
    atmosphere->initBuffer();

    unsigned int testVAO;
    glGenVertexArrays(1, &testVAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindVertexArray(testVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL( mWindow, true );
    ImGui_ImplOpenGL3_Init();

    float insideAtmosMethodBoundary = 0.0f;

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) 
    {
        // Necessary Calc
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        Do_Movement();

        float angle = currentFrame / 15 * PI;
        //lightDir = glm::fvec3(cos(angle), 0.0f, sin(angle));


        // ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        ImGui::Begin("Test");
        ImGui::SliderFloat("boundaryRatio", &insideAtmosMethodBoundary, 0.0f, 1.0f);
        float temp = (glm::length(camera.Position) - earthScale) / (atmosphereScale - earthScale);
        ImGui::Text("cameraRatio=%.2f", temp);
        ImGui::End();

        ImGui::Render();
        

        // Render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.0f);
        glm::mat4 view = camera.GetViewMatrix();

        glCullFace(GL_BACK);
        shader.use();
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("light.direction", lightDir);
        shader.setVec3("light.color", lightColor);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        glCheckError();
        earth->render(shader, textureManager);
        glCheckError();
 
        glCullFace(GL_FRONT);
        if (glm::length(camera.Position) >= atmosphereScale) {
            setAtmosphereShader(outAtmosphereShader, projection, view);
            glCheckError();
            atmosphere->render(outAtmosphereShader, textureManager);
            glCheckError();
        }
        else {
            setAtmosphereShader(insideAtmosphereShader, projection, view);
            glCheckError();
            insideAtmosphereShader.use();
            insideAtmosphereShader.setFloat("renderBoundary", insideAtmosMethodBoundary);
            atmosphere->render(insideAtmosphereShader, textureManager);
            glCheckError();
        }

        // glm::mat4 projection_T = glm::inverse(projection);
        // glm::mat4 view_T = glm::inverse(view);
        // glCullFace(GL_BACK);
        // testShader.use();
        // //testShader.setMat4("view_T", view_T);
        // //testShader.setMat4("projection_T", projection_T);
        // //testShader.setVec3("cameraPos", camera.Position);
        // glCheckError();
        // textureManager.BindTextureRec("AtmosphereRLUT", "texture1", testShader);
        // glCheckError();
        // glBindVertexArray(testVAO);
        // glCheckError();
        // glDrawArrays(GL_TRIANGLES, 0, 6);
        // glCheckError();
        // textureManager.unbindAllTextures();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return EXIT_SUCCESS;
}
