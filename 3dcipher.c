#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint64_t u64;

#define file_len(x) (unsigned long)x
#define N 4

static u64 MDS_4 = {0x0804020190b0ddee};//Матрица MDS_key для n=4
static u16 MDSL_a_4 = {0x421c};
static u16 MDSL_a13_4 = {0xb984};
static u8 SbTab_4[16] = {0x7, 0xa, 0x2, 0xc, 0x4, 0x8, 0xf, 0x0, 0x5, 0x9, 0x1, 0xe, 0x3, 0xd, 0xb, 0x6};//Таблица S-блоков для n=4

static u8 Mult_matrix(u8, u16);//Функция умножения матрицы MDS на вектор полубайта
static u64 Key_Matrix(u64, u64);//Функция умножения на матрицу MDS 
static u64 *KeyShedule(u64[]);//Генерация цикл. ключей
static u8 SboxByte(u8);//Функция преобразования S-блока для одного байта
void Word_in_Matrix(u64 *, u8 [][N][N]);//Функция преобразования информационного блока в 3D матрицу
static u64 Matrix_in_Word(u8 [][N][N]);//Функция преобразования 3D матрицы в информационный блок

void KeyAdd(u64 *, u64 *, int);//Добавление циклового ключа к информационному блоку
void SbLayer(u64 *);//Применение S-блоков к информационному блоку
void MDSLayer(u64 *);//Применение операции рассеивания к информационному блоку
void Perm(u64 *);//Применение перестановки к информационному блоку

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
			k <<= 1;
			MDS <<= 1;

			result[i][j] = 0;
		
			for (int k = 0; k < 8; k++)
				result[i][j] += (key[i][k] * mds[k][j]);
		
			retval <<= 1;
			retval |= (u64)(result[i][j] % 2);
		}
	}	

	return retval; 
}

//free memory after using!!!
static u64 *KeyShedule(u64 key[2]) {
	
	u64 *keyRound = (u64*)malloc(16 * sizeof(u64));
	keyRound[0] = key[1];
	keyRound[1] = key[0]^(u64)(1);
	
	for (int i = 0; i < 15; i++) {
		u64 K_A = Key_Matrix(keyRound[i+1], MDS_4); 
		keyRound[i+2] = K_A ^ keyRound[i] ^ (u64)(i+2);
	}

	//for (int i = 0; i < 16; i++) 
	//	printf("0x%016lx\n",keyRound[i]);	
		
	return keyRound; 
}

static u8 SboxByte(u8 x) {
	u8 a, b;
	a = x>>4;
	b = x&0x0f;
	x = ( (SbTab_4[a]<<4) | (SbTab_4[b]) );
	return x;
}

void KeyAdd(u64 *word, u64 *keyRound, int round) {
	
	*word ^= keyRound[round];
}

void SbLayer(u64 *word) {

	u64 _word = *word;
	u64 retval = 0;
	u8 wordtab[8];

	for (int i =0; i < 8; i++) {
		wordtab[i] = (u8)(_word>>56);
		_word <<= 8;
		wordtab[i] = SboxByte(wordtab[i]);
		retval <<= 8;
		retval |= (u64)(wordtab[i]);
		//printf("0x%016lx\n",retval);
	}
	
	*word = retval;
		
}

static u8 Mult_matrix(u8 vector, u16 matrix) {

	u8 retval = 0;
	u8 _matrix[4][4];
	u8 _vector[4];
	u8 result[4];

	
	for (int i = 0; i < 4; i++) {

		_vector[i] = (vector << 7)  >> 7;
		vector >>= 1;
		result[i] = 0;
	}

	for (int i = 0; i < 4; i++) {
	
		for (int j = 0; j < 4; j++) {
			_matrix[i][j] = (u8)(matrix >> 15);
			matrix <<= 1;
			result[i] += (_matrix[i][j] * _vector[j]);
		}

		retval <<= 1;
		retval |= (result[i] % 2);
	}
	return retval;

}

void MDSLayer(u64 *word) {

	u64 _word = *word;
	//printf("0x%lx\n",_word);
	u8 tab[4][4] = {}; //tab for nibbles
	u8 tab_res[4][4] = {}; //tab for result nibbles after MDS
	
	for (int i = 0; i < 4; i++) {

		for (int j = 0; j < 4; j++) {
			
			tab[i][j] = (u8)(_word >> 60);
			_word <<= 4;
			//printf("0x%x\n", tab[i]);
		}
	}

	for (int i = 0; i < 4; i ++) {
		
		for (int j = 0; j < 4; j++) {
			
			u8 i_0 = tab[i][0];
			u8 i_1 = tab[i][1];
			tab[i][0] = Mult_matrix(tab[i][1], MDSL_a13_4) ^ tab[i][2];
			tab[i][1] = Mult_matrix(tab[i][1], MDSL_a_4) ^ tab[i][3] ^ i_0;
			tab[i][2] = i_0;
			tab[i][3] = i_1;
		}	
	}
	
	_word = 0;
	int k = 0;

	for (int i = 0; i < 4; i ++) {		
		for (int j = 0; j < 4; j++) {
	//	printf("0x%x\t",tab[i][j]);	
			_word |= (u64)(tab[i][j]) << k;
			k += 4;
		}
	//	printf("\n");
	}
	*word = _word;

}	

void Word_in_Matrix(u64 *word, u8 matrix[][N][N]) {

	u64 _word = *word;
	u64 bit = 0;

	for (int i = 0; i < 4; i++) {		
		for (int j = 0; j < 4; j++){
			for (int k = 0; k < 4; k++) {
				bit = _word >> 63;
				matrix[i][j][k] = (u8)bit;
				_word <<= 1;
			}
		}
	}
}

static u64 Matrix_in_Word(u8 matrix[][N][N]) {

	u64 word = 0;
	for (int i = 0; i < 4; i ++)		
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k ++) {
			
				word <<= 1;
				word |= (u64)(matrix[i][j][k]);
			}
	return word;	
}

void Perm(u64 *word) {

	u8 matrix[4][4][4] = {0};
	u64 _word = *word;
	Word_in_Matrix(word, matrix);

	u8 matrix_res[4][4][4] = {0};

	for (int i = 0; i < 4; i++)		
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++) 
				matrix_res[i][j][k] = matrix[3-j][3-k][i];

	_word = Matrix_in_Word(matrix_res);
	*word = _word;
}


int main(int argc, char* argv[]) {
	
	u64 key[2] = {0x0001020304050607, 0x08090a0b0c0d0e0f};
	u64 *keytab = KeyShedule(key);
	free(keytab);

	u64 x = 1;
	u8 a[4][4][4] = {0};
	
	for (int i = 0; i < 15; i++) {
		KeyAdd(&x, keytab, i+1);
		SbLayer(&x);
		MDSLayer(&x);
		Perm(&x);
	}

	KeyAdd(&x, keytab, 16);
	
	printf("0x%016lx\n",x);

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

