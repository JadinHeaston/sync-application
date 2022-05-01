#pragma once
//This file contains all debugging related functions.
//This means the function that writes to the debug file and the function that writes content to the console.

//Writes to the the debug file.
void writeToDebug(std::chrono::system_clock::time_point givenTime, bool writeTime, std::string textToWrite)
{
	//Check if we are actually writing to the debug file.
	if (!argumentVariables["internalObject"]["Verbose Debugging"].get<bool>())
		return;

	//open the current debug file.
	std::ifstream temporaryDebugHandle(debugFilePath + debugFileName, std::ios::in | std::ios::binary | std::ios::ate); //Start the cursor at the very end.
	size_t fileSize = temporaryDebugHandle.tellg(); //Get the file size in bytes.
	temporaryDebugHandle.close(); //Close temporary handle reading 

	//Check if the file is at it's threshhold.
	if (fileSize >= 102857600) //100 MB
	{
		//Change the file name.
		if (debugFileName.find("."))
			debugFileName = debugFileName.insert(debugFileName.find("."), " - " + debugFileCount);
		else
			debugFileName = debugFileName.append(" - " + debugFileCount);
		++debugFileCount; //Increment debug file count.
	}

	verboseDebugOutput.open(debugFilePath + debugFileName, std::ios::out | std::ios::binary | std::ios::app); //Open the debug file for writing.

	if (writeTime) //Check if we are writing time.
	{
		char buff[20]; //Create buffer.

		std::time_t newTimeValue = std::chrono::system_clock::to_time_t(givenTime); //Convert given time to time_t.

		strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&newTimeValue)); //Format time. Place it into buffer.

		std::string temporaryString = buff; //Put the buffer into the string;

		std::string timeValue = temporaryString; //Storing time value.

		timeValue = timeValue + ": "; //Appending "delimiter".

		//Add the current time of writing.
		writeToFile(verboseDebugOutput, timeValue + textToWrite + newLine); //Write this to the debug file.
	}
	else
		writeToFile(verboseDebugOutput, textToWrite + newLine); //Write this to the debug file.

	verboseDebugOutput.close(); //Close file.
}

//Writes the message to the console, if the console is being shown/used.
void displayConsoleMessage(std::string givenMessage)
{
	if (!argumentVariables["internalObject"]["Show Console"])
		return;

	std::lock_guard<std::mutex> coutLock(coutMutex);
	std::cout << givenMessage << std::endl;
}

//This section ignores whether the console is shown or not.
void showWarningMessage()
{
	//Displaying warning information.
	std::cout << "-------------------------------------------------- WARNING --------------------------------------------------" << std::endl;
	if (argumentVariables["internalObject"]["Operation Mode"].get<std::string>() == "contribute" || argumentVariables["internalObject"]["Operation Mode"].get<std::string>() == "cont")
	{
		std::cout << "The \"CONTRIBUTE\" operation will take place. This will cause the first directory to contribute any new files or changes to the second directory." << std::endl;
		std::cout << "When a file is found within both directories, the program will compare the size and last modified time. When a difference is found, the first directories file will overwrite the second directories file. If the file is not present within the second directory, it is copied over to it. (Hashes are only used when files, size, and modification time are all the same and the --check-contents argument is provided.)" << std::endl;
		std::cout << "This option is best used when you are regularly archiving files and want to keep everything." << std::endl;
		std::cout << "No deletions are ever made. File overwrites are possible within the second directory." << std::endl;
		std::cout << "First Directory: " << firstGivenDirectoryPath << std::endl;
		std::cout << "Second Directory: " << secondGivenDirectoryPath << std::endl;
	}
	else if (argumentVariables["internalObject"]["Operation Mode"].get<std::string>() == "echo")
	{
		std::cout << "The \"ECHO\" operation will take place. This will cause the second directory to look *IDENTICAL* to the first." << std::endl;
		std::cout << "When a file is found within both directories, the program will compare the size and last modified time. When a difference is found, the first directories file will overwrite the seconds. (Hashes are only used when files, size, and modification time are all the same and the --check-contents argument is provided.)" << std::endl;
		std::cout << "This option is best used when you are backing up data and want a second directory to match the directory you make changes in." << std::endl;
		std::cout << "Deletions and file overwrites are possible within the second directory." << std::endl;
		std::cout << "First Directory: " << firstGivenDirectoryPath << std::endl;
		std::cout << "Second Directory: " << secondGivenDirectoryPath << std::endl;
	}

	std::cout << std::endl;
	std::cout << "(This warning can be disabled by adding the \"--no-warning\" argument)" << std::endl;
	std::cout << "File operations are permenant (ESPECIALLY DELETIONS). Do you wish to proceed, knowing what files are potentially at risk? (Y/N)" << std::endl;
	
	std::cin >> userInput; //Awaiting user input...

	//Verify if the user is okay with continuing.
	if (toupper(userInput) != 'Y') //The input is NOT a "Y".
	{
		std::cout << "Terminating Program. No changes have been made." << std::endl;
		exit(0);
		return;
	}

	system("CLS");
	std::cout << "Permission to continue granted. Moving forward with program.\n" << std::endl;
}

void displayHelpMessage(bool longHelpMessage, int& argc, char* argv[])
{
	//Long message provides the help text.
	//Use the link below to maintain formatting easily.
	//https://onlinestringtools.com/escape-string

	//Short message will look at what arguments have been provided and provide their section from the full list.

	if (longHelpMessage) 
		std::cout << "--add-to-config <PATH_TO_CONFIG_FILE>\n\tThis requires a fully functional set of arguments, as well as a --name <NAME> to identify the configuration.\n\tThis should also likely be used with the \"--configuration-name\" argument to avoid having gross numbered configurations.\n\t\n--check-content(s)\n\n--clean-config <PATH_TO_CONFIG_FILE>\n\tNote: Please create a backup your configuration prior to doing this.\n\tRemoves all properties that are not legitimate. This can be useful to do when using a newer version of this software as \n\n--configuration-name <NAME>\n\tThis is only necessary when using the \"--add-to-configuration\" argument.\n\tProvides a configuration ID that is used for using this configuration.\n\tIf a configuration name is not provided, then a number ID will be assigned one higher than the largest number ID found. \n\n--directory-one <DIRECTORY_PATH>\n\n--directory-two <DIRECTORY_PATH>\n\n-h <ARGUMENT>\n\tDisplays a short help message about the argument provided.\n\n--help\n\tDisplays extended help. This will likely end up being this document section.\n\n--hide-console\n\tHides console.\n\n-l\n\tBypasses Windows MAX_PATH limit of 260 characters. It appends \"\\\\?\\\", which requires utilizing backslashes for directory separators in the backend. (Thanks, Windows!)\n\n--no-recursive-one/two\n\tDefines where a recursive process should NOT be used.\n\tAdd either \"one\" or \"two\" at the end, to correspond with which directory shouldn\'t be recursive.\n\tYou can either use \"--no-recursive-one\" or \"--no-recursive-two\".\n\n--no-warning\n\tDisables warning that explains the operation and outlines what files are potentially at risk. This should probably only be used when automating the run process.\n\n--operation-mode <OPERATION_MODE>\n\tOperation Mode is defined as a string. The available options being \"contribute\" or \"cont\", and \"echo\".\n\n--output-files\n\tDumps all internal database vectors to .log files in the same directory as the application.\n\n--output-location <PATH>\n\tDetermines where output files are put.\n\n--output-verbose-debug <OUTPUT_LOCATION>\n\tOutputs log as the program runs to assist with debugging. If a log is present, new data is appended to prevent debugging data loss.\n\n--use-config <PATH_TO_CONFIG_FILE>\n\tIf multiple configurations are within the same file, a --configuration-name <NAME> MUST be specified for the program to know which configuration to use." << std::endl;
	else
	{
		//std::unordered_map<std::string, std::string> argumentHelp = 
		//{
		//	{}
		//};

		std::cout << "--add-to-config <PATH_TO_CONFIG_FILE>\n\tThis requires a fully functional set of arguments, as well as a --name <NAME> to identify the configuration.\n\tThis should also likely be used with the \"--configuration-name\" argument to avoid having gross numbered configurations.\n\t\n--check-content(s)\n\n--clean-config <PATH_TO_CONFIG_FILE>\n\tNote: Please create a backup your configuration prior to doing this.\n\tRemoves all properties that are not legitimate. This can be useful to do when using a newer version of this software as \n\n--configuration-name <NAME>\n\tThis is only necessary when using the \"--add-to-configuration\" argument.\n\tProvides a configuration ID that is used for using this configuration.\n\tIf a configuration name is not provided, then a number ID will be assigned one higher than the largest number ID found. \n\n--directory-one <DIRECTORY_PATH>\n\n--directory-two <DIRECTORY_PATH>\n\n-h <ARGUMENT>\n\tDisplays a short help message about the argument provided.\n\n--help\n\tDisplays extended help. This will likely end up being this document section.\n\n--hide-console\n\tHides console.\n\n-l\n\tBypasses Windows MAX_PATH limit of 260 characters. It appends \"\\\\?\\\", which requires utilizing backslashes for directory separators in the backend. (Thanks, Windows!)\n\n--no-recursive-one/two\n\tDefines where a recursive process should NOT be used.\n\tAdd either \"one\" or \"two\" at the end, to correspond with which directory shouldn\'t be recursive.\n\tYou can either use \"--no-recursive-one\" or \"--no-recursive-two\".\n\n--no-warning\n\tDisables warning that explains the operation and outlines what files are potentially at risk. This should probably only be used when automating the run process.\n\n--operation-mode <OPERATION_MODE>\n\tOperation Mode is defined as a string. The available options being \"contribute\" or \"cont\", and \"echo\".\n\n--output-files\n\tDumps all internal database vectors to .log files in the same directory as the application.\n\n--output-location <PATH>\n\tDetermines where output files are put.\n\n--output-verbose-debug <OUTPUT_LOCATION>\n\tOutputs log as the program runs to assist with debugging. If a log is present, new data is appended to prevent debugging data loss.\n\n--use-config <PATH_TO_CONFIG_FILE>\n\tIf multiple configurations are within the same file, a --configuration-name <NAME> MUST be specified for the program to know which configuration to use." << std::endl;
		//for (size_t i = 0; i < argc; i++) // Cycle through all arguments.
		//{
		//	if (argumentHelp[argv[i]])
		//}
	}

	std::cout << "You can also visit https://github.com/JadinHeaston/sync-application/ for more information." << std::endl;

	writeDebugThreadPool.wait_for_tasks();
	system("PAUSE");
	exit(0);
}
