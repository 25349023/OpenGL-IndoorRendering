#pragma once
#define GLM_FORCE_SWIZZLE

#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw.h>
#include <imgui\imgui_impl_opengl3.h>

#include <string>
#include <GLM/vec3.hpp>

#include "DeferredRenderer.h"

extern DeferredRenderer *deferredRenderer;

class MyImGuiPanel
{
public:
    MyImGuiPanel();
    virtual ~MyImGuiPanel();

    void update();
};
