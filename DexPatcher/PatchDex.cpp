#include "PatchDex.h"
#include "Utils.h"
#include <iostream>
#include <string>
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;
using namespace Utils;
using namespace std;

/// <summary>
/// 修复Dex Magic信息
/// </summary>
void PatchDex::fixDexMagic()
{
	if (mParseDex->checkValidity())
	{
		return;
	}
	const u1* magic = mParseDex->getDexFile()->pHeader->magic;
	const u1* version = &magic[4];
	memcpy((void*)magic, DEX_MAGIC_VERS, 4);
	memcpy((void*)version, DEX_MAGIC_VERS_API_13, 4);
}

/// <summary>
/// 修复被抽取的函数
/// </summary>
/// <param name="methodIdx">函数索引</param>
/// <param name="codeItem">The code item.</param>
/// <param name="codeItemLength">Length of the code item.</param>
void PatchDex::fixMethod(int methodIdx, char* codeItem, int codeItemLength)
{
	parseMethodInfo(nullptr);

	int codeOffset = mParseDex->getCodeOffset(methodIdx);
	// 解析并修复
	DexCode* dexCode = (DexCode*)(mParseDex->getDexBuffer() + codeOffset);
	if (codeItemLength != dexCode->insnsSize * 2)
	{
		printf("codeItemLength != dexCode->insnsSize * 2 异常!!!");
		throw;
	}
	memcpy(dexCode->insns, codeItem, dexCode->insnsSize * 2);
}

void PatchDex::parseMethodInfo(char*fileName)
{
	//int fileSize = 0;
	//File::openFile("C:\\Users\\love0\\Desktop\\json.json", &fileSize, &mMethodInfoBuffer);

	json root;
	fstream file;
	file.open(fileName, ios::in);
	if (file.is_open())
	{
		file >> root;
		int count = root["count"];
		for (int i = 0; i < root["data"].size(); i++)
		{
			string name;
			root["data"][i]["name"].get_to(name);
			root["data"][i]["name"].get_to(name);
			root["data"][i]["name"].get_to(name);
			root["data"][i]["name"].get_to(name);
			root["data"][i]["name"].get_to(name);
		}
	}
	file.close();
}
