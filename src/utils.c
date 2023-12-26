char* read_file(char* filename)
{
	FILE* f = fopen(filename, "r");
	if (f == NULL) {
		printf("Error: could not open file %s\n", filename);
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END); // Could use stat()
	long n = ftell(f);
	rewind(f);
	char *buf = (char *)malloc(n+1);

	fread(buf, n, 1, f);
	buf[n] = '\0';

	return buf;
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

