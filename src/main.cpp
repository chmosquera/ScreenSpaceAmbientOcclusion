/* Lab 6 base code - transforms using local matrix functions
	to be written by students -
	based on lab 5 by CPE 471 Cal Poly Z. Wood + S. Sueda
	& Ian Dunn, Christian Eckhardt
*/
#include <iostream>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "camera.h"
// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> progGBuffer, progSSAO, progBlur, progFinal;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape,sponza;
	
	//camera
	camera mycam;

	//texture for sim
	GLuint gFBO, TexColor, TexPos, TexNormal, depth_rb;
	GLuint ssaoFBO, TexSSAO;
	GLuint blurFBO, TexBlurredSSAO;

	GLuint VertexArrayIDBox, VertexBufferIDBox, VertexBufferTex;
	
	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	// Sample Kernels and rotation noise
	unsigned int TexNoise;
	vector<vec3> sample_kernal;
	const int SAMPLECOUNT = 64;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{


		GLSL::checkVersion();

		
		// Set background color.
		glClearColor(0.12f, 0.34f, 0.56f, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		//culling:
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		//transparency
		glEnable(GL_BLEND);
		//next function defines how to mix the background color with the transparent pixel in the foreground. 
		//This is the standard:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

		// Initialize the GLSL program.
		progGBuffer = make_shared<Program>();
		progGBuffer->setVerbose(true);
		progGBuffer->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		if (! progGBuffer->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		progGBuffer->init();
		progGBuffer->addUniform("P");
		progGBuffer->addUniform("V");
		progGBuffer->addUniform("M");
		progGBuffer->addUniform("campos");
		progGBuffer->addAttribute("vertPos");
		progGBuffer->addAttribute("vertNor");
		progGBuffer->addAttribute("vertTex");

		progSSAO = make_shared<Program>();
		progSSAO->setVerbose(true);
		progSSAO->setShaderNames(resourceDirectory + "/light_vert.glsl", resourceDirectory + "/ssao_frag.glsl");
		if (!progSSAO->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		progSSAO->init();
		progSSAO->addUniform("P");
		progSSAO->addUniform("V");
		progSSAO->addUniform("M");
		progSSAO->addUniform("campos");
		progSSAO->addUniform("kernSamp");
		progSSAO->addAttribute("vertPos");
		progSSAO->addAttribute("vertNor");
		progSSAO->addAttribute("vertTex");

		progBlur = make_shared<Program>();
		progBlur->setVerbose(true);
		progBlur->setShaderNames(resourceDirectory + "/light_vert.glsl", resourceDirectory + "/blur_frag.glsl");
		if (!progBlur->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		progBlur->init();
		progBlur->addUniform("P");
		progBlur->addUniform("V");
		progBlur->addUniform("M");
		progBlur->addAttribute("vertPos");
		progBlur->addAttribute("vertTex");

		progFinal = make_shared<Program>();
		progFinal->setVerbose(true);
		progFinal->setShaderNames(resourceDirectory + "/light_vert.glsl", resourceDirectory + "/light_frag.glsl");
		if (!progFinal->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		progFinal->init();
		progFinal->addUniform("P");
		progFinal->addUniform("V");
		progFinal->addUniform("M");
		progFinal->addUniform("campos");
		progFinal->addAttribute("vertPos");
		progFinal->addAttribute("vertTex");




	}

	void initGeom(const std::string& resourceDirectory)
	{
		//init rectangle mesh (2 triangles) for the post processing
		glGenVertexArrays(1, &VertexArrayIDBox);
		glBindVertexArray(VertexArrayIDBox);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDBox);

		GLfloat *rectangle_vertices = new GLfloat[18];
		// front
		int verccount = 0;

		rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0;


		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), rectangle_vertices, GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferTex);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferTex);

		float t = 1. / 100.;
		GLfloat *rectangle_texture_coords = new GLfloat[12];
		int texccount = 0;
		rectangle_texture_coords[texccount++] = 0, rectangle_texture_coords[texccount++] = 0;
		rectangle_texture_coords[texccount++] = 1, rectangle_texture_coords[texccount++] = 0;
		rectangle_texture_coords[texccount++] = 0, rectangle_texture_coords[texccount++] = 1;
		rectangle_texture_coords[texccount++] = 1, rectangle_texture_coords[texccount++] = 0;
		rectangle_texture_coords[texccount++] = 1, rectangle_texture_coords[texccount++] = 1;
		rectangle_texture_coords[texccount++] = 0, rectangle_texture_coords[texccount++] = 1;

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), rectangle_texture_coords, GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(2);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


		// Initialize mesh.
		sponza = make_shared<Shape>();
		string sponzamtl = resourceDirectory + "/sponza/";
		sponza->loadMesh(resourceDirectory + "/sponza/sponza.obj", &sponzamtl, stbi_load);
		sponza->resize();
		sponza->init();
		
		int width, height, channels;

		/***************************************************
		/ Generate G-Buffer FBO
		/	- Used to store information from gBuffer shaders
		/	--- to be used in ssao shader (color, pos, normal)
		***************************************************/
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glGenFramebuffers(1, &gFBO);
		glActiveTexture(GL_TEXTURE0);
		glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
		//RGBA8 2D texture, 24 bit depth texture, 256x256
		glGenTextures(1, &TexColor);
		glBindTexture(GL_TEXTURE_2D, TexColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);			

		glGenTextures(1, &TexPos);
		glBindTexture(GL_TEXTURE_2D, TexPos);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_BGRA, GL_FLOAT, NULL);

		glGenTextures(1, &TexNormal);
		glBindTexture(GL_TEXTURE_2D, TexNormal);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_BGRA, GL_FLOAT, NULL);
		
		//Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TexColor, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, TexPos, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, TexNormal, 0);
		//-------------------------
		glGenRenderbuffers(1, &depth_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
		//-------------------------
		//Does the GPU support current FBO configuration?

		// The below will change.
		glUseProgram(progFinal->pid);
		int Tex1Loc = glGetUniformLocation(progFinal->pid, "texColor");
		int Tex2Loc = glGetUniformLocation(progFinal->pid, "texPos");
		int Tex3Loc = glGetUniformLocation(progFinal->pid, "texNormal");
		int Tex4Loc = glGetUniformLocation(progFinal->pid, "texBlurredSSAO");
		glUniform1i(Tex1Loc, 0);
		glUniform1i(Tex2Loc, 1);
		glUniform1i(Tex3Loc, 2);
		glUniform1i(Tex4Loc, 3);

		GLenum status;
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			cout << "status framebuffer: good";
			break;
		default:
			cout << "status framebuffer: bad!!!!!!!!!!!!!!!!!!!!!!!!!";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/***********************
		/ Create Sample Kernels
		************************/
		// Add points to the hemisphere
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
		std::default_random_engine generator;

		for (int i = 0; i < SAMPLECOUNT; ++i) {
			vec3 sample = vec3(
				randomFloats(generator) * 2.0 - 1.0, 
				randomFloats(generator) * 2.0 - 1.0, 
				randomFloats(generator));

			sample = normalize(sample);

			// distribute samples within hemisphere
			sample *= randomFloats(generator);

			//sample = pow(sample, vec3(2.0f));	// push samples closer to the center of hemisphere
			sample_kernal.push_back(sample);
		}

		/******************************
		/ Create Random Rotation Noise
		******************************/
		std::vector<glm::vec3> rotationNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			
			noise = normalize(noise);
			rotationNoise.push_back(noise);
		}

		glGenTextures(1, &TexNoise);
		glBindTexture(GL_TEXTURE_2D, TexNoise);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &rotationNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// prog will use the following textures (TexPos, TexNormal, gNoise)
		glUseProgram(progSSAO->pid);
		Tex1Loc = glGetUniformLocation(progSSAO->pid, "texPos");
		Tex2Loc = glGetUniformLocation(progSSAO->pid, "texNormal");
		Tex3Loc = glGetUniformLocation(progSSAO->pid, "texNoise");
		glUniform1i(Tex1Loc, 0);
		glUniform1i(Tex2Loc, 1);
		glUniform1i(Tex3Loc, 2);

		/***************************************************
		/ Generate SSAO FBO
		/	- Used to store information from SSAO shaders 
		/	--- to be used in blur shader (occlusion values)
		***************************************************/
		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		glGenTextures(1, &TexSSAO);
		glBindTexture(GL_TEXTURE_2D, TexSSAO);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TexSSAO, 0);

		/*****************************************************************
		/ Generate Blur FBO
		/	- Used to store information from blur shaders
		/	--- to be used in lighting shader (blurred occlusion texture)
		*****************************************************************/
		glGenFramebuffers(1, &blurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);

		glGenTextures(1, &TexBlurredSSAO);
		glBindTexture(GL_TEXTURE_2D, TexBlurredSSAO);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TexBlurredSSAO, 0);

		glUseProgram(progBlur->pid);
		Tex1Loc = glGetUniformLocation(progBlur->pid, "texSSAO");
		glUniform1i(Tex1Loc, 0);


		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			cout << "status framebuffer: good";
			break;
		default:
			cout << "status framebuffer: bad!!!!!!!!!!!!!!!!!!!!!!!!!";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	//*************************************
	double get_last_elapsed_time()
		{
		static double lasttime = glfwGetTime();
		double actualtime = glfwGetTime();
		double difference = actualtime - lasttime;
		lasttime = actualtime;
		return difference;
		}
	//*************************************
	void render_to_texture() // aka render to framebuffer
	{
		glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
		glDrawBuffers(3, buffers);
		double frametime = get_last_elapsed_time();
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		glm::mat4 M, V, S, T, P;
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		V = mycam.process();


		//bind shader and copy matrices
		progGBuffer->bind();
		glUniformMatrix4fv(progGBuffer->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(progGBuffer->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniform3fv(progGBuffer->getUniform("campos"), 1, &mycam.pos.x);

		//	******		sponza		******
		float pihalf = 3.1415926 / 2.;
		M = rotate(mat4(1),pihalf,vec3(0,1,0)) * scale(mat4(1), vec3(2.3, 2.3, 2.3));
		glUniformMatrix4fv(progGBuffer->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		sponza->draw(progGBuffer, false);	//draw moon
		
		//done, unbind stuff
		progGBuffer->unbind();
		glBindTexture(GL_TEXTURE_2D, TexColor);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, TexPos);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, TexNormal);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	// SSAO shaders will use data from gFBO to calculate occlusion values
	// then render those values into a texture
	void render_SSAO() {
		// Setup textures this render pass will draw into
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		GLenum buffers[] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, buffers);

		// Render this FBO
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		auto P = std::make_shared<MatrixStack>();
		P->pushMatrix();
		P->perspective(70., width, height, 0.1, 100.0f);
		glm::mat4 M, V, S, T;
		V = glm::mat4(1);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		progSSAO->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TexPos);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TexNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, TexNoise);

		M = glm::scale(glm::mat4(1), glm::vec3(1.2, 1, 1)) * glm::translate(glm::mat4(1), glm::vec3(-0.5, -0.5, -1));
		glUniformMatrix4fv(progSSAO->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(progSSAO->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(progSSAO->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(progSSAO->getUniform("kernSamp"), SAMPLECOUNT, &sample_kernal[0].x);
		glBindVertexArray(VertexArrayIDBox);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		progSSAO->unbind();

		glBindTexture(GL_TEXTURE_2D, TexSSAO);
		glGenerateMipmap(GL_TEXTURE_2D);

	}

	void render_blurredSSAO() {
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);
		auto P = std::make_shared<MatrixStack>();
		P->pushMatrix();
		P->perspective(70., width, height, 0.1, 100.0f);
		glm::mat4 M, V, S, T;
		V = glm::mat4(1);

		progBlur->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TexSSAO);
		M = glm::scale(glm::mat4(1), glm::vec3(1.2, 1, 1)) * glm::translate(glm::mat4(1), glm::vec3(-0.5, -0.5, -1));
		glUniformMatrix4fv(progBlur->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(progBlur->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(progBlur->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDBox);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		progBlur->unbind();
	}

	void render_to_screen()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);


		auto P = std::make_shared<MatrixStack>();
		P->pushMatrix();
		P->perspective(70., width, height, 0.1, 100.0f);
		glm::mat4 M, V, S, T;
		V = glm::mat4(1);

		progFinal->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TexColor);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TexPos);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, TexNormal);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, TexBlurredSSAO);
		//glBindTexture(GL_TEXTURE_2D, TexSSAO);
		M = glm::scale(glm::mat4(1), glm::vec3(1.2, 1, 1)) * glm::translate(glm::mat4(1), glm::vec3(-0.5, -0.5, -1));
		glUniformMatrix4fv(progFinal->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(progFinal->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(progFinal->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(progFinal->getUniform("campos"), 1, &mycam.pos.x);
		glBindVertexArray(VertexArrayIDBox);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		progFinal->unbind();

	}

};
//*********************************************************************************************************
int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render_to_texture();
		application->render_SSAO();
		application->render_blurredSSAO();
		application->render_to_screen();
		
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
