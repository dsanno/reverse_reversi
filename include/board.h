#ifndef BOARD_H
#define BOARD_H

/* 盤面の大きさ */
#define BOARD_SIZE	8

/* マスの状態 */
#define WALL  -1
#define EMPTY 0
#define BLACK 1
#define WHITE 2

/* マスの位置または手の種類 */
#define PASS -1
#define NOMOVE -2

#define A1 10
#define B1 11
#define C1 12
#define D1 13
#define E1 14
#define F1 15
#define G1 16
#define H1 17

#define A2 19
#define B2 20
#define C2 21
#define D2 22
#define E2 23
#define F2 24
#define G2 25
#define H2 26

#define A3 28
#define B3 29
#define C3 30
#define D3 31
#define E3 32
#define F3 33
#define G3 34
#define H3 35

#define A4 37
#define B4 38
#define C4 39
#define D4 40
#define E4 41
#define F4 42
#define G4 43
#define H4 44

#define A5 46
#define B5 47
#define C5 48
#define D5 49
#define E5 50
#define F5 51
#define G5 52
#define H5 53

#define A6 55
#define B6 56
#define C6 57
#define D6 58
#define E6 59
#define F6 60
#define G6 61
#define H6 62

#define A7 64
#define B7 65
#define C7 66
#define D7 67
#define E7 68
#define F7 69
#define G7 70
#define H7 71

#define A8 73
#define B8 74
#define C8 75
#define D8 76
#define E8 77
#define F8 78
#define G8 79
#define H8 80

/* パターンID */
#define PATTERN_ID_LINE4_1	0
#define PATTERN_ID_LINE4_2	1
#define PATTERN_ID_LINE4_3	2
#define PATTERN_ID_LINE4_4	3
#define PATTERN_ID_LINE3_1	4
#define PATTERN_ID_LINE3_2	5
#define PATTERN_ID_LINE3_3	6
#define PATTERN_ID_LINE3_4	7
#define PATTERN_ID_LINE2_1	8
#define PATTERN_ID_LINE2_2	9
#define PATTERN_ID_LINE2_3	10
#define PATTERN_ID_LINE2_4	11
#define PATTERN_ID_DIAG8_1	12
#define PATTERN_ID_DIAG8_2	13
#define PATTERN_ID_DIAG7_1	14
#define PATTERN_ID_DIAG7_2	15
#define PATTERN_ID_DIAG7_3	16
#define PATTERN_ID_DIAG7_4	17
#define PATTERN_ID_DIAG6_1	18
#define PATTERN_ID_DIAG6_2	19
#define PATTERN_ID_DIAG6_3	20
#define PATTERN_ID_DIAG6_4	21
#define PATTERN_ID_DIAG5_1	22
#define PATTERN_ID_DIAG5_2	23
#define PATTERN_ID_DIAG5_3	24
#define PATTERN_ID_DIAG5_4	25
#define PATTERN_ID_DIAG4_1	26
#define PATTERN_ID_DIAG4_2	27
#define PATTERN_ID_DIAG4_3	28
#define PATTERN_ID_DIAG4_4	29
#define PATTERN_ID_EDGE8_1	30
#define PATTERN_ID_EDGE8_2	31
#define PATTERN_ID_EDGE8_3	32
#define PATTERN_ID_EDGE8_4	33
#define PATTERN_ID_EDGE8_5	34
#define PATTERN_ID_EDGE8_6	35
#define PATTERN_ID_EDGE8_7	36
#define PATTERN_ID_EDGE8_8	37
#define PATTERN_ID_CORNER8_1	38
#define PATTERN_ID_CORNER8_2	39
#define PATTERN_ID_CORNER8_3	40
#define PATTERN_ID_CORNER8_4	41
#define NUM_PATTERN_ID			42

typedef struct _Board Board;
typedef struct _HashKey HashKey;

#ifdef __cplusplus
extern "C" {
#endif
Board	*Board_New(void);
void	Board_Delete(Board *self);

void	Board_Clear(Board *self);

int		Board_Disk(const Board *self, int in_pos);
int		Board_CountDisks(const Board *self, int in_color);

int		Board_Flip(Board *self, int in_color, int in_pos);
int		Board_Unflip(Board *self);
int		Board_CanFlip(const Board *self, int in_color, int in_pos);
int		Board_CountFlips(const Board *self, int in_color, int in_pos);

void	Board_InitializePattern(Board *self);
int		Board_Pattern(const Board *self, int in_id);
int		Board_FlipPattern(Board *self, int in_color, int in_pos);
int		Board_UnflipPattern(Board *self);

void	Board_HashKey(Board *self, HashKey *out_key);

void	Board_Copy(const Board *self, Board *out_board);
void	Board_Reverse(Board *self);

int		Board_CanPlay(const Board *self, int in_color);

int		Board_Pos(int in_x, int in_y);
int		Board_X(int in_pos);
int		Board_Y(int in_pos);
int		Board_OpponentColor(int in_color);
#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
