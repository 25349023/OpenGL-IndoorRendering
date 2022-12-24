#define GLM_FORCE_SWIZZLE

#include "src\Shader.h"
#include "src\RenderSetting.h"
#include <GLFW\glfw3.h>
#include "src\MyImGuiPanel.h"

#include "glm/gtx/quaternion.hpp"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "src/DeferredRenderer.h"

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
RenderSetting* renderSetting = nullptr;
DeferredRenderer* deferredRenderer = nullptr;
// ShaderProgram* framebufShaderProgram = nullptr;

glm::mat4 playerProjMat;
glm::mat4 playerViewMat;

glm::vec3 playerUp(0.0, 1.0, 0.0);
glm::vec3 playerLocalZ(0, 0, -1), playerLocalY(0, 1, 0);

// ==============================================

Model scene, trice;

GLuint vao, raw_ssbo, valid_ssbo, drawCmd_ssbo;

enum Key { KEY_W, KEY_A, KEY_S, KEY_D, KEY_Z, KEY_X };
bool keyDown[6] = {};
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

    ImFontConfig config;
    config.SizePixels = 16;
    io.Fonts->AddFontDefault(&config);

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
    scene.setTransform(glm::vec3(-20, 0.0, -35), glm::vec3(0, glm::radians(-80.0f), 0), glm::vec3(10.0));

    trice = Model("assets/indoor/trice.obj", "assets/indoor/");
    trice.setTransform(glm::vec3(2.0, 6.3, -17), glm::vec3(0, glm::radians(-80.0f), 0), glm::vec3(0.01));
}

bool initializeGL()
{
    // initialize shader program
    ShaderProgram* shaderProgram = new ShaderProgram(
        "src\\shader\\oglVertexShader.glsl", "src\\shader\\oglFragmentShader.glsl");
    if (shaderProgram->status() != ShaderProgramStatus::READY)
    {
        return false;
    }

    ShaderProgram* screenShaderProgram = new ShaderProgram(
        "src\\shader\\fbufVertexShader.glsl", "src\\shader\\fbufFragmentShader.glsl");
    if (screenShaderProgram->status() != ShaderProgramStatus::READY)
    {
        return false;
    }

    // =================================================================
    m_imguiPanel = new MyImGuiPanel();

    // =================================================================
    // init renderer
    renderSetting = new RenderSetting();
    if (!renderSetting->initialize(FRAME_WIDTH, FRAME_HEIGHT, shaderProgram))
    {
        return false;
    }
    renderSetting->setViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);

    deferredRenderer = new DeferredRenderer(glm::ivec2(FRAME_WIDTH, FRAME_HEIGHT));
    deferredRenderer->fbufShaderProgram = shaderProgram;
    deferredRenderer->screenShaderProgram = screenShaderProgram;

    // =================================================================
    // initialize camera
    updatePlayerViewMat();
    resize(FRAME_WIDTH, FRAME_HEIGHT);

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
        m_imguiPanel->camEye += translateZAmount;
        m_imguiPanel->camCenter += translateZAmount;
    }
    else if (keyDown[KEY_S])
    {
        m_imguiPanel->camEye -= translateZAmount;
        m_imguiPanel->camCenter -= translateZAmount;
    }

    if (keyDown[KEY_Z])
    {
        m_imguiPanel->camEye += translateYAmount;
        m_imguiPanel->camCenter += translateYAmount;
    }
    else if (keyDown[KEY_X])
    {
        m_imguiPanel->camEye -= translateYAmount;
        m_imguiPanel->camCenter -= translateYAmount;
    }

    if (keyDown[KEY_A])
    {
        m_imguiPanel->camCenter = rotateCenterAccordingToEye(
            m_imguiPanel->camCenter, m_imguiPanel->camEye, playerViewMat, glm::radians(rotateSpeed));
        recalculatePlayerLocals();
    }
    else if (keyDown[KEY_D])
    {
        m_imguiPanel->camCenter = rotateCenterAccordingToEye(
            m_imguiPanel->camCenter, m_imguiPanel->camEye, playerViewMat, glm::radians(-rotateSpeed));
        recalculatePlayerLocals();
    }

    playerViewMat = glm::lookAt(m_imguiPanel->camEye, m_imguiPanel->camCenter, playerUp);
}

void recalculatePlayerLocals()
{
    playerLocalZ = m_imguiPanel->camCenter - m_imguiPanel->camEye;
    playerLocalZ.y = 0;
    playerLocalZ = glm::normalize(playerLocalZ);

    glm::vec3 side = glm::cross(playerLocalZ, playerUp);
    playerLocalY = glm::normalize(glm::cross(side, playerLocalZ));
}

void paintGL()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    updatePlayerViewMat();
    // ===============================
    // start new frame
    deferredRenderer->beforeFirstStage();
    
    renderSetting->setProjection(playerProjMat);
    renderSetting->setView(playerViewMat);
    renderSetting->prepareUniform();

    scene.render();
    trice.render();

    deferredRenderer->secondStage();

    // ===============================

    ImGui::Begin("Toolbox");
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
{}

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
{}

void resize(const int w, const int h)
{
    FRAME_WIDTH = w;
    FRAME_HEIGHT = h;

    const double PLAYER_PROJ_FAR = 150.0;

    playerProjMat = glm::perspective(glm::radians(45.0), w * 1.0 / h, 0.1, PLAYER_PROJ_FAR);

    renderSetting->resize(w, h);
    deferredRenderer->updateWindowSize(glm::ivec2(w, h));
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
