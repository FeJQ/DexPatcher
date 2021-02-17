// ***********************************************************************
// Assembly         : 
// Author           : FeJQ
// Created          : 02-15-2021
//
// Last Modified By : FeJQ
// Last Modified On : 02-15-2021
// ***********************************************************************
// <copyright file="ParseDex.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************

#pragma once
#include "DexFile.h"
#include <Windows.h>
#define OUT

class ParseDex
{
public:

	ParseDex()
	{

	}
	ParseDex(const char* filePath)
	{
		if (!openDexFile(filePath))
		{
			int error = GetLastError();
			printf("error:%d\n", error);

		}
	}
	~ParseDex()
	{
		if (mDexBuffer != NULL)
		{
			delete mDexBuffer;
			mDexBuffer = NULL;
		}
		if (mFileName != NULL)
		{
			delete mFileName;
			mFileName = NULL;
		}
		if (mFilePath != NULL)
		{
			delete mFilePath;
			mFilePath = NULL;
		}
	}


	DexFile* getDexFile()
	{
		return mDexFile;
	}

	char* getDexBuffer()
	{
		return mDexBuffer;
	}


	/// <summary>
	/// 打开Dex文件
	/// </summary>
	/// <param name="fileName">文件路径.</param>
	/// <returns>是否成功</returns>
	bool openDexFile(const char* filePath);

	/// <summary>
	/// 保存文件
	/// </summary>
	/// <returns>是否成功</returns>
	bool saveDexFile();

	/// <summary>
	/// 关闭Dex文件,用于善后清理
	/// </summary>
	void closeDexFile();

	/// <summary>
	/// 检查Dex有效性
	/// </summary>
	/// <returns>是否有效</returns>
	bool checkValidity();

	/// <summary>
	/// 获取code偏移
	/// </summary>
	/// <param name="methodIdx">函数索引</param>
	/// <returns>偏移</returns>
	int getCodeOffset(int methodIdx);





private:
	/// <summary>
	/// DexFile对象指针,指向和mDexBuffer相同的内存区域
	/// </summary>
	DexFile* mDexFile = NULL;

	char* mFilePath = NULL;
	char* mFileName = NULL;
	char* mDexBuffer = NULL;
	int mFileSize = 0;

	/// <summary>
	/// 获取method对应的类信息
	/// </summary>
	/// <param name="dexMethodId"></param>
	/// <param name="dexClassDef"></param>
	void getMethodClassDef(DexMethodId* dexMethodId, OUT DexClassDef** dexClassDef);

	/// <summary>
	/// 从string_ids里获取索引为stringIdx的项
	/// </summary>
	/// <param name="stringIdx">string_ids索引</param>
	/// <returns>字符串</returns>
	char* getString(int stringIdx);

	/// <summary>
	/// 从type_ids里获取索引为typeIdx的项
	/// </summary>
	/// <param name="typeIdx">type_ids索引</param>
	/// <returns>type的描述,值为string_ids索引</returns>
	int getType(int typeIdx);

	/// <summary>
	/// 解析ClassDef->ClassData
	/// </summary>
	/// <param name="classData"></param>
	/// <param name="classDataItem">传出参数</param>
	void parseClassDataItem(u1* classData, OUT ClassDataItem* classDataItem);

};

