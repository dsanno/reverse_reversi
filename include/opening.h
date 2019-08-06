#ifndef OPENING_H
#define OPENING_H

#include "board.h"

typedef struct _Opening Opening;

/* ‹Ç–Êî•ñ */
typedef struct _PositionInfo PositionInfo;
struct _PositionInfo
{
	int Value;		/* •]‰¿’l */
};

#define PositionInfo_Value(self)					((self)->Value)
#define PositionInfo_SetValue(self, in_value)		((self)->Value = (in_value))

#ifdef __cplusplus
extern "C" {
#endif
Opening	*Opening_New(void);
void	Opening_Delete(Opening *self);

int		Opening_Load(Opening *self, const char *in_file_name);
int		Opening_Save(const Opening *self, const char *in_file_name);

int		Opening_Info(const Opening *self, const Board *in_board, int in_color, PositionInfo *out_info);
int		Opening_SetInfo(Opening *self, const Board *in_board, int in_color, const PositionInfo *in_info);
#ifdef __cplusplus
}
#endif

#endif /* OPENING_H */
