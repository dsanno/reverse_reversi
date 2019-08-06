#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "board.h"
#include "com.h"
#include "evaluator.h"
#include "opening.h"

#define BUFFER_SIZE 64
#define MPC_NUM 14
#define MPC_FILE "data/mpc.dat"
#define MPC_LEARN_FILE "data/mpc_learn.dat"
#define EVALUATOR_FILE "data/eval.dat"
#define OPENING_TRANSCRIPT_FILE "data/open_trans.txt"
#define OPENING_FILE "data/open.dat"
#define TRANSCRIPT_SIZE 128
struct _MainParam
{
	Board *Board;
	Evaluator *Evaluator;
	Opening *Opening;
	Com *Com;
};
typedef struct _MainParam MainParam;

static int get_rand(int in_max);
static void move_random(Board *board, int in_color);
static char * get_stream(char *out_buffer, int in_size, FILE *stream);
static void print_board(const Board *in_board);
static void play(Board *board, Com *com);
static void learn(Board *board, Evaluator *evaluator, Com *com);
static int main_param_initialize_each(MainParam *self);
static int main_param_initialize(MainParam *self);
static void main_param_finalize(MainParam *self);
static void opening_initialize(Board *iboard, Opening *opening);
static void calc_mpc(Board *board, Com *com);
static int write_mpc(FILE *fp, int in_num, const MPCInfo *in_info);
static int save_mpc(int in_num, const MPCInfo *in_info);

static int get_rand(int in_max)
{
	return (int)((double)in_max * rand() / (RAND_MAX + 1.0));
}

static void move_random(Board *board, int in_color)
{
	while (!Board_Flip(board, in_color, Board_Pos(get_rand(BOARD_SIZE), get_rand(BOARD_SIZE)))) {}
}

static char * get_stream(char *out_buffer, int in_size, FILE *stream)
{
	char *result;
	int i;

	result = fgets(out_buffer, in_size, stream);
	if (result != NULL) {
		for (i = 0; i < in_size; i++) {
			if (out_buffer[i] == '\r' || out_buffer[i] == '\n') {
				out_buffer[i] = '\0';
			}
		}
	}
	return result;
}

static void print_board(const Board *in_board)
{
	int x, y;
	printf("  A B C D E F G H\n");
	for (y = 0; y < BOARD_SIZE; y++) {
		printf("%d ", y+1);
		for (x = 0; x < BOARD_SIZE; x++) {
			switch (Board_Disk(in_board, Board_Pos(x, y))) {
			case BLACK:
				printf("O ");
				break;
			case WHITE:
				printf("X ");
				break;
			case EMPTY:
				printf(". ");
				break;
			default:
				printf("# ");
				break;
			}
		}
		printf("\n");
	}
	printf("O %2d - X %2d\n", Board_CountDisks(in_board, BLACK), Board_CountDisks(in_board, WHITE));
	printf("\n");
}

static void play(Board *board, Com *com)
{
	char buffer[BUFFER_SIZE];
	int history[BOARD_SIZE * BOARD_SIZE * 2];
	int color = BLACK;
	int player_color;
	int move;
	int score;
	clock_t clock_start, clock_end;
	int n, x, y;

	for (n = 0; n < BOARD_SIZE * BOARD_SIZE * 2; n++) {
		history[n] = NOMOVE;
	}
	Board_Clear(board);

	n = 0;
	while (1) {
		printf("あなたの色を選択してください (1:黒 2:白)\n");
		get_stream(buffer, BUFFER_SIZE, stdin);
		if (!strcmp(buffer, "1")) {
			player_color = BLACK;
			break;
		} else if (!strcmp(buffer, "2")) {
			player_color = WHITE;
			break;
		}
	}
	while (1) {
		printf("コンピュータのレベルを選択してください (1-4)\n");
		get_stream(buffer, BUFFER_SIZE, stdin);
		if (!strcmp(buffer, "1")) {
			Com_SetLevel(com, 2, 8, 10);
			break;
		} else if (!strcmp(buffer, "2")) {
			Com_SetLevel(com, 4, 10, 12);
			break;
		} else if (!strcmp(buffer, "3")) {
			Com_SetLevel(com, 6, 12, 14);
			break;
		} else if (!strcmp(buffer, "4")) {
			Com_SetLevel(com, 8, 14, 16);
			break;
		}
	}
	Com_SetOpening(com, 1);
	Com_LoadMPCInfo(com, MPC_FILE);
	while (1) {
		print_board(board);
		if (!Board_CanPlay(board, BLACK) && !Board_CanPlay(board, WHITE)) {
			if (Board_CountDisks(board, player_color) == 0) {
				score = -BOARD_SIZE * BOARD_SIZE;
			} else if (Board_CountDisks(board, Board_OpponentColor(player_color)) == 0) {
				score = BOARD_SIZE * BOARD_SIZE;
			} else {
				score = Board_CountDisks(board, player_color) - Board_CountDisks(board, Board_OpponentColor(player_color));
			}
			if (score > 0) {
				printf("コンピュータの%d石負けです\n", score);
			} else if (score < 0) {
				printf("あなたの%d石負けです\n", -score);
			} else {
				printf("引き分けです\n");
			}
			break;
		}
		if (color == player_color) {
			printf("あなたの番です、次の手を入力してください\n");
			get_stream(buffer, BUFFER_SIZE, stdin);
			if (!strcmp(buffer, "q") || !strcmp(buffer, "quit")) {
				printf("ゲームを中断します\n");
				break;
			} else if (!strcmp(buffer, "u") || !strcmp(buffer, "undo")) {
				if (n > 1) {
					n--;
					if (history[n] != PASS) {
						Board_Unflip(board);
					}
					n--;
					if (history[n] != PASS) {
						Board_Unflip(board);
					}
				}
			} else if (!strcmp(buffer, "p") || !strcmp(buffer, "pass")) {
				if (!Board_CanPlay(board, color) && Board_CanPlay(board, Board_OpponentColor(color))) {
					color = Board_OpponentColor(color);
					history[n] = PASS;
					n++;
				} else {
					printf("パスはできません\n");
				}
			} else if (strlen(buffer) != 2) {
				printf("無効なコマンドです\n");
			} else {
				x = tolower(buffer[0]) - 'a';
				y = buffer[1] - '1';
				if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
					printf("無効なコマンドです\n");
				} else if (Board_Flip(board, color, Board_Pos(x, y))) {
					color = Board_OpponentColor(color);
					history[n] = Board_Pos(x, y);
					n++;
				} else {
					printf("そこには置けません\n");
				}
			}
		} else {
			printf("コンピュータ思考中...\n");
			if (Board_CanPlay(board, color)) {
				clock_start = clock();
				move = Com_NextMove(com, board, color, &score);
				clock_end = clock();
				printf("%c%cに置きます\n", "ABCDEFGH"[Board_X(move)], "12345678"[Board_Y(move)]);
				printf("評価値: %.2f\n", (double)score / DISK_VALUE);
				printf("思考時間: %.2f 秒 ノード数: %d NPS: %.2f knps \n", (double)(clock_end - clock_start) / CLOCKS_PER_SEC, 
					Com_CountNodes(com), (double)Com_CountNodes(com) / (clock_end - clock_start + 1) * CLOCKS_PER_SEC / 1000);
				if (Com_CountHashGet(com) > 0) {
					printf("ヒット率 %.2f %%\n", (double)Com_CountHashHit(com) / Com_CountHashGet(com) * 100);
				} else {
					printf("ヒット率 %.2f %%\n", 0.0);
				}
				Board_Flip(board, color, move);
			} else {
				printf("パスします\n");
				move = PASS;
			}
			color = Board_OpponentColor(color);
			history[n] = move;
			n++;
		}
		printf("\n");
	}
}

static void learn(Board *board, Evaluator *evaluator, Com *com)
{
	char buffer[BUFFER_SIZE];
	int history_color[BOARD_SIZE * BOARD_SIZE];
	int i, j, move, num, turn, value;
	int color;
	int result;
	const int middle_depth = 6;
	const int exact_depth = 14;

	printf("対戦回数を入力してください\n");
	get_stream(buffer, BUFFER_SIZE, stdin);
	num = atoi(buffer);

	Com_SetLevel(com, middle_depth, exact_depth, exact_depth);
	Com_SetOpening(com, 0);
	Com_LoadMPCInfo(com, MPC_LEARN_FILE);
	for (i = 0; i < num; i++) {
		Board_Clear(board);
		color = BLACK;
		turn = 0;
		for (j = 0; j < 8; j++) {
			if (Board_CanPlay(board, color)) {
				move_random(board, color);
				history_color[turn] = color;
				turn++;
			}
			color = Board_OpponentColor(color);
		}
		while (1) {
			if (Board_CanPlay(board, color)) {
				if (Board_CountDisks(board, EMPTY) > exact_depth && get_rand(100) < 1) {
					move_random(board, color);
				} else {
					move = Com_NextMove(com, board, color, &value);
					Board_Flip(board, color, move);
				}
				history_color[turn] = color;
				turn++;
			} else if (!Board_CanPlay(board, Board_OpponentColor(color))) {
				break;
			}
			color = Board_OpponentColor(color);
		}
		result = (Board_CountDisks(board, BLACK) - Board_CountDisks(board, WHITE)) * DISK_VALUE;
		for (j = Board_CountDisks(board, EMPTY); j < 8; j++) {
			turn--;
			Board_Unflip(board);
		}
		for (j = Board_CountDisks(board, EMPTY); j < BOARD_SIZE * BOARD_SIZE - 12; j++) {
			turn--;
			Board_Unflip(board);
			if (history_color[turn] == BLACK) {
				Evaluator_Add(evaluator, board, result);
			} else {
				Board_Reverse(board);
				Evaluator_Add(evaluator, board, -result);
				Board_Reverse(board);
			}
		}
		if ((i + 1) % 10 == 0) {
			Evaluator_Update(evaluator);
		}
		if ((i + 1) % 100 == 0) {
			printf("学習中... %d / %d\r", i + 1 , num );
			Evaluator_Save(evaluator, EVALUATOR_FILE);
		}
	}
	Evaluator_Save(evaluator, EVALUATOR_FILE);
	printf("終了しました                    \n");
}

static void opening_initialize(Board *board, Opening *opening)
{
	FILE *fp;
	char buffer[TRANSCRIPT_SIZE], value_buffer[BUFFER_SIZE];
	PositionInfo info;
	int color, turn, value, min;
	int history_move[BOARD_SIZE * BOARD_SIZE * 2];
	int i;

	fopen_s(&fp, OPENING_TRANSCRIPT_FILE, "r");
	if (!fp) {
		return;
	}
	while (1) {
		Board_Clear(board);
		color = BLACK;
		turn = 0;
		if (!get_stream(buffer, TRANSCRIPT_SIZE, fp)) {
			break;
		}
		if (!get_stream(value_buffer, BUFFER_SIZE, fp)) {
			break;
		}
		value = (int)(atof(value_buffer) * DISK_VALUE);
		for (i = 0; buffer[i] != '\0' && buffer[i + 1] != '\0'; i += 2) {
			if (!Board_CanPlay(board, color)) {
				history_move[turn] = PASS;
			} else {
				history_move[turn] = Board_Pos(tolower(buffer[i]) - 'a', buffer[i + 1] - '1');
				if (!Board_Flip(board, color, history_move[turn])) {
					break;
				}
			}
			turn++;
			color = Board_OpponentColor(color);
		}
		history_move[turn] = NOMOVE;
		for (; turn >= 0; turn--) {
			if (color == BLACK) {
				min = -value;
			} else {
				min = value;
			}
			if (history_move[turn] == PASS) {
				if (Opening_Info(opening, board, Board_OpponentColor(color), &info)) {
					min = PositionInfo_Value(&info);
				}
			} else {
				for (i = A1; i <= H8; i++) {
					if (Board_Flip(board, color, i)) {
						if (Opening_Info(opening, board, Board_OpponentColor(color), &info)) {
							if (PositionInfo_Value(&info) < min) {
								min = PositionInfo_Value(&info);
							}
						}
						Board_Unflip(board);
					}
				}
			}
			PositionInfo_SetValue(&info, -min);
			Opening_SetInfo(opening, board, color, &info);
			Board_Unflip(board);
			color = Board_OpponentColor(color);
		}
	}
	fclose(fp);
	Opening_Save(opening, OPENING_FILE);
	printf("登録完了しました\n");
}

static int write_mpc(FILE *fp, int in_num, const MPCInfo *in_info)
{
	if (fwrite(&in_num, sizeof(int), 1, fp) < 1) {
		return 0;
	}
	if (fwrite(in_info, sizeof(MPCInfo), in_num, fp) < (size_t)in_num) {
		return 0;
	}
	return 1;
}

static int save_mpc(int in_num, const MPCInfo *in_info)
{
	FILE *fp;

	fopen_s(&fp, MPC_FILE, "wb");
	if (!fp) {
		return 0;
	}
	if (!write_mpc(fp, in_num, in_info)) {
		return 0;
	}
	fclose(fp);
	return 1;
}

static void calc_mpc(Board *board, Com *com)
{
	MPCInfo info[MPC_NUM];
	int depth[] = {1, 2, 1, 2, 3, 4, 3, 4, 5, 6, 5, 6, 5, 6};
	int i, j, k;
	int num, value_low, value_high;
	int color;
	double mean, var;
	char transcript[][TRANSCRIPT_SIZE] = {
		"c4c3c2d6e6f6f5e3f4f3g5c6d3c5e2b4f7f2d2b3g4g6f1h5e7h3d7g3b5d8f8c8e8g8a4b6a7a5a6c7h4d1h6a3a2b7e1g1h2h7h8g7g2h1b8a8a1b1b2c1",
		"e6d6c6f4g3e7f6d7d8f5e8f3g4g5h4f8g8f7c7h3g6h5h6h7e3c5f2b6c3c4b4b5a4d3b3a5h2h1g2e1c2e2a6b8c8h8g7b7a8a7d1d2c1g1f1b2a3a1b1a2",
		"f5d6c3g5g6f3f4e3d3c4h5f6g3f2e6e7c6f7c5g4h4h3h2b5d2b4e2c1d1f1c2e1d7b3a5a6a4h7a7c8a3b2h6h1a1g2g1a2b1b6d8e8f8a8b7c7b8g7g8h8",
		"e6f4c3c4d3c2f3d6f5e3e2f1c5c6d7f2d2e1c1g3b1g4g5c7b4d1g1b3b6f6e7b5g6d8h5e8a6f7a5a3a4a7h4a2g7h3f8h6h7h8g8b7a8c8a1b2b8g2h2",
		"e6d6c5b4c6e7b5f6e8b6d7f4g6f5c4c7c8f7g5d8f8g4a3h5c3e3d3h6e2f2a4a5a6e1f1g1f3g3g7d1h3d2h4h8c2c1b3g8b1a1g2a2b2h2b7b8h1a7a8h7",
		"c4e3f5c6e2f3d3d2c3f4f2b4c5g4c1b6b3e6f6c2d6a3b5a4b1g3c7g5g6f7e7h5b2a1a5e8h3b8a6d7h4f1d1h2b7e1h7a7h6h8g7g2c8d8a8g8h1g1f8a2",
		"c4e3f2c3f3c5b4d3e2d2c1f4b3e1f1d1c2b5f5a5a4a3f6e6e7g3g4g2d6c6b6d7a6f8a2b2a1b1c8c7b8h4h5h6b7g6g5f7h1g1h3h2h7a8a7d8g8e8g7h8",
		"f5f4c3g6f3d3g4f2g3h4e6h5e3f6g5d6f1c5f7c4h6e7h3f8g7e2e1d2d1c2b3c1b4g1b6c6c7d7c8h8d8a5e8h7g8g2a4a6b7b5a7h2h1a3a2b2a1b1b8a8",
		"c4c5f6f3d6f5c6c3e6b5d3b4g4e3f4c7f2d7e2g5c2h3b6c1g6a5d8d2c8g3d1b7b1f7e8e7b3f8h6b8a8a7h5h4h2g7h7b2g2h1g1e1f1a1a4h8g8a3a6a2",
		"c4c3c2f4f6d6f3c5f5d3e3d2e6g4d7c6b4e7c7c8e2f2g3b5a6b3a3f1b6c1d8e8f8g8f7h4e1d1g2g6h5h6h3g5g1b8b1h1h2a1b2a2b7a8a7a5a4g7h8h7",
		"f5f4f3g4h3f6g5e6e3g3c4f2g6e2f1c6h4c5e7c3d6d7c8f7c7d8e8g8f8b8b3b4e1g7d3d2a4g1h1b5a5h2b6h7g2a6d1c2h8b2a2a3a1h5a8b1h6b7a7c1",
		"d3e3f2c5d6c2f3e2c3c6f5e6f6d2c4f4f1e1c1g1g5g6d1b1e7g3g4b5b3b4h3a3d7h6h5c7b6d8a5a4a6a7g2f8f7h1e8h2h4g7g8h8h7b7c8b8a8a2b2a1",
		"c4c3e6f6f5d6c5e3d3c6f2d2c2c1d1f3f4e1e7g4g3e2h3g6g5f1h6f8d8g7f7e8d7c8c7g8h8b6b5a6a5b7b3a4b8b4a8a7b2h4a3a1h5h7g1h1h2a2b1g2",
		"c4c3c2f4f5b2e3c5d3e2b3f2g3f6f3d6b6g4c6e6h4d2g5g6a1b4a4b5a5c7c1a3a2b1e1d1f1h5g2h3h2a7e7f7a6e8a8b7d8h1g1b8c8d7f8g7g8h8h7h6",
		"e6f4e3d2g3g5g4f6d6d7c5f5c4f3d3c3c1c6e7c2b5f2b6c7e2b4a3d1f1b1h6d8f8h4b3e8c8h2g6h7h8f7h3h5e1b2a1a2g2a5a6h1a4g1g8g7b7b8a7a8",
		"f5f4c3c6e3f6g3f3g4d3g5e6d6g6c5c4f7h4h3h5d7c7b4b5b3c2e7f2h6a5a3d8c8b8a4a2e2e1d1c1d2g2h1h2g1g8f8a6b7e8b6g7h8a8a7h7a1b2b1f1",
		""
	};

	save_mpc(0, info);
	Com_LoadMPCInfo(com, MPC_FILE);
	for (i = 0; i < MPC_NUM; i++) {
		num = 0;
		mean = var = 0.0;
		printf("MPC計算中 %d / %d\r", i, MPC_NUM);
		for (j = 0; transcript[j][0] != '\0';j++) {
			Board_Clear(board);
			color = BLACK;
			for (k = 0; transcript[j][k] != '\0' && transcript[j][k+1] != '\0'; k += 2) {
				if (!Board_Flip(board, color, Board_Pos(tolower(transcript[j][k]) - 'a', transcript[j][k+1] - '1'))) {
					break;
				}
				if (Board_CanPlay(board, color)) {
					color = Board_OpponentColor(color);
				}
				if (k < 16) {
					continue;
				} else if (Board_CountDisks(board, EMPTY) <= i + MPC_NUM + 6) {
					break;
				}
				Com_SetLevel(com, depth[i], 0, 0);
				Com_NextMove(com, board, color, &value_low);
				Com_SetLevel(com, i + MPC_DEPTH_MIN, 0, 0);
				Com_NextMove(com, board, color, &value_high);
				num++;
				mean += (double)(value_high - value_low);
				var += (double)(value_high - value_low) * (double)(value_high - value_low);
			}
		}
		mean /= num;
		var /= num;
		info[i].Depth = depth[i];
		info[i].Offset = (int)mean;
		info[i].Deviation = (int)(1.4 * sqrt(var - mean * mean));
		save_mpc(i + 1, info);
		Com_LoadMPCInfo(com, MPC_FILE);
	}
	printf("計算完了しました                    \n");
}

static int main_param_initialize_each(MainParam *self)
{
	self->Board = Board_New();
	if (!self->Board) {
		return 0;
	}
	self->Evaluator = Evaluator_New();
	if (!self->Evaluator) {
		return 0;
	}
	Evaluator_Load(self->Evaluator, EVALUATOR_FILE);
	self->Opening = Opening_New();
	if (!self->Opening) {
		return 0;
	}
	Opening_Load(self->Opening, OPENING_FILE);
	self->Com = Com_New(self->Evaluator, self->Opening);
	if (!self->Com) {
		return 0;
	}

	return 1;
}

static int main_param_initialize(MainParam *self)
{
	memset(self, 0, sizeof(MainParam));
	if (!main_param_initialize_each(self)) {
		main_param_finalize(self);
		return 0;
	}

	return 1;
}

static void main_param_finalize(MainParam *self)
{
	if (self->Com) {
		Com_Delete(self->Com);
	}
	if (self->Opening) {
		Opening_Delete(self->Opening);
	}
	if (self->Evaluator) {
		Evaluator_Delete(self->Evaluator);
	}
	if (self->Board) {
		Board_Delete(self->Board);
	}
}

int main(int argc, char **argv)
{
	MainParam param;
	char buffer[BUFFER_SIZE];

	srand((unsigned)time(NULL));
	if (!main_param_initialize(&param)) {
		printf("初期化に失敗しました\n");
		return 0;
	}

	while (1) {
		printf("モードを選択してください (1:対戦 2:学習 3:定石登録 4:MPC計算 q:終了)\n");
		get_stream(buffer, BUFFER_SIZE, stdin);
		if (!strcmp(buffer, "1")) {
			play(param.Board, param.Com);
		} else if (!strcmp(buffer, "2")) {
			learn(param.Board, param.Evaluator, param.Com);
		} else if (!strcmp(buffer, "3")) {
			opening_initialize(param.Board, param.Opening);
		} else if (!strcmp(buffer, "4")) {
			calc_mpc(param.Board, param.Com);
		} else if (!strcmp(buffer, "q")) {
			break;
		}
	}

	main_param_finalize(&param);

	return 0;
}
