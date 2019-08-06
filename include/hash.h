#ifndef HASH_H
#define HASH_H

struct _HashKey
{
	unsigned long Low;
	unsigned long High;
};
typedef struct _HashKey RevHashKey;

struct _HashInfo
{
	int Lower;
	int Upper;
	unsigned char Depth;
	unsigned char Move;
};
typedef struct _HashInfo HashInfo;

typedef struct _Hash Hash;

#ifdef __cplusplus
extern "C" {
#endif
Hash	*Hash_New(int in_size);
void	Hash_Delete(Hash *self);
void	Hash_Clear(Hash *self);
int		Hash_Set(Hash *self, const RevHashKey *in_key, const HashInfo *in_info);
int		Hash_Get(Hash *self, const RevHashKey *in_key, HashInfo *out_info);
void	Hash_ClearInfo(Hash *self);
int		Hash_CountGet(Hash *self);
int		Hash_CountHit(Hash *self);
#ifdef __cplusplus
}
#endif

#endif /* HASH_H */
