#include "hash.h"
#include <stdlib.h>
#include <string.h>

struct _HashData
{
	HashKey Key;
	HashInfo Info;
};
typedef struct _HashData HashData;

struct _Hash
{
	int Num;
	unsigned long Mask;
	HashData *Data;
	int GetNum;
	int HitNum;
};

static int Hash_Initialize(Hash *self, int in_num);
static void Hash_Finalize(Hash *self);

static int Hash_Initialize(Hash *self, int in_size)
{
	memset(self, 0, sizeof(Hash));
	self->Num = 1 << in_size;
	self->Mask = (1 << in_size) - 1;
	self->Data = malloc(sizeof(HashData) * self->Num);
	if (!self->Data) {
		return 0;
	}
	Hash_Clear(self);
	return 1;
}

static void Hash_Finalize(Hash *self)
{
	if (self->Data) {
		free(self->Data);
	}
}

Hash *Hash_New(int in_size)
{
	Hash *self;

	self = malloc(sizeof(Hash));
	if (self) {
		if (!Hash_Initialize(self, in_size)) {
			Hash_Delete(self);
			self = NULL;
		}
	}
	return self;
}

void Hash_Delete(Hash *self)
{
	Hash_Finalize(self);
	free(self);
}

void Hash_Clear(Hash *self)
{
	int i;

	for (i = 0; i < self->Num; i++) {
		self->Data[i].Key.Low = ~i;
	}
	self->GetNum = 0;
	self->HitNum = 0;
}

int Hash_Set(Hash *self, const HashKey *in_key, const HashInfo *in_info)
{
	int i;

	i = in_key->Low & self->Mask;
	memcpy(&self->Data[i].Key, in_key, sizeof(HashKey));
	memcpy(&self->Data[i].Info, in_info, sizeof(HashInfo));
	return 1;
}

int Hash_Get(Hash *self, const HashKey *in_key, HashInfo *out_info)
{
	int i;

	i = in_key->Low & self->Mask;
	self->GetNum++;
	if (self->Data[i].Key.Low == in_key->Low && self->Data[i].Key.High == in_key->High) {
		memcpy(out_info, &self->Data[i].Info, sizeof(HashInfo));
		self->HitNum++;
		return 1;
	}
	return 0;
}

void Hash_ClearInfo(Hash *self)
{
	self->GetNum = 0;
	self->HitNum = 0;
}

int Hash_CountGet(Hash *self)
{
	return self->GetNum;
}

int Hash_CountHit(Hash *self)
{
	return self->HitNum;
}
