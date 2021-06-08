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
#ifdef _WIN32
#include <Windows.h>
#else
#include <errno.h>
#endif
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
#ifdef _WIN32
			int error = GetLastError();
			printf("openDexFile failed:%d\n", error);
#else
                        printf("openDexFile failed:%d\n", errno);
#endif
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
	/// ��Dex�ļ�
	/// </summary>
	/// <param name="fileName">�ļ�·��.</param>
	/// <returns>�Ƿ�ɹ�</returns>
	bool openDexFile(const char* filePath);

	/// <summary>
	/// �����ļ�
	/// </summary>
	/// <returns>�Ƿ�ɹ�</returns>
	bool saveDexFile();

	/// <summary>
	/// �ر�Dex�ļ�,�����ƺ�����
	/// </summary>
	void closeDexFile();

	/// <summary>
	/// ���Dex��Ч��
	/// </summary>
	/// <returns>�Ƿ���Ч</returns>
	bool checkValidity();

	virtual void fixMagic() {};
	virtual void fixMethod(string methodInfoPath, bool noLog) {};
	
	/// <summary>
	/// ��ȡ����������
	/// </summary>
	/// <param name="methodIdx">��������</param>
	/// <returns>������</returns>
	string getMethodName(int methodIdx);
	
	// ���ڴ洢dex�����еĺ�����Ϣ
	vector<ClassDefMethod*> classDefMethods;
	
	/// <summary>
	/// ����class_data
	/// �������еĺ�����Ϣ��ӵ�classDefMethods
	/// </summary>
	void parseClassDataItem();

private:
	/// <summary>
	/// DexFile����ָ��,ָ���mDexBuffer��ͬ���ڴ�����
	/// </summary>
	DexFile* mDexFile = NULL;

	char* mFilePath = NULL;
	char* mFileName = NULL;
	char* mDexBuffer = NULL;
	int mFileSize = 0;

	/// <summary>
	/// ��string_ids���ȡ����ΪstringIdx����
	/// </summary>
	/// <param name="stringIdx">string_ids����</param>
	/// <returns>�ַ���</returns>
	string getString(int stringIdx);

	/// <summary>
	/// ��type_ids���ȡ����ΪtypeIdx����
	/// </summary>
	/// <param name="typeIdx">type_ids����</param>
	/// <returns>type������,ֵΪstring_ids����</returns>
	int getType(int typeIdx);

	
};

