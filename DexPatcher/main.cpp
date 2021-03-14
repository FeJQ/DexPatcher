// DexPatcher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "Command.h"
#include "Utils.h"
#include <thread>

using namespace std;




int main(int argc, char* argv[])
{

	Command cmd;
	cmd.initHandler(argc, argv);
	return 0;

}
