#include "remicss/ssalg.h"


//copied from ssalg_dup.h
//should be a field here...probably.
struct ssalg_multi
{
	struct ssalg super;
	int n; //number of channels
	int k; //threshold.
};

int ssalg_multi_init(struct ssalg_multi *this, int n, int k);

