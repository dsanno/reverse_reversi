#include "com.h"
#include "board.h"
#include "evaluator.h"
#include "opening.h"
#include "hash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_VALUE	(DISK_VALUE * 200)
#define HASH_SIZE	20

typedef struct _MoveList MoveList;
struct _MoveList
{
	MoveList *Prev;
	MoveList *Next;
	int Pos;
};

typedef struct _MoveInfo MoveInfo;
struct _MoveInfo
{
	MoveList *Move;
	int Value;
};

struct _Com
{
	Board *Board;
	Evaluator *Evaluator;
	Opening *Opening;
	Hash *Hash;
	int UseOpening;
	int MidDepth;
	int WLDDepth;
	int ExactDepth;
	int Node;
	MoveList Moves[BOARD_SIZE * BOARD_SIZE];
	int MPCInfoNum;
	MPCInfo *MPCInfo;
};

static int Com_Initialize(Com *self, Evaluator *evaluator, Opening *opening);
static int Com_OpeningSearch(Com *self, int in_color, int in_opponent, int *out_move);
static int Com_MidSearch(Com *self, int in_depth, int in_alpha, int in_beta, int in_color, int in_opponent, int in_pass, int *out_move);
static int Com_EndSearch(Com *self, int in_depth, int in_alpha, int in_beta, int in_color, int in_opponent, int in_pass, int *out_move);
static void Com_MakeList(Com *self);
static int Com_Sort(Com *self, int in_color, MoveInfo *out_info);
static int Com_ReadMPCInfo(Com *self, FILE *fp);
static void RemoveList(MoveList *self);
static void RecoverList(MoveList *self);
#define get_rand(in_max) ((int)((double)(in_max) * rand() / (RAND_MAX + 1.0)))

static int Com_Initialize(Com *self, Evaluator *evaluator, Opening *opening)
{
	memset(self, 0, sizeof(Com));
	self->Board = Board_New();
	if (!self->Board) {
		return 0;
	}
	self->Evaluator = evaluator;
	if (!self->Evaluator) {
		return 0;
	}
	self->Opening = opening;
	if (!self->Opening) {
		return 0;
	}
	self->Hash = Hash_New(HASH_SIZE);
	if (!self->Hash) {
		return 0;
	}
	Hash_Clear(self->Hash);
	self->UseOpening = 0;
	self->MidDepth = 1;
	self->WLDDepth = 1;
	self->ExactDepth = 1;
	self->Node = 0;
	self->MPCInfoNum = 0;
	self->MPCInfo = NULL;
	return 1;
}

Com *Com_New(Evaluator *evaluator, Opening *opening)
{
	Com *self;
	self = malloc(sizeof(Com));
	if (self) {
		if (!Com_Initialize(self, evaluator, opening)) {
			Com_Delete(self);
			self = NULL;
		}
	}
	return self;
}

void Com_Delete(Com *self)
{
	if (self->MPCInfo) {
		free(self->MPCInfo);
	}
	if (self->Board) {
		Board_Delete(self->Board);
	}
	free(self);
}

void Com_SetOpening(Com *self, int in_use)
{
	self->UseOpening = in_use;
}

void Com_SetLevel(Com *self, int in_mid, int in_exact, int in_wld)
{
	self->MidDepth = in_mid;
	self->WLDDepth = in_wld;
	self->ExactDepth = in_exact;
}

int Com_NextMove(Com *self, const Board *in_board, int in_color, int *out_value)
{
	int result;
	int left;
	int value;
	int color;
	int window = 1;//(int)(DISK_VALUE * 2.0);
	int lower;
	int upper;
	int current;

	Board_Copy(in_board, self->Board);
	self->Node = 0;
	left = Board_CountDisks(self->Board, EMPTY);
	Com_MakeList(self);
	Board_InitializePattern(self->Board);
	Hash_ClearInfo(self->Hash);
	value = Com_OpeningSearch(self, in_color, Board_OpponentColor(in_color), &result);
	if (result != NOMOVE) {
	} else if (left <= self->ExactDepth) {
		value = Com_EndSearch(self, left, -BOARD_SIZE * BOARD_SIZE, BOARD_SIZE * BOARD_SIZE, in_color, Board_OpponentColor(in_color), 0, &result);
		value *= DISK_VALUE;
	} else if (left <= self->WLDDepth) {
		value = Com_EndSearch(self, left, -BOARD_SIZE * BOARD_SIZE, 1, in_color, Board_OpponentColor(in_color), 0, &result);
		value *= DISK_VALUE;
	} else {
		if ((in_color == WHITE && self->MidDepth % 2 == 0) ||
			(in_color == BLACK && self->MidDepth % 2 == 1)) {
			Board_Reverse(self->Board);
			color = Board_OpponentColor(in_color);
		} else {
			color = in_color;
		}
		if (self->MidDepth < 4) {
			value = Com_MidSearch(self, self->MidDepth, -MAX_VALUE, MAX_VALUE, color, Board_OpponentColor(color), 0, &result);
		} else {
			value = Com_MidSearch(self, self->MidDepth - 2, -MAX_VALUE, MAX_VALUE, color, Board_OpponentColor(color), 0, &result);
			lower = -MAX_VALUE;
			upper = MAX_VALUE;
			current = value - window / 2;
			while (upper > lower) {
				value = Com_MidSearch(self, self->MidDepth, current, current + window, color, Board_OpponentColor(color), 0, &result);
				if (value > current && value < current + window) {
					upper = lower = value;
				} else if (value <= current) {
					upper = value;
					current = value - window;
				} else {
					lower = current = value;
				}
			}
		}
	}
	if (out_value) {
		*out_value = value;
	}

	return result;
}

static int Com_OpeningSearch(Com *self, int in_color, int in_opponent, int *out_move)
{
	MoveList *p;
	PositionInfo info;
	int value, max = -MAX_VALUE;
	int count = 0;

	*out_move = NOMOVE;
	if (!self->UseOpening || !self->Opening) {
		return max;
	}
	for (p = self->Moves->Next; p; p = p->Next) {
		if (Board_Flip(self->Board, in_color, p->Pos)) {
			if (Opening_Info(self->Opening, self->Board, in_opponent, &info)) {
				value = -info.Value;
				if (value > max) {
					*out_move = p->Pos;
					max = value;
					count = 1;
				} else if (value == max) {
					count++;
					if (get_rand(count) < 1) {
						*out_move = p->Pos;
					}
				}
			}
			Board_Unflip(self->Board);
		}
	}
	return max;
}

static int Com_MidSearch(Com *self, int in_depth, int in_alpha, int in_beta, int in_color, int in_opponent, int in_pass, int *out_move)
{
	MoveList *p;
	int value, max = -MAX_VALUE;
	int can_move = 0;
	int move;
	MoveInfo info[BOARD_SIZE * BOARD_SIZE / 2];
	int i, info_num;
	MPCInfo *mpc_info;
	RevHashKey hash_key;
	HashInfo hash_info;

	if (in_depth == 0) {
		self->Node++;
		return Evaluator_Value(self->Evaluator, self->Board);
	}
	if (in_depth > 2) {
		Board_HashKey(self->Board, &hash_key);
		if (Hash_Get(self->Hash, &hash_key, &hash_info)) {
			if (hash_info.Depth >= in_depth) {
				if (hash_info.Upper <= in_alpha) {
					*out_move = hash_info.Move;
					return in_alpha;
				} else if (hash_info.Lower >= in_beta) {
					*out_move = hash_info.Move;
					return in_beta;
				} else if (hash_info.Lower >= hash_info.Upper) {
					*out_move = hash_info.Move;
					return hash_info.Lower;
				}
				if (hash_info.Upper < in_beta) {
					in_beta = hash_info.Upper;
				}
				if (hash_info.Lower > in_alpha) {
					in_alpha = hash_info.Lower;
				}
			} else {
				hash_info.Depth = in_depth;
				hash_info.Lower = -MAX_VALUE;
				hash_info.Upper = MAX_VALUE;
				hash_info.Move = NOMOVE;
			}
		} else {
			hash_info.Depth = in_depth;
			hash_info.Lower = -MAX_VALUE;
			hash_info.Upper = MAX_VALUE;
			hash_info.Move = NOMOVE;
		}
	}
	if (in_depth >= MPC_DEPTH_MIN && in_depth < MPC_DEPTH_MIN + self->MPCInfoNum) {
		mpc_info = &self->MPCInfo[in_depth - MPC_DEPTH_MIN];
		value = in_alpha - mpc_info->Deviation + mpc_info->Offset;
		if (Com_MidSearch(self, mpc_info->Depth, value - 1, value, in_color, in_opponent, in_pass, out_move) < value) {
			return in_alpha;
		}
		value = in_beta + mpc_info->Deviation + mpc_info->Offset;
		if (Com_MidSearch(self, mpc_info->Depth, value, value + 1, in_color, in_opponent, in_pass, out_move) > value) {
			return in_beta;
		}
	}
	*out_move = NOMOVE;
	if (in_depth > 2) {
		info_num = Com_Sort(self, in_color, info);
		if (info_num > 0) {
			*out_move = info[0].Move->Pos;
			can_move = 1;
		}
		for (i = 0; i < info_num; i++) {
			Board_FlipPattern(self->Board, in_color, info[i].Move->Pos);
			RemoveList(info[i].Move);
			if (i == 0) {
				value = -Com_MidSearch(self, in_depth - 1, -in_beta, -max, in_opponent, in_color, 0, &move);
			} else {
				value = -Com_MidSearch(self, in_depth - 1, -max - 1, -max, in_opponent, in_color, 0, &move);
				if (value > max && value < in_beta) {
					value = -Com_MidSearch(self, in_depth - 1, -in_beta, -value, in_opponent, in_color, 0, &move);
				}
			}
			Board_UnflipPattern(self->Board);
			RecoverList(info[i].Move);
			if (value > max) {
				max = value;
				*out_move = info[i].Move->Pos;
				if (max >= in_beta) {
					hash_info.Lower = max;
					hash_info.Move = *out_move;
					Hash_Set(self->Hash, &hash_key, &hash_info);
					return max;
				}
			}
		}
		if (*out_move != PASS) {
			hash_info.Upper = max;
			if (hash_info.Move == NOMOVE) {
				hash_info.Move = *out_move;
			}
			Hash_Set(self->Hash, &hash_key, &hash_info);
		}
	} else {
		for (p = self->Moves->Next; p; p = p->Next) {
			if (Board_FlipPattern(self->Board, in_color, p->Pos)) {
				RemoveList(p);
				if (!can_move) {
					*out_move = p->Pos;
					can_move = 1;
				}
				value = -Com_MidSearch(self, in_depth - 1, -in_beta, -max, in_opponent, in_color, 0, &move);
				Board_UnflipPattern(self->Board);
				RecoverList(p);
				if (value > max) {
					max = value;
					*out_move = p->Pos;
					if (max >= in_beta) {
						return max;
					}
				}
			}
		}
	}
	if (!can_move) {
		if (in_pass) {
			*out_move = NOMOVE;
			self->Node++;
			max = DISK_VALUE * (Board_CountDisks(self->Board, in_opponent) - Board_CountDisks(self->Board, in_color));
		} else {
			*out_move = PASS;
			max = -Com_MidSearch(self, in_depth - 1, -in_beta, -max, in_opponent, in_color, 1, &move);
		}
	}
	return max;
}

static void Com_PnSearch(Com *self, int in_depth, int in_color, int in_opponent, int in_pass, int *out_pn, int *out_dn)
{
	MoveList *p;
	int can_move = 0;
	int pn, dn;
	*out_pn = 0;
	*out_dn = 0;

	if (in_depth == 1) {
		for (p = self->Moves->Next; p; p = p->Next) {
			if (Board_CanFlip(self->Board, in_color, p->Pos)) {
				(*out_dn)++;
			}
		}
		*out_pn = 1;
		return;
	} else {
		for (p = self->Moves->Next; p; p = p->Next) {
			if (Board_Flip(self->Board, in_color, p->Pos)) {
				RemoveList(p);
				can_move = 1;
				Com_PnSearch(self, in_depth - 1, in_opponent, in_color, 0, &pn, &dn);
				Board_Unflip(self->Board);
				RecoverList(p);
				*out_dn += pn;
				if (dn > *out_pn) {
					*out_pn = dn;
				}
			}
		}
	}
	if (!can_move) {
		if (!in_pass) {
			*out_pn = 1;
			*out_dn = 1;
		} else {
			Com_PnSearch(self, in_depth-1, in_opponent, in_color, 1, &pn, &dn);
			*out_dn = pn;
			*out_pn = dn;
		}
	}
}

static int Com_PnSort(Com *self, int in_color, int in_depth, MoveInfo *out_info)
{
	int info_num = 0;
	MoveList *p;
	MoveInfo info_tmp, *best_info;
	int i, j;
	int pn, dn;

	for (p = self->Moves->Next; p; p = p->Next) {
		if (Board_Flip(self->Board, in_color, p->Pos)) {
			out_info[info_num].Move = p;
			Com_PnSearch(self, in_depth, Board_OpponentColor(in_color), in_color, 0, &pn, &dn);
			out_info[info_num].Value = - dn;
			info_num++;
			Board_Unflip(self->Board);
		}
	}
	for (i = 0; i < info_num; i++) {
		best_info = &out_info[i];
		for (j = i + 1; j < info_num; j++) {
			if (out_info[j].Value > best_info->Value) {
				best_info = &out_info[j];
			}
		}
		info_tmp = *best_info;
		*best_info = out_info[i];
		out_info[i] = info_tmp;
	}
	return info_num;
}

static int Com_EndSearch(Com *self, int in_depth, int in_alpha, int in_beta, int in_color, int in_opponent, int in_pass, int *out_move)
{
	MoveList *p;
	int value, max = in_alpha;
	int can_move = 0;
	int move;
	MoveInfo info[BOARD_SIZE * BOARD_SIZE / 2];
	int i, info_num;
	RevHashKey hash_key;
	HashInfo hash_info;

	if (in_depth == 1) {
		self->Node++;
		p = self->Moves->Next;
		value = Board_CountFlips(self->Board, in_color, p->Pos);
		max = Board_CountDisks(self->Board, in_opponent) - Board_CountDisks(self->Board, in_color);
		if (value > 0) {
			*out_move = p->Pos;
			return max - value - value - 1;
		}
		value = Board_CountFlips(self->Board, in_opponent, self->Moves->Next->Pos);
		if (value > 0) {
			*out_move = PASS;
			return max + value + value + 1;
		}
		*out_move = NOMOVE;
		return max;
	}
	*out_move = NOMOVE;
	if (in_depth > 8) {
		Board_HashKey(self->Board, &hash_key);
		if (Hash_Get(self->Hash, &hash_key, &hash_info)) {
			if (hash_info.Depth >= in_depth) {
				if (hash_info.Upper <= in_alpha) {
					*out_move = hash_info.Move;
					return in_alpha;
				} else if (hash_info.Lower >= in_beta) {
					*out_move = hash_info.Move;
					return in_beta;
				} else if (hash_info.Lower >= hash_info.Upper) {
					*out_move = hash_info.Move;
					return hash_info.Lower;
				}
				if (hash_info.Upper < in_beta) {
					in_beta = hash_info.Upper;
				}
				if (hash_info.Lower > in_alpha) {
					in_alpha = hash_info.Lower;
				}
			} else {
				hash_info.Depth = in_depth;
				hash_info.Lower = -MAX_VALUE;
				hash_info.Upper = MAX_VALUE;
				hash_info.Move = NOMOVE;
			}
		} else {
			hash_info.Depth = in_depth;
			hash_info.Lower = -MAX_VALUE;
			hash_info.Upper = MAX_VALUE;
			hash_info.Move = NOMOVE;
		}
		info_num = Com_Sort(self, in_color, info);
		if (info_num > 0) {
			*out_move = info[0].Move->Pos;
			can_move = 1;
		}
		for (i = 0; i < info_num; i++) {
			Board_FlipPattern(self->Board, in_color, info[i].Move->Pos);
			RemoveList(info[i].Move);
			if (i == 0) {
				value = -Com_EndSearch(self, in_depth - 1, -in_beta, -max, in_opponent, in_color, 0, &move);
			} else {
				value = -Com_EndSearch(self, in_depth - 1, -max - 1, -max, in_opponent, in_color, 0, &move);
				if (value > max && value < in_beta) {
					value = -Com_EndSearch(self, in_depth - 1, -in_beta, -value, in_opponent, in_color, 0, &move);
				}
			}
			Board_UnflipPattern(self->Board);
			RecoverList(info[i].Move);
			if (value > max) {
				max = value;
				*out_move = info[i].Move->Pos;
				if (max >= in_beta) {
					hash_info.Lower = max;
					hash_info.Move = *out_move;
					Hash_Set(self->Hash, &hash_key, &hash_info);
					return in_beta;
				}
			}
		}
		if (*out_move != PASS && *out_move != NOMOVE) {
			hash_info.Upper = max;
			if (hash_info.Move == NOMOVE) {
				hash_info.Move = *out_move;
			}
			Hash_Set(self->Hash, &hash_key, &hash_info);
		}
	} else {
		for (p = self->Moves->Next; p; p = p->Next) {
			if (Board_Flip(self->Board, in_color, p->Pos)) {
				RemoveList(p);
				if (!can_move) {
					*out_move = p->Pos;
					can_move = 1;
				}
				value = -Com_EndSearch(self, in_depth - 1, -in_beta, -max, in_opponent, in_color, 0, &move);
				Board_Unflip(self->Board);
				RecoverList(p);
				if (value > max) {
					max = value;
					*out_move = p->Pos;
					if (max >= in_beta) {
						return in_beta;
					}
				}
			}
		}
	}
	if (!can_move) {
		if (in_pass) {
			*out_move = NOMOVE;
			self->Node++;
			max = Board_CountDisks(self->Board, in_opponent) - Board_CountDisks(self->Board, in_color);
		} else {
			*out_move = PASS;
			max = -Com_EndSearch(self, in_depth, -in_beta, -max, in_opponent, in_color, 1, &move);
		}
	}
	return max;
}

int Com_CountNodes(const Com *self)
{
	return self->Node;
}

int Com_CountHashGet(const Com *self)
{
	return Hash_CountGet(self->Hash);
}

int Com_CountHashHit(const Com *self)
{
	return Hash_CountHit(self->Hash);
}

static void Com_MakeList(Com *self)
{
	int pos_list[] = {
		A1, A8, H8, H1,
		D3, D6, E3, E6, C4, C5, F4, F5,
		C3, C6, F3, F6,
		D2, D7, E2, E7, B4, B5, G4, G5,
		C2, C7, F2, F7, B3, B6, G3, G6,
		D1, D8, E1, E8, A4, A5, H4, H5,
		C1, C8, F1, F8, A3, A6, H3, H6,
		B2, B7, G2, G7,
		B1, B8, G1, G8, A2, A7, H2, H7,
		D4, D5, E4, E5,
		NOMOVE
	};
	int i;
	MoveList *prev;

	prev = self->Moves;
	prev->Pos = NOMOVE;
	prev->Prev = NULL;
	prev->Next = NULL;
	for (i = 0; pos_list[i] != NOMOVE; i++) {
		if (Board_Disk(self->Board, pos_list[i]) == EMPTY) {
			prev[1].Pos = pos_list[i];
			prev[1].Prev = prev;
			prev[1].Next = NULL;
			prev->Next = &prev[1];
			prev++;
		}
	}
}

static void RemoveList(MoveList *self)
{
	if (self->Prev) {
		self->Prev->Next = self->Next;
	}
	if (self->Next) {
		self->Next->Prev = self->Prev;
	}
}

static void RecoverList(MoveList *self)
{
	if (self->Prev) {
		self->Prev->Next = self;
	}
	if (self->Next) {
		self->Next->Prev = self;
	}
}

static int Com_Sort(Com *self, int in_color, MoveInfo *out_info)
{
	int info_num = 0;
	MoveList *p;
	MoveInfo info_tmp, *best_info;
	int i, j;
	RevHashKey hash_key;

	for (p = self->Moves->Next; p; p = p->Next) {
		if (Board_FlipPattern(self->Board, in_color, p->Pos)) {
			Board_HashKey(self->Board, &hash_key);
			out_info[info_num].Move = p;
			out_info[info_num].Value = Evaluator_Value(self->Evaluator, self->Board);
			info_num++;
			Board_UnflipPattern(self->Board);
		}
	}
	if (in_color == WHITE) {
		for (i = 0; i < info_num; i++) {
			out_info[i].Value = -out_info[i].Value;
		}
	}
	for (i = 0; i < info_num; i++) {
		best_info = &out_info[i];
		for (j = i + 1; j < info_num; j++) {
			if (out_info[j].Value > best_info->Value) {
				best_info = &out_info[j];
			}
		}
		info_tmp = *best_info;
		*best_info = out_info[i];
		out_info[i] = info_tmp;
	}
	return info_num;
}

int Com_LoadMPCInfo(Com *self, const char *in_file_name)
{
	FILE *fp;

	fopen_s(&fp, in_file_name, "rb");
	if (!fp) {
		return 0;
	}
	if (!Com_ReadMPCInfo(self, fp)) {
		fclose(fp);
		self->MPCInfoNum = 0;
		if (self->MPCInfo) {
			free(self->MPCInfo);
			self->MPCInfo = NULL;
		}
		return 0;
	}
	fclose(fp);
	return 1;
}

static int Com_ReadMPCInfo(Com *self, FILE *fp)
{
	MPCInfo *info;

	if (fread(&self->MPCInfoNum, sizeof(int), 1, fp) < 1) {
		return 0;
	}
	if (self->MPCInfoNum == 0) {
		if (self->MPCInfo) {
			free(self->MPCInfo);
		}
		self->MPCInfo = NULL;
		return 1;
	}
	info = realloc(self->MPCInfo, sizeof(MPCInfo) * self->MPCInfoNum);
	if (!info) {
		return 0;
	}
	self->MPCInfo = info;
	if (fread(self->MPCInfo, sizeof(MPCInfo), self->MPCInfoNum, fp) < (size_t)self->MPCInfoNum) {
		return 0;
	}
	return 1;
}
