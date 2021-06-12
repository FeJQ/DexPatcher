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
#include <vector>
using namespace Utils;
using namespace std;




/// <summary>
/// ��Dex�ļ�
/// </summary>
/// <param name="fileName">�ļ�·��.</param>
/// <returns>�Ƿ�ɹ�</returns>
bool ParseDex::openDexFile(const char* filePath)
{
	// ���ļ�
	if (!File::openFile(filePath, &mFileSize, &mDexBuffer))
	{
		return false;
	}
	this->mDexFile = (DexFile*)&mDexBuffer;

	// �����ļ�����·��
	mFilePath = (char*)malloc(MAX_PATH);
	if (mFilePath == NULL)
	{
		return false;
	}
	strncpy(mFilePath, filePath, MAX_PATH);

	// �������׺���ļ���
	mFileName = (char*)malloc(MAX_PATH);
	if (mFileName == NULL)
	{
		return false;
	}
	File::getFileName(mFilePath, mFileName);

	return true;
}

/// <summary>
/// �����ļ�
/// </summary>
/// <returns>�Ƿ�ɹ�</returns>
bool ParseDex::saveDexFile()
{
	if (mDexFile == NULL)
		return false;
	// ��ȡ�򿪵�dex�ļ�·��
	char path[MAX_PATH] = { 0 };
	File::getPath(mFilePath, path);

	// ��ȡ������׺���ļ���
	char fileNameWithoutExtension[MAX_PATH] = { 0 };
	File::getFileNameWithoutExtension(mFileName, fileNameWithoutExtension);

	// ���챣���ļ���
	char fileName[MAX_PATH] = { 0 };
	sprintf(fileName, "%s%s_patched.dex", path, fileNameWithoutExtension);

	// �����ļ�
	bool result = File::saveFile(mDexBuffer, fileName, mFileSize);
	if (result)
	{
		cout << "Saved file to:" << fileName << END;
	}
	return result;
}

/// <summary>
/// �ر�Dex�ļ�,�����ƺ�����
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
/// ���Dex��Ч��
/// </summary>
/// <returns>�Ƿ���Ч</returns>
bool ParseDex::checkValidity()
{
	if (mDexFile == NULL)
		return false;

	// magicΪ����ֵʱ������Ч
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
/// ��string_ids���ȡ����ΪstringIdx����
/// </summary>
/// <param name="stringIdx">string_ids����</param>
/// <returns>�ַ���</returns>
string ParseDex::getString(int stringIdx)
{
	int* stringIdsOff = (int*)(mDexBuffer + mDexFile->pHeader->stringIdsOff);
	char* str = mDexBuffer + stringIdsOff[stringIdx];
	Leb128::decodeUnsignedLeb128((u1**)&str);
	return string(str);
}

/// <summary>
/// ��type_ids���ȡ����ΪtypeIdx����
/// </summary>
/// <param name="typeIdx">type_ids����</param>
/// <returns>type������,ֵΪstring_ids����</returns>
int ParseDex::getType(int typeIdx)
{
	int* typeIdsOff = (int*)(mDexBuffer + mDexFile->pHeader->typeIdsOff);
	if (mDexFile->pHeader->typeIdsOff != 203032)
	{
		int a = 0;
	}
	int* type = typeIdsOff + typeIdx;
	if (*type < 0)
	{
		int a = 0;
	}
	return *type;
}

/// <summary>
/// ��ȡ����������
/// </summary>
/// <param name="methodIdx">��������</param>
/// <returns>������</returns>
string ParseDex::getMethodName(int methodIdx)
{
	DexMethodId *method=(DexMethodId*)(mDexBuffer + mDexFile->pHeader->methodIdsOff);
	if (methodIdx < 0 || methodIdx >=mDexFile->pHeader->methodIdsSize)
	{
		throw "methodIdx ����";
		return nullptr;
	}
	string methodName=getString(method[methodIdx].nameIdx);
	return methodName;
}

void ParseDex::parseClassDataItem()
{
	// ��ȡclass def��
	DexClassDef* classDef = (DexClassDef*)(mDexBuffer + mDexFile->pHeader->classDefsOff);
	// ��ȡclass def�������
	int size = mDexFile->pHeader->classDefsSize;
	for (int i = 0; i < size; i++)
	{
		ClassDataItem classDataItem;
		if (classDef[i].classDataOff == 0)
		{
			// ��ЩclassDefû��class_data,��class_data_offΪ0,����ֱ������
			continue;
		}
		u1* classData = (u1*)(mDexBuffer + classDef[i].classDataOff);
		classDataItem.staticFiledsSize = Leb128::decodeUnsignedLeb128(&classData);
		classDataItem.instanceFieldsSize = Leb128::decodeUnsignedLeb128(&classData);
		classDataItem.directMethodsSize = Leb128::decodeUnsignedLeb128(&classData);
		classDataItem.virtualMethodsSize = Leb128::decodeUnsignedLeb128(&classData);

		// ����static_fields����
		EncodedField* staticField = (EncodedField*)malloc(sizeof(EncodedField) * classDataItem.staticFiledsSize);
		if (staticField == NULL)
		{
			throw "�ڴ�����ʧ��";
		}
		memset(staticField, 0, sizeof(EncodedField) * classDataItem.staticFiledsSize);
		classDataItem.staticFields = staticField;
		for (int j = 0; j < classDataItem.staticFiledsSize; j++)
		{
			staticField[j].filed_idx_diff = Leb128::decodeUnsignedLeb128(&classData);
			staticField[j].access_flags = Leb128::decodeUnsignedLeb128(&classData);
		}

		// ����instance_fields����
		EncodedField* instanceField = (EncodedField*)malloc(sizeof(EncodedField) * classDataItem.instanceFieldsSize);
		if (instanceField == NULL)
		{
			throw "�ڴ�����ʧ��";
		}
		memset(instanceField, 0, sizeof(EncodedField) * classDataItem.instanceFieldsSize);
		classDataItem.instanceFields = instanceField;
		for (int j = 0; j < classDataItem.instanceFieldsSize; j++)
		{
			instanceField[j].filed_idx_diff = Leb128::decodeUnsignedLeb128(&classData);
			instanceField[j].access_flags = Leb128::decodeUnsignedLeb128(&classData);
		}

		//  ����ֱ�ӷ���
		//EncodedMethod* directMethod = (EncodedMethod*)malloc(sizeof(EncodedMethod) * classDataItem->directMethodsSize);
		//if (directMethod == NULL)
		//{
		//	throw "�ڴ�����ʧ��";
		//}
		//memset(directMethod, 0, sizeof(EncodedMethod) * classDataItem->directMethodsSize);
		//classDataItem->directMethods = directMethod;
		int directMethodIdx = 0;
		for (int j = 0; j < classDataItem.directMethodsSize; j++)
		{
			//directMethod[i].method_idx_diff = Leb128::decodeUnsignedLeb128(&classData);
			//directMethod[i].access_flags = Leb128::decodeUnsignedLeb128(&classData);
			//directMethod[i].code_off = Leb128::decodeUnsignedLeb128(&classData);
			ClassDefMethod* method = new ClassDefMethod();
			int method_idx_diff = Leb128::decodeUnsignedLeb128(&classData);
			directMethodIdx += method_idx_diff;
			method->index = directMethodIdx;
			method->accessFlags = Leb128::decodeUnsignedLeb128(&classData);
			method->codeOffset = Leb128::decodeUnsignedLeb128(&classData);
			method->classDefIndex = i;
			if (method->codeOffset != 0)
			{
				classDefMethods.push_back(method);
			}
		}

		// �����鷽��
		//EncodedMethod* virtualMethod = (EncodedMethod*)malloc(sizeof(EncodedMethod) * classDataItem->virtualMethodsSize);
		//if (virtualMethod == NULL)
		//{
		//	throw "�ڴ�����ʧ��";
		//}
		//memset(virtualMethod, 0, sizeof(EncodedMethod) * classDataItem->virtualMethodsSize);
		//classDataItem->virtualMethods = virtualMethod;
		int virtualMethodIdx = 0;
		for (int j = 0; j < classDataItem.virtualMethodsSize; j++)
		{
			//virtualMethod[i].method_idx_diff = Leb128::decodeUnsignedLeb128(&classData);
			//virtualMethod[i].access_flags = Leb128::decodeUnsignedLeb128(&classData);
			//virtualMethod[i].code_off = Leb128::decodeUnsignedLeb128(&classData);
			ClassDefMethod* method = new ClassDefMethod();
			int method_idx_diff= Leb128::decodeUnsignedLeb128(&classData);
			virtualMethodIdx += method_idx_diff;
			method->index = virtualMethodIdx;
			method->accessFlags = Leb128::decodeUnsignedLeb128(&classData);
			method->codeOffset = Leb128::decodeUnsignedLeb128(&classData);
			method->classDefIndex = i;
			if (method->codeOffset != 0)
			{
				classDefMethods.push_back(method);
			}
			
		}
	}
}



