#define GLM_ENABLE_EXPERIMENTAL
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
#include <cstdio>
#include "GLObject.h"

#ifdef _DEBUG
#include <cstdlib>
#include <crtdbg.h>
#endif

int g_width = 512;
int g_height = 512;
int g_x = 0;
int g_y = 0;
float g_aspect = (float)g_width / (float)g_height;
bool g_pause = false;

void initContext(bool useDefault, int major = 3, int minor = 3, bool useCompatibility = false);
void framebufferSizeCallback(GLFWwindow*, int w, int h);
void mousebuttonCallback(GLFWwindow*, int btn, int act, int);
void cursorPosCallback(GLFWwindow*, double x, double y);

class Scene
{
	// objects
	FBO colorFBO;
	FBO pickFBO;
	Shader colorShader;
	Shader pickShader;
	VAO ballVAO;
	VAO monkeyVAO;
	QuadRenderer logQR;
	QuadRenderer baseQR;

public:
	bool create() {
		//if (!) return false;
		if (!colorFBO.create(2048, 2048)) return false;
		if (!pickFBO.create(2048, 2048)) return false;
		if (!colorShader.load("resources/shaders/color")) return false;
		if (!pickShader.load("resources/shaders/pick")) return false;
		if (!ballVAO.load("resources/objects/ball.obj")) return false;
		if (!monkeyVAO.load("resources/objects/monkey.obj")) return false;

		logQR.create(3, 3);

		return true;
	}

	void render() {
		// 색상 이미지 만들기
		makeColorMap();

		// 피킹 이미지 만들기
		makePickMap();

		// 일반 렌더링
		glViewport(0, 0, g_width, g_height);
		glClearColor(0.5f, 0.5f, 0.5f, 1.f);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);

		// 색상 이미지
		baseQR.use();
		baseQR.setBorder(0.f);
		baseQR.render(0, 0, pickFBO.getColorTex());
		baseQR.unuse();

		// 피킹 이미지
		logQR.use();
		logQR.render(0, 0, colorFBO.getColorTex());
		logQR.unuse();
	}

	Scene() = default;
	~Scene() = default;

private:
	void makeColorMap() {
		colorFBO.bind();
		glClearColor(0, 0, 0, 0);
		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.f);
		glDepthFunc(GL_LESS);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		colorShader.use();
		glm::mat4 pmat = glm::perspective(45.f, g_aspect, 0.1f, 100.f);
		glm::mat4 vmat = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(0, 1, GL_FALSE, &pmat[0][0]);
		glUniformMatrix4fv(1, 1, GL_FALSE, &vmat[0][0]);

		renderScene(true);

		colorShader.unuse();
		colorFBO.unbind();
	}

	void makePickMap() {
		pickFBO.bind();
		glClearColor(0, 0, 0, 0);
		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.f);
		glDepthFunc(GL_LESS);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pickShader.use();
		glm::mat4 pmat = glm::perspective(45.f, g_aspect, 0.1f, 100.f);
		glm::mat4 vmat = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(0, 1, GL_FALSE, &pmat[0][0]);
		glUniformMatrix4fv(1, 1, GL_FALSE, &vmat[0][0]);

		// pick color
		glUniform3f(3, 1.f, 0.f, 0.f);
		glUniform2f(4, (float)(g_x)/(float)g_width , (float)(g_height - g_y)/(float)(g_height));

		// color texture
		colorFBO.bindColorTexture();

		renderScene(false);

		pickShader.unuse();
		pickFBO.unbind();
	}

	void renderScene(bool hadID) {
		glm::mat4 mmat;
		float color_id = 0.f;
		float color_step = 0.1f;

		monkeyVAO.bind();

		printAllErrors("1");
		mmat = glm::mat4(1.f); 
		mmat *= glm::translate(glm::vec3(0, -1, 0));
		glUniformMatrix4fv(2, 1, GL_FALSE, &mmat[0][0]); // model matrix
		if(hadID) glUniform1f(15, color_id += color_step); // color id
		monkeyVAO.render();

		printAllErrors("2");
		mmat = glm::mat4(1.f);
		mmat *= glm::translate(glm::vec3(3, 0, 0));
		glUniformMatrix4fv(2, 1, GL_FALSE, &mmat[0][0]); // model matrix
		if (hadID) glUniform1f(15, color_id += color_step); // color id
		monkeyVAO.render();

		mmat = glm::mat4(1.f);
		mmat *= glm::translate(glm::vec3(-4, -2, 0));
		mmat *= glm::scale(glm::vec3(3, 3, 3));
		glUniformMatrix4fv(2, 1, GL_FALSE, &mmat[0][0]); // model matrix
		if (hadID) glUniform1f(15, color_id += color_step); // color id
		monkeyVAO.render();

		mmat = glm::mat4(1.f);
		mmat *= glm::translate(glm::vec3(0, 2, 0));
		mmat *= glm::scale(glm::vec3(3, 3, 3));
		glUniformMatrix4fv(2, 1, GL_FALSE, &mmat[0][0]); // model matrix
		if (hadID) glUniform1f(15, color_id += color_step); // color id
		monkeyVAO.render();

		monkeyVAO.unbind();
	}
};

int main()
{
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
	_CrtSetDbgFlag(0x21);
#endif

	/* 초기화, 에러 핸들링 등록, 이벤트 콜백 등록, OpenGL 초기화 */
	/* -------------------------------------------------------------------------------------- */
	glfwInit();
	glfwSetErrorCallback([](int err, const char* desc) { puts(desc); });
	initContext(/*use dafault = */ true);
	GLFWwindow *window = glfwCreateWindow(g_width, g_height, "Order Independent Transparency Rendering!", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetMouseButtonCallback(window, mousebuttonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwMakeContextCurrent(window);
	glewInit();

	/* 객체 생성 및 초기화 */
	/* -------------------------------------------------------------------------------------- */
	auto scene = new Scene();

	if (scene->create()) {
		puts("객체 생성 성공!");
	}
	else {
		puts("객체 생성 실패");
		delete scene;
		return -1;
	}

	/* 객체 생성 및 초기화 검사 */
	/* -------------------------------------------------------------------------------------- */
	printAllErrors("객체 생성 및 초기화 검사");

	/* 메인 루프 */
	/* -------------------------------------------------------------------------------------- */
	while (!glfwWindowShouldClose(window))
	{
		if (!g_pause) {
			// 렌더링
			scene->render();
			// 버퍼 스왑, 이벤트 폴
			glfwSwapBuffers(window);
		}

		glfwPollEvents();
	}

	/* 루프 종료 검사 */
	/* -------------------------------------------------------------------------------------- */
	printAllErrors("루프 종료 검사");

	/* 객체 제거 */
	/* -------------------------------------------------------------------------------------- */
	delete scene;

	/* 객체 제거 검사 */
	/* -------------------------------------------------------------------------------------- */
	printAllErrors("객체 제거 검사");

	/* 종료 */
	/* -------------------------------------------------------------------------------------- */
	glfwTerminate();
}

void initContext(bool useDefault, int major, int minor, bool useCompatibility)
{
	if (useDefault)
	{
		// 기본적으로  Default로 설정해주면
		// 가능한 최신의 OpenGL Vesion을 선택하며 (연구실 컴퓨터는 4.5)
		// Profile은 Legacy Function도 사용할 수 있게 해줍니다.(Compatibility 사용)
		glfwDefaultWindowHints();
	}
	else
	{
		// Major 와 Minor 결정
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

		// Core 또는 Compatibility 선택
		if (useCompatibility)
		{
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
		}
		else
		{
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		}
	}
}

void framebufferSizeCallback(GLFWwindow*, int w, int h)
{
	g_width = w;
	g_height = h;
	g_aspect = (float)g_width / (float)g_height;
	//printf("%d, %d\n", w, h);

	if (g_pause) {
		g_pause = false;
		puts("start !");
	}
}

void mousebuttonCallback(GLFWwindow*, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			g_pause = !g_pause;
			puts(g_pause ? "pause !" : "start !");
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		}
		else {
			g_pause = !g_pause;
			puts(g_pause ? "pause !" : "start !");
		}
	}
}

void cursorPosCallback(GLFWwindow*, double x, double y)
{
	g_x = (int)x;
	g_y = (int)y;
	//printf("%d, %d\n", g_x, g_y);
}