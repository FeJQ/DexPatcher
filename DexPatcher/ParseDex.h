#pragma once
class ParseDex
{
public:
	char* getDexFile()
	{
		return dexFile;
	}
	void setDexFile(char* buffer)
	{
		dexFile = buffer;
	}

	/*
	 * 打开Dex文件
	 */
	char* openDexFile(char*fileName);

	void closeDexFile();
	

private:
	char* dexFile;

};

