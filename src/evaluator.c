#include "evaluator.h"
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 評価パラメータ更新の度合い */
#define UPDATE_RATIO 0.005

/* パターンの最大評価値 */
#define MAX_PATTERN_VALUE (DISK_VALUE * 20)

/* 評価値更新に必要な出現数 */
#define MIN_FREQUENCY 10

/* 3の冪を表現する定数 */
#define POW_3_0		1
#define POW_3_1		3
#define POW_3_2		9
#define POW_3_3		27
#define POW_3_4		81
#define POW_3_5		243
#define POW_3_6		729
#define POW_3_7		2187
#define POW_3_8		6561
#define POW_3_9		19683
#define POW_3_10	59049

/* パターンID */
enum {
	PATTERN_ID_LINE4,
	PATTERN_ID_LINE3,
	PATTERN_ID_LINE2,
	PATTERN_ID_DIAG8,
	PATTERN_ID_DIAG7,
	PATTERN_ID_DIAG6,
	PATTERN_ID_DIAG5,
	PATTERN_ID_DIAG4,
	PATTERN_ID_EDGE8,
	PATTERN_ID_CORNER8,
	PATTERN_ID_PARITY,
	PATTERN_ID_NUM
};

/* 各パターンの状態数 */
static const int PatternSize[] =
{
	POW_3_8,		/* A4-H4 */
	POW_3_8,		/* A3-H3 */
	POW_3_8,		/* A2-H2 */
	POW_3_8,		/* A1-H8 */
	POW_3_7,		/* A2-G8 */
	POW_3_6,		/* A3-F8 */
	POW_3_5,		/* A4-E8 */
	POW_3_4,		/* A5-D8 */
	POW_3_8,		/* A1-G1 + B2 */
	POW_3_8,		/* A1-C1 + A2-C2 + A3-B3 */
	2,			/* Parity */
	0			/* dummy */
};

struct _Evaluator
{
	int *Value[PATTERN_ID_NUM];
	int *PatternNum[PATTERN_ID_NUM];
	double *PatternSum[PATTERN_ID_NUM];
	int MirrorLine[POW_3_8];
	int MirrorCorner[POW_3_8];
};

static int Evaluator_Initialize(Evaluator *self);
static void Evaluator_Finalize(Evaluator *self);
static void Evaluator_AddPattern(Evaluator *self, int in_pattern, int in_id, int in_mirror, double in_diff);
static void Evaluator_UpdatePattern(Evaluator *self, int in_pattern, int in_id);

static int Evaluator_Initialize(Evaluator *self)
{
	int i, j;
	int mirror_in, mirror_out, coeff;
	int mirror_corner_coeff[] = { POW_3_2, POW_3_5, POW_3_0, POW_3_3, POW_3_6, POW_3_1, POW_3_4, POW_3_7 };

	memset(self, 0, sizeof(Evaluator));
	for (i = 0; i < PATTERN_ID_NUM; i++) {
		self->Value[i] = calloc(PatternSize[i], sizeof(int));
		if (!self->Value[i]) {
			return 0;
		}
		self->PatternNum[i] = calloc(PatternSize[i], sizeof(int));
		if (!self->PatternNum[i]) {
			return 0;
		}
		self->PatternSum[i] = calloc(PatternSize[i], sizeof(double));
		if (!self->PatternSum[i]) {
			return 0;
		}
	}
	for (i = 0; i < POW_3_8; i++) {
		mirror_in = i;
		mirror_out = 0;
		coeff = POW_3_7;
		for (j = 0; j < 8; j++) {
			mirror_out += mirror_in % 3 * coeff;
			mirror_in /= 3;
			coeff /= 3;
		}
		if (mirror_out < i) {
			self->MirrorLine[i] = mirror_out;
		} else {
			self->MirrorLine[i] = i;
		}
	}
	for (i = 0; i < POW_3_8; i++) {
		mirror_in = i;
		mirror_out = 0;
		for (j = 0; j < 8; j++) {
			mirror_out += mirror_in % 3 * mirror_corner_coeff[j];
			mirror_in /= 3;
		}
		if (mirror_out < i) {
			self->MirrorCorner[i] = mirror_out;
		} else {
			self->MirrorCorner[i] = i;
		}
	}

	return 1;
}

static void Evaluator_Finalize(Evaluator *self)
{
	int i;
	for (i = 0; i < PATTERN_ID_NUM; i++) {
		if (self->PatternSum[i]) {
			free(self->PatternSum[i]);
		}
		if (self->PatternNum[i]) {
			free(self->PatternNum[i]);
		}
		if (self->Value[i]) {
			free(self->Value[i]);
		}
	}
}

Evaluator *Evaluator_New(void)
{
	Evaluator *self;

	self = malloc(sizeof(Evaluator));
	if (self) {
		if (!Evaluator_Initialize(self)) {
			Evaluator_Delete(self);
			self = NULL;
		}
	}
	return self;
}

void Evaluator_Delete(Evaluator *self)
{
	Evaluator_Finalize(self);
	free(self);
}

int Evaluator_Load(Evaluator *self, const char *in_file_name)
{
	FILE *fp;
	int i;

	fp = fopen(in_file_name, "rb");
	if (!fp) {
		return 0;
	}
	for (i = 0; i < PATTERN_ID_NUM; i++) {
		if (fread(self->Value[i], sizeof(int), PatternSize[i], fp) < (size_t)PatternSize[i]) {
			fclose(fp);
			return 0;
		}
	}
	fclose(fp);
	return 1;
}

int Evaluator_Save(const Evaluator *self, const char *in_file_name)
{
	FILE *fp;
	int i;

	fp = fopen(in_file_name, "wb");
	if (!fp) {
		return 0;
	}
	for (i = 0; i < PATTERN_ID_NUM; i++) {
		if (fwrite(self->Value[i], sizeof(int), PatternSize[i], fp) < (size_t)PatternSize[i]) {
			fclose(fp);
			return 0;
		}
	}
	fclose(fp);
	return 1;
}

int Evaluator_Value(const Evaluator *self, const Board *in_board)
{
	int result = 0;

	result += self->Value[PATTERN_ID_LINE4][Board_Pattern(in_board, PATTERN_ID_LINE4_1)];
	result += self->Value[PATTERN_ID_LINE4][Board_Pattern(in_board, PATTERN_ID_LINE4_2)];
	result += self->Value[PATTERN_ID_LINE4][Board_Pattern(in_board, PATTERN_ID_LINE4_3)];
	result += self->Value[PATTERN_ID_LINE4][Board_Pattern(in_board, PATTERN_ID_LINE4_4)];
	result += self->Value[PATTERN_ID_LINE3][Board_Pattern(in_board, PATTERN_ID_LINE3_1)];
	result += self->Value[PATTERN_ID_LINE3][Board_Pattern(in_board, PATTERN_ID_LINE3_2)];
	result += self->Value[PATTERN_ID_LINE3][Board_Pattern(in_board, PATTERN_ID_LINE3_3)];
	result += self->Value[PATTERN_ID_LINE3][Board_Pattern(in_board, PATTERN_ID_LINE3_4)];
	result += self->Value[PATTERN_ID_LINE2][Board_Pattern(in_board, PATTERN_ID_LINE2_1)];
	result += self->Value[PATTERN_ID_LINE2][Board_Pattern(in_board, PATTERN_ID_LINE2_2)];
	result += self->Value[PATTERN_ID_LINE2][Board_Pattern(in_board, PATTERN_ID_LINE2_3)];
	result += self->Value[PATTERN_ID_LINE2][Board_Pattern(in_board, PATTERN_ID_LINE2_4)];
	result += self->Value[PATTERN_ID_DIAG8][Board_Pattern(in_board, PATTERN_ID_DIAG8_1)];
	result += self->Value[PATTERN_ID_DIAG8][Board_Pattern(in_board, PATTERN_ID_DIAG8_2)];
	result += self->Value[PATTERN_ID_DIAG7][Board_Pattern(in_board, PATTERN_ID_DIAG7_1)];
	result += self->Value[PATTERN_ID_DIAG7][Board_Pattern(in_board, PATTERN_ID_DIAG7_2)];
	result += self->Value[PATTERN_ID_DIAG7][Board_Pattern(in_board, PATTERN_ID_DIAG7_3)];
	result += self->Value[PATTERN_ID_DIAG7][Board_Pattern(in_board, PATTERN_ID_DIAG7_4)];
	result += self->Value[PATTERN_ID_DIAG6][Board_Pattern(in_board, PATTERN_ID_DIAG6_1)];
	result += self->Value[PATTERN_ID_DIAG6][Board_Pattern(in_board, PATTERN_ID_DIAG6_2)];
	result += self->Value[PATTERN_ID_DIAG6][Board_Pattern(in_board, PATTERN_ID_DIAG6_3)];
	result += self->Value[PATTERN_ID_DIAG6][Board_Pattern(in_board, PATTERN_ID_DIAG6_4)];
	result += self->Value[PATTERN_ID_DIAG5][Board_Pattern(in_board, PATTERN_ID_DIAG5_1)];
	result += self->Value[PATTERN_ID_DIAG5][Board_Pattern(in_board, PATTERN_ID_DIAG5_2)];
	result += self->Value[PATTERN_ID_DIAG5][Board_Pattern(in_board, PATTERN_ID_DIAG5_3)];
	result += self->Value[PATTERN_ID_DIAG5][Board_Pattern(in_board, PATTERN_ID_DIAG5_4)];
	result += self->Value[PATTERN_ID_DIAG4][Board_Pattern(in_board, PATTERN_ID_DIAG4_1)];
	result += self->Value[PATTERN_ID_DIAG4][Board_Pattern(in_board, PATTERN_ID_DIAG4_2)];
	result += self->Value[PATTERN_ID_DIAG4][Board_Pattern(in_board, PATTERN_ID_DIAG4_3)];
	result += self->Value[PATTERN_ID_DIAG4][Board_Pattern(in_board, PATTERN_ID_DIAG4_4)];
	result += self->Value[PATTERN_ID_EDGE8][Board_Pattern(in_board, PATTERN_ID_EDGE8_1)];
	result += self->Value[PATTERN_ID_EDGE8][Board_Pattern(in_board, PATTERN_ID_EDGE8_2)];
	result += self->Value[PATTERN_ID_EDGE8][Board_Pattern(in_board, PATTERN_ID_EDGE8_3)];
	result += self->Value[PATTERN_ID_EDGE8][Board_Pattern(in_board, PATTERN_ID_EDGE8_4)];
	result += self->Value[PATTERN_ID_EDGE8][Board_Pattern(in_board, PATTERN_ID_EDGE8_5)];
	result += self->Value[PATTERN_ID_EDGE8][Board_Pattern(in_board, PATTERN_ID_EDGE8_6)];
	result += self->Value[PATTERN_ID_EDGE8][Board_Pattern(in_board, PATTERN_ID_EDGE8_7)];
	result += self->Value[PATTERN_ID_EDGE8][Board_Pattern(in_board, PATTERN_ID_EDGE8_8)];
	result += self->Value[PATTERN_ID_CORNER8][Board_Pattern(in_board, PATTERN_ID_CORNER8_1)];
	result += self->Value[PATTERN_ID_CORNER8][Board_Pattern(in_board, PATTERN_ID_CORNER8_2)];
	result += self->Value[PATTERN_ID_CORNER8][Board_Pattern(in_board, PATTERN_ID_CORNER8_3)];
	result += self->Value[PATTERN_ID_CORNER8][Board_Pattern(in_board, PATTERN_ID_CORNER8_4)];
	/* parity */
	result += self->Value[PATTERN_ID_PARITY][Board_CountDisks(in_board, EMPTY) & 1];

	return result;
}

static void Evaluator_AddPattern(Evaluator *self, int in_pattern, int in_id, int in_mirror, double in_diff)
{
	self->PatternNum[in_pattern][in_id]++;
	self->PatternSum[in_pattern][in_id] += in_diff;
	if (in_mirror >= 0) {
		self->PatternNum[in_pattern][in_mirror] = self->PatternNum[in_pattern][in_id];
		self->PatternSum[in_pattern][in_mirror] = self->PatternSum[in_pattern][in_id];
	}
}

void Evaluator_Add(Evaluator *self, const Board *in_board, int in_value)
{
	int index;
	double diff;

	diff = (double)(in_value - Evaluator_Value(self, in_board));
	index = Board_Pattern(in_board, PATTERN_ID_LINE4_1);
	Evaluator_AddPattern(self, PATTERN_ID_LINE4, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE4_2);
	Evaluator_AddPattern(self, PATTERN_ID_LINE4, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE4_3);
	Evaluator_AddPattern(self, PATTERN_ID_LINE4, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE4_4);
	Evaluator_AddPattern(self, PATTERN_ID_LINE4, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE3_1);
	Evaluator_AddPattern(self, PATTERN_ID_LINE3, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE3_2);
	Evaluator_AddPattern(self, PATTERN_ID_LINE3, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE3_3);
	Evaluator_AddPattern(self, PATTERN_ID_LINE3, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE3_4);
	Evaluator_AddPattern(self, PATTERN_ID_LINE3, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE2_1);
	Evaluator_AddPattern(self, PATTERN_ID_LINE2, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE2_2);
	Evaluator_AddPattern(self, PATTERN_ID_LINE2, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE2_3);
	Evaluator_AddPattern(self, PATTERN_ID_LINE2, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_LINE2_4);
	Evaluator_AddPattern(self, PATTERN_ID_LINE2, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG8_1);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG8, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG8_2);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG8, self->MirrorLine[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG7_1);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG7, self->MirrorLine[index * POW_3_1], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG7_2);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG7, self->MirrorLine[index * POW_3_1], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG7_3);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG7, self->MirrorLine[index * POW_3_1], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG7_4);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG7, self->MirrorLine[index * POW_3_1], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG6_1);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG6, self->MirrorLine[index * POW_3_2], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG6_2);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG6, self->MirrorLine[index * POW_3_2], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG6_3);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG6, self->MirrorLine[index * POW_3_2], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG6_4);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG6, self->MirrorLine[index * POW_3_2], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG5_1);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG5, self->MirrorLine[index * POW_3_3], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG5_2);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG5, self->MirrorLine[index * POW_3_3], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG5_3);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG5, self->MirrorLine[index * POW_3_3], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG5_4);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG5, self->MirrorLine[index * POW_3_3], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG4_1);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG4, self->MirrorLine[index * POW_3_4], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG4_2);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG4, self->MirrorLine[index * POW_3_4], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG4_3);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG4, self->MirrorLine[index * POW_3_4], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_DIAG4_4);
	Evaluator_AddPattern(self, PATTERN_ID_DIAG4, self->MirrorLine[index * POW_3_4], index, diff);
	Evaluator_AddPattern(self, PATTERN_ID_EDGE8, Board_Pattern(in_board, PATTERN_ID_EDGE8_1), -1, diff);
	Evaluator_AddPattern(self, PATTERN_ID_EDGE8, Board_Pattern(in_board, PATTERN_ID_EDGE8_2), -1, diff);
	Evaluator_AddPattern(self, PATTERN_ID_EDGE8, Board_Pattern(in_board, PATTERN_ID_EDGE8_3), -1, diff);
	Evaluator_AddPattern(self, PATTERN_ID_EDGE8, Board_Pattern(in_board, PATTERN_ID_EDGE8_4), -1, diff);
	Evaluator_AddPattern(self, PATTERN_ID_EDGE8, Board_Pattern(in_board, PATTERN_ID_EDGE8_5), -1, diff);
	Evaluator_AddPattern(self, PATTERN_ID_EDGE8, Board_Pattern(in_board, PATTERN_ID_EDGE8_6), -1, diff);
	Evaluator_AddPattern(self, PATTERN_ID_EDGE8, Board_Pattern(in_board, PATTERN_ID_EDGE8_7), -1, diff);
	Evaluator_AddPattern(self, PATTERN_ID_EDGE8, Board_Pattern(in_board, PATTERN_ID_EDGE8_8), -1, diff);
	index = Board_Pattern(in_board, PATTERN_ID_CORNER8_1);
	Evaluator_AddPattern(self, PATTERN_ID_CORNER8, self->MirrorCorner[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_CORNER8_2);
	Evaluator_AddPattern(self, PATTERN_ID_CORNER8, self->MirrorCorner[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_CORNER8_3);
	Evaluator_AddPattern(self, PATTERN_ID_CORNER8, self->MirrorCorner[index], index, diff);
	index = Board_Pattern(in_board, PATTERN_ID_CORNER8_4);
	Evaluator_AddPattern(self, PATTERN_ID_CORNER8, self->MirrorCorner[index], index, diff);
	Evaluator_AddPattern(self, PATTERN_ID_PARITY, Board_CountDisks(in_board, EMPTY) & 1, -1, diff);
}

static void Evaluator_UpdatePattern(Evaluator *self, int in_pattern, int in_id)
{
	int diff;

	if (self->PatternNum[in_pattern][in_id] > MIN_FREQUENCY) {
		diff = (int)(self->PatternSum[in_pattern][in_id] / self->PatternNum[in_pattern][in_id] * UPDATE_RATIO);
		if (MAX_PATTERN_VALUE - diff < self->Value[in_pattern][in_id]) {
			self->Value[in_pattern][in_id] = MAX_PATTERN_VALUE;
		} else if (-MAX_PATTERN_VALUE - diff > self->Value[in_pattern][in_id]) {
			self->Value[in_pattern][in_id] = -MAX_PATTERN_VALUE;
		} else {
			self->Value[in_pattern][in_id] += diff;
		}
		self->PatternNum[in_pattern][in_id] = 0;
		self->PatternSum[in_pattern][in_id] = 0;
	}
}

void Evaluator_Update(Evaluator *self)
{
	int i, j;

	for (i = 0; i < PATTERN_ID_NUM; i++) {
		for (j = 0; j < PatternSize[i]; j++) {
			Evaluator_UpdatePattern(self, i, j);
		}
	}
}
