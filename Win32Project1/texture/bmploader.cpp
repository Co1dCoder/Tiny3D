#include "bmploader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BmpLoader::BmpLoader() {
	header=(unsigned char*)malloc(54*sizeof(unsigned char));
	memset(header, 0, 54 * sizeof(unsigned char));
	data = NULL;
	tmp = NULL;
}

BmpLoader::~BmpLoader() {
	if (header) free(header); header = NULL;
	if (data) free(data); data = NULL;
	if (tmp) free(tmp); tmp = NULL;
}

bool BmpLoader::loadBitmap(const char* fileName) {
	FILE* file = fopen(fileName,"rb");
	if (!file) {
		printf("Image could not be opened %s\n", fileName);
		return false;
	}
	if (fread(header, 1, 54, file)!=54) {//�ļ�ͷ����54�ֽ� ��ȡʧ��
	    printf("Not a correct BMP file\n");
		fclose(file);
	    return false;
	}
	if (header[0]!='B' || header[1]!='M') {//�ļ�ͷ��ͷ����BM ��ȡʧ��
	    printf("Not a correct BMP file\n");
		fclose(file);
	    return false;
	}

	dataPos    = *(int*)&(header[0x0A]);//��ȡλ�� λ�����ļ�ͷ0x0A��
	imageSize  = *(int*)&(header[0x22]);//ͼƬ���ݴ�С���� λ�����ļ�ͷ0x22��
	width      = *(int*)&(header[0x12]);//ͼƬ������� λ�����ļ�ͷ0x12��
	height     = *(int*)&(header[0x16]);//ͼƬ�߶����� λ�����ļ�ͷ0x16��
	bit        = *(unsigned short*)&(header[0x1C]);//ͼƬλ�� λ�����ļ�ͷ0x1C��

	hasAlpha=bit==32?true:false; //32λ?24λ?

	int channelCount=hasAlpha?4:3;

	if (imageSize==0)
		imageSize=width*height*channelCount; //ͼƬ��Сx��ɫͨ����
	if (dataPos==0)
		dataPos=54;//�ļ�ͷ���� λ����54�ֽڴ�

	tmp = (unsigned char*)malloc(imageSize * sizeof(unsigned char));//data��������Ϣ
	memset(tmp, 0, imageSize * sizeof(unsigned char));
	fread(tmp, 1, imageSize, file);//��ȡ����
	fclose(file);

	for(int i = 0;i<(int)imageSize;i+=channelCount)
		swapcolor(tmp[i],tmp[i+2]);//bgr��Ϊrgb

	if (!hasAlpha) {
		imageSize = width * height * 4;
		bit = 32;
	}

	data = (unsigned char*)malloc(imageSize * sizeof(unsigned char));
	memset(data, 0, imageSize * sizeof(unsigned char));
	if (hasAlpha)
		memcpy(data, tmp, imageSize * sizeof(unsigned char));
	else {
		for (unsigned int p = 0; p < width * height; p++) {
			data[p * 4 + 0] = tmp[p * 3 + 0];
			data[p * 4 + 1] = tmp[p * 3 + 1];
			data[p * 4 + 2] = tmp[p * 3 + 2];
			data[p * 4 + 3] = 255;
		}
		hasAlpha = true;
	}

	printf("load image: %s OK!\n", fileName);
	return true;
}

int BmpLoader::getWidth() {
	return width;
}

int BmpLoader::getHeight() {
	return height;
}


