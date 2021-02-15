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


/// <summary>
/// 打开Dex文件
/// </summary>
/// <param name="fileName">文件路径.</param>
/// <returns>是否成功</returns>
bool ParseDex::openDexFile(const char* filePath)
{
	FILE* stream;
	fopen_s(&stream, filePath, "rb+");
	if (stream == NULL)
	{
		return false;
	}
	fseek(stream, 0, SEEK_END);
	mFileSize = ftell(stream);
	mDexBuffer = (char*)malloc(mFileSize);
	fseek(stream, 0, SEEK_SET);
	if (mDexBuffer != NULL)
	{
		memset(mDexBuffer, 0, mFileSize);
		if (fread(mDexBuffer, mFileSize, 1, stream) == 1)
		{
			this->mDexFile = (DexFile*)&mDexBuffer;
			fclose(stream);
			// 保存文件完整路径
			if (mFilePath == NULL)
			{
				mFilePath = (char*)malloc(MAX_PATH);
			}
			strcpy_s(mFilePath, MAX_PATH, filePath);

			// 保存带后缀的文件名
			if (mFileName == NULL)
			{
				mFileName = (char*)malloc(MAX_PATH);
			}
			Utils::getFileName(mFilePath, mFileName);


			return true;
		}
	}
	fclose(stream);
	return false;
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
	Utils::getPath(mFilePath, path);

	// 获取不带后缀的文件名
	char fileNameWithoutExtension[MAX_PATH] = { 0 };
	Utils::getFileNameWithoutExtension(mFileName, fileNameWithoutExtension);

	// 构造保存文件名
	char fileName[MAX_PATH] = { 0 };
	sprintf_s(fileName, "%s%s_patched.dex", path, fileNameWithoutExtension);

	// 保存文件
	FILE* stream;
	fopen_s(&stream, fileName, "wb+");
	if (stream == NULL)
	{
		return false;
	}
	if (fwrite(mDexBuffer, mFileSize, 1, stream) == 1)
	{
		fclose(stream);
		return true;
	}
	remove(fileName);
	fclose(stream);
	return false;
}

/// <summary>
/// 关闭Dex文件,用于善后清理
/// </summary>
void ParseDex::closeDexFile()
{
	if (mDexFile != NULL)
	{
		free(mDexFile);
	}
	if (mFileName != NULL)
	{
		free(mFileName);
	}
	if (mFilePath != NULL)
	{
		free(mFilePath);
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
/// 修复Dex Magic信息
/// </summary>
void ParseDex::fixDexMagic()
{
	if (checkValidity())
	{
		return;
	}
	const u1* magic = mDexFile->pHeader->magic;
	const u1* version = &magic[4];
	memcpy((void*)magic, DEX_MAGIC_VERS, 4);
	memcpy((void*)version, DEX_MAGIC_VERS_API_13, 4);
}



void ParseDex::parseDexClassDef()
{
	//转为DexMethodId结构体
	DexClassDef* dexClassDef = (DexClassDef*)(mDexBuffer + mDexFile->pHeader->classDefsOff);
	for (int i = 0; i < mDexFile->pHeader->classDefsSize; ++i)
	{
		printf("\n\tstruct class_def_item class_def[%d]", i);
		int* typeIdsOff = (int*)(mDexBuffer + mDexFile->pHeader->typeIdsOff);
		int* stringIdsOff = (int*)(mDexBuffer + mDexFile->pHeader->stringIdsOff);
		u1* classIdx_stringdata = (u1*)mDexBuffer + stringIdsOff[(*(typeIdsOff + dexClassDef->classIdx))];
		//解码
		Utils::decodeUnsignedLeb128(&classIdx_stringdata);
		printf("\n\t\tclassIdx\t\t--> ");
		while (*classIdx_stringdata != '\0')
		{
			printf("%c", (*classIdx_stringdata));
			classIdx_stringdata++;
		}


		printf("\n\t\taccessFlags\t\t-->\t%x", dexClassDef->accessFlags);

		//通过类型去寻找
		u1* superclassIdx_stringdata = (u1*)mDexBuffer + stringIdsOff[(*(typeIdsOff + dexClassDef->superclassIdx))];
		//解码
		Utils::decodeUnsignedLeb128(&superclassIdx_stringdata);
		printf("\n\t\tsuperclassIdx\t--> ");
		while (*superclassIdx_stringdata != '\0')
		{
			printf("%c", (*superclassIdx_stringdata));
			superclassIdx_stringdata++;
		}

		printf("\n\t\tinterfacesOff\t-->\t%d", dexClassDef->interfacesOff);

		if (dexClassDef->sourceFileIdx == -1)
		{
			printf("\n\t\tsourceFileIdx\t-->\tNO_INDEX");
		}
		printf("\n\t\tannotationsOff\t-->\t%d", dexClassDef->annotationsOff);
		printf("\n\t\tclassDataOff\t-->\t%d", dexClassDef->classDataOff);
		printf("\n\t\tstaticValuesOff\t-->\t%d", dexClassDef->staticValuesOff);


		dexClassDef++;
	}
}

void ParseDex::parseDexMethod(int methodIdx)
{
	DexMethodId* dexMethodId = (DexMethodId*)(mDexBuffer + mDexFile->pHeader->methodIdsOff);
	dexMethodId += methodIdx;

	int* typeIdsOff = (int*)(mDexBuffer + mDexFile->pHeader->typeIdsOff);
	int* protoIdsOff = (int*)(mDexBuffer + mDexFile->pHeader->protoIdsOff);
	int* stringIdsOff = (int*)(mDexBuffer + mDexFile->pHeader->stringIdsOff);

	// type to type_ids
	u1* strClassType = (u1*)mDexBuffer + stringIdsOff[(*(typeIdsOff + dexMethodId->classIdx))];
	Utils::decodeUnsignedLeb128(&strClassType);
	printf("\n\t\tclassIdx--> ");
	printf("%s", strClassType);

	// proto to proto_ids
	DexProtoId* dexProtoId = (DexProtoId*)protoIdsOff + dexMethodId->protoIdx;
	u1* strProtoShoty = (u1*)mDexBuffer + stringIdsOff[dexProtoId->shortyIdx];
	Utils::decodeUnsignedLeb128(&strProtoShoty);
	printf("\n\t\tprotoIdx--> ");
	printf("%s", strProtoShoty);

	// name to string_ids
	u1* strName = (u1*)mDexBuffer + stringIdsOff[dexMethodId->nameIdx];
	Utils::decodeUnsignedLeb128(&strName);
	printf("\n\n\t\tnameIdx --> ");
	printf("%s", strName);
}

void ParseDex::fixMethod(int methodIdx, char* codeItem,int codeItemLength)
{
	// 获取methodIdx对应的method_ids_item
	DexMethodId* dexMethodId = (DexMethodId*)(mDexBuffer + mDexFile->pHeader->methodIdsOff);
	dexMethodId += methodIdx;

	// 获取methodIdx所属的类信息
	bool flag1 = false;
	bool flag2 = false;
	DexClassDef* dexClassDef = (DexClassDef*)(mDexBuffer + mDexFile->pHeader->classDefsOff);
	for (int i = 0; i < mDexFile->pHeader->classDefsSize; i++)
	{
		int type1 = getType(dexMethodId->classIdx);
		int type2 = getType(dexClassDef[i].classIdx);
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
			dexClassDef += i;
			break;
		}
	}
	// 获取ClassDataItem
	u1* classData = (u1*)mDexBuffer + dexClassDef->classDataOff;
	ClassDataItem classDataItem;
	parseClassDataItem(classData, &classDataItem);

	int codeOffset = getCodeOffset(&classDataItem, methodIdx);
	DexCode *dexCode=(DexCode*)(mDexBuffer+ codeOffset);
	if (codeItemLength != dexCode->insnsSize * 2)
	{
		printf("codeItemLength != dexCode->insnsSize * 2 异常!!!");
		throw;
	}
	memcpy(dexCode->insns, codeItem, dexCode->insnsSize*2);
}

int ParseDex::getCodeOffset(ClassDataItem* classDataItem, int methodIdx)
{
	int codeOffset = NULL;
	int directMethodIdx = 0;
	for (int i = 0; i < classDataItem->directMethodsSize; i++)
	{
		if (classDataItem->directMethods[i].method_idx_diff == methodIdx)
		{
			codeOffset = classDataItem->directMethods[i].code_off;
			return codeOffset;
		}
		directMethodIdx += classDataItem->directMethods[i].method_idx_diff;
	}

	int virtualMethodIdx = 0;
	for (int i = 0; i < classDataItem->virtualMethodsSize; i++)
	{
		if (classDataItem->virtualMethods[i].method_idx_diff == methodIdx)
		{
			codeOffset = classDataItem->virtualMethods[i].code_off;
			return codeOffset;;
		}
		virtualMethodIdx += classDataItem->virtualMethods[i].method_idx_diff;
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
	Utils::decodeUnsignedLeb128((u1**)&str);
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
	classDataItem->staticFiledsSize = Utils::decodeUnsignedLeb128(&classData);
	classDataItem->instanceFieldsSize = Utils::decodeUnsignedLeb128(&classData);
	classDataItem->directMethodsSize = Utils::decodeUnsignedLeb128(&classData);
	classDataItem->virtualMethodsSize = Utils::decodeUnsignedLeb128(&classData);

	//解析static_fields数组
	EncodedField* staticField = (EncodedField*)malloc(sizeof(EncodedField) * classDataItem->staticFiledsSize);
	classDataItem->staticFields = staticField;
	for (int i = 0; i < classDataItem->staticFiledsSize; i++)
	{
		staticField[i].filed_idx_diff = Utils::decodeUnsignedLeb128(&classData);
		staticField[i].access_flags = Utils::decodeUnsignedLeb128(&classData);
	}

	//解析instance_fields数组
	EncodedField* instanceField = (EncodedField*)malloc(sizeof(EncodedField) * classDataItem->instanceFieldsSize);
	classDataItem->instanceFields = instanceField;
	for (int i = 0; i < classDataItem->instanceFieldsSize; i++)
	{
		instanceField[i].filed_idx_diff = Utils::decodeUnsignedLeb128(&classData);
		instanceField[i].access_flags = Utils::decodeUnsignedLeb128(&classData);
	}

	// 解析直接方法
	EncodedMethod* directMethod = (EncodedMethod*)malloc(sizeof(EncodedMethod) * classDataItem->directMethodsSize);
	classDataItem->directMethods = directMethod;
	for (int i = 0; i < classDataItem->directMethodsSize; i++)
	{
		directMethod[i].method_idx_diff = Utils::decodeUnsignedLeb128(&classData);
		directMethod[i].access_flags = Utils::decodeUnsignedLeb128(&classData);
		directMethod[i].code_off = Utils::decodeUnsignedLeb128(&classData);
	}

	//解析虚方法
	EncodedMethod* virtualMethod = (EncodedMethod*)malloc(sizeof(EncodedMethod) * classDataItem->virtualMethodsSize);
	classDataItem->virtualMethods = virtualMethod;
	for (int i = 0; i < classDataItem->virtualMethodsSize; i++)
	{
		virtualMethod[i].method_idx_diff = Utils::decodeUnsignedLeb128(&classData);
		virtualMethod[i].access_flags = Utils::decodeUnsignedLeb128(&classData);
		virtualMethod[i].code_off = Utils::decodeUnsignedLeb128(&classData);
	}
}


