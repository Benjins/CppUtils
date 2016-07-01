#include "commandline.h"

#include "strings.h"

void CommandLineParser::InitializeFromArgcArgv(int argc, const char** argv){
	argCount = isProgramNamePresent ? (argc - 1) : argc;

	if (isProgramNamePresent){
		for(int i = 1; i < argc; i++){
			args[i-1] = argv[i];
		}
	}
	else{
		for (int i = 0; i < argc; i++){
			args[i] = argv[i];
		}
	}

	InternalFlagSetup();
}

void CommandLineParser::InitializeFromStringNoCopy(char* string){
	int len = StrLen(string);
	bool startOfString = true;
	for (int i = 0; i < len; i++){
		if (string[i] == ' '){
			string[i] = '\0';
			startOfString = true;
		}
		else{
			args[argCount] = &string[i];
			argCount++;
			startOfString = false;
		}
	}

	InternalFlagSetup();
}

void CommandLineParser::InitializeFromString(const char* string){
	cmdLineOwnedString = StrDup(string);
	InitializeFromStringNoCopy(cmdLineOwnedString);
}

void CommandLineParser::SetOptions(CommandLineOption* _options, int _optionCount){
	options = _options;
	optionCount = _optionCount;
}

int GetFlagIndex(CommandLineOption* options, int optionCount, const char* flagName){
	for(int i = 0; i < optionCount; i++){
		if (StrEqual(options[i].name, flagName)){
			return i;
		}
	}

	return -1;
}

void CommandLineParser::InternalFlagSetup(){
	for (int i = 0; i < argCount; i++){
		int flagIdx = GetFlagIndex(options, optionCount, args[i]);
		if (flagIdx != -1){
			flagIndices[flagCount] = i;
			flagCount++;

			if (flagCount > 1){
				int prevFlagIndex = flagIndices[flagCount - 2];
				int prevFlagOptionIndex = GetFlagIndex(options, optionCount, args[prevFlagIndex]);

				int numParams = i - prevFlagIndex - 1;
				int minParams = options[prevFlagOptionIndex].minParamCount;
				int maxParams = options[prevFlagOptionIndex].maxParamCount;

				if (numParams < minParams || numParams > maxParams){
					ASSERT_WARN("\nError: arg '%s' passed %d parameters, expecting %d-%d.\n", args[prevFlagIndex], numParams, minParams, maxParams);
				}
			}
		}
	}
}

bool CommandLineParser::IsFlagPresent(const char* flagName){
	int flagIndex = GetFlagIndex(options, optionCount, flagName);
	if (flagIndex < 0){
		return false;
	}

	for(int i = 0; i < flagCount; i++){
		if (StrEqual(args[flagIndices[i]], flagName)){
			return true;
		}
	}

	return false;
}

const char* CommandLineParser::FlagValue(const char* flagName){
	int flagIndex = GetFlagIndex(options, optionCount, flagName);
	if (flagIndex < 0){
		return nullptr;
	}

	for(int i = 0; i < flagCount; i++){
		int flagIndex = flagIndices[i];
		if (StrEqual(args[flagIndex], flagName)){
			if (flagIndex < argCount - 1 && (i == flagCount - 1 || flagIndices[i+1] > flagIndex + 1)){
				return args[flagIndex + 1];
			}
		}
	}


	return nullptr;
}

int CommandLineParser::FlagIntValue(const char* flagName){
	return Atoi(FlagValue(flagName));
}

int CommandLineParser::FlagArgCount(const char* flagName){
	int flagIndex = GetFlagIndex(options, optionCount, flagName);
	if (flagIndex < 0){
		return 0;
	}

	for (int i = 0; i < flagCount; i++){
		if (StrEqual(args[flagIndices[i]], flagName)){
			int nextFlagIndex = (i == flagCount - 1) ? argCount : flagIndices[i + 1];
			return nextFlagIndex - flagIndices[i] - 1;
		}
	}

	return 0;
}

const char** CommandLineParser::FlagArgValues(const char* flagName){
	int flagIndex = GetFlagIndex(options, optionCount, flagName);
	if (flagIndex < 0){
		return nullptr;
	}

	for (int i = 0; i < flagCount; i++){
		if (StrEqual(args[flagIndices[i]], flagName)){
			if (flagIndices[i] < argCount - 1){
				return &args[flagIndices[i]+1];
			}
		}
	}

	return nullptr;
}

//const char** FlagArgValues();

#if defined(COMMANDLINE_TEST_MAIN)

int main(int argc, char** argv){

	BNS_UNUSED(argc);
	BNS_UNUSED(argv);

	{
		CommandLineParser parser;

		CommandLineOption options[] = {
			{"-f", "File", 1, 1},
			{"--expect", "Expected input", 1, 1}
		};

		parser.SetOptions(options, BNS_ARRAY_COUNT(options));

		const char* argvs[] = {"Blah_blah/blah.exe", "-f", "123", "--expect", "Lolwerd"};

		parser.InitializeFromArgcArgv(BNS_ARRAY_COUNT(argvs), argvs);

		ASSERT(parser.IsFlagPresent("-f"));
		ASSERT(parser.IsFlagPresent("--expect"));
		ASSERT(!parser.IsFlagPresent("-expect"));
		ASSERT(!parser.IsFlagPresent("Lolwerd"));

		ASSERT(parser.FlagIntValue("-f") == 123);
		ASSERT(StrEqual(parser.FlagValue("--expect"), "Lolwerd"));

		ASSERT(parser.FlagArgCount("-f") == 1);
		ASSERT(parser.FlagArgCount("--expect") == 1);
	}

	{
		CommandLineParser parser;

		CommandLineOption options[] = {
			{"-t", "tile", 1, 1},
			{"-g", "debug", 0, 0},
			{"--controllers", "controllers", 1, 4}
		};

		parser.SetOptions(options, BNS_ARRAY_COUNT(options));

		const char* argvs[] = {"Blah.exe", "-t", "234", "-g", "--controllers", "12", "23", "34"};

		parser.InitializeFromArgcArgv(BNS_ARRAY_COUNT(argvs), argvs);

		ASSERT(parser.IsFlagPresent("-t"));
		ASSERT(parser.IsFlagPresent("-g"));
		ASSERT(parser.IsFlagPresent("--controllers"));

		ASSERT(parser.FlagArgCount("-t") == 1);
		ASSERT(parser.FlagArgCount("-g") == 0);
		ASSERT(parser.FlagArgCount("--controllers") == 3);
		ASSERT(StrEqual(parser.FlagArgValues("--controllers")[0], "12"));
		ASSERT(StrEqual(parser.FlagArgValues("--controllers")[1], "23"));
	}

	return 0;
}




#endif



