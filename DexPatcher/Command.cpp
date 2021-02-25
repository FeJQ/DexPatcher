#include "Command.h"
#include <string>
#include "Build.h"
#include "ParseDex.h"
#include "PatchDex.h"

using namespace std;


void Command::initHandler(int argc, char* argv[])
{
	// create a parser
	parser* p = new parser();
	// add specified type of variable.
	// 1st argument is long name
	// 2nd argument is short name (no short name if '\0' specified)
	// 3rd argument is description
	// 4th argument is mandatory (optional. default is false)
	// 5th argument is default value  (optional. it used when mandatory is false)
	p->add("version", 'v', "get versison and building information");
	p->add("fix", 'f', "fix the dex file");
	p->add<string>("dex-path", 'd', "the dex file path", false);
	p->add<string>("json-path", 'j', "the json file path", false);
	p->set_program_name("DexPatcher");

	p->parse_check(argc, argv);

	if (p->exist("version"))
	{
		handleVersion();
		return;
	}

	if (p->exist("fix"))
	{
		if (p->exist("json-path") && p->exist("dex-path"))
		{
			string dexFilePath = p->get<string>("dex-path");
			string methodInfoPath = p->get<string>("json-path");
			handleFixMethod(dexFilePath, methodInfoPath);
			return;
		}
		else if (p->exist("dex-path"))
		{
			string dexFilePath = p->get<string>("dex-path");
			handleFixMagic(dexFilePath);
			return;
		}
		else
		{
			cout << "dp fix -d dexFilePath<string> [-j methodInfoPath<string>]";
			return;
		}
	}
	else
	{
		p->usage();
	}
}

void Command::handleVersion()
{
	char* date = Build::GetBuildDate();
	printf("DexPatcher version %s (build %s by %s)\n", VERSION, date, AUTHOR);
}

void Command::handleFixMagic(string dexFilePath)
{
	PatchDex* patchDex = new PatchDex(dexFilePath);
	patchDex->fixDexMagic(true);
	delete patchDex;
}

void Command::handleFixMethod(string dexFilePath, string methodInfoPath)
{
	ParseDex* parseDex = new PatchDex(dexFilePath);
	
	parseDex->fixMethod(methodInfoPath);
	delete parseDex;
}
