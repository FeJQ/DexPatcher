#include "ParseDex.h"
#include <iostream>

char* ParseDex::openDexFile(char* fileName)
{
	FILE* stream = fopen(fileName, "rb+");
	fseek(stream, 0, SEEK_END);
	int fileSize = ftell(stream);
	char* buffer = (char*)malloc(fileSize);
	if (buffer != NULL)
	{
		fread(buffer, fileSize, 1, stream);
	}
	return buffer;
}

void ParseDex::closeDexFile()
{
	if (dexFile != NULL)
	{
		free(dexFile);
	}
}


