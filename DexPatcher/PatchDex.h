#pragma once

#include "ParseDex.h"
class PatchDex
{
public:

	PatchDex(ParseDex* parseDex)
	{
		this->mParseDex = parseDex;
	}

	/// <summary>
	/// 修复Dex Magic信息
	/// </summary>
	void fixDexMagic();

	/// <summary>
	/// 修复被抽取的函数
	/// </summary>
	/// <param name="methodIdx">函数索引</param>
	/// <param name="codeItem">The code item.</param>
	/// <param name="codeItemLength">Length of the code item.</param>
	void fixMethod(int methodIdx, char* codeItem, int codeItemLength);

	void parseMethodInfo();

private:
	ParseDex* mParseDex=NULL;

	char* mMethodInfoBuffer=NULL;
};

