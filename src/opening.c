#include "opening.h"
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 局面情報のブロックサイズ
   PositionInfoのメモリ領域はNUM_INFO_BLOCK * sizeof(PositionInfo)
   の整数倍確保するようにする
*/
#define NUM_INFO_BLOCK		0x010000

/* 盤面の状態をあらわす構造体
   bl、bhに手番の石の状態を、wl、whに手番でない石の状態を格納する
   石が存在する場合には各ビットを1にする */
typedef struct _PositionKey PositionKey;
struct _PositionKey
{
	unsigned long int bl;
	unsigned long int bh;
	unsigned long int wl;
	unsigned long int wh;
};

/* 局面データ */
typedef struct _PositionData PositionData;
struct _PositionData
{
	PositionKey key;
	PositionInfo info;
};

struct _Opening
{
	int Num;			/* 局面データの数 */
	int Max;			/* 保持可能な局面データの数 */
	PositionData *Data;	/* 局面データ */
};

static int Opening_Initialize(Opening *self);
static void Opening_Finalize(Opening *self);
static int Opening_Read(Opening *self, FILE *fp);
static int Opening_Write(const Opening *self, FILE *fp);
static PositionInfo * Opening_Find(const Opening *self, const Board *in_board, int in_color);
static int PositionKey_Comp(const PositionKey *in_key1, const PositionKey *in_key2);
static void Board_Key(const Board *in_board, int in_color, PositionKey *out_key);
static int Board_RotatePos(int in_x, int in_y, int in_type);

static int Opening_Initialize(Opening *self)
{
	memset(self, 0, sizeof(Opening));
	self->Num = 0;
	self->Max = NUM_INFO_BLOCK;
	self->Data = malloc(self->Max * sizeof(PositionData));
	if (!self->Data) {
		return 0;
	}
	return 1;
}

static void Opening_Finalize(Opening *self)
{
	if (self->Data) {
		free(self->Data);
	}
}

Opening *Opening_New(void)
{
	Opening *self;
	self = malloc(sizeof(Opening));
	if (self) {
		if (!Opening_Initialize(self)) {
			Opening_Delete(self);
			self = NULL;
		}
	}
	return self;
}

void Opening_Delete(Opening *self)
{
	Opening_Finalize(self);
	free(self);
}

static int Opening_Read(Opening *self, FILE *fp)
{
	PositionData *data = NULL;

	if (fread(&self->Num, sizeof(int), 1, fp) < 1) {
		return 0;
	}
	if (self->Num % NUM_INFO_BLOCK == 0) {
		self->Max = (self->Num / NUM_INFO_BLOCK + 1) * NUM_INFO_BLOCK;
	} else {
		self->Max = (self->Num / NUM_INFO_BLOCK + 2) * NUM_INFO_BLOCK;
	}
	data = realloc(self->Data, self->Max * sizeof(PositionData));
	if (!data) {
		return 0;
	}
	self->Data = data;
	if (fread(self->Data, sizeof(PositionData), self->Num, fp) < (size_t)self->Num) {
		return 0;
	}
	return 1;
}

static int Opening_Write(const Opening *self, FILE *fp)
{
	if (fwrite(&self->Num, sizeof(int), 1, fp) < 1) {
		return 0;
	}
	if (fwrite(self->Data, sizeof(PositionData), self->Num, fp) < (size_t)self->Num) {
		return 0;
	}
	return 1;
}

int Opening_Load(Opening *self, const char *in_file_name)
{
	FILE *fp;

	fp = fopen(in_file_name, "rb");
	if (!fp) {
		return 0;
	}
	if (!Opening_Read(self, fp)) {
		fclose(fp);
		return 0;
	}
	fclose(fp);
	return 1;
}

int Opening_Save(const Opening *self, const char *in_file_name)
{
	FILE *fp;

	fp = fopen(in_file_name, "wb");
	if (!fp) {
		return 0;
	}
	if (!Opening_Write(self, fp)) {
		fclose(fp);
		return 0;
	}
	fclose(fp);
	return 1;
}

static int PositionKey_Comp(const PositionKey *in_key1, const PositionKey *in_key2)
{
	if (in_key1->wh > in_key2->wh) {
		return 1;
	} else if (in_key1->wh < in_key2->wh) {
		return -1;
	} else if (in_key1->wl > in_key2->wl) {
		return 1;
	} else if (in_key1->wl < in_key2->wl) {
		return -1;
	} else if (in_key1->bh > in_key2->bh) {
		return 1;
	} else if (in_key1->bh < in_key2->bh) {
		return -1;
	} else if (in_key1->bl > in_key2->bl) {
		return 1;
	} else if (in_key1->bl < in_key2->bl) {
		return -1;
	}
	return 0;
}

static int Board_RotatePos(int in_x, int in_y, int in_type)
{
	switch (in_type) {
	case 0:
		return Board_Pos(in_x, in_y);
	case 1:
		return Board_Pos(BOARD_SIZE - in_x - 1, in_y);
	case 2:
		return Board_Pos(in_x, BOARD_SIZE - in_y - 1);
	case 3:
		return Board_Pos(BOARD_SIZE - in_x - 1, BOARD_SIZE - in_y - 1);
	case 4:
		return Board_Pos(in_y, in_x);
	case 5:
		return Board_Pos(BOARD_SIZE - in_y - 1, in_x);
	case 6:
		return Board_Pos(in_y, BOARD_SIZE - in_x - 1);
	case 7:
		return Board_Pos(BOARD_SIZE - in_y - 1, BOARD_SIZE - in_x - 1);
	default:
		break;
	}
	return 0;
}

static void Board_Key(const Board *in_board, int in_color, PositionKey *out_key)
{
	PositionKey key;
	unsigned int flag;
	int i, x, y, c;
	int op_color;

	op_color = Board_OpponentColor(in_color);
	for (i = 0; i < 8; i++) {
		memset(&key, 0, sizeof(PositionKey));
		flag = 1;
		for (y = 0; y < BOARD_SIZE / 2; y++) {
			for (x = 0; x < BOARD_SIZE; x++) {
				c = Board_Disk(in_board, Board_RotatePos(x, y, i));
				if (c == in_color) {
					key.bl |= flag;
				} else if (c == op_color) {
					key.wl |= flag;
				}
				c = Board_Disk(in_board, Board_RotatePos(x, y + BOARD_SIZE / 2, i));
				if (c == in_color) {
					key.bh |= flag;
				} else if (c == op_color) {
					key.wh |= flag;
				}
				flag <<= 1;
			}
		}
		if (i == 0 || PositionKey_Comp(&key, out_key) < 0) {
			*out_key = key;
		}
	}
}

static PositionInfo * Opening_Find(const Opening *self, const Board *in_board, int in_color)
{
	int i;
	PositionKey key;

	Board_Key(in_board, in_color, &key);
	for (i = 0; i < self->Num; i++) {
		if (!PositionKey_Comp(&key, &self->Data[i].key)) {
			return &self->Data[i].info;
		}
	}
	return NULL;
}

int Opening_Info(const Opening *self, const Board *in_board, int in_color, PositionInfo *out_info)
{
	PositionInfo *info;

	info = Opening_Find(self, in_board, in_color);
	if (!info) {
		return 0;
	}
	*out_info = *info;
	return 1;
}

int Opening_SetInfo(Opening *self, const Board *in_board, int in_color, const PositionInfo *in_info)
{
	PositionInfo *info;
	PositionData *data;

	info = Opening_Find(self, in_board, in_color);
	if (info) {
		*info = *in_info;
	} else {
		if (self->Num >= self->Max) {
			self->Max += NUM_INFO_BLOCK;
			data = realloc(self->Data, self->Max * sizeof(PositionData));
			if (!data) {
				return 0;
			}
			self->Data = data;
		}
		Board_Key(in_board, in_color, &self->Data[self->Num].key);
		self->Data[self->Num].info = *in_info;
		self->Num++;
	}
	return 1;
}
