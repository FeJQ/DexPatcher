// DexPatcher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "ParseDex.h"
#include "PatchDex.h"
#include "aupk.h"

int main()
{
	int result=art::isFakeInvoke(2,3);

	ParseDex* parseDex = new ParseDex("C:\\Users\\love0\\Desktop\\apps\\趣智校园\\aupk\\8273364_Execute.dex");
	PatchDex* patchDex = new PatchDex(parseDex);
	patchDex->fixMethod(24441, NULL, 0);
	system("pause");

}
