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
#define OUT

class ParseDex
{
public:

	ParseDex()
	{

	}
	ParseDex(const char* filePath)
	{
		openDexFile(filePath);
	}


	DexFile* getDexFile()
	{
		return mDexFile;
	}
	void setDexFile(DexFile* buffer)
	{
		mDexFile = buffer;
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
	/// 修复Dex Magic信息
	/// </summary>
	void fixDexMagic();


	void parseDexClassDef();

	void parseDexMethod(int methodIdx);

	void fixMethod(int methodIdx, char* codeItem,int codeItemLength);

	int getCodeOffset(ClassDataItem* classDataItem,int methodIdx);

	char* getString(int stringIdx);
	int getType(int typeIdx);

	
	void parseClassDataItem(u1* classData, OUT ClassDataItem* classDataItem);



private:
	/// <summary>
	/// The dex file
	/// </summary>
	DexFile* mDexFile = NULL;

	char* mFilePath = NULL;
	char* mFileName = NULL;
	char* mDexBuffer = NULL;
	int mFileSize = 0;

};

