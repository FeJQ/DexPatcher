#include "PatchDex.h"
#include "Utils.h"
#include <iostream>
#include "json.hpp"
#include <fstream>
#include <map>
#include <sstream>

using json = nlohmann::json;
using namespace Utils;


/// <summary>
/// 修复Dex Magic信息
/// </summary>
void PatchDex::fixDexMagic(bool isSave)
{
	if (checkValidity())
	{
		return;
	}
	const u1* magic = getDexFile()->pHeader->magic;
	const u1* version = &magic[4];
	memcpy((void*)magic, DEX_MAGIC_VERS, 4);
	memcpy((void*)version, DEX_MAGIC_VERS_API_13, 4);
	cout << "Fixed Dex Magic" << endl;
	if (isSave)
	{
		if (saveDexFile())
		{
			cout << "All operations completed" << endl;
		}
		else
		{
			cout << "save file failed" << endl;
		}
	}
}


/// <summary>
/// 修复被抽取的函数
/// </summary>
/// <param name="dexFilePath">The dex file path.</param>
/// <param name="methodInfoPath">The method information path.</param>
void PatchDex::fixMethod(string methodInfoPath)
{
	parseClassDataItem();
	parseMethodInfo(methodInfoPath);
	fixDexMagic(false);

	cout << classDefMethods.size() << " methods to be fixed" << endl;
	int fixedNumber = 0;
	int notFixedNumber = 0;
	for (int i = 0; i < classDefMethods.size(); i++)
	{
		bool isRepaired = false;
		ClassDefMethod* classDefMethod = classDefMethods[i];
		string methodName = "null";
		for (int j = 0; j < methods.size(); j++)
		{
			if (classDefMethod->index == methods[j]->methodIndex)
			{
				// 解析并修复
				DexCode* dexCode = (DexCode*)(getDexBuffer() + classDefMethod->codeOffset);
				int outLen;
				char* codeItem = NULL;
				Utils::Base64::decode(methods[j]->codeItem.c_str(), methods[j]->codeItem.size(), &codeItem, &outLen);

				memcpy(dexCode, codeItem, outLen);

				free(codeItem);
				isRepaired = true;
				methodName = methods[j]->methodName;
				break;
			}
		}	
		if (!isRepaired)
		{
			notFixedNumber++;
			methodName = getMethodName(classDefMethod->index);
			//cout << "method information not found:" << methodName << endl;
		}
		else
		{
			fixedNumber++;
			cout << "Fixed method[" << i + 1 <<"/"<< classDefMethods.size() << "]:" << methodName << endl;
		}
	}
	cout << "Total:" << classDefMethods.size() << endl;
	cout << "Fixed:" << fixedNumber << endl;
	cout << "Not being fixed:" << notFixedNumber << endl;

	if (saveDexFile())
	{
		cout << "All operations completed" << endl;
	}
	else
	{
		cout << "save file failed" << endl;
	}
}

void PatchDex::parseMethodInfo(string methodInfoPath)
{
	fstream file;
	file.open(methodInfoPath, ios::in);
	if (!file.is_open())
	{
		cout << "open file:" << methodInfoPath << " failed" << endl;
		throw "open file failed";
		return;
	}
	json root;
	try
	{
		file >> root;
		file.close();
	}
	catch (const std::exception&)
	{
		cout << "parse file:" << methodInfoPath << " failed" << endl;
		throw "parse file failed";
		return;
	}
	if (root.contains("count"))
	{
		//cout << "method count:" << root["count"] << endl;
	}
	if (root.contains("data"))
	{
		for (int i = 0; i < root["data"].size(); i++)
		{
			MethodInfo* method = new MethodInfo();
			method->methodName = "";
			method->methodIndex = 0;
			method->methodOffset = 0;
			method->codeItemLength = 0;
			method->codeItem = "";
			if (root["data"][i].contains("name"))
			{
				methods.push_back(method);
				method->methodName = root["data"][i]["name"];
			}
			if (root["data"][i].contains("index"))
			{
				method->methodIndex = root["data"][i]["index"];
			}
			if (root["data"][i].contains("offset"))
			{
				method->methodOffset = root["data"][i]["offset"];
			}
			if (root["data"][i].contains("codeItemLength"))
			{
				method->codeItemLength = root["data"][i]["codeItemLength"];
			}
			if (root["data"][i].contains("inst"))
			{
				method->codeItem = root["data"][i]["inst"];
			}
		}
	}
}


