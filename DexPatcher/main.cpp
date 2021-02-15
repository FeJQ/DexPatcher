// DexPatcher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "ParseDex.h"

int main()
{
	ParseDex* parseDex = new ParseDex("C:\\Users\\love0\\Desktop\\apps\\趣智校园\\aupk\\8273364_Execute.dex");
	//char* magic = (char*)parseDex->getDexFile()->pHeader->magic;
	//parseDex->saveDexFile();
	//parseDex->parseDexClassDef();
	parseDex->fixMethod(25417,NULL);
	system("pause");

}
