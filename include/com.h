#ifndef COM_H
#define COM_H

#include "board.h"
#include "evaluator.h"
#include "opening.h"

typedef struct _Com Com;

#define MPC_DEPTH_MIN 3
typedef struct _MPCInfo MPCInfo;
struct _MPCInfo
{
	int Depth;
	int Offset;
	int Deviation;
};

#ifdef __cplusplus
extern "C" {
#endif
Com		*Com_New(Evaluator *evaluator, Opening *opening);
void	Com_Delete(Com *self);

void	Com_SetLevel(Com *self, int in_mid, int in_exact, int in_wld);
void	Com_SetOpening(Com *self, int in_use);
int		Com_NextMove(Com *self, const Board *in_board, int in_color, int *out_value);
int		Com_LoadMPCInfo(Com *self, const char *in_file_name);

int		Com_CountNodes(const Com *self);
int		Com_CountHashGet(const Com *self);
int		Com_CountHashHit(const Com *self);
#ifdef __cplusplus
}
#endif

#endif /* COM_H */
