#pragma once

#include "ParseDex.h"
#include <string>
#include <vector>
using namespace std;
#define override 

struct MethodInfo
{
	string methodName;
	int methodIndex;
	int methodOffset;
	int codeItemLength;
	// 16byte+指令长度+try块
	string codeItem;
};

class PatchDex :public ParseDex
{
public:
	PatchDex(string dexFilePath) :ParseDex(dexFilePath.c_str())
	{
	}
	~PatchDex()
	{
		for (int i = 0; i < methods.size(); i++)
		{
			MethodInfo* method = methods[i];
			delete method;
		}
	}

	/// <summary>
	/// 修复Dex Magic信息
	/// </summary>
	override void fixDexMagic(bool isSave);

	/// <summary>
	/// 修复被抽取的函数
	/// </summary>
	/// <param name="methodInfoPath">The method information path.</param>
	override void fixMethod(string methodInfoPath);
	 
	// 用于存储dump下来的存有信息
	vector< MethodInfo*> methods;

	/// <summary>
	/// 解析dump下来的函数信息
	/// 并将所有信息添加到methods
	/// </summary>
	/// <param name="methodInfoPath">The method information path.</param>
	void parseMethodInfo(string methodInfoPath);
};

