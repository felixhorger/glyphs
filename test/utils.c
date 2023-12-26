
GLFWwindow* open_window(int width, int height)
{
	GLFWwindow* window = glfwCreateWindow(width, height, "", NULL, NULL); //glfwGetPrimaryMonitor()
	if (!window) {
		printf("Error: could not open window");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	return window;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	return;
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error: %s\n", description);
	exit(EXIT_FAILURE);
	return;
}

