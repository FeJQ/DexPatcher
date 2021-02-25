#pragma once
#include "cmdline.h"
#include <string>
using namespace std;
using namespace cmdline;

class Command
{
public:
	void initHandler(int argc, char* argv[]);
	static void handleVersion();
	static void handleFixMagic(string dexFilePath);
	static void handleFixMethod(string dexFilePath, string methodInfoPath);
};

