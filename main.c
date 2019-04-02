#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "general.h"
#include "encrypt.h"
#include "template.h"

int largerThan(const void *a, const void *b)
{
	Match va = *(Match *)a;
	Match vb = *(Match *)b;
	if(va.corr < vb.corr) return 1;
	if(va.corr > vb.corr) return -1;
	return 0;
}

int main(int argc, char *argv[]) {
	char secretKey[101], imgIn[101], imgOut[101];
	printf("Encryption\n----------\n");
	
	printf("Path to secret key: ");
	fgets(secretKey, 101, stdin);
	secretKey[strlen(secretKey) - 1] = '\0';
	
	if(xorGen(secretKey, imgIn)==(unsigned int *)-1)
	{
		printf("Cannot open file, run the program again.");
		return 0;
	}
	
	printf("Path to image: ");
	fgets(imgIn, 101, stdin);
	imgIn[strlen(imgIn) - 1] = '\0';
	
	if(getHeight(imgIn)==-1 || getWidth(imgIn)==-1)
	{
		printf("Cannot open file, run the program again.");
		return 0;
	}

	printf("Encrypted image filename: ");
	fgets(imgOut, 101, stdin);
	imgOut[strlen(imgOut) - 1] = '\0';
	
	encrypt(secretKey, imgIn, imgOut);
	printf("Encrypted image saved as %s.\n\n",imgOut);
	
	printf("Path to image for decryption: ");
	fgets(imgIn, 101, stdin);
	imgIn[strlen(imgIn) - 1] = '\0';
	
	if(getHeight(imgIn)==-1 || getWidth(imgIn)==-1)
	{
		printf("Cannot open file, run the program again.");
		return 0;
	}
	
	printf("Decrypted image filename: ");
	fgets(imgOut, 101, stdin);
	imgOut[strlen(imgOut) - 1] = '\0';
	
	decrypt(secretKey, imgIn, imgOut);
	printf("Decrypted image saved as %s.\n\n",imgOut);
	
	printf("Chi-squared test values for encrypted image:\n");
	chiSq(imgIn);
	printf("Chi-squared test values for decrypted image:\n");
	chiSq(imgOut);
	
	printf("\n");
	printf("Template Matching\n-----------------\n");
	
	printf("Main image filename: ");
	fgets(imgIn, 101, stdin);
	imgIn[strlen(imgIn) - 1] = '\0';
	
	if(getHeight(imgIn)==-1 || getWidth(imgIn)==-1)
	{
		printf("Cannot open file, run the program again.");
		return 0;
	}

	unsigned int **I_color = importAsMatrix(imgIn); 
	grayscaleConvertor(imgIn);
	unsigned int **I = importAsMatrix(imgIn);
	unsigned int W_img = getWidth(imgIn);
	unsigned int H_img = getHeight(imgIn);
		
	unsigned char *header = getHeader(imgIn);
	int cnt, corrTotal=0, templateNumber, ctrl=0;
	printf("Number of templates: ");
	scanf("%d",&templateNumber);
	fgetc(stdin);
	printf("\n");
	double ps = 0.5;
	int i, j, k;
	Match *v_total;
		
	for(i=0; i<templateNumber; i++)
	{
		printf("Path for template %d: ",i+1);
		fgets(imgIn, 101, stdin);
		imgIn[strlen(imgIn) - 1] = '\0';
		
		if(getHeight(imgIn)==-1 || getWidth(imgIn)==-1)
		{
			printf("Cannot open file, run the program again.");
			return 0;
		}

		grayscaleConvertor(imgIn);
		unsigned int **S = importAsMatrix(imgIn);
		unsigned int W_tmp = getWidth(imgIn);
		unsigned int H_tmp = getHeight(imgIn);
		
		Match *v = templateMatching(I, S, ps, W_img, H_img, W_tmp, H_tmp, &cnt);
		unsigned char c[3];
				
		printf("Enter color code values for template %d, separated by spaces: ",i+1);
		scanf("%hhu %hhu %hhu", &c[0], &c[1], &c[2]);
		fgetc(stdin);
		printf("\n");
		for(j=0; j<cnt; j++)
		{
			v[j].c[0] = c[0];
			v[j].c[1] = c[1];
			v[j].c[2] = c[2];
		}
		
		corrTotal += cnt;
		if(i==0) v_total = malloc(corrTotal * sizeof(Match));
		else{
			Match *v2_total = realloc(v_total, corrTotal * sizeof(Match));
			v_total = v2_total;
		}
		for(j=ctrl, k=0; j<corrTotal; j++, k++)
		{
			v_total[j] = v[k];
		}
		ctrl+=cnt;
	}
	
	unsigned int W_tmp = getWidth(imgIn);
	unsigned int H_tmp = getHeight(imgIn);
	qsort(v_total, corrTotal, sizeof(Match), largerThan);
	v_total = matchSelect(v_total, corrTotal, W_tmp, H_tmp);
	printf("Generating borders...\n");
	for(j=0; j<corrTotal; j++)
	{
		if(v_total[j].corr!=-1)
		I = generateBorder(I_color, v_total[j].i, v_total[j].j, v_total[j].c, W_tmp, H_tmp);
	}
	exportFromMatrix(I_color, W_img, H_img, header, "processedImage.bmp");
	
	printf("\n---End---");
}