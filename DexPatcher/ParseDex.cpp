// ***********************************************************************
// Assembly         : 
// Author           : FeJQ
// Created          : 02-15-2021
//
// Last Modified By : FeJQ
// Last Modified On : 02-15-2021
// ***********************************************************************
// <copyright file="ParseDex.cpp" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************


#include "ParseDex.h"
#include <iostream>
#include "Utils.h"
using namespace Utils;


/// <summary>
/// 打开Dex文件
/// </summary>
/// <param name="fileName">文件路径.</param>
/// <returns>是否成功</returns>
bool ParseDex::openDexFile(const char* filePath)
{
	// 打开文件
	if (!File::openFile(filePath, &mFileSize, &mDexBuffer))
	{
		return false;
	}
	this->mDexFile = (DexFile*)&mDexBuffer;

	// 保存文件完整路径
	mFilePath = (char*)malloc(MAX_PATH);
	if (mFilePath == NULL)
	{
		return false;
	}
	strcpy_s(mFilePath, MAX_PATH, filePath);

	// 保存带后缀的文件名
	mFileName = (char*)malloc(MAX_PATH);
	if (mFileName == NULL)
	{
		return false;
	}
	File::getFileName(mFilePath, mFileName);

	return true;
}

/// <summary>
/// 保存文件
/// </summary>
/// <returns>是否成功</returns>
bool ParseDex::saveDexFile()
{
	if (mDexFile == NULL)
		return false;
	// 获取打开的dex文件路径
	char path[MAX_PATH] = { 0 };
	File::getPath(mFilePath, path);

	// 获取不带后缀的文件名
	char fileNameWithoutExtension[MAX_PATH] = { 0 };
	File::getFileNameWithoutExtension(mFileName, fileNameWithoutExtension);

	// 构造保存文件名
	char fileName[MAX_PATH] = { 0 };
	sprintf_s(fileName, "%s%s_patched.dex", path, fileNameWithoutExtension);

	// 保存文件
	Utils::File::saveFile(mDexBuffer, fileName, mFileSize);
	return false;
}

/// <summary>
/// 关闭Dex文件,用于善后清理
/// </summary>
void ParseDex::closeDexFile()
{
	if (mFilePath != NULL)
	{
		free(mFilePath);
		mFilePath = NULL;
	}
	if (mFileName != NULL)
	{
		free(mFileName);
		mFileName = NULL;
	}
	if (mDexBuffer != NULL)
	{
		free(mDexBuffer);
		mDexBuffer = NULL;
		mDexFile = NULL;
	}
	
	
}

/// <summary>
/// 检查Dex有效性
/// </summary>
/// <returns>是否有效</returns>
bool ParseDex::checkValidity()
{
	if (mDexFile == NULL)
		return false;

	// magic为以下值时方才有效
	//"dex\n035\0"
	//"dex\n036\0"
	//"dex\n037\0"

	const u1* magic = mDexFile->pHeader->magic;
	const u1* version = &magic[4];

	if (memcmp(magic, DEX_MAGIC, 4) != 0)
	{
		return false;
	}
	if ((memcmp(version, DEX_MAGIC_VERS, 4) != 0) &&
		(memcmp(version, DEX_MAGIC_VERS_API_13, 4) != 0) &&
		(memcmp(version, DEX_MAGIC_VERS_37, 4) != 0)) {
		return false;
	}
	return true;
}



/// <summary>
/// 获取method对应的类信息
/// </summary>
/// <param name="dexMethodId"></param>
/// <param name="dexClassDef"></param>
void ParseDex::getMethodClassDef(DexMethodId* dexMethodId, OUT DexClassDef** dexClassDef)
{
	bool flag1 = false;
	bool flag2 = false;
	*dexClassDef = (DexClassDef*)(mDexBuffer + mDexFile->pHeader->classDefsOff);
	for (int i = 0; i < mDexFile->pHeader->classDefsSize; i++)
	{
		int type1 = getType(dexMethodId->classIdx);
		int type2 = getType((*dexClassDef)[i].classIdx);
		char* className1 = getString(type1);
		char* className2 = getString(type2);

		flag1 = false;
		flag2 = false;

		if (type1 == type2)
		{
			flag1 = true;
		}
		if (strcmp(className1, className2) == 0)
		{
			flag2 = true;
		}
		if (flag1 != flag2)
		{
			printf("flag1!=flag2 异常!!!");
			throw;
		}
		if (flag1 == true && flag2 == true)
		{
			*dexClassDef += i;
			break;
		}
	}
}

/// <summary>
/// 获取code偏移
/// </summary>
/// <param name="methodIdx">函数索引</param>
/// <returns>偏移</returns>
int ParseDex::getCodeOffset(int methodIdx)
{
	// 获取methodIdx对应的method_ids_item
	DexMethodId* dexMethodId = (DexMethodId*)(mDexBuffer + mDexFile->pHeader->methodIdsOff);
	dexMethodId += methodIdx;

	// Method对应的DexClassDefItem信息
	DexClassDef* dexClassDef = NULL;
	getMethodClassDef(dexMethodId, &dexClassDef);

	// 获取classData偏移地址
	u1* classData = (u1*)mDexBuffer + dexClassDef->classDataOff;
	// 获取ClassDataItem
	ClassDataItem classDataItem;
	parseClassDataItem(classData, &classDataItem);

	int codeOffset = NULL;
	int directMethodIdx = 0;
	for (int i = 0; i < classDataItem.directMethodsSize; i++)
	{
		if (classDataItem.directMethods[i].method_idx_diff == methodIdx)
		{
			codeOffset = classDataItem.directMethods[i].code_off;
			return codeOffset;
		}
		directMethodIdx += classDataItem.directMethods[i].method_idx_diff;
	}

	int virtualMethodIdx = 0;
	for (int i = 0; i < classDataItem.virtualMethodsSize; i++)
	{
		if (classDataItem.virtualMethods[i].method_idx_diff == methodIdx)
		{
			codeOffset = classDataItem.virtualMethods[i].code_off;
			return codeOffset;;
		}
		virtualMethodIdx += classDataItem.virtualMethods[i].method_idx_diff;
	}
	if (codeOffset == NULL)
	{
		printf("未找到函数对应的codeOffset");
	}
	return codeOffset;
}

/// <summary>
/// 从string_ids里获取索引为stringIdx的项
/// </summary>
/// <param name="stringIdx">string_ids索引</param>
/// <returns>字符串</returns>
char* ParseDex::getString(int stringIdx)
{
	int* stringIdsOff = (int*)(mDexBuffer + mDexFile->pHeader->stringIdsOff);
	char* str = mDexBuffer + stringIdsOff[stringIdx];
	Leb128::decodeUnsignedLeb128((u1**)&str);
	return str;
}

/// <summary>
/// 从type_ids里获取索引为typeIdx的项
/// </summary>
/// <param name="typeIdx">type_ids索引</param>
/// <returns>type的描述,值为string_ids索引</returns>
int ParseDex::getType(int typeIdx)
{
	int* typeIdsOff = (int*)(mDexBuffer + mDexFile->pHeader->typeIdsOff);

	int* type = typeIdsOff + typeIdx;
	return *type;
}

/// <summary>
/// 解析ClassDef->ClassData
/// </summary>
/// <param name="classData"></param>
/// <param name="classDataItem">传出参数</param>
void ParseDex::parseClassDataItem(u1* classData, OUT ClassDataItem* classDataItem)
{
	classDataItem->staticFiledsSize = Leb128::decodeUnsignedLeb128(&classData);
	classDataItem->instanceFieldsSize = Leb128::decodeUnsignedLeb128(&classData);
	classDataItem->directMethodsSize = Leb128::decodeUnsignedLeb128(&classData);
	classDataItem->virtualMethodsSize = Leb128::decodeUnsignedLeb128(&classData);

	//解析static_fields数组
	EncodedField* staticField = (EncodedField*)malloc(sizeof(EncodedField) * classDataItem->staticFiledsSize);
	if (staticField == NULL)
	{
		throw "内存申请失败";
	}
	memset(staticField, 0, sizeof(EncodedField) * classDataItem->staticFiledsSize);
	classDataItem->staticFields = staticField;
	for (int i = 0; i < classDataItem->staticFiledsSize; i++)
	{
		staticField[i].filed_idx_diff = Leb128::decodeUnsignedLeb128(&classData);
		staticField[i].access_flags = Leb128::decodeUnsignedLeb128(&classData);
	}

	//解析instance_fields数组
	EncodedField* instanceField = (EncodedField*)malloc(sizeof(EncodedField) * classDataItem->instanceFieldsSize);
	if (staticField == NULL)
	{
		throw "内存申请失败";
	}
	memset(instanceField, 0, sizeof(EncodedField) * classDataItem->instanceFieldsSize);
	classDataItem->instanceFields = instanceField;
	for (int i = 0; i < classDataItem->instanceFieldsSize; i++)
	{
		instanceField[i].filed_idx_diff = Leb128::decodeUnsignedLeb128(&classData);
		instanceField[i].access_flags = Leb128::decodeUnsignedLeb128(&classData);
	}

	// 解析直接方法
	EncodedMethod* directMethod = (EncodedMethod*)malloc(sizeof(EncodedMethod) * classDataItem->directMethodsSize);
	if (staticField == NULL)
	{
		throw "内存申请失败";
	}
	memset(directMethod, 0, sizeof(EncodedMethod) * classDataItem->directMethodsSize);
	classDataItem->directMethods = directMethod;
	for (int i = 0; i < classDataItem->directMethodsSize; i++)
	{
		directMethod[i].method_idx_diff = Leb128::decodeUnsignedLeb128(&classData);
		directMethod[i].access_flags = Leb128::decodeUnsignedLeb128(&classData);
		directMethod[i].code_off = Leb128::decodeUnsignedLeb128(&classData);
	}

	//解析虚方法
	EncodedMethod* virtualMethod = (EncodedMethod*)malloc(sizeof(EncodedMethod) * classDataItem->virtualMethodsSize);
	if (staticField == NULL)
	{
		throw "内存申请失败";
	}
	memset(virtualMethod, 0, sizeof(EncodedMethod) * classDataItem->virtualMethodsSize);
	classDataItem->virtualMethods = virtualMethod;
	for (int i = 0; i < classDataItem->virtualMethodsSize; i++)
	{
		virtualMethod[i].method_idx_diff = Leb128::decodeUnsignedLeb128(&classData);
		virtualMethod[i].access_flags = Leb128::decodeUnsignedLeb128(&classData);
		virtualMethod[i].code_off = Leb128::decodeUnsignedLeb128(&classData);
	}
}


