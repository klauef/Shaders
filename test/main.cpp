#define _USE_MATH_DEFINES
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

using namespace glm;

inline void framebuffer_size_callback(GLFWwindow* window, int width, int height);
inline void mouse_callback(GLFWwindow* window, double xpos, double ypos);
inline void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
inline void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

// camera
Camera camera(glm::vec3(0.0f, 10.0f, 50.0f));
float lastX = SCR_WIDTH / 3.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// lumière
vec3 lightPos = vec3(0.0f, 1000.0f, 100.0f);
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

// shader
inline void prepareShaderCT(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle);
inline void prepareShaderCellShading(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle);
inline void prepareShaderCT(ShaderG shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle);
inline void prepareShaderTransformation(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle);
inline void prepareShaderTransformation(ShaderG shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle);

float TAILLE = 1.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

/*
depth stencil
cookTorrance
cellshading (seulement sur modèle pas de calcul de sobel)
normal map classique
test de geometric shader sans rendu
*/
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	Shader shaderCellShading("cellShadingPartial.vs", "cellShadingPartial.fs");
	Shader shaderCTPlan("cookTorrancePlan.vs", "cookTorrancePlan.fs");
	Shader shaderStencil("stencil.vs", "stencil.fs");
	ShaderG geoShader("geoShader.vs", "geoShader.fs", "geoShader.gs");

	float testGeo[] = {
	100.0f,  1.0f, 0.0f, 0.0f, 0.0f,
	-100.0f,  1.0f, 0.0f, 0.0f, 0.0f,
	};
	unsigned int VBOGeo, VAOGeo;
	glGenBuffers(1, &VBOGeo);
	glGenVertexArrays(1, &VAOGeo);
	glBindVertexArray(VAOGeo);
	glBindBuffer(GL_ARRAY_BUFFER, VBOGeo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(testGeo), &testGeo, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);

	Model dk("./../resources/objects/dk/dk.obj");
	Model sol("./../resources/objects/plan/plan.obj");

	unsigned int diffuseMapSol = TextureFromFile("brickwall.jpg", "./../resources/textures/");
	unsigned int normalMapSol = TextureFromFile("brickwall_normal.jpg", "./../resources/textures/");

	shaderCTPlan.use();
	shaderCTPlan.setInt("normalMap", 1);
	shaderCTPlan.setInt("diffuseMap", 0);

	float movementLight = 0.0f;

	while (!glfwWindowShouldClose(window))
	{

		movementLight = fmod(movementLight + deltaTime, 2 * M_PI);
		lightPos = vec3(15 * cos(movementLight), 20 + 15 * -sin(movementLight), 15 * sin(movementLight));

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |GL_STENCIL_BUFFER_BIT);

		glStencilMask(0x00);

		shaderStencil.use();
		prepareShaderTransformation(shaderStencil, vec3(20.0f, 0.0f, 0.0f), vec3(1.05f, 1.05f, 1.05f), vec3(1.0f, 1.0f, 1.0f), 0.0f);
		geoShader.use();
		prepareShaderTransformation(geoShader, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), 0.0f);
		glBindVertexArray(VAOGeo);
		glDrawArrays(GL_POINTS, 0, 4);

		prepareShaderCT(shaderCTPlan, vec3(0.0f, -1.0f, 0.0f), vec3(25.0f, 1.0f, 25.0f), vec3(1.0f, 0.0f, 0.0f),-M_PI_2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMapSol);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMapSol);
		sol.Draw(shaderCTPlan);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		prepareShaderCT(shaderCellShading, vec3(20.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), 0.0f);
		dk.Draw(shaderCellShading);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);

		shaderStencil.use();
		dk.Draw(shaderStencil);

		glStencilMask(0xFF);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

inline void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime * 10.0f);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime * 10.0f);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime * 10.0f);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime * 10.0f);
}

inline void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

inline void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

inline void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

inline void prepareShaderCT(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle) {
	shader.use();
	shader.setVec3("objectAmbiantColor", 0.1f, 0.1f, 0.1f);
	shader.setVec3("objectDiffuseColor", 1.0f, 1.0f, 1.0f);
	shader.setVec3("objectSpecularColor", 1.0f, 1.0f, 1.0f);
	shader.setVec3("lightColor", lightColor);
	shader.setVec3("lightPos", lightPos);
	shader.setVec3("viewPos", camera.Position);

	shader.setFloat("n1", 3.5);
	shader.setFloat("n2", 1.0);
	shader.setFloat("m", 0.1);

	prepareShaderTransformation(shader, translation, echelle, rotationAxe, rotationAngle);
}

inline void prepareShaderCellShading(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle) {
	shader.use();
	shader.setVec3("lightColor", lightColor);
	shader.setVec3("lightPos", lightPos);
	prepareShaderTransformation(shader, translation, echelle, rotationAxe, rotationAngle);
}

inline void prepareShaderCT(ShaderG shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle) {
	shader.use();
	shader.setVec3("objectAmbiantColor", 0.1f, 0.1f, 0.1f);
	shader.setVec3("objectDiffuseColor", 1.0f, 1.0f, 1.0f);
	shader.setVec3("objectSpecularColor", 1.0f, 1.0f, 1.0f);
	shader.setVec3("lightColor", lightColor);
	shader.setVec3("lightPos", lightPos);
	shader.setVec3("viewPos", camera.Position);

	shader.setFloat("n1", 3.5);
	shader.setFloat("n2", 1.0);
	shader.setFloat("m", 0.1);

	prepareShaderTransformation(shader, translation, echelle, rotationAxe, rotationAngle);
}

inline void prepareShaderTransformation(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle) {

	mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
	mat4 view = camera.GetViewMatrix();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);

	mat4 model = glm::mat4(1.0f);
	model = translate(model, translation);
	model = scale(model, echelle);
	model = rotate(model, rotationAngle, rotationAxe);
	shader.setMat4("model", model);
}

inline void prepareShaderTransformation(ShaderG shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle) {

	mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
	mat4 view = camera.GetViewMatrix();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);

	mat4 model = glm::mat4(1.0f);
	model = translate(model, translation);
	model = scale(model, echelle);
	model = rotate(model, rotationAngle, rotationAxe);
	shader.setMat4("model", model);
}