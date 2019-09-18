#pragma once
#include <gl/GL.h>
#include <initializer_list>
#include <string>

/*
	return:
	if 0, there is no error
	otherwise, there is number of return value error.
*/
int printAllErrors(const char * caption = nullptr);
const char * getGLErrorStr(GLenum err);

/************************************************************/
/*															*/
// OpenGL Objects						
/*															*/
/************************************************************/

//Vertex Array Object
class VAO
{
	GLuint m_vao = 0;
	GLuint m_vbo = 0;
	int m_faceCount = 0;

public:
	VAO() = default;
	~VAO();

	bool load(const char *obj_file);
	void unload();
	bool isLoaded() const;

	/*
		다음이 순서대로 1번씩 실행됩니다.
		bind()
		render()
		unbind()
	*/
	void render_once();
	/*
		다음이 순서대로 1번씩 실행됩니다.
		bind()
		render()
	*/
	void bind_render();
	void bind();
	void render();
	static void unbind();

	GLuint getVAO() const;
	GLuint getVBO() const;
};

class Shader
{
	GLuint m_program = 0;

public:
	Shader() = default;
	~Shader();

	/*
		file:
		converted two string 'file.vert' and 'file.frag'.
	*/
	bool load(const std::string& file);
	/*
		vert_file,frag_file:
		*.vert
		*.frag
	*/
	bool load(const char *vert_file, const char *frag_File);
	bool loadFromSource(const char *vert, const char *frag);
	void unload();
	bool isLoaded() const;

	void use();
	static void unuse();

	GLuint getProgram() const;

private:
	GLuint genShader(GLenum type, const char * file);
	GLuint genShaderFromSource(GLenum type, const char * source);
};

//Frame buffer object
class FBO
{
	static constexpr int MAX_COLOR_TEXTURE = 15;

	GLuint m_fbo = 0;
	GLuint m_depthTex = 0;
	GLuint m_colorTex[MAX_COLOR_TEXTURE];
	int m_colorTexCount = 0;
	int m_width;
	int m_height;

public:
	FBO() = default;
	~FBO();

	/*
		colorFormat:
		rgba 32 float:	GL_RGBA32F (default)
		rgba 8 uint:	GL_RGBA8UI
	*/
	bool create(int width, int height, int colorTextureCount = 1, 
		bool hasDepthTexture = true, GLenum colorFormat = GL_RGBA32F);
	void destroy();
	bool isCreated();

	/*
		내부에서 viewport를 호출합니다.
	*/
	void bind();
	static void unbind();
	void bindColorTexture(int texture_index = 0, int unit = 0);
	void bindDepthTexture(int unit = 0);
	static void unbindTexture();

	/*
		{} 중괄호를 사용하여 출력을 정할 수 있습니다.
		ex)	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT1 를 순서대로 출력으로 쓰려면
		setDrawbuffers({0, 4, 1});
	*/
	void setDrawbuffers(const std::initializer_list<GLenum>& buffer_list);

	/*
		만들어진 모든 색상 버퍼를 출력으로 사용합니다. 순서는 만들어진 순서를 따릅니다.
		create() 를 해서 성공할 경우 기본적으로 호출됩니다.
	*/
	void setAllDrawbuffers();
	
	GLuint getFBO() const;
	GLuint getDepthTex() const;
	GLuint getColorTex(int texture_index = 0) const;
	int getWidth() const;
	int getHeight() const;
	int getColorTexCount() const;
};

class Texture
{
	GLuint m_texture;
	int m_width;
	int m_height;

public:
	Texture() = default;
	~Texture();

	bool load(const char *image_file);
	void unload();
	bool isLoaded() const;

	void bind(int unit = 0);
	static void unbind();

	GLuint getTexture() const;
};


/************************************************************/
/*															*/
// My Utility						
/*															*/
/************************************************************/

class QuadRenderer
{
	enum ShaderLocation {
		SL_tmat,
		SL_smat,
		SL_border_coef,
		SL_border_color,
	};

	Shader m_quadShader;
	int m_row;
	int m_col;

	float w, h, offset_x, offset_y;

public:
	QuadRenderer();
	QuadRenderer(int row, int col);
	~QuadRenderer();

	void create(int row, int col);
	void destroy();
	bool isCreated() const;

	void use();
	void unuse();
	
	/*
		coef:
		if coef is 0.0, then no border.
		if coef is 0.5, then half of row and column(from outside to inside) is used by border.
		if coef is 1.0, then all row and column is used by border.
		default border is 0.01
	*/
	void setBorder(float coef);
	/*
		default color is gray(0.5, 0.5, 0.5).
	*/
	void setBorderColor(float r, float g, float b);
	void render(int row, int col, GLuint texture);

private:
	void loadShader();
};