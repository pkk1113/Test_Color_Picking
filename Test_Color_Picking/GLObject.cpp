#include <gl/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vector>
#include "GLObject.h"

int printAllErrors(const char * caption /*= nullptr*/)
{
	if (caption)
		puts(caption);

	int err;
	int err_count = 0;

	do {
		// ���� ���
		err = glGetError();
		printf(" Error: %s\n", getGLErrorStr(err));
		err_count++;
	} while (err != GL_NO_ERROR);

	return err_count - 1;
}

const char * getGLErrorStr(GLenum err)
{
	switch (err)
	{
	case GL_NO_ERROR:          return "No error";
	case GL_INVALID_ENUM:      return "Invalid enum";
	case GL_INVALID_VALUE:     return "Invalid value";
	case GL_INVALID_OPERATION: return "Invalid operation";
	case GL_STACK_OVERFLOW:    return "Stack overflow";
	case GL_STACK_UNDERFLOW:   return "Stack underflow";
	case GL_OUT_OF_MEMORY:     return "Out of memory";
	default:                   return "Unknown error";
	}
}

/*////////////////////////////////////////////////////////////////////////*/
/*																		  */
/* VAO																	  */
/*																		  */
/*////////////////////////////////////////////////////////////////////////*/

VAO::~VAO()
{
	if (isLoaded())
		unload();
}

bool VAO::load(const char *obj_file)
{
	std::vector<float> vbuf, nbuf, tbuf; //������: ���Ⱑ �߿��մϴ�. �������� �ӽ�
	std::vector<GLuint> f;

#pragma region ____Read Obj File and Make Buffers
	FILE* fin;
	fopen_s(&fin, obj_file, "rt");
	if (!fin) {
		puts("file ����!");
		return 0;
	}

	struct f3 { float x, y, z; };
	struct f2 { float s, t; };

	char cmd[16]; //��� ���� �б�
	std::vector<f3> v, n;
	std::vector<f2> t;
	f3 xyz;
	f2 uv;
	GLuint iFace = 0; //0����.. f�� ���ϴ�.
	bool hasTexCoord = false;
	bool hasNormal = false;

	//�����͸� ����ϴ�.
	while (!feof(fin))
	{
		fscanf_s(fin, "%s", cmd, sizeof(cmd));

		//v..
		if (strcmp(cmd, "v") == 0) {
			fscanf_s(fin, "%f %f %f", &xyz.x, &xyz.y, &xyz.z);
			v.push_back(xyz);
		}
		//vt..
		else if (strcmp(cmd, "vt") == 0) {
			fscanf_s(fin, "%f %f", &uv.s, &uv.t);
			t.push_back(uv);

			hasTexCoord = true;
		}
		//vn..
		else if (strcmp(cmd, "vn") == 0) {
			fscanf_s(fin, "%f %f %f", &xyz.x, &xyz.y, &xyz.z);
			n.push_back(xyz);

			hasNormal = true;
		}
		//f..
		else if (strcmp(cmd, "f") == 0) {
			int vi[3], ti[3], ni[3]; //'s id

			if (hasTexCoord && hasNormal) {
				int map = fscanf_s(fin, "%d/%d/%d %d/%d/%d %d/%d/%d", &vi[0], &ti[0], &ni[0], &vi[1], &ti[1], &ni[1], &vi[2], &ti[2], &ni[2]);
				if (map != 9) {
					puts("there is no vertex + texture coord + normal set.");
					fclose(fin);
					return false;
				}

				for (int i = 0; i < 3; i++) { //3���� ���� ���ؼ�
					f.push_back(iFace++); //�ε����Դϴ�.

					vbuf.push_back(v[vi[i] - 1].x); //x
					vbuf.push_back(v[vi[i] - 1].y); //y
					vbuf.push_back(v[vi[i] - 1].z); //z

					tbuf.push_back(t[ti[i] - 1].s); //u
					tbuf.push_back(t[ti[i] - 1].t); //v

					nbuf.push_back(n[ni[i] - 1].x); //nx
					nbuf.push_back(n[ni[i] - 1].y); //ny
					nbuf.push_back(n[ni[i] - 1].z); //nz
				}
			}
			else if (hasNormal) {
				int map = fscanf_s(fin, "%d//%d %d//%d %d//%d", &vi[0], &ni[0], &vi[1], &ni[1], &vi[2], &ni[2]);
				if (map != 6) {
					puts("there is no vertex + normal set.");
					fclose(fin);
					return false;
				}

				for (int i = 0; i < 3; i++) { //3���� ���� ���ؼ�
					f.push_back(iFace++); //�ε����Դϴ�.

					vbuf.push_back(v[vi[i] - 1].x); //x
					vbuf.push_back(v[vi[i] - 1].y); //y
					vbuf.push_back(v[vi[i] - 1].z); //z

					nbuf.push_back(n[ni[i] - 1].x); //nx
					nbuf.push_back(n[ni[i] - 1].y); //ny
					nbuf.push_back(n[ni[i] - 1].z); //nz
				}
			}
			else if (hasTexCoord) {
				int map = fscanf_s(fin, "%d/%d %d/%d %d/%d", &vi[0], &ti[0], &vi[1], &ti[1], &vi[2], &ti[2]);
				if (map != 6) {
					puts("there is no vertex + normal set.");
					fclose(fin);
					return false;
				}

				for (int i = 0; i < 3; i++) { //3���� ���� ���ؼ�
					f.push_back(iFace++); //�ε����Դϴ�.

					vbuf.push_back(v[vi[i] - 1].x); //x
					vbuf.push_back(v[vi[i] - 1].y); //y
					vbuf.push_back(v[vi[i] - 1].z); //z

					tbuf.push_back(t[ti[i] - 1].s); //u
					tbuf.push_back(t[ti[i] - 1].t); //v
				}
			}
			else {
				int map = fscanf_s(fin, "%d %d %d", &vi[0], &vi[1], &vi[2]);
				if (map != 3) {
					puts("there is no vertex");
					fclose(fin);
					return false;
				}

				for (int i = 0; i < 3; i++) { //3���� ���� ���ؼ�
					f.push_back(iFace++); //�ε����Դϴ�.

					vbuf.push_back(v[vi[i] - 1].x); //x
					vbuf.push_back(v[vi[i] - 1].y); //y
					vbuf.push_back(v[vi[i] - 1].z); //z
				}
			}
		}

		cmd[0] = '\0';
	}
	fclose(fin);
#pragma endregion

	size_t buffer_size = sizeof(float) * (vbuf.size() + nbuf.size() + tbuf.size());
	size_t vbuf_size = sizeof(float) * vbuf.size();
	size_t nbuf_size = sizeof(float) * nbuf.size();
	size_t tbuf_size = sizeof(float) * tbuf.size();

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao); //1

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo); //2
	glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_STATIC_DRAW);

	// Vertex
	size_t v_offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, v_offset, vbuf_size, vbuf.data());
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)v_offset);

	// Normal
	size_t n_offset = v_offset + vbuf_size;
	glBufferSubData(GL_ARRAY_BUFFER, n_offset, nbuf_size, nbuf.data());
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)n_offset);

	// Texture
	size_t t_offset = n_offset + nbuf_size;
	glBufferSubData(GL_ARRAY_BUFFER, t_offset, tbuf_size, tbuf.data());
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)t_offset);

	glBindBuffer(GL_ARRAY_BUFFER, 0); //-2
	glBindVertexArray(0); //-1

	m_faceCount = f.size();

	if (m_faceCount == 0)
	{
		unload();
		return false;
	}

	return true;
}

void VAO::unload()
{
	m_faceCount = 0;

	glDeleteBuffers(1, &m_vbo);
	m_vbo = 0;

	glDeleteVertexArrays(1, &m_vao);
	m_vao = 0;
}

bool VAO::isLoaded() const
{
	return (m_vao != 0);
}

void VAO::render_once()
{
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, m_faceCount);
	glBindVertexArray(0);
}

void VAO::render()
{
	glDrawArrays(GL_TRIANGLES, 0, m_faceCount);
}

void VAO::bind_render()
{
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, m_faceCount);
}

void VAO::bind()
{
	glBindVertexArray(m_vao);
}

void VAO::unbind()
{
	glBindVertexArray(0);
}

GLuint VAO::getVAO() const
{
	return m_vao;
}

GLuint VAO::getVBO() const
{
	return m_vbo;
}

/*////////////////////////////////////////////////////////////////////////*/
/*																		  */
/* Shader																  */
/*																		  */
/*////////////////////////////////////////////////////////////////////////*/

Shader::~Shader()
{
	if (isLoaded())
		unload();
}

bool Shader::load(const std::string & file)
{
	return load(
		(file + ".vert").c_str(), 
		(file + ".frag").c_str()
	);
}

bool Shader::load(const char *vert_file, const char *frag_File)
{
	//���̴����� �����.
	GLuint vertID = genShader(GL_VERTEX_SHADER, vert_file);
	GLuint fragID = genShader(GL_FRAGMENT_SHADER, frag_File);

	//���α׷��� ����� ��ũ�մϴ�.
	GLuint id = glCreateProgram();
	glAttachShader(id, vertID);
	glAttachShader(id, fragID);
	glLinkProgram(id);

	//���̵��� �����մϴ�.
	glDetachShader(id, vertID);
	glDeleteShader(vertID);
	glDetachShader(id, fragID);
	glDeleteShader(fragID);

	//��ũ���и� ���α׷��� �����մϴ�.
	GLint link_checker;
	glGetProgramiv(id, GL_LINK_STATUS, &link_checker);
	if (link_checker == GL_FALSE || vertID == 0 || fragID == 0) {
#ifdef _DEBUG
		GLchar infoLog[512];
		glGetProgramInfoLog(id, 512, NULL, infoLog);
		printf_s("Link Fail: ");
		puts(infoLog);
#endif
		glDeleteProgram(id);
		return false;
	}

	m_program = id;

	return true;
}

bool Shader::loadFromSource(const char *vert, const char *frag)
{
	//���̴����� �����.
	GLuint vertID = genShaderFromSource(GL_VERTEX_SHADER, vert);
	GLuint fragID = genShaderFromSource(GL_FRAGMENT_SHADER, frag);

	//���α׷��� ����� ��ũ�մϴ�.
	GLuint id = glCreateProgram();
	glAttachShader(id, vertID);
	glAttachShader(id, fragID);
	glLinkProgram(id);

	//���̵��� �����մϴ�.
	glDetachShader(id, vertID);
	glDeleteShader(vertID);
	glDetachShader(id, fragID);
	glDeleteShader(fragID);

	//��ũ���и� ���α׷��� �����մϴ�.
	GLint link_checker;
	glGetProgramiv(id, GL_LINK_STATUS, &link_checker);
	if (link_checker == GL_FALSE || vertID == 0 || fragID == 0) {
#ifdef _DEBUG
		GLchar infoLog[512];
		glGetProgramInfoLog(id, 512, NULL, infoLog);
		printf_s("Link Fail: ");
		puts(infoLog);
#endif
		glDeleteProgram(id);
		return false;
	}

	m_program = id;

	return true;
}

void Shader::unload()
{
	glDeleteProgram(m_program);
	m_program = 0;
}

bool Shader::isLoaded() const
{
	return (m_program != 0);
}

void Shader::use()
{
	glUseProgram(m_program);
}

void Shader::unuse()
{
	glUseProgram(0);
}

GLuint Shader::getProgram() const
{
	return m_program;
}

GLuint Shader::genShader(GLenum type, const char * file)
{
	//������ �а� �ҽ��� str�� ��´�.
	FILE* fin;
	fopen_s(&fin, file, "r");
	if (!fin)
		return 0;
	fseek(fin, 0, SEEK_END);
	const int length = ftell(fin);
	std::vector<char> str(length, '\0');
	fseek(fin, 0, SEEK_SET);
	fread_s(str.data(), str.size(), str.size(), 1, fin);
	fclose(fin);

	//�ҽ��� ������� ���̴��� ����ϴ�.
	GLuint id = glCreateShader(type);
	const char* source = str.data();
	glShaderSource(id, 1, &source, &length);
	glCompileShader(id);

	//������ ���и� ���̴��� �����մϴ�.
	int compile_checker;
	glGetShaderiv(id, GL_COMPILE_STATUS, &compile_checker);
	if (compile_checker == GL_FALSE) {
#ifdef _DEBUG
		GLchar infoLog[512];
		glGetShaderInfoLog(id, 512, NULL, infoLog);
		printf_s("Compile Fail: ");
		puts(file);
		puts(infoLog);
#endif
		glDeleteShader(id);
		return 0;
	}

	return id;
}

GLuint Shader::genShaderFromSource(GLenum type, const char * source)
{
	//�ҽ��� ������� ���̴��� ����ϴ�.
	GLuint id = glCreateShader(type);
	glShaderSource(id, 1, &source, nullptr);
	glCompileShader(id);

	//������ ���и� ���̴��� �����մϴ�.
	int compile_checker;
	glGetShaderiv(id, GL_COMPILE_STATUS, &compile_checker);
	if (compile_checker == GL_FALSE) {
#ifdef _DEBUG
		GLchar infoLog[512];
		glGetShaderInfoLog(id, 512, NULL, infoLog);
		printf_s("Compile Fail: ");
		puts("source");
		puts(infoLog);
#endif
		glDeleteShader(id);
		return 0;
	}

	return id;
}

/*////////////////////////////////////////////////////////////////////////*/
/*																		  */
/* FBO																	  */
/*																		  */
/*////////////////////////////////////////////////////////////////////////*/

FBO::~FBO()
{
	if (isCreated())
		destroy();
}

bool FBO::create(int width, int height, int colorTextureCount /*= 1*/, 
	bool hasDepthTexture /*= true*/, GLenum colorFormat /*= GL_RGBA32F*/)
{
	if (colorTextureCount > MAX_COLOR_TEXTURE) {
		puts("Many Color Texture is requested.");
		return false;
	}

	m_width = width;
	m_height = height;
	m_colorTexCount = colorTextureCount;

	// ����� ����
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// ���� Texture
	glGenTextures(m_colorTexCount, m_colorTex);
	for (int i = 0; i < m_colorTexCount; i++) {
		glBindTexture(GL_TEXTURE_2D, m_colorTex[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, colorFormat, m_width, m_height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
			GL_TEXTURE_2D, m_colorTex[i], 0);
	}

	// ���� Texture
	if (hasDepthTexture) {
		glGenTextures(1, &m_depthTex);
		glBindTexture(GL_TEXTURE_2D, m_depthTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, m_width, m_height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D, m_depthTex, 0);
	}

	// �˻��մϴ�.
	bool result;

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printAllErrors("FraneBuffer Fail!");
		destroy();

		result = false;
	}
	else {
		setAllDrawbuffers();

		result = true;
	}

	// �����ϰ� �����մϴ�.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return result;
}

void FBO::destroy()
{
	glDeleteFramebuffers(1, &m_fbo);
	m_fbo = 0;

	glDeleteTextures(m_colorTexCount, m_colorTex);
	for (int i = 0; i < MAX_COLOR_TEXTURE; i++)
		m_colorTex[i] = 0;
	m_colorTexCount = 0;

	glDeleteTextures(1, &m_depthTex);
	m_depthTex = 0;

	m_width = 0;
	m_height = 0;
}

bool FBO::isCreated()
{
	return (m_fbo != 0);
}

void FBO::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, m_width, m_height);
}

void FBO::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::setDrawbuffers(const std::initializer_list<GLenum>& buffer_list)
{
	std::vector<GLenum> buffers(buffer_list.begin(), buffer_list.end());

	for (auto& value : buffers) {
		value += GL_COLOR_ATTACHMENT0;
	}

	glDrawBuffers(buffers.size(), buffers.data());
}

void FBO::setAllDrawbuffers()
{
	std::vector<GLenum> buffers(m_colorTexCount);

	for (int i = 0; i < m_colorTexCount; i++) {
		buffers[i] = i + GL_COLOR_ATTACHMENT0;
	}

	glDrawBuffers(buffers.size(), buffers.data());
}

void FBO::bindColorTexture(int texture_index, int unit)
{
	glActiveTexture(unit + GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, getColorTex(texture_index));
}

void FBO::bindDepthTexture(int unit)
{
	glActiveTexture(unit + GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, getDepthTex());
}

void FBO::unbindTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint FBO::getFBO() const
{
	return m_fbo;
}

GLuint FBO::getDepthTex() const
{
	return m_depthTex;
}

GLuint FBO::getColorTex(int texture_index) const
{
	if (texture_index < 0 || texture_index > m_colorTexCount)
		return 0;

	return m_colorTex[texture_index];
}

int FBO::getWidth() const
{
	return m_width;
}

int FBO::getHeight() const
{
	return m_height;
}

int FBO::getColorTexCount() const
{
	return m_colorTexCount;
}

/*////////////////////////////////////////////////////////////////////////*/
/*																		  */
/* TEXTURE																  */
/*																		  */
/*////////////////////////////////////////////////////////////////////////*/

Texture::~Texture()
{
	if (isLoaded())
		unload();
}

bool Texture::load(const char *image_file)
{
	stbi_set_flip_vertically_on_load(true);

	int channel;
	auto data = stbi_load(image_file, &m_width, &m_height, &channel, 0);

	if (!data) {
		puts("input image file!");
		return false;
	}

	GLenum format = (channel == 3) ? GL_RGB : GL_RGBA;

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	return true;
}

void Texture::unload()
{
	glDeleteTextures(1, &m_texture);
	m_texture = 0;
}

bool Texture::isLoaded() const
{
	return (m_texture != 0);
}

GLuint Texture::getTexture() const
{
	return m_texture;
}

void Texture::bind(int bind)
{
	glActiveTexture(bind + GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

void Texture::unbind()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/*////////////////////////////////////////////////////////////////////////*/
/*																		  */
/* Quad Renderer														  */
/*																		  */
/*////////////////////////////////////////////////////////////////////////*/

QuadRenderer::QuadRenderer(int row, int col)
{
	create(row, col);
}

QuadRenderer::QuadRenderer()
{
	create(1, 1);
}

QuadRenderer::~QuadRenderer()
{
	if (isCreated())
		destroy();
}

void QuadRenderer::use()
{
	m_quadShader.use();

	setBorder(0.01f);
	setBorderColor(0.5f, 0.5f, 0.5f);
}

void QuadRenderer::create(int row, int col)
{
	m_row = row;
	m_col = col;

	w = 1.f / col;
	h = 1.f / row;
	offset_x = 1.f / col - 1.f;
	offset_y = 1.f - 1.f / row;

	if (m_quadShader.isLoaded())
		m_quadShader.unload();

	loadShader();
}

void QuadRenderer::destroy()
{
	m_quadShader.unload();
}

bool QuadRenderer::isCreated() const
{
	return (m_quadShader.isLoaded());
}

void QuadRenderer::unuse()
{
	Shader::unuse();
	Texture::unbind();
}

void QuadRenderer::setBorder(float coef)
{
	glUniform2f(SL_border_coef, coef, 1.f - coef);
}

void QuadRenderer::setBorderColor(float r, float g, float b)
{
	glUniform3f(SL_border_color, r, g, b);
}

void QuadRenderer::render(int row, int col, GLuint texture)
{
	float pos_x = offset_x + 2.f * w * col;
	float pos_y = offset_y - 2.f * h * row;

	float tmat[] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		pos_x, pos_y, 0.f, 1.f
	};

	float smat[] = {
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	glUniformMatrix4fv(SL_tmat, 1, GL_FALSE, tmat);
	glUniformMatrix4fv(SL_smat, 1, GL_FALSE, smat);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glDrawArrays(GL_QUADS, 0, 4);
}

void QuadRenderer::loadShader()
{
	const char* vertexSource = R"(
// Vertex Shader
#version 430 core

const vec2 aPosition[4] = {
	vec2(-1, -1),
	vec2(1, -1),
	vec2(1, 1),
	vec2(-1, 1),
};

const vec2 aTexCoord[4] = {
	vec2(0, 0),
	vec2(1, 0),
	vec2(1, 1),
	vec2(0, 1),
};

out vec2 texCoord;

layout(location = 0) uniform mat4 tmat;
layout(location = 1) uniform mat4 smat;

void main()
{
	gl_Position = tmat * smat * vec4(aPosition[gl_VertexID], 0, 1);

	texCoord = aTexCoord[gl_VertexID];
}

)";

	const char* fragSource = R"(
// Fragment Shader
#version 430 core

layout(binding = 0) uniform sampler2D map;

/*
	st:
	s: value for comparing left and up.
	t: value for comparing right and down.
*/
layout(location = 2) uniform vec2 border_coef;
layout(location = 3) uniform vec3 border_color;

in vec2 texCoord;

layout(location = 0) out vec4 frag_color;

void main()
{
	if( texCoord.x <= border_coef.s || texCoord.x >= border_coef.t ||
		texCoord.y <= border_coef.s || texCoord.y >= border_coef.t) 
	{
		frag_color = vec4(border_color, 1);
		return;
	}
	frag_color = texture(map, texCoord);
}

)";

	m_quadShader.loadFromSource(vertexSource, fragSource);
}
