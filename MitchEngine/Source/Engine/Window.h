// 2018 Mitchell Andrews
#pragma once
#include <iostream>
#include <string>

#if ME_PLATFORM_WIN64
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#ifdef MAN_ENABLE_RENDERDOC
#include "Utility/Debug/RenderDocManager.h"
#endif
class Window
{
private:
public:
	static int WINDOW_HEIGHT, WINDOW_WIDTH;
	Window(std::string title, int width = WINDOW_WIDTH, int height = WINDOW_HEIGHT);
	~Window();

#if ME_PLATFORM_WIN64

	bool ShouldClose();
	void PollInput();
	void Swap();

private:
	void MouseCallback(GLFWwindow* window, double xpos, double ypos);
	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void ProcessInput(GLFWwindow *window);
	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

private:
	GLFWwindow * window;
#endif

#ifdef MAN_ENABLE_RENDERDOC
	RenderDocManager* RenderDoc;
#endif
};

