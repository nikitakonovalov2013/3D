#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

typedef uint8_t  u8;
typedef uint64_t u64;

#define file_len(x) (unsigned long)x
#define b_64(x) 0x0000000000000000 | x

#define N 4

static u64 MDS_4 = {0x0804020190b0ddee};//Матрица MDS для n=4
static u8 SbTab_4[16] = {0x7, 0xa, 0x2, 0xc, 0x4, 0x8, 0xf, 0x0, 0x5, 0x9, 0x1, 0xe, 0x3, 0xd, 0xb, 0x6};//Таблица S-блоков для n=4


static u64 Key_Matrix(u64, u64);//Функция умножения на матрицу MDS 
static u64 *KeyShedule(u64[]);//Генерация цикл. ключей
static u8 SboxByte(u8);//Функция преобразования S-блока для одного байта
static u8 InTab(u64);//Функция преобразования информационного блока в 3D матрицу

static u64 KeyAdd(u64, u64);//Добавление циклового ключа к информационному блоку
static u8 SbLayer(u8);//Применение S-блоков к информационному блоку
static u8 MDSLayer(u8);//Применение операции рассеивания к информационному блоку
static u8 Perm(u8);//Применение перестановки к информационному блоку

static u64 Cipher(u64, u64);//Функция шифрования
static u64 InvCipher(u64, u64);//Функция расшифрования

static u64 Key_Matrix(u64 k, u64 MDS) {

	u8 key[8][8];
	u8 mds[8][8];
	u8 result[8][8];
	u64 retval = 0;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			key[i][j] = (u8)(k>>63);
			mds[i][j] = (u8)(MDS>>63);
			k = k<<1;
			MDS = MDS<<1;

			result[i][j] = 0;
		
			for (int k = 0; k < 8; k++)
				result[i][j] = result[i][j] + (key[i][k]*mds[k][j]);
		
			retval |= (u64)result[i][j];
			retval <<= 1;
		}
	}	

	return retval; 
}

static u64 *KeyShedule(u64 key[2]) {
	
	u64 *keyRound = (u64*)malloc(16 * sizeof(u64));
	keyRound[0] = key[1];
	keyRound[1] = key[0]^b_64(1);
	
	for (int i = 2; i <= 15; i++) {
		u64 K_A = Key_Matrix(keyRound[i-1], MDS_4); 
		keyRound[i] =K_A^keyRound[i-2]^b_64(i);
		printf("0x%016lx\n",keyRound[i]);	
	}
		
	return keyRound; 
}

static u8 SboxByte(u8 x) {
	u8 a, b;
	a = x>>4;
	b = x&0x0f;
	x = ( (SbTab_4[a]<<4) | (SbTab_4[b]) );
	return x;
}

int main(int argc, char* argv[]) {

	u64 key[2] = {0x0001020304050607, 0x08090a0b0c0d0e0f};
	u64 *keytab = KeyShedule(key);
	free(keytab);

	u8 *in[N][N][N];
	u8 *out[N][N][N];

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


/*	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
			}
		}
	}
*/
}

