
#define FONTSIZE 48

// TODO: with pointers, this could be opaque
typedef struct GLYfont {
	float rect[128][5];
	GLuint texture;
} GLYfont;



void glyInit();
void glyFinish();

GLYfont glyLoadFont(char* filename);
void glyDeleteFont(GLYfont font);
void glyDrawText(GLYfont font, char *text, size_t n, float x, float y, float scale, float color[4]);
