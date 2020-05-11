#define _USE_MATH_DEFINES
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

using namespace glm;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

// camera
Camera camera(glm::vec3(0.0f, 10.0f, 50.0f));
float lastX = SCR_WIDTH / 3.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// lumière
vec3 lightPos = vec3(0.0f, 20.0f, 0.0f);
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
float movementLight = 0.0f;

glm::mat4 lightSpaceMatrix;

//fonction préparant le shader à l'aide de constantes prédeterminées spécifiquement pour le modèle de coot-Torrance
void prepareShaderCT(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle);
//fonction préparant le shader à l'aide de constantes prédeterminées spécifiquement pour le modèle de phong
void prepareShaderBP(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle);
//fonction préparant le shader avec les matrices de projections/déformations/déplacement
void prepareShaderTransformation(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

/*
shadow map
cookTorrance
blinnPhong
parallaxMapping (encore bugué)
normal map classique
*/

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Parallax", NULL, NULL);
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

	//shader de test de profondeur depuis la lumière
	Shader shaderDepthBuffer("depthBuffer.vs", "depthBuffer.fs");
	//shader simple pour rendre un écran sur lequel on applique une texture pour du rendu différé
	Shader shaderRenderQuad("renderQuad.vs", "renderQuad.fs");
	//shader blinn phong avec ombre
	Shader shaderBP("blinnPhong.vs", "blinnPhong.fs");
	//shader coot-torrance avec ombre
	Shader shaderCT("cookTorrancePlan.vs", "cookTorrancePlan.fs");
	shaderCT.use();
	shaderCT.setInt("shadowMap", 0);
	shaderCT.setInt("diffuseMap", 1);
	shaderCT.setInt("normalMap", 2);

	//shader cook-torrance avec ombre dans le cadre d'une utilisation de parallax oclusion mapping
	Shader shaderPMCT("parallaxMappingCT.vs", "parallaxMappingCT.fs");
	shaderPMCT.use();
	shaderPMCT.setInt("diffuseMap", 0);
	shaderPMCT.setInt("normalMap", 1);
	shaderPMCT.setInt("dispMap", 2);
	shaderPMCT.setInt("shadowMap", 3);
	shaderPMCT.setFloat("heightScale",0.2f);

	//modèle donkey kong
	Model dk("./../resources/objects/dk/dk.obj");
	//modèle d'un simple plan qui servira de sol mais aussi de mur
	Model plan("./../resources/objects/plan/plan.obj");

	// texture diffuse et normal map du sol
	unsigned int diffuseMapSol = TextureFromFile("brickwall.jpg", "./../resources/textures/");
	unsigned int normalMapSol = TextureFromFile("brickwall_normal.jpg", "./../resources/textures/");

	// texture diffuse, normal map et heigth map du mur en brique
	unsigned int diffuseMapMur = TextureFromFile("bricks2.jpg", "./../resources/textures/");
	unsigned int normalMapMur = TextureFromFile("bricks2_normal.jpg", "./../resources/textures/");
	unsigned int dispMapMur = TextureFromFile("bricks2_disp.jpg", "./../resources/textures/");

	// vertex de l'écran de rendu
	float screenVertices[] = {

		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int screenVAO, screenVBO;
	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenVBO);
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	float near_plane = 0.0f, far_plane = 100.0f;

	//création du z buffer
	GLuint depthMapFBO = 0;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	// création de la texture récupérant le rendu
	GLuint depthTexture;
	glGenTextures(1, &depthTexture);

	// on bind la texture pour que l'on puisse la modifier
	glBindTexture(GL_TEXTURE_2D, depthTexture);

	// On y met une image "vide"
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	// On y ajoute des filtres
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shaderRenderQuad.use();
	shaderRenderQuad.setInt("depthMap", 0);

	while (!glfwWindowShouldClose(window))
	{

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		//mouvement de la lumière dans le temps de manière cyclique
		movementLight = fmod(movementLight + deltaTime, 2 * M_PI);
		lightPos = vec3(15 * cos(movementLight), 20 + 15 * -sin(movementLight), 15 * sin(movementLight));

		glm::mat4 lightProjection, lightView;
		lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		shaderDepthBuffer.use();
		shaderDepthBuffer.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, 4096, 4096);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		/*
		Calcul du shadow mapping du donkey kong car c'est le seul modèle dont on souhaite calculer les ombre (car le mur et le sol n'auront pas d'ombre visible)
		*/
		prepareShaderTransformation(shaderDepthBuffer, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), 0.0f);
		dk.Draw(shaderDepthBuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//préparation du shader blinn phong avec ombre
		prepareShaderBP(shaderBP, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f),  vec3(1.0f, 1.0f, 1.0f), 0.0f);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		dk.Draw(shaderBP);

		//préparation du shader cookTorrance avec ombre
		prepareShaderCT(shaderCT, vec3(0.0f, -1.0f, 0.0f), vec3(25.0f, 1.0f, 25.0f), vec3(1.0f, 0.0f, 0.0f), -M_PI_2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, diffuseMapSol);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normalMapSol);
		//affichage du sol
		plan.Draw(shaderCT);

		//préparation du shader cookTorrance avec ombre et parallax occlusion mapping
		prepareShaderCT(shaderPMCT, vec3(0.0f, 24.0f, -25.0f), vec3(25.0f, 25.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMapMur);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMapMur);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, dispMapMur);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		//affichage du mur
		plan.Draw(shaderPMCT);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void prepareShaderCT(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle) {
	shader.use();
	shader.setVec3("objectAmbiantColor", 0.1f, 0.1f, 0.1f);
	shader.setVec3("objectDiffuseColor", 1.0f, 1.0f, 1.0f);
	shader.setVec3("objectSpecularColor", 1.0f, 1.0f, 1.0f);
	shader.setVec3("lightColor", lightColor);
	shader.setVec3("lightPos", lightPos);
	shader.setVec3("viewPos", camera.Position);

	shader.setFloat("n1", 1.5);
	shader.setFloat("n2", 1.0);
	shader.setFloat("m", 0.1);

	//shadowmapping
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	prepareShaderTransformation(shader, translation, echelle, rotationAxe, rotationAngle);
}

void prepareShaderBP(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle) {
	shader.use();
	shader.setVec3("objectAmbiantColor", 0.01f, 0.01f, 0.01f);
	shader.setVec3("objectDiffuseColor", 1.0f, 1.0f, 1.0f);
	shader.setVec3("objectSpecularColor", 1.0f, 1.0f, 1.0f);
	shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	shader.setVec3("lightPos", lightPos);
	shader.setVec3("viewPos", camera.Position);
	shader.setFloat("specIndice", 10.0);

	//shadowmapping
	shader.setInt("diffuseTexture", 0);
	shader.setInt("shadowMap", 1);
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	prepareShaderTransformation(shader, translation, echelle, rotationAxe, rotationAngle);
}

void prepareShaderTransformation(Shader shader, vec3 translation, vec3 echelle, vec3 rotationAxe, float rotationAngle) {

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