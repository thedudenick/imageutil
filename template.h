#include "struct.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void grayscaleConvertor(char *imgIn)
{
	FILE *fin, *fout;
	unsigned int W = getWidth(imgIn);
	unsigned int H = getHeight(imgIn);
	unsigned char pRGB[3], header[54], aux;

    fin = fopen(imgIn, "rb");
    printf("Currently converting %s to grayscale...\n",imgIn);
	int x = strlen(imgIn) + 4;
	char imgOut[x];
	strcpy(imgOut,imgIn);
	imgOut[x-8]='G';
	imgOut[x-7]='r';
	imgOut[x-6]='a';
	imgOut[x-5]='y';
	imgOut[x-4]='.';
	imgOut[x-3]='b';
	imgOut[x-2]='m';
	imgOut[x-1]='p';
	imgOut[x]='\0';
    fout = fopen(imgOut, "wb+");

	fseek(fin,0,SEEK_SET);
	unsigned char c;
	while(fread(&c,1,1,fin)==1)
	{
		fwrite(&c,1,1,fout);
	}
	fclose(fin);
	
	int padding;
    if(W % 4 != 0)
        padding = 4 - (3 * W) % 4;
    else
        padding = 0;

	fseek(fout, 54, SEEK_SET);
	int i,j;
	for(i = 0; i < H; i++)
	{
		for(j = 0; j < W; j++)
		{
			fread(pRGB, 3, 1, fout);
			aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
			pRGB[0] = pRGB[1] = pRGB[2] = aux;
        	fseek(fout, -3, SEEK_CUR);
        	fwrite(pRGB, 3, 1, fout);
		}
		fseek(fout,padding,SEEK_CUR);
	}
	fclose(fout);
	printf("%s was successfully converted to grayscale, under the following filename: %s.\n",imgIn,imgOut);
	strcpy(imgIn, imgOut);
}

double avgValueCalculator (unsigned int **M, unsigned int W_tmp, unsigned int H_tmp, int x, int y, double n)
{
	int i, j;
	double bar = 0;
	for(i=x; i < x + H_tmp; i++)
	{
		for(j=y; j < y + W_tmp; j++)
		{
			bar += (M[i][j] & 0xff);
		}
	}
	bar /= n;
	return bar;
}

double grayscaleDiffCalculator(unsigned int **M, double bar, unsigned int W_tmp, unsigned int H_tmp, int x, int y, double n)
{
	double sigmaM = 0;
	int i, j;
	for(i=x; i < x + H_tmp; i++)
	{
		for(j=y; j < y + W_tmp; j++)
		{
			sigmaM += pow(( (M[i][j] & 0xff) - bar),2);
		}
	}
	return sqrt(sigmaM / (n-1));
}

double corrCalculator (unsigned int **I, unsigned int **S, int x, int y, unsigned int W_tmp, unsigned int H_tmp, double S_bar, double sigmaS)
{
	double corr = 0;
	double n = W_tmp * H_tmp;
	int i, j;
	double F_bar = avgValueCalculator(I, W_tmp, H_tmp, x, y, n);
	double sigmaF = grayscaleDiffCalculator(I, F_bar, W_tmp, H_tmp, x, y, n);
	for(i=0; i < H_tmp; i++)
	{
		for(j=0; j < W_tmp; j++)
		{
			corr += (( (I[x+i][y+j] & 0xff) - F_bar) * ( (S[i][j] & 0xff) - S_bar)) / (sigmaF * sigmaS);
		}
	}
	corr /= n;
	return corr;
}

Match *templateMatching(unsigned int **I, unsigned int **S, double ps, unsigned int W_img, unsigned int H_img, unsigned int W_tmp, unsigned int H_tmp, int *cnt)
{
	printf("Currently processing template...\n");
	*cnt=0;
	int i, j;
	double corr;
	double n = W_tmp * H_tmp;
	unsigned int H = H_img - H_tmp;
	unsigned int W = W_img - W_tmp;
	Match *v = malloc(W * H * sizeof(Match));
	double S_bar = avgValueCalculator(S, W_tmp, H_tmp, 0, 0, n);
	double sigmaS = grayscaleDiffCalculator(S, S_bar, W_tmp, H_tmp, 0, 0, n);
	for(i=0; i<H; i++)
	{
		for(j=0; j<W; j++)
		{
			corr = corrCalculator(I, S, i, j, W_tmp, H_tmp, S_bar, sigmaS);
			if(corr >= ps)
			{
				v[(*cnt)].corr = corr;
				v[(*cnt)].i = i;
				v[(*cnt)].j = j;
				(*cnt)++;
			}
		}
	}
	Match *v2 = realloc(v, (*cnt) * sizeof(Match));
	v=v2;
	printf("Template processed successfully.\n\n");
	return v;
}

unsigned int **generateBorder(unsigned int **I, unsigned int x, unsigned int y, unsigned char c[3], unsigned int W_tmp, unsigned int H_tmp)
{
	unsigned int color;
	color = color & 0;
	color = color | c[0];
	color = color << 8;
	color = color | c[1];
	color = color << 8;
	color = color | c[2];
	int i, j;
	for(i = x; i < x + H_tmp; i++)
	{
		for(j = y; j < y + W_tmp; j++)
		{
			if(i==x || i==x+H_tmp-1) I[i][j] = color;
			else {
				I[i][y] = color;
				I[i][y+W_tmp-1] = color;
			}
		}
	}
	return I;
}

Match *matchSelect(Match *v, int corrTotal, unsigned int W_tmp, unsigned int H_tmp)
{
	printf("Currently eliminating ineligible matches...\n");
	int i, j;
	double overlap, overlapArea, combinedArea;
	for(i=0; i<corrTotal-1; i++)
	{
		for(j=i+1; j<corrTotal; j++)
		{
			int topLeft = MAX(v[i].j, v[j].j);
			int topRight = MIN(v[i].j + W_tmp, v[j].j + W_tmp);
			int bottomLeft = MIN(v[i].i + H_tmp, v[j].i + H_tmp);
			int bottomRight = MAX(v[i].i, v[j].i);
						
			overlapArea = 0;
			if(topLeft < topRight && bottomLeft > bottomRight)
				overlapArea = (topRight - topLeft) * (bottomLeft - bottomRight);
				
			combinedArea = 2 * W_tmp * H_tmp - overlapArea;
			overlap = overlapArea/combinedArea;
			
			if(overlap > 0.2 && v[i].corr > v[j].corr)
			{
				v[j].corr = -1;
			}
		}
	}
	return v;
}
