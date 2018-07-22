#include "PCH.h"
#include "Engine/Window.h"
#include "Engine/Input.h"
#include "Utility/Logger.h"
#include <glad.c>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assert.h>

int Window::WINDOW_WIDTH = 960;
int Window::WINDOW_HEIGHT = 540;

Window::Window(std::string title, int width, int height)
{
	WINDOW_HEIGHT = height;
	WINDOW_WIDTH = width;

	// Init GLFW
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr); // Windowed
	if (!window)
	{
		Logger::GetInstance().Log(Logger::LogType::Error, "Failed to create window.");
		glfwTerminate();
		assert(0);
	}

#if MAN_ENABLE_RENDERDOC
	RenderDoc = new RenderDocManager();
#endif
	
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, Window::FramebufferSizeCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		assert(0);
	}
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetKeyCallback(window, &Input::KeyCallback);
	glfwSetCursorPosCallback(window, &Input::MouseCallback);
	glfwSetScrollCallback(window, &Input::ScrollCallback);
}

Window::~Window()
{
}

bool Window::ShouldClose()
{
	return (glfwWindowShouldClose(window) == 1) ? true : false;
}


void Window::PollInput()
{
	glfwPollEvents();
}
void Window::Swap()
{
	glfwSwapBuffers(window);
}

void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
