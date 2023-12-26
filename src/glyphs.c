#include <stdlib.h>
#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "../../glaze/include/glaze.h"
#include "../include/glyphs.h"

#include "utils.c"


// TODO: static texts (don't change often) could be prepared using a standard vertex array
// (and appropriate shader) with vertices according to width, height, advance and with the
// third texture coordinate (the character)


GLuint shaders[2];
GLuint program;
GLuint buffer;
GLuint vertex_array;
GLuint color_uniform, rect_uniform, scale_uniform, char_uniform;


void glyInit()
{
	char* vertex_shader_source = read_file("src/vertex_shader.glsl");
	char* fragment_shader_source = read_file("src/fragment_shader.glsl");

	shaders[0] = glMakeShader(GL_VERTEX_SHADER, (const char**)&vertex_shader_source);
	shaders[1] = glMakeShader(GL_FRAGMENT_SHADER, (const char**)&fragment_shader_source);

	free(vertex_shader_source);
	free(fragment_shader_source);

	program = glMakeProgram(shaders, 2);

	color_uniform = glGetUniformLocation(program, "text_color");
	rect_uniform = glGetUniformLocation(program, "rect");
	scale_uniform = glGetUniformLocation(program, "scale");
	char_uniform = glGetUniformLocation(program, "char");

	return;
}


void glyFinish()
{
	glDeleteBuffers(1, &buffer);
	glDeleteVertexArrays(1, &vertex_array);
	glDeleteShader(shaders[0]);
	glDeleteShader(shaders[1]);
	glDeleteProgram(program);

	return;
}


// TODO pointer
GLYfont glyLoadFont(char* filename)
{
	GLYfont font;

	GLint memory_alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &memory_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	float vertices[8];
	vertices[0] = 0.0f;
	vertices[1] = 0.0f;
	vertices[2] = 1.0f;
	vertices[3] = 0.0f;
	vertices[4] = 1.0f;
	vertices[5] = 1.0f;
	vertices[6] = 0.0f;
	vertices[7] = 1.0f;

	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

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
	FT_Set_Pixel_Sizes(face, FONTSIZE, FONTSIZE);

	unsigned char* bitmaps = (unsigned char*)calloc(sizeof(unsigned char*), 128 * FONTSIZE * FONTSIZE);

	for (unsigned char c = 0; c < 128; c++) {

		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			printf("Error: failed to load glyph %c\n", c);
			exit(EXIT_FAILURE); // in original code this was a continue, good?
		}

		int width  = face->glyph->bitmap.width;
		int height = face->glyph->bitmap.rows;
		if (width == 0 || height == 0) continue;

		// Reverse y-axis of bitmap
		int left = face->glyph->bitmap_left;
		int top = face->glyph->bitmap_top;
		for (int y = 0; y < height; y++) {
			memcpy(
				&(bitmaps[y * FONTSIZE + c * FONTSIZE * FONTSIZE]),
				&(face->glyph->bitmap.buffer[(height - y - 1) * width]),
				width
			);
		}

		font.rect[c][0] = left / (float)FONTSIZE;
		font.rect[c][1] = (top - height) / (float)FONTSIZE;
		font.rect[c][2] = (face->glyph->advance.x >> 6) / (float)FONTSIZE;
		font.rect[c][3] = width  / (float)FONTSIZE;
		font.rect[c][4] = height / (float)FONTSIZE;

		//	printf("%u: %d %d %d %d\n\n", c, width, left, height, top);
		//	debug_ascii(&(bitmaps[((int)']')*FONTSIZE*FONTSIZE]), FONTSIZE, FONTSIZE);
		//	printf("\n\n\n\n\n");
	}

	glGenTextures(1, &(font.texture));
	glBindTexture(GL_TEXTURE_2D_ARRAY, font.texture);
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY, 0, GL_R32F,
		FONTSIZE, FONTSIZE, 128, 0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		bitmaps
	);

	free(bitmaps);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, memory_alignment);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return font;
}


void glyDeleteFont(GLYfont font)
{
	glDeleteTextures(1, &(font.texture));
	return;
}


// TODO: use pointer for font?
void glyDrawText(GLYfont font, char *text, size_t n, float x, float y, float scale, float color[4])
{
	glUseProgram(program);
	glUniform4f(color_uniform, color[0], color[1], color[2], color[3]);

	float scale_x, scale_y;
	{
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		float width, height;
		width  = viewport[2];
		height = viewport[3];
		scale_x = scale;
		scale_y = scale;
		if      (height > width) scale_x *= height / width;
		else if (height < width) scale_y *= width / height;
	}
	glUniform2f(scale_uniform, scale_x, scale_y);

	glActiveTexture(GL_TEXTURE0); // TODO: necessary?
	glBindVertexArray(vertex_array);

	glBindTexture(GL_TEXTURE_2D_ARRAY, font.texture);

	unsigned char *c = (unsigned char*) text;

	while (*c != '\0') {

		if (*c < 0 || *c > 128) {
			printf("Error: encountered non-ASCII character\n");
			exit(EXIT_FAILURE);
		}

		float offset_x = font.rect[*c][0];
		float offset_y = font.rect[*c][1];
		float advance  = font.rect[*c][2];
		float width    = font.rect[*c][3];
		float height   = font.rect[*c][4];

		glUniform1i(char_uniform, *c);
		glUniform4f(rect_uniform,
			x + scale_x * offset_x,
			y + scale_y * offset_y,
			width,
			height
		);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		x += advance * scale_x;
		c++;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glUseProgram(0);

	return;
}

