#define GLM_FORCE_SWIZZLE

#include <array>

#include "src\Shader.h"
#include "src\SceneRenderer.h"
#include <GLFW\glfw3.h>
#include "src\MyImGuiPanel.h"

#include "glm/gtx/quaternion.hpp"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "src/Shape.h"
#include "src/Material.h"
#include "src/Model.h"

#pragma comment (lib, "lib-vc2015\\glfw3.lib")
#pragma comment(lib, "assimp-vc141-mt.lib")

int FRAME_WIDTH = 960;
int FRAME_HEIGHT = 640;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void gameLoop(GLFWwindow* window);

void updatePlayerViewMat();
void recalculatePlayerLocals();
bool initializeGL();
void resizeGL(GLFWwindow* window, int w, int h);
void paintGL();
void resize(const int w, const int h);

glm::vec3 rotateCenterAccordingToEye(const glm::vec3& center, const glm::vec3& eye,
                                     const glm::mat4& viewMat, const float rad);

bool leftButtonPressed = false;
bool rightButtonPressed = false;

glm::vec2 lastCursorPos;

MyImGuiPanel* m_imguiPanel = nullptr;


// ==============================================
SceneRenderer* defaultRenderer = nullptr;
ShaderProgram* defaultShaderProgram = new ShaderProgram();

glm::mat4 playerProjMat;
glm::mat4 playerViewMat;

glm::vec3 playerEye(0.0, 9.0, 10.0), playerCenter(0.0, 9.0, 0.0), playerUp(0.0, 1.0, 0.0);
glm::vec3 playerLocalZ(0, 0, -1), playerLocalY(0, 1, 0);

// ==============================================

Model scene;

GLuint vao, raw_ssbo, valid_ssbo, drawCmd_ssbo;

enum Key { KEY_W, KEY_A, KEY_S, KEY_D, KEY_Z, KEY_X };
bool keyDown[6] = { };
// ==============================================

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(FRAME_WIDTH, FRAME_HEIGHT, "111062566_AS3", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // load OpenGL function pointer
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetFramebufferSizeCallback(window, resizeGL);

    if (initializeGL() == false)
    {
        std::cout << "initialize GL failed\n";
        glfwTerminate();
        system("pause");
        return 0;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    // start game-loop
    gameLoop(window);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void gameLoop(GLFWwindow* window)
{
    double previousTimeForFPS = glfwGetTime();
    int frameCount = 0;

    while (!glfwWindowShouldClose(window))
    {
        // measure speed
        const double currentTime = glfwGetTime();
        frameCount = frameCount + 1;
        const double deltaTime = currentTime - previousTimeForFPS;
        if (deltaTime >= 1.0)
        {
            m_imguiPanel->setAvgFPS(frameCount * 1.0);
            m_imguiPanel->setAvgFrameTime(deltaTime * 1000.0 / frameCount);

            // reset
            frameCount = 0;
            previousTimeForFPS = currentTime;
        }

        glfwPollEvents();
        paintGL();
        glfwSwapBuffers(window);
    }
}

void initScene()
{
    scene = Model("assets/indoor/Grey_White_Room.obj", "assets/indoor/");
}

bool initializeGL()
{
    // initialize shader program
    // vertex shader
    Shader* vsShader = new Shader(GL_VERTEX_SHADER);
    vsShader->createShaderFromFile("src\\shader\\oglVertexShader.glsl");
    std::cout << vsShader->shaderInfoLog() << "\n";

    // fragment shader
    Shader* fsShader = new Shader(GL_FRAGMENT_SHADER);
    fsShader->createShaderFromFile("src\\shader\\oglFragmentShader.glsl");
    std::cout << fsShader->shaderInfoLog() << "\n";

    // shader program
    ShaderProgram* shaderProgram = new ShaderProgram();
    shaderProgram->init();
    shaderProgram->attachShader(vsShader);
    shaderProgram->attachShader(fsShader);
    shaderProgram->checkStatus();
    if (shaderProgram->status() != ShaderProgramStatus::READY)
    {
        return false;
    }
    shaderProgram->linkProgram();

    vsShader->releaseShader();
    fsShader->releaseShader();

    delete vsShader;
    delete fsShader;

    // =================================================================
    // init renderer
    defaultRenderer = new SceneRenderer();
    if (!defaultRenderer->initialize(FRAME_WIDTH, FRAME_HEIGHT, shaderProgram))
    {
        return false;
    }
    defaultRenderer->setViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);

    // =================================================================
    // initialize camera
    updatePlayerViewMat();

    const glm::vec4 directionalLightDir = glm::vec4(0.4, 0.5, 0.8, 0.0);

    defaultRenderer->setDirectionalLightDir(directionalLightDir);

    // =================================================================
    resize(FRAME_WIDTH, FRAME_HEIGHT);
    m_imguiPanel = new MyImGuiPanel();

    // =================================================================	
    // load objs, init buffers
    initScene();

    return true;
}

void resizeGL(GLFWwindow* window, int w, int h)
{
    resize(w, h);
}

void updatePlayerViewMat()
{
    const float translateSpeed = 0.01f, rotateSpeed = 0.05f;
    const glm::vec3 translateZAmount = translateSpeed * playerLocalZ;
    const glm::vec3 translateYAmount = translateSpeed * playerLocalY;

    if (keyDown[KEY_W])
    {
        playerEye += translateZAmount;
        playerCenter += translateZAmount;
    }
    else if (keyDown[KEY_S])
    {
        playerEye -= translateZAmount;
        playerCenter -= translateZAmount;
    }

    if (keyDown[KEY_Z])
    {
        playerEye += translateYAmount;
        playerCenter += translateYAmount;
    }
    else if (keyDown[KEY_X])
    {
        playerEye -= translateYAmount;
        playerCenter -= translateYAmount;
    }

    if (keyDown[KEY_A])
    {
        playerCenter = rotateCenterAccordingToEye(
            playerCenter, playerEye, playerViewMat, glm::radians(rotateSpeed));
        recalculatePlayerLocals();
    }
    else if (keyDown[KEY_D])
    {
        playerCenter = rotateCenterAccordingToEye(
            playerCenter, playerEye, playerViewMat, glm::radians(-rotateSpeed));
        recalculatePlayerLocals();
    }

    playerViewMat = glm::lookAt(playerEye, playerCenter, playerUp);
}

void recalculatePlayerLocals()
{
    playerLocalZ = playerCenter - playerEye;
    playerLocalZ.y = 0;
    playerLocalZ = glm::normalize(playerLocalZ);

    glm::vec3 side = glm::cross(playerLocalZ, playerUp);
    playerLocalY = glm::normalize(glm::cross(side, playerLocalZ));
}

void drawScene()
{
    auto sm = SceneManager::Instance();

    defaultRenderer->useProgram();
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(sm->m_fs_albedoTexHandle, 0);

    glm::mat4 id(1);
    id = glm::scale(id, glm::vec3(10.0));
    glm::quat q = glm::angleAxis(glm::radians(-80.0f), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 rotMat = glm::toMat4(q);

    id = rotMat * id;
    id = glm::translate(id, glm::vec3(-3.5, 0.0, 1.5));
    glUniformMatrix4fv(sm->m_modelMatHandle, 1, false, glm::value_ptr(id));

    for (const auto& shape : scene.shapes)
    {
        glBindVertexArray(shape.vao);
        Material& material = scene.materials[shape.materialId];

        if (material.hasTex)
        {
            glUniform1i(sm->m_fs_pixelProcessIdHandle, sm->m_fs_textureMapping);
        }
        else
        {
            glUniform1i(sm->m_fs_pixelProcessIdHandle, sm->m_fs_simpleShading);
        }

        glUniform3fv(sm->m_fs_kaHandle, 1, glm::value_ptr(material.ambient));
        glUniform3fv(sm->m_fs_kdHandle, 1, glm::value_ptr(material.diffuse));
        glUniform3fv(sm->m_fs_ksHandle, 1, glm::value_ptr(material.specular));

        if (material.hasTex)
        {
            glBindTexture(GL_TEXTURE_2D, material.diffuseTex);
        }
        glDrawElements(GL_TRIANGLES, shape.drawCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void paintGL()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    updatePlayerViewMat();
    // ===============================
    // start new frame
    defaultRenderer->startNewFrame();

    defaultRenderer->setProjection(playerProjMat);
    defaultRenderer->setView(playerViewMat);
    defaultRenderer->renderPass();
    drawScene();
    // ===============================

    ImGui::Begin("FPS Info");
    m_imguiPanel->update();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

////////////////////////////////////////////////
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            leftButtonPressed = true;
        }
        else
        {
            leftButtonPressed = false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            rightButtonPressed = true;
        }
        else
        {
            rightButtonPressed = false;
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case GLFW_KEY_W:
        keyDown[KEY_W] = action != GLFW_RELEASE;
        break;
    case GLFW_KEY_S:
        keyDown[KEY_S] = action != GLFW_RELEASE;
        break;
    case GLFW_KEY_A:
        keyDown[KEY_A] = action != GLFW_RELEASE;
        break;
    case GLFW_KEY_D:
        keyDown[KEY_D] = action != GLFW_RELEASE;
        break;
    case GLFW_KEY_Z:
        keyDown[KEY_Z] = action != GLFW_RELEASE;
        break;
    case GLFW_KEY_X:
        keyDown[KEY_X] = action != GLFW_RELEASE;
        break;
    }
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
}

void resize(const int w, const int h)
{
    FRAME_WIDTH = w;
    FRAME_HEIGHT = h;

    const double PLAYER_PROJ_FAR = 150.0;

    playerProjMat = glm::perspective(glm::radians(45.0), w * 1.0 / h, 0.1, PLAYER_PROJ_FAR);

    defaultRenderer->resize(w, h);
}

glm::vec3 rotateCenterAccordingToEye(const glm::vec3& center, const glm::vec3& eye,
                                     const glm::mat4& viewMat, const float rad)
{
    glm::mat4 vt = glm::transpose(viewMat);
    glm::vec4 yAxisVec4 = vt[1];
    glm::vec3 yAxis(yAxisVec4.x, yAxisVec4.y, yAxisVec4.z);
    glm::quat q = glm::angleAxis(rad, yAxis);
    glm::mat4 rotMat = glm::toMat4(q);
    glm::vec3 p = center - eye;
    glm::vec4 resP = rotMat * glm::vec4(p.x, p.y, p.z, 1.0);
    return glm::vec3(resP.x + eye.x, resP.y + eye.y, resP.z + eye.z);
}
