#ifndef COMMANDLINE_H
#define COMMANDLINE_H


// Define to change from its default
#ifndef MAX_COMMAND_ARGS
#define MAX_COMMAND_ARGS 50
#endif

#pragma once

#include <stdlib.h>

struct CommandLineOption{
	const char* name;
	const char* description;

	int minParamCount;
	int maxParamCount;
};

struct CommandLineParser{
	const char* args[MAX_COMMAND_ARGS];
	int argCount;

	int flagIndices[MAX_COMMAND_ARGS];
	int flagCount;

	bool isProgramNamePresent;

	char* cmdLineOwnedString;

	CommandLineOption* options;
	int optionCount;

	CommandLineParser(){
		argCount = 0;
		flagCount = 0;
		isProgramNamePresent = true;
		cmdLineOwnedString = nullptr;
		optionCount;
	}

	~CommandLineParser(){
		free(cmdLineOwnedString);
	}

	void SetOptions(CommandLineOption* options, int optionCount);

	void InternalFlagSetup();
	void InitializeFromArgcArgv(int argc, const char** argv);
	void InitializeFromStringNoCopy(char* string);
	void InitializeFromString(const char* string);

	bool IsFlagPresent(const char* flagName);
	const char* FlagValue(const char* flagName);
	int FlagIntValue(const char* flagName);

	//Assumes that are command-line flags start with "-"
	//get however many after it don't
	int FlagArgCount(const char* flagName);
	const char** FlagArgValues(const char* flagName);
};


#endif

