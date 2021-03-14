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
#include <vector>
#include <string>

#define OUT

using namespace std;

struct ClassDefMethod
{
	u4 accessFlags;
	u4 index;
	u4 codeOffset;
	u4 classDefIndex;
};

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
			printf("openDexFile failed:%d\n", error);
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
		for (int i = 0; i < classDefMethods.size(); i++)
		{
			ClassDefMethod* method = classDefMethods[i];
			delete method;
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
	char* getFilePath()
	{
		return mFilePath;
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

	virtual void fixMagic() {};
	virtual void fixMethod(string methodInfoPath, bool noLog) {};
	
	/// <summary>
	/// 获取函数的名字
	/// </summary>
	/// <param name="methodIdx">函数索引</param>
	/// <returns>函数名</returns>
	string getMethodName(int methodIdx);
	
	// 用于存储dex内所有的函数信息
	vector<ClassDefMethod*> classDefMethods;
	
	/// <summary>
	/// 解析class_data
	/// 并将所有的函数信息添加到classDefMethods
	/// </summary>
	void parseClassDataItem();

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
	/// 从string_ids里获取索引为stringIdx的项
	/// </summary>
	/// <param name="stringIdx">string_ids索引</param>
	/// <returns>字符串</returns>
	string getString(int stringIdx);

	/// <summary>
	/// 从type_ids里获取索引为typeIdx的项
	/// </summary>
	/// <param name="typeIdx">type_ids索引</param>
	/// <returns>type的描述,值为string_ids索引</returns>
	int getType(int typeIdx);

	
};

