#include "PatchDex.h"
#include "cJSON.h"
#include "Utils.h"

using namespace Utils;

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
	parseMethodInfo();

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

void PatchDex::parseMethodInfo()
{
	int fileSize = 0;
	File::openFile("C:\\Users\\love0\\Desktop\\json.json", &fileSize, &mMethodInfoBuffer);
	cJSON* jsonObj = cJSON_Parse(mMethodInfoBuffer);
	cJSON* arrayItem = cJSON_GetObjectItem(jsonObj, "data");
	cJSON* method = cJSON_GetArrayItem(arrayItem, 0);
	for (int i = 0; i < cJSON_GetArraySize(arrayItem); i++)
	{
		cJSON* nameItem = cJSON_GetObjectItem(method, "name");
		cJSON* methodIndexItem = cJSON_GetObjectItem(method, "methodIndex");
		cJSON* offsetItem = cJSON_GetObjectItem(method, "offset");
		cJSON* codeItemLengthItem = cJSON_GetObjectItem(method, "codeItemLength");
		cJSON* insItem = cJSON_GetObjectItem(method, "ins");
	}
}
