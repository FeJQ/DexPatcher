#include "PatchDex.h"
#include "Utils.h"
#include <iostream>
#include "json.hpp"
#include <fstream>
#include <map>
#include <sstream>

using json = nlohmann::json;
using namespace Utils;

float PatchDex::progress = 0;

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
	cout << "Fixed Dex Magic" << END;
	if (isSave)
	{
		if (saveDexFile())
		{
			cout << "All operations completed" << END;
		}
		else
		{
			cout << "save file failed" << END;
		}
	}
}


/// <summary>
/// 修复被抽取的函数
/// </summary>
/// <param name="dexFilePath">The dex file path.</param>
/// <param name="methodInfoPath">The method information path.</param>
/// /// <param name="noLog">是否不打印日志信息</param>
void PatchDex::fixMethod(string methodInfoPath, bool noLog)
{
	// 初始化进度
	setProgress(0);
	cout << "Parsing " << methodInfoPath <<"..." <<END;
	parseClassDataItem();
	parseMethodInfo(methodInfoPath);
	fixDexMagic(false);

	cout << classDefMethods.size() << " methods to be fixed" << END;
	cout << "Fixing"<< "..." << END;
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
			//cout << "method information not found:" << methodName << END;
		}
		else
		{
			fixedNumber++;
			setProgress((float)(i + 1) /(float)classDefMethods.size());

			
			char strProgress[64] = { 0 };
			sprintf_s(strProgress,"[%3d%% %d/%d] ", (int)(getProgress() * 100), i + 1,classDefMethods.size());
			string strTitle = strProgress;
			strTitle += methodName;
			if (!noLog)
			{
				// [100% 175/175]
				cout << strTitle << "Fixed method:" << methodName << END;
			}
			Utils::setTitle(strTitle);
		}
	}
	cout << "Total:" << classDefMethods.size() << END;
	cout << "Fixed:" << fixedNumber << END;
	cout << "Not being fixed:" << notFixedNumber << END;

	if (saveDexFile())
	{
		cout << "All operations completed" << END;
	}
	else
	{
		cout << "Save file failed" << END;
	}

}

void PatchDex::parseMethodInfo(string methodInfoPath)
{
	fstream file;
	file.open(methodInfoPath, ios::in);
	if (!file.is_open())
	{
		cout << "open file:" << methodInfoPath << " failed" << END;
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
		cout << "parse file:" << methodInfoPath << " failed" << END;
		throw "parse file failed";
		return;
	}
	if (root.contains("count"))
	{
		//cout << "method count:" << root["count"] << END;
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


