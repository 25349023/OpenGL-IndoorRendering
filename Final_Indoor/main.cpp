#define GLM_FORCE_SWIZZLE

#include "src\Shader.h"
#include <GLFW\glfw3.h>
#include "src\MyImGuiPanel.h"

#include "glm/gtx/quaternion.hpp"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "src/DeferredRenderer.h"
#include "src/DirectionalShadowMapper.h"

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

void updateViewMat();
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
DeferredRenderer* deferredRenderer = nullptr;

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

    GLFWwindow* window = glfwCreateWindow(FRAME_WIDTH, FRAME_HEIGHT, "Final Indoor", nullptr, nullptr);
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
    scene.setTransform(glm::vec3(0), glm::vec3(0), glm::vec3(1.0));

    trice = Model("assets/indoor/trice.obj", "assets/indoor/");
    trice.setTransform(glm::vec3(2.05, 0.628725, -1.9), glm::vec3(0), glm::vec3(0.001f));

    deferredRenderer->appendSceneObj(&scene);
    deferredRenderer->appendSceneObj(&trice);
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

    ShaderProgram* depthShaderProgram = new ShaderProgram(
        "src\\shader\\depthVertexShader.glsl", "src\\shader\\depthFragmentShader.glsl");
    if (depthShaderProgram->status() != ShaderProgramStatus::READY)
    {
        return false;
    }

    // =================================================================
    m_imguiPanel = new MyImGuiPanel();

    // =================================================================
    // init renderer

    deferredRenderer = new DeferredRenderer(glm::ivec2(FRAME_WIDTH, FRAME_HEIGHT));
    deferredRenderer->fbufSP = shaderProgram;
    deferredRenderer->screenSP = screenShaderProgram;

    deferredRenderer->dirShadowMapper = new DirectionalShadowMapper(depthShaderProgram);

    // =================================================================
    // initialize camera
    updateViewMat();
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

void updateViewMat()
{
    const float translateSpeed = 0.01f, rotateSpeed = 0.5f;
    const glm::vec3 translateZAmount = translateSpeed * deferredRenderer->camLocalZ;
    const glm::vec3 translateYAmount = translateSpeed * deferredRenderer->camLocalY;

    if (keyDown[KEY_W])
    {
        deferredRenderer->translateCamera(translateZAmount);
    }
    else if (keyDown[KEY_S])
    {
        deferredRenderer->translateCamera(-translateZAmount);
    }

    if (keyDown[KEY_Z])
    {
        deferredRenderer->translateCamera(translateYAmount);
    }
    else if (keyDown[KEY_X])
    {
        deferredRenderer->translateCamera(-translateYAmount);
    }

    if (keyDown[KEY_A])
    {
        deferredRenderer->rotateCamera(glm::radians(rotateSpeed));
    }
    else if (keyDown[KEY_D])
    {
        deferredRenderer->rotateCamera(glm::radians(-rotateSpeed));
    }

    deferredRenderer->updateViewMat();
}

void paintGL()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    updateViewMat();
    // ===============================
    // start new frame
    deferredRenderer->shadowMapStage();
    deferredRenderer->firstStage();
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

    deferredRenderer->projMat = glm::perspective(glm::radians(60.0), w * 1.0 / h, 0.1, PLAYER_PROJ_FAR);

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
