#include <stdlib.h>
#include <stdio.h>
#include "../../glaze/include/glaze.h"
#include "../include/glyphs.h"
#include <GLFW/glfw3.h>


#include "utils.c"


int main(int argc, char** argv)
{
	if (!glfwInit()) {
		printf("Error: could not initialise GLFW");
		exit(EXIT_FAILURE);
	}

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = open_window(800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	gladLoadGL();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(glErrorCallback, NULL);
	glClearColor(0, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glyInit();

	float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLYfont font = glyLoadFont("/usr/share/fonts/truetype/dejavu/DejaVuMathTeXGyre.ttf");

	while (!glfwWindowShouldClose(window)) {

		glfwWaitEvents();

		glClear(GL_COLOR_BUFFER_BIT);
		char text[] = "jYUSDFBSPCIJEQWOTLNCX";
		glyDrawText(font, text, sizeof(text)-1, -1.0f, 0.0f, 0.15f, color);
		char text1[] = "qwertyyuiopapasdfhgjlklzxcvmjklz";
		glyDrawText(font, text1, sizeof(text1)-1, -1.0f, 0.2f, 0.15f, color);
		char text2[] = "1234567890!@#$%&*)_-=+[{.>?";
		glyDrawText(font, text2, sizeof(text2)-1, -1.0f, 0.4f, 0.15f, color);
		//char text[] = "ly";
		//glDrawText(font, text, sizeof(text)-1, -0.8f, 0.2f, 0.1f, color);

		glHandleErrors("Error in main loop");

		glFlush();
		glfwSwapBuffers(window);
	}

	glyDeleteFont(font);
	glyFinish();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

