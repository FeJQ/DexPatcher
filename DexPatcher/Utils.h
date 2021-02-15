#pragma once
#include <string.h>
#include <Windows.h>
#include "DexFile.h"
class Utils
{
public:	
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
				j = i+1;
			}
		}
		strcpy_s(fileName, MAX_PATH, &filePath[j]);
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
		int i,j= 0;
		for (i = 0; i < strlen(filePath); i++)
		{
			if (filePath[i] == '\\')
			{
				j = i+1;
			}
		}
		strcpy_s(path,MAX_PATH, filePath);
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
		strcpy_s(fileNameWithoutExtension, MAX_PATH, fileName);
		memcpy(fileNameWithoutExtension+i, "\0", 1);
	}

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

