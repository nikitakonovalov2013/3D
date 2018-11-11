#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

typedef uint8_t            u8;
typedef unsigned short int u16;
typedef unsigned int       u32;
typedef uint64_t           u64;

#define file_len(x) (unsigned long)x
#define N 4

static u8 SbTab[16] = {0x7, 0xa, 0x2, 0xc, 0x4, 0x8, 0xf, 0x0, 0x5, 0x9, 0x1, 0xe, 0x3, 0xd, 0xb, 0x6};//Таблица S-блоков для n=4

static u8 xtime(u8);
static u8 bmul(u8, u8);

static u64 KeyShedule(u64, u64);//Генерация цикл. ключей
static u8 SboxByte(u8);//Функция преобразования S-блока для одного байта
static u8 InTab(u64);//Функция преобразования информационного блока в 3D матрицу

static u64 KeyAdd(u64, u64);//Добавление циклового ключа к информационному блоку
static u8 SbLayer(u8);//Применение S-блоков к информационному блоку
static u8 MDSLayer(u8);//Применение операции рассеивания к информационному блоку
static u8 Perm(u8);//Применение перестановки к информационному блоку

void Cipher(u64, u64);//Функция шифрования
void InvCipher(u64, u64);//Функция расшифрования

static u8 SboxByte(u8 x) {
	u8 a, b;
	a = x>>4;
	b = x&0x0f;
	x = ( (SbTab[a]<<4) | (SbTab[b]) );
	return x;
}

int main(int argc, char* argv[]) {

	u8 *in[N][N][N];
	u8 *out[N][N][N];

	uint8_t x = 0x68;
	x = SboxByte(x);
	printf("0x%x \n", x);
	FILE* stream_in;
	FILE* stream_out;

	if ( !(stream_in = fopen(argv[1], "rb")) ) {
		printf("File in: %s cannot be open\n", argv[1]);
		return -1;
	}
	

	if ( !(stream_out = fopen(argv[2], "wb")) ) {
		printf("File out: %s cannot be open\n", argv[3]);
                return -1;
        }

	fpos_t flen;
	// определение длины файла
        fseek(stream_in, 0, SEEK_END);
        fgetpos(stream_in, &flen); 
        unsigned long rlen = file_len(&flen);
	
	// переход на начало
	fseek(stream_in, 0, SEEK_SET);
	unsigned long len;


/*	char *array[4][4][4];
	char *name = "plaintxt";
	FILE *f = fopen( fname, "r");

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				//fread( fd, array[i][j][k], 1);
				printf("%s ", array[i][j][k]);
			}
			printf("\n");
		}
		printf("\n");
	}
	fclose(f);*/
}

