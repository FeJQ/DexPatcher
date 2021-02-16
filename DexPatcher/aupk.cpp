#include "aupk.h"

namespace art
{
	int self = 0;
	int  artMethod = 0;

	 int isFakeInvoke(int a, int b)
	{
		self = a;
		artMethod = b;
		return self + artMethod;
	}
}


