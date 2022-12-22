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

int FRAME_WIDTH = 768;
int FRAME_HEIGHT = 576;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void gameLoop(GLFWwindow* window);

void updateGodViewMat();
void updatePlayerViewMat();
void recalculateLocalZ();
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

glm::mat4 godProjMat;
glm::mat4 godViewMat;
glm::mat4 playerProjMat;
glm::mat4 playerViewMat;

glm::vec3 godEye(0.0, 50.0, 20.0), godViewDir(0.0, -30.0, -30.0), godUp(0.0, 1.0, 0.0);
glm::vec3 godLocalX(-1, 0, 0), godLocalY(0, 1, 0), godLocalZ(0, 0, -1);
glm::vec3 playerEye(0.0, 8.0, 10.0), playerCenter(0.0, 5.0, 0.0), playerUp(0.0, 1.0, 0.0);
glm::vec3 playerLocalZ(0, 0, -1);

// ==============================================

Model mergedGrass, slime;
int numSamples[3];
const float* samplePositions[3];
int totalInstanceCount;

GLuint vao, raw_ssbo, valid_ssbo, drawCmd_ssbo;

enum Key { KEY_W, KEY_A, KEY_S, KEY_D };
bool keyDown[4] = { false, false, false, false };
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

void initGrass()
{
    std::vector<Model> grasses(3);

    grasses[0] = Model("assets/grassB.obj", "assets/grassB_albedo.png");
    grasses[1] = Model("assets/bush01_lod2.obj", "assets/bush01.png");
    grasses[2] = Model("assets/bush05_lod2.obj", "assets/bush05.png");

    mergedGrass = Model::merge(grasses);
}

struct InstanceProperties
{
    glm::vec4 position;
};

void initSSBO()
{
    InstanceProperties* rawInsData = new InstanceProperties[totalInstanceCount];

    for (int i = 0, j = 0; j < 3; ++j)
    {
        auto positions = samplePositions[j];
        for (int k = 0; k < numSamples[j]; ++i, ++k)
        {
            rawInsData[i].position = glm::vec4(
                positions[k * 3],
                positions[k * 3 + 1],
                positions[k * 3 + 2], j);
        }
    }

    glGenBuffers(1, &raw_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, raw_ssbo);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, totalInstanceCount * 4 * sizeof(int),
        rawInsData, GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, raw_ssbo);

    glGenBuffers(1, &valid_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, valid_ssbo);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, totalInstanceCount * 4 * sizeof(int),
        nullptr, GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, valid_ssbo);
}

void initInstancedSettings()
{
    GLuint offsetHandel = SceneManager::Instance()->m_offsetHandel;

    glBindVertexArray(mergedGrass.shape.vao);
    glBindBuffer(GL_ARRAY_BUFFER, valid_ssbo);
    glEnableVertexAttribArray(offsetHandel);
    glVertexAttribPointer(offsetHandel, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribDivisor(offsetHandel, 1);

    glBindVertexArray(0);
}

struct DrawCommand
{
    unsigned int count;
    unsigned int instanceCount;
    unsigned int firstIndex;
    unsigned int baseVertex;
    unsigned int baseInstance;
};

void genDrawCommands()
{
    const int numCmd = 3;
    DrawCommand cmdList[numCmd];

    unsigned int offset = 0;
    unsigned int baseInst = 0;

    for (int i = 0; i < numCmd; i++)
    {
        cmdList[i].count = mergedGrass.drawCounts[i];
        cmdList[i].instanceCount = numSamples[i];
        cmdList[i].firstIndex = offset;
        cmdList[i].baseVertex = mergedGrass.baseVertices[i];
        cmdList[i].baseInstance = baseInst;

        offset += mergedGrass.drawCounts[i];
        baseInst += numSamples[i];
    }

    GLuint indirectBufHandle;

    glGenBuffers(1, &indirectBufHandle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indirectBufHandle);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(DrawCommand) * numCmd, cmdList, GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, indirectBufHandle);

    glBindVertexArray(mergedGrass.shape.vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBufHandle);
    glBindVertexArray(0);
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
    // godViewMat = glm::lookAt(glm::vec3(0.0, 50.0, 20.0), glm::vec3(0.0, 20.0, -10.0), glm::vec3(0.0, 1.0, 0.0));
    godLocalZ = glm::normalize(godViewDir);
    godLocalY = glm::normalize(glm::cross(godLocalZ, godLocalX));
    updateGodViewMat();
    updatePlayerViewMat();

    const glm::vec4 directionalLightDir = glm::vec4(0.4, 0.5, 0.8, 0.0);

    defaultRenderer->setDirectionalLightDir(directionalLightDir);

    // =================================================================
    resize(FRAME_WIDTH, FRAME_HEIGHT);
    m_imguiPanel = new MyImGuiPanel();

    // =================================================================	
    // load objs, init buffers
    initGrass();
    initSSBO();
    initInstancedSettings();
    genDrawCommands();

    return true;
}

void resizeGL(GLFWwindow* window, int w, int h)
{
    resize(w, h);
}

void updateGodViewMat()
{
    godViewMat = glm::lookAt(godEye, godEye + godViewDir, godUp);
}

void updatePlayerViewMat()
{
    const float translateSpeed = 0.1f, rotateSpeed = 0.5f;
    const glm::vec3 translateAmount = translateSpeed * playerLocalZ;

    if (keyDown[KEY_W])
    {
        playerEye += translateAmount;
        playerCenter += translateAmount;
    }
    else if (keyDown[KEY_S])
    {
        playerEye -= translateAmount;
        playerCenter -= translateAmount;
    }

    if (keyDown[KEY_A])
    {
        playerCenter = rotateCenterAccordingToEye(
            playerCenter, playerEye, playerViewMat, glm::radians(rotateSpeed));
        recalculateLocalZ();
    }
    else if (keyDown[KEY_D])
    {
        playerCenter = rotateCenterAccordingToEye(
            playerCenter, playerEye, playerViewMat, glm::radians(-rotateSpeed));
        recalculateLocalZ();
    }

    playerViewMat = glm::lookAt(playerEye, playerCenter, playerUp);
}

void recalculateLocalZ()
{
    playerLocalZ = playerCenter - playerEye;
    playerLocalZ.y = 0;
    playerLocalZ = glm::normalize(playerLocalZ);
}

void drawGrass()
{
    auto sm = SceneManager::Instance();

    defaultRenderer->useProgram();
    glUniform1i(sm->m_instancedDrawHandle, 1);
    glBindVertexArray(mergedGrass.shape.vao);

    glActiveTexture(sm->m_albedoTexUnit);
    glUniform1i(sm->m_fs_albedoTexHandle, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mergedGrass.material.diffuse_tex);

    glUniform1i(sm->m_fs_pixelProcessIdHandle, sm->m_fs_textureMapping);
    glm::mat4 id(1);
    glUniformMatrix4fv(sm->m_modelMatHandle, 1, false, glm::value_ptr(id));

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, 3, 0);

    glBindVertexArray(0);
    glUniform1i(sm->m_instancedDrawHandle, 0);
}

void paintGL()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    updateGodViewMat();
    updatePlayerViewMat();
    // ===============================
    // start new frame
    defaultRenderer->startNewFrame();

    defaultRenderer->setProjection(playerProjMat);
    defaultRenderer->setView(playerViewMat);
    defaultRenderer->renderPass();
    drawGrass();
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
    if (leftButtonPressed)
    {
        using namespace glm;
        glm::vec2 changed = glm::vec2(x, y) - lastCursorPos;
        vec2 change = 0.2f * (lastCursorPos - vec2(x, y));

        int sign = (godViewDir.z > 0) ? -1 : 1;
        mat4 R = mat4_cast(quat(vec3(radians(sign * change.y), radians(change.x), 0)));

        godViewDir = (R * vec4(godViewDir, 1)).xyz;

        godLocalZ = (R * vec4(godLocalZ, 1)).xyz;
        godLocalX = normalize(cross(godUp, godLocalZ));
        godLocalY = normalize(cross(godLocalZ, godLocalX));
    }
    lastCursorPos = glm::vec2(x, y);
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
    }
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // printf("%f, %f\n", xoffset, yoffset);
    godEye += (float)yoffset * godLocalZ;
}

void resize(const int w, const int h)
{
    FRAME_WIDTH = w;
    FRAME_HEIGHT = h;

    // half for god view, half for player view
    const int HALF_W = w * 0.5;
    const double PLAYER_PROJ_FAR = 150.0;

    godProjMat = glm::perspective(glm::radians(75.0), HALF_W * 1.0 / h, 0.1, 500.0);
    playerProjMat = glm::perspective(glm::radians(45.0), HALF_W * 1.0 / h, 0.1, PLAYER_PROJ_FAR);

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
