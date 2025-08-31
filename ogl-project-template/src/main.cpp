extern "C"{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}

#include <iostream>
#include "shader.h"
#include "vertex_buffer.h"
#include "vertex_array.h"

static const char* kWindowTitle = "OpenGL template project";
static const int kWindowWidth   = 800;
static const int kWindowHeight  = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void print_gl_version(void);

float vertices[] = {
    // posición         // color
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // vértice 1: rojo
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // vértice 2: verde
     0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f   // vértice 3: azul
};

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, nullptr, nullptr);

	if (window == nullptr){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	print_gl_version();

	glViewport(0, 0, kWindowWidth, kWindowHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Usar la clase Shader con archivos externos
	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

	// Configurar VAO con múltiples atributos
	VertexBuffer VBO(vertices, sizeof(vertices));
	VertexArray VAO;

	VAO.bind();
	VBO.bind();

	// Atributo 0: posición (coincide con layout (location = 0) en el shader)
	VAO.addVertexAttribute(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0);

	// Atributo 1: color (coincide con layout (location = 1) en el shader)
	VAO.addVertexAttribute(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	VBO.unbind();
	VAO.unbind();

	while(!glfwWindowShouldClose(window)) {
		// input
		processInput(window);

		// rendering commands here
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.use();
		VAO.bind();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		VAO.unbind();

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();
	
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	glfwSetWindowShouldClose(window, true);
}

void print_gl_version(void){
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported " << version << std::endl;
}
