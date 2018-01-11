#ifndef _BMPLOADER_H_
#define _BMPLOADER_H_

#define swapcolor(a,b){ \
        (a) ^= (b);     \
        (b) ^= (a);     \
        (a) ^= (b);     \
}

class BmpLoader {
private:
	unsigned char* header;//�ļ�ͷ
	unsigned int dataPos;//��ȡλ��
	unsigned int width, height;//ͼƬ��� �߶�
	unsigned int imageSize;//ͼƬ���ݴ�С
	unsigned short bit;//λ��
public:
	unsigned char* data;//ͼƬ���� rgb
	bool hasAlpha;
	BmpLoader();
	~BmpLoader();
	int getWidth();
	int getHeight();
	bool loadBitmap(const char* fileName);
};

#endif
