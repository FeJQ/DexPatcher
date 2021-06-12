#pragma once

#include "config.h"


#include <string.h>
#include "DexFile.h"
#include <string>
#include <errno.h>
#ifdef _WIN32
#include <Windows.h>
#else
inline errno_t fopen_s(FILE **f, const char *name, const char *mode) {
    errno_t ret = 0;
    assert(f);
    *f = fopen(name, mode);
    /* Can't be sure about 1-to-1 mapping of errno and MS' errno_t */
    if (!*f)
        ret = errno;
    return ret;
}
#endif 


#define OUT 
#ifdef _DEBUG
#define END "\n"
#else 
#define END endl
#endif
namespace Utils
{
	static bool setTitle(std::string title)
	{
#ifdef WINDOWS
		return SetConsoleTitleA(title.c_str());
#endif 

		
	}
	class File
	{
	public:
		static bool openFile(const char* filePath, OUT int* fileSize, OUT char** buffer)
		{
			FILE* stream;
			fopen_s(&stream, filePath, "rb+");
			if (stream == NULL)
			{
				return false;
			}
			fseek(stream, 0, SEEK_END);
			if (fileSize == NULL)
			{
				int temp = 0;
				fileSize = &temp;
			}
			*fileSize = ftell(stream);
			*buffer = (char*)malloc(*fileSize);
			fseek(stream, 0, SEEK_SET);
			if (*buffer != NULL)
			{
				memset(*buffer, 0, *fileSize);
				if (fread(*buffer, *fileSize, 1, stream) == 1)
				{
					fclose(stream);
					return true;
				}
			}
			fclose(stream);
			return false;
		}

		static bool saveFile(char* buffer, char* fileName, int size)
		{
			FILE* stream;
			fopen_s(&stream, fileName, "wb+");
			if (stream == NULL)
			{
				return false;
			}
			if (fwrite(buffer, size, 1, stream) == 1)
			{
				fclose(stream);
				return true;
			}
			remove(fileName);
			fclose(stream);
			return false;
		}


		/// <summary>
		/// Gets the name of the file.
		/// </summary>
		/// <param name="filePath">The file path.</param>
		/// <param name="fileName">Name of the file.</param>
		static void getFileName(const char* filePath, OUT char* fileName)
		{
			int i, j = 0;
			for (i = 0; i < strlen(filePath); i++)
			{
				if (filePath[i] == '\\')
				{
					j = i + 1;
				}
			}
			strncpy(fileName, &filePath[j], MAX_PATH);
		}

		/// <summary>
		/// Gets the path.
		/// </summary>
		/// <param name="filePath">The file path.</param>
		/// <param name="path">The path.</param>
		static void getPath(char* filePath, OUT char* path)
		{
			if (path == NULL)
			{
				return;
			}
			int i, j = 0;
			for (i = 0; i < strlen(filePath); i++)
			{
				if (filePath[i] == '\\')
				{
					j = i + 1;
				}
			}
			strncpy(path, filePath, MAX_PATH);
			memcpy(path + j, "\0", 1);
		}

		/// <summary>
		/// Gets the file name without extension.
		/// </summary>
		/// <param name="fileName">Name of the file.</param>
		/// <param name="fileNameWithoutExtension">The file name without extension.</param>
		static void getFileNameWithoutExtension(const char* fileName, OUT char* fileNameWithoutExtension)
		{
			if (fileNameWithoutExtension == NULL)
			{
				return;
			}
			int i = 0;
			for (i = 0; i < strlen(fileName); i++)
				if (fileName[i] == '.') break;
			strncpy(fileNameWithoutExtension, fileName, MAX_PATH);
			memcpy(fileNameWithoutExtension + i, "\0", 1);
		}
	};

	class Leb128
	{
	public:
		static int decodeUnsignedLeb128(u1** pStream)
		{
			u1* ptr = *pStream;
			int result = *(ptr++);
			if (result > 0x7f) {
				int cur = *(ptr++);
				result = (result & 0x7f) | ((cur & 0x7f) << 7);
				if (cur > 0x7f) {
					cur = *(ptr++);
					result |= (cur & 0x7f) << 14;
					if (cur > 0x7f) {
						cur = *(ptr++);
						result |= (cur & 0x7f) << 21;
						if (cur > 0x7f) {
							/*
							 * Note: We don't check to see if cur is out of
							 * range here, meaning we tolerate garbage in the
							 * high four-order bits.
							 */
							cur = *(ptr++);
							result |= cur << 28;
						}
					}
				}
			}
			*pStream = ptr;
			return result;
		}
	};

	class Base64
	{
	public:
		static u1* encode(const u1* str)
		{
			long len;
			long str_len;
			u1* res;
			int i, j;
			//定义base64编码表  
			const char* base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

			//计算经过base64编码后的字符串长度  
			str_len = strlen((char*)str);
			if (str_len % 3 == 0)
				len = str_len / 3 * 4;
			else
				len = (str_len / 3 + 1) * 4;

			res = (u1*)malloc(len + 1);
			res[len] = '\0';

			//以3个8位字符为一组进行编码  
			for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
			{
				res[i] = base64_table[str[j] >> 2]; //取出第一个字符的前6位并找出对应的结果字符  
				res[i + 1] = base64_table[(str[j] & 0x3) << 4 | (str[j + 1] >> 4)]; //将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符  
				res[i + 2] = base64_table[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)]; //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符  
				res[i + 3] = base64_table[str[j + 2] & 0x3f]; //取出第三个字符的后6位并找出结果字符  
			}

			switch (str_len % 3)
			{
			case 1:
				res[i - 2] = '=';
				res[i - 1] = '=';
				break;
			case 2:
				res[i - 1] = '=';
				break;
			}

			return res;
		}

		static void decode(
			IN const char* pEncode,
			IN int uEncLen,
			OUT char** pDecode,
			OUT int* uDecLen)
		{
			assert(pEncode && uEncLen > 0);

			// 先将编码数据转换为索引表中的索引
			char* uIndexs = (char*)calloc(uEncLen, 1);

			int i, j;
			char cTmp;
			for (i = 0; i < uEncLen; i++)
			{
				cTmp = pEncode[i];

				/* = */
				if (cTmp == '=')
					uIndexs[i] = 0;
				/* + / */
				else if (cTmp < '0')
					uIndexs[i] = (cTmp == '/' ? 63 : 62);
				/* 0123456789 */
				else if (cTmp <= '9')
					uIndexs[i] = 52 + (cTmp - '0');
				/* A-Z */
				else if (cTmp <= 'Z')
					uIndexs[i] = cTmp - 'A';
				/* a-z */
				else if (cTmp <= 'z')
					uIndexs[i] = 26 + cTmp - 'a';
			}

			// 现在得到了原来编码时用的索引值列表
			// 因为编码时是在原数据上按6bit的单位依次取得的索引
			// 所以现在的每个索引应该去掉高位2bit然后再组装成原数据就行了

			// 计算所需空间
			*uDecLen = (uEncLen / 4) * 3 ;
			*pDecode = (char*)calloc(*uDecLen + 1, 1);
			char* pTmp = *pDecode;

			// 解码按4个索引对3个原字节来分组处理
			for (i = 0, j = 0; i < uEncLen; i += 4, j += 3)
			{
				/* 每组第一个索引后6位 + 第二个索引去高两位后的次高2位 */
				pTmp[j] = (uIndexs[i] << 2) | ((uIndexs[i + 1] >> 4) & 0x3);
				/* 每组第二个索引余下的低4位 + 第三个索引的去高两位后的次高4位*/
				pTmp[j + 1] = ((uIndexs[i + 1] & 0xf) << 4) | ((uIndexs[i + 2] >> 2) & 0xF);
				/* 每组第三个索引的低2位 + 第四个索引的低6位 */
				pTmp[j + 2] = ((uIndexs[i + 2] & 0x3) << 6) | (uIndexs[i + 3] & 0x3F);
			}

			free(uIndexs);
		}
	};
}

