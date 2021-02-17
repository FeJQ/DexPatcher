// DexPatcher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "ParseDex.h"
#include "PatchDex.h"
#include "Build.h"


int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++)
	{
		char* arg = argv[i];
		if (strcmp(arg, "-v") == 0 || strcmp(arg, "-version") == 0)
		{
			char* date = Build::GetBuildDate();
			printf("DexPatcher version %s (build %s by %s)\n", VERSION, date, AUTHOR);
			return 0;
		}
		if (strcmp(arg, "fix") == 0)
		{
			if (i == argc - 2)
			{
				// 2个参数 fix,dexFilePath
				ParseDex* parseDex = new ParseDex(argv[i + 1]);
				PatchDex* patchDex = new PatchDex(parseDex);
				patchDex->fixDexMagic();
				delete patchDex;
				delete parseDex;
			}
			else if (i == argc - 3)
			{
				// 3个参数 fix,dexFilePath,jsonFilePath
				ParseDex* parseDex = new ParseDex(argv[i+1]);
				PatchDex* patchDex = new PatchDex(parseDex);
				patchDex->fixMethod(0,0,0);
				delete patchDex;
				delete parseDex;
			}
			else
			{

			}

		}
	}
	system("pause");


	return 0;

}
