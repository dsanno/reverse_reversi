#include <stdlib.h>
#include "board.h"
#include "hash.h"

#define NUM_DISK	((BOARD_SIZE+1)*(BOARD_SIZE+2)+1)
#define NUM_STACK	(((BOARD_SIZE-2)*3+3)*BOARD_SIZE*BOARD_SIZE)
#define NUM_PATTERN_DIFF	6
#define NUM_HASH_PATTERN	10
#define NUM_HASH_DIFF		6561

#define DIR_UP_LEFT		(-BOARD_SIZE-2)
#define DIR_UP			(-BOARD_SIZE-1)
#define DIR_UP_RIGHT	(-BOARD_SIZE)
#define DIR_LEFT		(-1)
#define DIR_RIGHT		(1)
#define DIR_DOWN_LEFT	(BOARD_SIZE)
#define DIR_DOWN		(BOARD_SIZE+1)
#define DIR_DOWN_RIGHT	(BOARD_SIZE+2)

struct _Board
{
	int Disk[NUM_DISK];
	int Stack[NUM_STACK];
	int *Sp;
	int DiskNum[3];
	int Pattern[NUM_PATTERN_ID];
	int PatternID[NUM_DISK][NUM_PATTERN_DIFF];
	int PatternDiff[NUM_DISK][NUM_PATTERN_DIFF];
	HashKey HashDiff[NUM_HASH_PATTERN][NUM_HASH_DIFF];
};

#define OPPONENT_COLOR(c)			(BLACK + WHITE - c)
#define BOARD_STACK_POP(self)		(*(--self->Sp))
#define BOARD_STACK_PUSH(self, n)	(*(self->Sp++) = n)

static int Board_Initialize(Board *self);
static int Board_Finalize(Board *self);
static int Board_FlipLine(Board *self, int in_color, int in_pos, int in_dir);
static int Board_CountFlipsLine(const Board *self, int in_color, int in_pos, int in_dir);
static void Board_InitializePatternDiff(Board *self);
static void Board_AddPattern(Board *self, int in_id, const int *in_pos_list, int in_num);
static int Board_FlipLinePattern(Board *self, int in_color, int in_pos, int in_dir);
static void Board_FlipSquareBlack(Board *self, int in_pos);
static void Board_FlipSquareWhite(Board *self, int in_pos);
static void Board_PutSquareBlack(Board *self, int in_pos);
static void Board_PutSquareWhite(Board *self, int in_pos);
static void Board_RemoveSquareBlack(Board *self, int in_pos);
static void Board_RemoveSquareWhite(Board *self, int in_pos);
#define get_rand(in_max) ((int)((double)(in_max) * rand() / (RAND_MAX + 1.0)))

Board *Board_New(void)
{
	Board *self;

	self = malloc(sizeof(Board));
	if (self) {
		Board_InitializePatternDiff(self);
		Board_Clear(self);
	}
	return self;
}

void Board_Delete(Board *self)
{
	free(self);
}

void Board_Clear(Board *self)
{
	int i, j;

	for (i = 0; i < NUM_DISK; i++) {
		self->Disk[i] = WALL;
	}
	for (i = 0; i < BOARD_SIZE; i++) {
		for (j = 0; j < BOARD_SIZE; j++) {
			self->Disk[Board_Pos(i, j)] = EMPTY;
		}
	}
	self->Disk[E4] = BLACK;
	self->Disk[D5] = BLACK;
	self->Disk[D4] = WHITE;
	self->Disk[E5] = WHITE;

	self->Sp = self->Stack;
	self->DiskNum[BLACK] = 2;
	self->DiskNum[WHITE] = 2;
	self->DiskNum[EMPTY] = BOARD_SIZE * BOARD_SIZE - 4;

	Board_InitializePattern(self);
}

int Board_Disk(const Board *self, int in_pos)
{
	return self->Disk[in_pos];
}

int Board_CountDisks(const Board *self, int in_color)
{
	return self->DiskNum[in_color];
}

static int Board_FlipLine(Board *self, int in_color, int in_pos, int in_dir)
{
	int result = 0;
	int op = OPPONENT_COLOR(in_color);
	int pos;

	pos = in_pos + in_dir;
	if (self->Disk[pos] != op) {
		return 0;
	}
	pos += in_dir;
	if (self->Disk[pos] == op) {
		pos += in_dir;
		if (self->Disk[pos] == op) {
			pos += in_dir;
			if (self->Disk[pos] == op) {
				pos += in_dir;
				if (self->Disk[pos] == op) {
					pos += in_dir;
					if (self->Disk[pos] == op) {
						pos += in_dir;
						if (self->Disk[pos] != in_color) {
							return 0;
						}
						pos -= in_dir;
						result ++;
						self->Disk[pos] = in_color;
						BOARD_STACK_PUSH(self, pos);
					} else if (self->Disk[pos] != in_color) {
						return 0;
					}
					pos -= in_dir;
					result ++;
					self->Disk[pos] = in_color;
					BOARD_STACK_PUSH(self, pos);
				} else if (self->Disk[pos] != in_color) {
					return 0;
				}
				pos -= in_dir;
				result ++;
				self->Disk[pos] = in_color;
				BOARD_STACK_PUSH(self, pos);
			} else if (self->Disk[pos] != in_color) {
				return 0;
			}
			pos -= in_dir;
			result ++;
			self->Disk[pos] = in_color;
			BOARD_STACK_PUSH(self, pos);
		} else if (self->Disk[pos] != in_color) {
			return 0;
		}
		pos -= in_dir;
		result ++;
		self->Disk[pos] = in_color;
		BOARD_STACK_PUSH(self, pos);
	} else if (self->Disk[pos] != in_color) {
		return 0;
	}
	pos -= in_dir;
	result ++;
	self->Disk[pos] = in_color;
	BOARD_STACK_PUSH(self, pos);

	return result;
}

int Board_Flip(Board *self, int in_color, int in_pos)
{
	int result = 0;

	if (self->Disk[in_pos] != EMPTY) {
		return 0;
	}
	switch (in_pos) {
	case C1:
	case C2:
	case D1:
	case D2:
	case E1:
	case E2:
	case F1:
	case F2:
		result += Board_FlipLine(self, in_color, in_pos, DIR_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_RIGHT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	case C8:
	case C7:
	case D8:
	case D7:
	case E8:
	case E7:
	case F8:
	case F7:
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_RIGHT);
		break;
	case A3:
	case A4:
	case A5:
	case A6:
	case B3:
	case B4:
	case B5:
	case B6:
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_RIGHT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	case H3:
	case H4:
	case H5:
	case H6:
	case G3:
	case G4:
	case G5:
	case G6:
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLine(self, in_color, in_pos, DIR_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN);
		break;
	case A1:
	case A2:
	case B1:
	case B2:
		result += Board_FlipLine(self, in_color, in_pos, DIR_RIGHT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	case A8:
	case A7:
	case B8:
	case B7:
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_RIGHT);
		break;
	case H1:
	case H2:
	case G1:
	case G2:
		result += Board_FlipLine(self, in_color, in_pos, DIR_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN);
		break;
	case H8:
	case H7:
	case G8:
	case G7:
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLine(self, in_color, in_pos, DIR_LEFT);
		break;
	default:
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLine(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_RIGHT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN);
		result += Board_FlipLine(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	}
	if (result > 0) {
		self->Disk[in_pos] = in_color;
		BOARD_STACK_PUSH(self, in_pos);
		BOARD_STACK_PUSH(self, OPPONENT_COLOR(in_color));
		BOARD_STACK_PUSH(self, result);
		self->DiskNum[in_color] += result + 1;
		self->DiskNum[OPPONENT_COLOR(in_color)] -= result;
		self->DiskNum[EMPTY]--;
	}

	return result;
}

int Board_Unflip(Board *self)
{
	int result;
	int i, color;

	if (self->Sp <= self->Stack) {
		return 0;
	}
	result = BOARD_STACK_POP(self);
	color = BOARD_STACK_POP(self);
	self->Disk[BOARD_STACK_POP(self)] = EMPTY;
	for (i = 0; i < result; i++) {
		self->Disk[BOARD_STACK_POP(self)] = color;
	}
	self->DiskNum[color] += result;
	self->DiskNum[OPPONENT_COLOR(color)] -= result + 1;
	self->DiskNum[EMPTY]++;

	return result;
}

void Board_InitializePattern(Board *self)
{
	int i;
	for (i = 0; i < NUM_PATTERN_ID; i++) {
		self->Pattern[i] = 0;
	}
	for (i = 0; i < NUM_DISK; i++) {
		if (self->Disk[i] == BLACK) {
			Board_PutSquareBlack(self, i);
		} else if (self->Disk[i] == WHITE) {
			Board_PutSquareWhite(self, i);
		}
	}
}

int Board_Pattern(const Board *self, int in_id)
{
	return self->Pattern[in_id];
}

static void Board_AddPattern(Board *self, int in_id, const int *in_pos_list, int in_num)
{
	int i, j, n;

	n = 1;
	for (i = 0; i < in_num; i++) {
		for (j = 0; self->PatternDiff[in_pos_list[i]][j] != 0; j++) {
		}
		self->PatternID[in_pos_list[i]][j] = in_id;
		self->PatternDiff[in_pos_list[i]][j] = n;
		n *= 3;
	}
}

static void Board_InitializePatternDiff(Board *self)
{
	int i, j;
	int pattern_list[][9] = {
		{ A4, B4, C4, D4, E4, F4, G4, H4, -1 },
		{ A5, B5, C5, D5, E5, F5, G5, H5, -1 },
		{ D1, D2, D3, D4, D5, D6, D7, D8, -1 },
		{ E1, E2, E3, E4, E5, E6, E7, E8, -1 },
		{ A3, B3, C3, D3, E3, F3, G3, H3, -1 },
		{ A6, B6, C6, D6, E6, F6, G6, H6, -1 },
		{ C1, C2, C3, C4, C5, C6, C7, C8, -1 },
		{ F1, F2, F3, F4, F5, F6, F7, F8, -1 },
		{ A2, B2, C2, D2, E2, F2, G2, H2, -1 },
		{ A7, B7, C7, D7, E7, F7, G7, H7, -1 },
		{ B1, B2, B3, B4, B5, B6, B7, B8, -1 },
		{ G1, G2, G3, G4, G5, G6, G7, G8, -1 },
		{ A1, B2, C3, D4, E5, F6, G7, H8, -1 },
		{ A8, B7, C6, D5, E4, F3, G2, H1, -1 },
		{ A2, B3, C4, D5, E6, F7, G8, -1 },
		{ B1, C2, D3, E4, F5, G6, H7, -1 },
		{ A7, B6, C5, D4, E3, F2, G1, -1 },
		{ B8, C7, D6, E5, F4, G3, H2, -1 },
		{ A3, B4, C5, D6, E7, F8, -1 },
		{ C1, D2, E3, F4, G5, H6, -1 },
		{ A6, B5, C4, D3, E2, F1, -1 },
		{ C8, D7, E6, F5, G4, H3, -1 },
		{ A4, B5, C6, D7, E8, -1 },
		{ D1, E2, F3, G4, H5, -1 },
		{ A5, B4, C3, D2, E1, -1 },
		{ D8, E7, F6, G5, H4, -1 },
		{ A5, B6, C7, D8, -1 },
		{ E1, F2, G3, H4, -1 },
		{ A4, B3, C2, D1, -1 },
		{ E8, F7, G6, H5, -1 },
		{ B2, G1, F1, E1, D1, C1, B1, A1, -1 },
		{ G2, B1, C1, D1, E1, F1, G1, H1, -1 },
		{ B7, G8, F8, E8, D8, C8, B8, A8, -1 },
		{ G7, B8, C8, D8, E8, F8, G8, H8, -1 },
		{ B2, A7, A6, A5, A4, A3, A2, A1, -1 },
		{ B7, A2, A3, A4, A5, A6, A7, A8, -1 },
		{ G2, H7, H6, H5, H4, H3, H2, H1, -1 },
		{ G7, H2, H3, H4, H5, H6, H7, H8, -1 },
		{ B3, A3, C2, B2, A2, C1, B1, A1, -1 },
		{ G3, H3, F2, G2, H2, F1, G1, H1, -1 },
		{ B6, A6, C7, B7, A7, C8, B8, A8, -1 },
		{ G6, H6, F7, G7, H7, F8, G8, H8, -1 },
		{ -1 }
	};

	for (i = 0; i < NUM_DISK; i++) {
		for (j = 0; j < NUM_PATTERN_DIFF; j++) {
			self->PatternID[i][j] = 0;
			self->PatternDiff[i][j] = 0;
		}
	}
	for (i = 0; pattern_list[i][0] >= 0; i++) {
		for (j = 0; pattern_list[i][j] >= 0; j++) {}
		Board_AddPattern(self, i, pattern_list[i], j);
	}
	for (i = 0; i < NUM_HASH_PATTERN; i++) {
		for (j = 0; j < NUM_HASH_DIFF; j++) {
			self->HashDiff[i][j].Low = (unsigned long)get_rand(256) | ((unsigned long)get_rand(256) << 8) |
				((unsigned long)get_rand(256) << 16) | ((unsigned long)get_rand(256) << 24);
			self->HashDiff[i][j].High = (unsigned long)get_rand(256) | ((unsigned long)get_rand(256) << 8) |
				((unsigned long)get_rand(256) << 16) | ((unsigned long)get_rand(256) << 24);
		}
	}
}

static void Board_FlipSquareBlack(Board *self, int in_pos)
{
	self->Disk[in_pos] = BLACK;
	self->Pattern[self->PatternID[in_pos][0]] -= self->PatternDiff[in_pos][0];
	self->Pattern[self->PatternID[in_pos][1]] -= self->PatternDiff[in_pos][1];
	self->Pattern[self->PatternID[in_pos][2]] -= self->PatternDiff[in_pos][2];
	self->Pattern[self->PatternID[in_pos][3]] -= self->PatternDiff[in_pos][3];
	self->Pattern[self->PatternID[in_pos][4]] -= self->PatternDiff[in_pos][4];
	self->Pattern[self->PatternID[in_pos][5]] -= self->PatternDiff[in_pos][5];
}

static void Board_FlipSquareWhite(Board *self, int in_pos)
{
	self->Disk[in_pos] = WHITE;
	self->Pattern[self->PatternID[in_pos][0]] += self->PatternDiff[in_pos][0];
	self->Pattern[self->PatternID[in_pos][1]] += self->PatternDiff[in_pos][1];
	self->Pattern[self->PatternID[in_pos][2]] += self->PatternDiff[in_pos][2];
	self->Pattern[self->PatternID[in_pos][3]] += self->PatternDiff[in_pos][3];
	self->Pattern[self->PatternID[in_pos][4]] += self->PatternDiff[in_pos][4];
	self->Pattern[self->PatternID[in_pos][5]] += self->PatternDiff[in_pos][5];
}

static void Board_PutSquareBlack(Board *self, int in_pos)
{
	self->Disk[in_pos] = BLACK;
	self->Pattern[self->PatternID[in_pos][0]] += self->PatternDiff[in_pos][0];
	self->Pattern[self->PatternID[in_pos][1]] += self->PatternDiff[in_pos][1];
	self->Pattern[self->PatternID[in_pos][2]] += self->PatternDiff[in_pos][2];
	self->Pattern[self->PatternID[in_pos][3]] += self->PatternDiff[in_pos][3];
	self->Pattern[self->PatternID[in_pos][4]] += self->PatternDiff[in_pos][4];
	self->Pattern[self->PatternID[in_pos][5]] += self->PatternDiff[in_pos][5];
}

static void Board_PutSquareWhite(Board *self, int in_pos)
{
	self->Disk[in_pos] = WHITE;
	self->Pattern[self->PatternID[in_pos][0]] += self->PatternDiff[in_pos][0] + self->PatternDiff[in_pos][0];
	self->Pattern[self->PatternID[in_pos][1]] += self->PatternDiff[in_pos][1] + self->PatternDiff[in_pos][1];
	self->Pattern[self->PatternID[in_pos][2]] += self->PatternDiff[in_pos][2] + self->PatternDiff[in_pos][2];
	self->Pattern[self->PatternID[in_pos][3]] += self->PatternDiff[in_pos][3] + self->PatternDiff[in_pos][3];
	self->Pattern[self->PatternID[in_pos][4]] += self->PatternDiff[in_pos][4] + self->PatternDiff[in_pos][4];
	self->Pattern[self->PatternID[in_pos][5]] += self->PatternDiff[in_pos][5] + self->PatternDiff[in_pos][5];
}

static void Board_RemoveSquareBlack(Board *self, int in_pos)
{
	self->Disk[in_pos] = EMPTY;
	self->Pattern[self->PatternID[in_pos][0]] -= self->PatternDiff[in_pos][0];
	self->Pattern[self->PatternID[in_pos][1]] -= self->PatternDiff[in_pos][1];
	self->Pattern[self->PatternID[in_pos][2]] -= self->PatternDiff[in_pos][2];
	self->Pattern[self->PatternID[in_pos][3]] -= self->PatternDiff[in_pos][3];
	self->Pattern[self->PatternID[in_pos][4]] -= self->PatternDiff[in_pos][4];
	self->Pattern[self->PatternID[in_pos][5]] -= self->PatternDiff[in_pos][5];
}

static void Board_RemoveSquareWhite(Board *self, int in_pos)
{
	self->Disk[in_pos] = EMPTY;
	self->Pattern[self->PatternID[in_pos][0]] -= self->PatternDiff[in_pos][0] + self->PatternDiff[in_pos][0];
	self->Pattern[self->PatternID[in_pos][1]] -= self->PatternDiff[in_pos][1] + self->PatternDiff[in_pos][1];
	self->Pattern[self->PatternID[in_pos][2]] -= self->PatternDiff[in_pos][2] + self->PatternDiff[in_pos][2];
	self->Pattern[self->PatternID[in_pos][3]] -= self->PatternDiff[in_pos][3] + self->PatternDiff[in_pos][3];
	self->Pattern[self->PatternID[in_pos][4]] -= self->PatternDiff[in_pos][4] + self->PatternDiff[in_pos][4];
	self->Pattern[self->PatternID[in_pos][5]] -= self->PatternDiff[in_pos][5] + self->PatternDiff[in_pos][5];
}

static int Board_FlipLinePattern(Board *self, int in_color, int in_pos, int in_dir)
{
	int result = 0;
	int op = OPPONENT_COLOR(in_color);
	int pos;
	void (*func_flip)(Board *, int);

	if (in_color == BLACK) {
		func_flip = Board_FlipSquareBlack;
	} else {
		func_flip = Board_FlipSquareWhite;
	}

	pos = in_pos + in_dir;
	if (self->Disk[pos] != op) {
		return 0;
	}
	pos += in_dir;
	if (self->Disk[pos] == op) {
		pos += in_dir;
		if (self->Disk[pos] == op) {
			pos += in_dir;
			if (self->Disk[pos] == op) {
				pos += in_dir;
				if (self->Disk[pos] == op) {
					pos += in_dir;
					if (self->Disk[pos] == op) {
						pos += in_dir;
						if (self->Disk[pos] != in_color) {
							return 0;
						}
						pos -= in_dir;
						result ++;
						func_flip(self, pos);
						BOARD_STACK_PUSH(self, pos);
					} else if (self->Disk[pos] != in_color) {
						return 0;
					}
					pos -= in_dir;
					result ++;
					func_flip(self, pos);
					BOARD_STACK_PUSH(self, pos);
				} else if (self->Disk[pos] != in_color) {
					return 0;
				}
				pos -= in_dir;
				result ++;
				func_flip(self, pos);
				BOARD_STACK_PUSH(self, pos);
			} else if (self->Disk[pos] != in_color) {
				return 0;
			}
			pos -= in_dir;
			result ++;
			func_flip(self, pos);
			BOARD_STACK_PUSH(self, pos);
		} else if (self->Disk[pos] != in_color) {
			return 0;
		}
		pos -= in_dir;
		result ++;
		func_flip(self, pos);
		BOARD_STACK_PUSH(self, pos);
	} else if (self->Disk[pos] != in_color) {
		return 0;
	}
	pos -= in_dir;
	result ++;
	func_flip(self, pos);
	BOARD_STACK_PUSH(self, pos);

	return result;
}

int Board_FlipPattern(Board *self, int in_color, int in_pos)
{
	int result = 0;

	if (self->Disk[in_pos] != EMPTY) {
		return 0;
	}
	switch (in_pos) {
	case C1:
	case C2:
	case D1:
	case D2:
	case E1:
	case E2:
	case F1:
	case F2:
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_RIGHT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	case C8:
	case C7:
	case D8:
	case D7:
	case E8:
	case E7:
	case F8:
	case F7:
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_RIGHT);
		break;
	case A3:
	case A4:
	case A5:
	case A6:
	case B3:
	case B4:
	case B5:
	case B6:
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_RIGHT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	case H3:
	case H4:
	case H5:
	case H6:
	case G3:
	case G4:
	case G5:
	case G6:
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN);
		break;
	case A1:
	case A2:
	case B1:
	case B2:
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_RIGHT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	case A8:
	case A7:
	case B8:
	case B7:
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_RIGHT);
		break;
	case H1:
	case H2:
	case G1:
	case G2:
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN);
		break;
	case H8:
	case H7:
	case G8:
	case G7:
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_LEFT);
		break;
	default:
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_RIGHT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN);
		result += Board_FlipLinePattern(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	}
	if (result > 0) {
		if (in_color == BLACK) {
			Board_PutSquareBlack(self, in_pos);
		} else {
			Board_PutSquareWhite(self, in_pos);
		}
		BOARD_STACK_PUSH(self, in_pos);
		BOARD_STACK_PUSH(self, OPPONENT_COLOR(in_color));
		BOARD_STACK_PUSH(self, result);
		self->DiskNum[in_color] += result + 1;
		self->DiskNum[OPPONENT_COLOR(in_color)] -= result;
		self->DiskNum[EMPTY]--;
	}

	return result;
}

int Board_UnflipPattern(Board *self)
{
	int result;
	int i, color;

	if (self->Sp <= self->Stack) {
		return 0;
	}
	result = BOARD_STACK_POP(self);
	color = BOARD_STACK_POP(self);
	if (color == BLACK) {
		Board_RemoveSquareWhite(self, BOARD_STACK_POP(self));
		for (i = 0; i < result; i++) {
			Board_FlipSquareBlack(self, BOARD_STACK_POP(self));
		}
	} else {
		Board_RemoveSquareBlack(self, BOARD_STACK_POP(self));
		for (i = 0; i < result; i++) {
			Board_FlipSquareWhite(self, BOARD_STACK_POP(self));
		}
	}
	self->DiskNum[color] += result;
	self->DiskNum[OPPONENT_COLOR(color)] -= result + 1;
	self->DiskNum[EMPTY]++;

	return result;
}

void Board_HashKey(Board *self, HashKey *out_key)
{
#if 1
	int i;
	HashKey *key;
	int pattern_id[NUM_HASH_PATTERN] = {
		PATTERN_ID_LINE4_1, PATTERN_ID_LINE4_2,
		PATTERN_ID_LINE3_1, PATTERN_ID_LINE3_2, PATTERN_ID_LINE2_1, PATTERN_ID_LINE2_2,
		PATTERN_ID_EDGE8_1, PATTERN_ID_EDGE8_2, PATTERN_ID_EDGE8_3, PATTERN_ID_EDGE8_4 };
	out_key->Low = 0;
	out_key->High = 0;
	for (i = 0; i < NUM_HASH_PATTERN; i++) {
		key = &self->HashDiff[i][self->Pattern[pattern_id[i]]];
		out_key->Low ^= key->Low;
		out_key->High ^= key->High;
	}
#else
	memcpy(out_key, &self->Key, sizeof(HashKey));
#endif
}

static int Board_CountFlipsLine(const Board *self, int in_color, int in_pos, int in_dir)
{
	int result = 0;
	int op = OPPONENT_COLOR(in_color);
	int pos;

	for (pos = in_pos + in_dir; self->Disk[pos] == op; pos += in_dir) {
		result++;
	}
	if (self->Disk[pos] != in_color) {
		return 0;
	}

	return result;
}

int Board_CountFlips(const Board *self, int in_color, int in_pos)
{
	int result = 0;

	if (self->Disk[in_pos] != EMPTY) {
		return 0;
	}
	switch (in_pos) {
	case C1:
	case C2:
	case D1:
	case D2:
	case E1:
	case E2:
	case F1:
	case F2:
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_RIGHT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	case C8:
	case C7:
	case D8:
	case D7:
	case E8:
	case E7:
	case F8:
	case F7:
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_RIGHT);
		break;
	case A3:
	case A4:
	case A5:
	case A6:
	case B3:
	case B4:
	case B5:
	case B6:
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_RIGHT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	case H3:
	case H4:
	case H5:
	case H6:
	case G3:
	case G4:
	case G5:
	case G6:
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN);
		break;
	case A1:
	case A2:
	case B1:
	case B2:
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_RIGHT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	case A8:
	case A7:
	case B8:
	case B7:
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_RIGHT);
		break;
	case H1:
	case H2:
	case G1:
	case G2:
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN);
		break;
	case H8:
	case H7:
	case G8:
	case G7:
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_LEFT);
		break;
	default:
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_UP_RIGHT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_RIGHT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN_LEFT);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN);
		result += Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN_RIGHT);
		break;
	}

	return result;
}

int Board_CanFlip(const Board *self, int in_color, int in_pos)
{
	if (self->Disk[in_pos] != EMPTY) {
		return 0;
	}
	if (Board_CountFlipsLine(self, in_color, in_pos, DIR_UP_LEFT)) {
		return 1;
	}
	if (Board_CountFlipsLine(self, in_color, in_pos, DIR_UP)) {
		return 1;
	}
	if (Board_CountFlipsLine(self, in_color, in_pos, DIR_UP_RIGHT)) {
		return 1;
	}
	if (Board_CountFlipsLine(self, in_color, in_pos, DIR_LEFT)) {
		return 1;
	}
	if (Board_CountFlipsLine(self, in_color, in_pos, DIR_RIGHT)) {
		return 1;
	}
	if (Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN_LEFT)) {
		return 1;
	}
	if (Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN)) {
		return 1;
	}
	if (Board_CountFlipsLine(self, in_color, in_pos, DIR_DOWN_RIGHT)) {
		return 1;
	}

	return 0;
}

void Board_Copy(const Board *self, Board *out_board)
{
	*out_board = *self;
	out_board->Sp = self->Sp - self->Stack + out_board->Stack;
}

void Board_Reverse(Board *self)
{
	int pos;
	int *p;
	int n;

	for (pos = 0; pos < NUM_DISK; pos++) {
		if (self->Disk[pos] == BLACK) {
			self->Disk[pos] = WHITE;
			self->DiskNum[BLACK]--;
			self->DiskNum[WHITE]++;
		} else if (self->Disk[pos] == WHITE) {
			self->Disk[pos] = BLACK;
			self->DiskNum[WHITE]--;
			self->DiskNum[BLACK]++;
		}
	}
	for (p = self->Sp; p > self->Stack;) {
		p--;
		n = *p;
		p--;
		*p = OPPONENT_COLOR(*p);
		p -= n + 1;
	}
	Board_InitializePattern(self);
}

int Board_CanPlay(const Board *self, int in_color)
{
	int x, y;

	for (x = 0; x < BOARD_SIZE; x++) {
		for (y = 0; y < BOARD_SIZE; y++) {
			if (Board_CanFlip(self, in_color, Board_Pos(x, y))) {
				return 1;
			}
		}
	}
	return 0;
}

int Board_Pos(int in_x, int in_y)
{
	return (in_y + 1) * (BOARD_SIZE + 1) + in_x + 1;
}

int Board_X(int in_pos)
{
	return in_pos % (BOARD_SIZE + 1) - 1;
}

int Board_Y(int in_pos)
{
	return in_pos / (BOARD_SIZE + 1) - 1;
}

int Board_OpponentColor(int in_color)
{
	return OPPONENT_COLOR(in_color);
}
