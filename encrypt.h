unsigned int *xorGen (char *secretKey, char *imgIn)
{
	unsigned int *R;
	unsigned int W = getWidth(imgIn);
	unsigned int H = getHeight(imgIn);
	unsigned int n = 2 * W * H, i;
	R = (unsigned int *) malloc(n * sizeof(unsigned int));
	
	FILE *fin_key = fopen(secretKey,"r");
	if(fin_key==NULL) return (unsigned int *)-1;
	fscanf(fin_key,"%u",&R[0]);
	fclose(fin_key);
	for(i=1; i<n; i++)
	{
		unsigned int temp = R[i-1];
		R[i-1] = R[i-1] ^ R[i-1] << 13;
		R[i-1] = R[i-1] ^ R[i-1] >> 17;
		R[i-1] = R[i-1] ^ R[i-1] << 5;
		R[i] = R[i-1];
		R[i-1] = temp;
	}
	return R;
}

unsigned int *importAsArray (char *imgIn)
{
	unsigned int *L;
	unsigned int W = getWidth(imgIn);
	unsigned int H = getHeight(imgIn);
	unsigned int n = W * H, i, j, k;
	L = (unsigned int *) malloc(n * sizeof(unsigned int));

	int padding, fill = 0;
	if(W % 4 != 0)
		padding = 4 - (3 * W) % 4;
	else
		padding = 0;

	FILE *fin = fopen(imgIn,"rb");
	fseek(fin,54,SEEK_SET);
	for(i=0; i<H; i++)
	{
		for(j=0; j<W; j++)
		{
			fread(&L[n-W*(i+1)+j],3,1,fin);
		}
		fread(&fill, padding, 1, fin);
	}
	fclose(fin);
	return L;
}

void exportFromArray (unsigned int *L, unsigned int W, unsigned int H, unsigned char *buffer, char *imgOut)
{
	unsigned int i, j, n = W * H;
	FILE *fout = fopen(imgOut,"wb");
	fseek(fout,0,SEEK_SET);
	for(i=0; i<54; i++)
		fwrite(&buffer[i],1,1,fout);
		
	fseek(fout,54,SEEK_SET);
	
	int padding, fill = 0;
	if(W % 4 != 0)
		padding = 4 - (3 * W) % 4;
	else
		padding = 0;
	
	for(i=0; i<H; i++)
	{
		for(j=0; j<W; j++)
		{
			fwrite(&L[n-W*(i+1)+j],3,1,fout);
		}
		fwrite(&fill, padding, 1, fout);
	}
	fclose(fout);
}

unsigned int **importAsMatrix(char *imgIn)
{
	unsigned int **L;
	unsigned int W = getWidth(imgIn);
	unsigned int H = getHeight(imgIn);
	int i, j;
	L = (unsigned int **) malloc(H * sizeof(unsigned int*));
	for(i=0; i<H; i++) L[i] = (unsigned int *) malloc(W * sizeof(unsigned int));

	unsigned char *buffer = getHeader(imgIn);

	int padding, fill = 0;
	if(W % 4 != 0)
		padding = 4 - (3 * W) % 4;
	else
		padding = 0;

	FILE *fin = fopen(imgIn,"rb");
	fseek(fin,54,SEEK_SET);
	for(i=H-1; i>=0; i--)
	{
		for(j=0; j<W; j++)
		{
			fread(&L[i][j],3,1,fin);
		}
		fread(&fill, padding, 1, fin);
	}
	fclose(fin);
	return L;
}

void exportFromMatrix (unsigned int **I, unsigned int W, unsigned int H, unsigned char *buffer, char *imgOut)
{
	printf("Currently exporting image...\n");
	int i, j;
	unsigned int n = W * H;
	FILE *fout = fopen(imgOut,"wb");
	fseek(fout,0,SEEK_SET);
	for(i=0; i<54; i++)
		fwrite(&buffer[i],1,1,fout);
		
	fseek(fout,54,SEEK_SET);
	
	int padding, fill = 0;
	if(W % 4 != 0)
		padding = 4 - (3 * W) % 4;
	else
		padding = 0;
	
	for(i=H-1; i>=0; i--)
	{
		for(j=0; j<W; j++)
		{
			fwrite(&I[i][j],3,1,fout);
		}
		fwrite(&fill, padding, 1, fout);
	}
	fclose(fout);
	printf("The image was exported successfully under the following filename: %s.",imgOut);
}

unsigned int *permGen (char *secretKey, char *imgIn)
{
	unsigned int W = getWidth(imgIn);
	unsigned int H = getHeight(imgIn);
	unsigned int n = W * H;
	unsigned int *sigma = (unsigned int *) malloc(n * sizeof(unsigned int));
	unsigned int *R = xorGen(secretKey, imgIn);
	int i;
	for(i=0; i<n; i++)
	{
		sigma[i]=i;
	}
	unsigned int x=1;
	for(i=n-1; i>0; i--)
	{
		unsigned int j=R[x]%(i+1);
		unsigned int temp;
		temp=sigma[j];
		sigma[j]=sigma[i];
		sigma[i]=temp;
		x++;
	}
	free(R);
	return sigma;
}

void encrypt (char *secretKey, char *imgIn, char *imgOut)
{
	unsigned int SV;
	FILE *fin_key = fopen(secretKey,"r");
	fscanf(fin_key,"%u",&SV);
	fscanf(fin_key,"%u",&SV);
	fclose(fin_key);
	
	unsigned int W = getWidth(imgIn);
	unsigned int H = getHeight(imgIn);
	unsigned char *buffer = getHeader(imgIn);

	unsigned int i, j, n = W * H;
	unsigned int *C = (unsigned int *) malloc(n * sizeof(unsigned int));
	
	unsigned int *L_perm = (unsigned int *) malloc(n * sizeof(unsigned int));
	unsigned int *sigma = permGen(secretKey, imgIn);
	unsigned int *L = importAsArray(imgIn);
	for(i=0; i<n; i++)
	{
		L_perm[sigma[i]] = L[i];
	}
	
	unsigned int *R = xorGen(secretKey, imgIn);
	
	for (i=0; i<n; i++)
	{
		if (i == 0)
			C[i] = SV ^ L_perm[i] ^ R[n];
		else
			C[i] = C[i-1] ^ L_perm[i] ^ R[n+i];
	}
	
	exportFromArray(C, W, H, buffer, imgOut);

	free(C);
	free(L_perm);
	free(R);
	free(L);
	free(sigma);
}

void decrypt (char *secretKey, char *imgIn, char *imgOut)
{
	unsigned int SV;
	FILE *fin_key = fopen(secretKey,"r");
	fscanf(fin_key,"%u",&SV);
	fscanf(fin_key,"%u",&SV);
	fclose(fin_key);
	
	unsigned int W = getWidth(imgIn);
	unsigned int H = getHeight(imgIn);
	unsigned char *buffer = getHeader(imgIn);

	unsigned int i, j, n = W * H;
	unsigned int *C = (unsigned int *) malloc(n * sizeof(unsigned int));
	unsigned int *L = importAsArray(imgIn);
	
	unsigned int *R = xorGen(secretKey, imgIn);
		
	for(i=0; i<n; i++)
	{
		if(i==0)
			C[i] = SV ^ L[i] ^ R[n];
		else
			C[i] = L[i-1] ^ L[i] ^ R[n+i];
	}
	
	unsigned int *L_perm = (unsigned int *) malloc(n * sizeof(unsigned int));
	unsigned int *sigma = permGen(secretKey, imgIn);

	for(i=0; i<n; i++)
	{
		L_perm[i] = C[sigma[i]];
	}

	exportFromArray(L_perm, W, H, buffer, imgOut);
	
	free(C);
	free(L_perm);
	free(R);
	free(L);
	free(sigma);
}

void chiSq (char *imgIn)
{
	FILE *fin = fopen(imgIn,"rb");
	unsigned char *pRGB = (unsigned char *) malloc(3 * sizeof(unsigned char));
	fseek(fin,54,SEEK_SET);
	double chiB=0, chiG=0, chiR=0;
	int i, j, k;
	unsigned int W = getWidth(imgIn);
	unsigned int H = getHeight(imgIn);
	double f_bar = W * H / 256;
	double *freqB = (double*) calloc(256, sizeof(double));
	double *freqG = (double*) calloc(256, sizeof(double));
	double *freqR = (double*) calloc(256, sizeof(double));
	
	int padding, fill = 0;
	if(W % 4 != 0)
		padding = 4 - (3 * W) % 4;
	else
		padding = 0;
	
	for(i=0; i<H; i++)
	{
		for(j=0; j<W; j++)
		{
			fread(&pRGB[2],1,1,fin);
			fread(&pRGB[1],1,1,fin);
			fread(&pRGB[0],1,1,fin);
			freqB[pRGB[2]]++;
			freqG[pRGB[1]]++;
			freqR[pRGB[0]]++;
		}
		fread(&fill, padding, 1, fin);
	}
	for(i=0; i<=255; i++)
	{
		chiB += ((freqB[i] - f_bar) * (freqB[i] - f_bar)) / f_bar;
		chiG += ((freqG[i] - f_bar) * (freqG[i] - f_bar)) / f_bar;
		chiR += ((freqR[i] - f_bar) * (freqR[i] - f_bar)) / f_bar;
	}
	printf("B: %.02f\n",chiB);
	printf("G: %.02f\n",chiG);
	printf("R: %.02f\n",chiR);
}
