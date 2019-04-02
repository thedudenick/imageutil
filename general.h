unsigned int getWidth (char *imgIn)
{
	FILE *fin = fopen(imgIn,"rb");
	if(fin==NULL) return -1;
	unsigned int W;
	fseek(fin,18,SEEK_SET);
	fread(&W, sizeof(unsigned int), 1, fin);
	fclose(fin);
	return W;
}

unsigned int getHeight (char *imgIn)
{
	FILE *fin = fopen(imgIn,"rb");
	if(fin==NULL) return -1;
	unsigned int H;
	fseek(fin,22,SEEK_SET);
	fread(&H, sizeof(unsigned int), 1, fin);
	fclose(fin);
	return H;
}

unsigned char *getHeader (char *imgIn)
{
	FILE *fin = fopen(imgIn,"rb");
	unsigned char *buffer = (unsigned char *) malloc(54 * sizeof(unsigned char));
	int i;
	for(i=0; i<54; i++)
	{
		fread(&buffer[i], 1, 1, fin);
	}
	fclose(fin);
	return buffer;
}
