#include <stdlib.h>
#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "../../glaze/include/glaze.h" // TODO need to register this somehow, how?
#include <GLFW/glfw3.h>


typedef struct GLfont {
	float offsets[128][3]; // offset_x, offset_y (from texture origin), advance
	GLuint texture;
	GLuint buffer;
	GLuint vertex_array;
} GLfont;


//void glText(GLfont font, char *text, float x, float y, float scale, float color[4]); // TODO: const pointer?

GLuint shaders[2];
GLuint program;
GLuint color_uniform, pos_uniform, scale_uniform, char_uniform;

// TODO
GLFWwindow* open_window(int width, int height)
{
	GLFWwindow* window = glfwCreateWindow(width, height, "", NULL, NULL); //glfwGetPrimaryMonitor()
	if (!window) {
		printf("Error: could not open window");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // TODO: turn on?

	return window;
}
void error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error: %s\n", description);
	exit(EXIT_FAILURE);
	return;
}

void debug_ascii(unsigned char* buf, size_t width, size_t height)
{
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			unsigned char v = buf[j + i*width];
			if (v == 0) printf(" ");
			else printf("@");
		}
		printf("\n");
	}
	return;
}


GLfont glLoadFont(char* filename)
{
	GLfont font;

	GLint memory_alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &memory_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	font.buffer = buffer;
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	font.vertex_array = vertex_array;
	glBindVertexArray(vertex_array);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	float vertices[128][8];

	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		printf("Error: could not initialize FreeType library");
		exit(EXIT_FAILURE);
	}

	FT_Face face;
	if (FT_New_Face(ft, filename, 0, &face)) {
		printf("Error: Failed to load font from file\n");
		exit(EXIT_FAILURE);
	}
	FT_Set_Pixel_Sizes(face, 48, 48);

	glGenTextures(1, &(font.texture));
	glBindTexture(GL_TEXTURE_2D_ARRAY, font.texture);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32F, 48, 48, 128); //glTexImage2D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

	for (unsigned char c = 0; c < 128; c++) {

		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			printf("Error: failed to load glyph %c\n", c);
			exit(EXIT_FAILURE); // in original code this was a continue, good?
		}

		int width  = face->glyph->bitmap.width;
		int height = face->glyph->bitmap.rows;
		if (width == 0 || height == 0) continue;

		int left = face->glyph->bitmap_left;
		int top = face->glyph->bitmap_top;

		float offset_x =  left          / 48.0f; // Normalisation by size
		float offset_y = (height - top) / 48.0f;
		float end_x = width  / 48.0f;
		float end_y = height / 48.0f;
		font.offsets[c][0] = offset_x;
		font.offsets[c][1] = offset_y;
		font.offsets[c][2] = (face->glyph->advance.x >> 6) / 48.0f;

		vertices[c][0] = 0.0f;
		vertices[c][1] = 0.0f;
		vertices[c][2] = end_x;
		vertices[c][3] = 0.0f;
		vertices[c][4] = end_x;
		vertices[c][5] = end_y;
		vertices[c][6] = 0.0f;
		vertices[c][7] = end_y;

		//printf("Vertex %c\n%f %f\n%f %f\n%f %f\n%f %f\n",
		//	c,
		//	vertices[c][0],
		//	vertices[c][1],
		//	vertices[c][2],
		//	vertices[c][3],
		//	vertices[c][4],
		//	vertices[c][5],
		//	vertices[c][6],
		//	vertices[c][7]
		//);

		// Reverse y-axis of bitmap
		unsigned char* bitmap = (unsigned char*)malloc(sizeof(unsigned char*) * width * height);
		for (int y = 0; y < height; y++) {
			memcpy(
				&(bitmap[width * y]),
				&(face->glyph->bitmap.buffer[width * (height - y - 1)]),
				width
			);
		}

		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY, 0, 0, 0, c,
			width, height, 1,
			GL_RED, GL_UNSIGNED_BYTE,
			bitmap
		);

		free(bitmap);


		//printf("%u\n\n", c);
		//debug_ascii(face->glyph->bitmap.buffer, width, height);
		//printf("\n\n\n\n\n");
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, memory_alignment);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return font;
}


//GLtext glPrepareText(GLfont font, char *text)
//{
//	GLtext vertices;
//
//	// TODO: slightly weird
//	float vertices[128][8];
//	glBindBuffer(GL_ARRAY_BUFFER, font.buffer);
//	glGetBufferSubData(GL_ARRAY_BUFFER, 0, 4*2*128);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//  	GLintptr offset,
//  	GLsizeiptr size,
//  	void * data);
//
//	GLint memory_alignment;
//	glGetIntegerv(GL_UNPACK_ALIGNMENT, &memory_alignment);
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
//	GLuint buffer;
//	glGenBuffers(1, &buffer);
//	font.buffer = buffer;
//	glBindBuffer(GL_ARRAY_BUFFER, buffer);
//
//	GLuint vertex_array;
//	glGenVertexArrays(1, &vertex_array);
//	font.vertex_array = vertex_array;
//	glBindVertexArray(vertex_array);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
//
//	float vertices[128][8];
//
//	FT_Library ft;
//	if (FT_Init_FreeType(&ft)) {
//		printf("Error: could not initialize FreeType library");
//		exit(EXIT_FAILURE);
//	}
//
//	FT_Face face;
//	if (FT_New_Face(ft, filename, 0, &face)) {
//		printf("Error: Failed to load font from file\n");
//		exit(EXIT_FAILURE);
//	}
//	FT_Set_Pixel_Sizes(face, 48, 48);
//
//	glGenTextures(1, &(font.texture));
//	glBindTexture(GL_TEXTURE_2D_ARRAY, font.texture);
//	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32F, 48, 48, 128); //glTexImage2D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
//
//	for (unsigned char c = 0; c < 128; c++) {
//
//		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
//			printf("Error: failed to load glyph %c\n", c);
//			exit(EXIT_FAILURE); // in original code this was a continue, good?
//		}
//
//		int width  = face->glyph->bitmap.width;
//		int height = face->glyph->bitmap.rows;
//		if (width == 0 || height == 0) continue;
//
//		int left = face->glyph->bitmap_left;
//		int top = face->glyph->bitmap_top;
//
//		float offset_x =  left          / 48.0f; // Normalisation by size
//		float offset_y = (height - top) / 48.0f;
//		float end_x = width  / 48.0f;
//		float end_y = height / 48.0f;
//		font.offsets[c][0] = offset_x;
//		font.offsets[c][1] = offset_y;
//		font.offsets[c][2] = (face->glyph->advance.x >> 6) / 48.0f;
//
//		vertices[c][0] = 0.0f;
//		vertices[c][1] = 0.0f;
//		vertices[c][2] = end_x;
//		vertices[c][3] = 0.0f;
//		vertices[c][4] = end_x;
//		vertices[c][5] = end_y;
//		vertices[c][6] = 0.0f;
//		vertices[c][7] = end_y;
//
//		//printf("Vertex %c\n%f %f\n%f %f\n%f %f\n%f %f\n",
//		//	c,
//		//	vertices[c][0],
//		//	vertices[c][1],
//		//	vertices[c][2],
//		//	vertices[c][3],
//		//	vertices[c][4],
//		//	vertices[c][5],
//		//	vertices[c][6],
//		//	vertices[c][7]
//		//);
//
//		// Reverse y-axis of bitmap
//		unsigned char* bitmap = (unsigned char*)malloc(sizeof(unsigned char*) * width * height);
//		for (int y = 0; y < height; y++) {
//			memcpy(
//				&(bitmap[width * y]),
//				&(face->glyph->bitmap.buffer[width * (height - y - 1)]),
//				width
//			);
//		}
//
//		glTexSubImage3D(
//			GL_TEXTURE_2D_ARRAY, 0, 0, 0, c,
//			width, height, 1,
//			GL_RED, GL_UNSIGNED_BYTE,
//			bitmap
//		);
//
//		free(bitmap);
//
//
//		//printf("%u\n\n", c);
//		//debug_ascii(face->glyph->bitmap.buffer, width, height);
//		//printf("\n\n\n\n\n");
//	}
//
//	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindVertexArray(0);
//	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
//	glPixelStorei(GL_UNPACK_ALIGNMENT, memory_alignment);
//
//	FT_Done_Face(face);
//	FT_Done_FreeType(ft);
//
//	return font;
//
//
//	return vertices;
//}

// TODO: use pointer for font?
void glDrawText(GLfont font, char *text, size_t n, float x, float y, float scale, float color[4])
{
	glUseProgram(program);
	glUniform4f(color_uniform, color[0], color[1], color[2], color[3]);
	glUniform1f(scale_uniform, scale);

	glActiveTexture(GL_TEXTURE0); // TODO: necessary?
	glBindVertexArray(font.vertex_array);

	glBindTexture(GL_TEXTURE_2D_ARRAY, font.texture);

	unsigned char *c = (unsigned char*) text;

	while (*c != '\0') {

		if (*c < 0 || *c > 128) {
			printf("Error: encountered non-ASCII character\n");
			exit(EXIT_FAILURE);
		}

		float offset_x = font.offsets[*c][0];
		float offset_y = font.offsets[*c][1];
		float advance = font.offsets[*c][2];

		glUniform1i(char_uniform, *c);
		//glUniform2f(pos_uniform, x + scale * offset_x, y + scale * offset_y);
		glUniform2f(pos_uniform, x + scale * offset_x, y - scale * offset_y);
		glDrawArrays(GL_TRIANGLE_FAN, (*c) * 4, 4);
		//glDrawArrays(GL_POINTS, *c, 4);
		//printf("%f %f\n", x, font.advances[*c] * scale);
		x += advance * scale;
		c++;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glUseProgram(0);

	return;
}


char* read_file(char* filename)
{
	FILE* f = fopen(filename, "r");
	if (f == NULL) {
		printf("Error: could not open file %s\n", filename);
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END); // Could use stat()
	long n = 1 + ftell(f);
	rewind(f);
	char *buf = (char *)malloc(n);

	fread(buf, n, 1, f);

	return buf;
}

void glyphs_finish()
{
	// TODO
}

int main(int argc, char **argv)
{

	// TODO this has to go into a test
	if (!glfwInit()) {
		printf("Error: could not initialise GLFW");
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwSetErrorCallback(error_callback);

	GLFWwindow* window = open_window(800, 600);
	// TODO break this down?
	gladLoadGL();

	char* vertex_shader_source = read_file("src/vertex_shader.glsl");
	char* fragment_shader_source = read_file("src/fragment_shader.glsl");


	shaders[0] = glMakeShader(GL_VERTEX_SHADER, (const char**)&vertex_shader_source);
	shaders[1] = glMakeShader(GL_FRAGMENT_SHADER, (const char**)&fragment_shader_source);

	free(vertex_shader_source);
	free(fragment_shader_source);

	program = glMakeProgram(shaders, 2);

	color_uniform = glGetUniformLocation(program, "text_color");
	pos_uniform = glGetUniformLocation(program, "pos");
	scale_uniform = glGetUniformLocation(program, "scale");
	char_uniform = glGetUniformLocation(program, "char");

	float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfont font = glLoadFont("/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf");

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(glErrorCallback, NULL);
	glClearColor(1, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glLineWidth(2.0);

	while (!glfwWindowShouldClose(window)) {
		glHandleErrors("asd");
		glClear(GL_COLOR_BUFFER_BIT);
		char* text = "1234567890!@#$%^&*()_-=+[{]};:'\"\\|,<.>/?";
		glDrawText(font, text, sizeof(text), -0.8f, 0.0f, 0.05f, color);
		glfwPollEvents();
		glFlush();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

