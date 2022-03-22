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

	if (fileSize >= 102857600) //100 MB
	{
		//Change the file name.
		if (debugFileName.find("."))
			debugFileName = debugFileName.insert(debugFileName.find("."), " - " + debugFileCount);
		else
			debugFileName = debugFileName.append(" - " + debugFileCount);
		++debugFileCount; //Incrememnt debug file count.
	}

	verboseDebugOutput.open(debugFilePath + debugFileName, std::ios::out | std::ios::binary | std::ios::app); //Open the debug file for writing.

	//Get the size.
	//If that size is larger than 
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
	
	std::cin >> userInput[0]; //Awaiting user input...

	//Verify if the user is okay with continuing.
	if (toupper(userInput[0]) != 'Y') //The input is NOT a "Y".
	{
		std::cout << "Terminating Program. No changes have been made." << std::endl;
		exit(0);
		return;
	}

	system("CLS");
	std::cout << "Permission to continue granted. Moving forward with program.\n" << std::endl;
}

void displayHelpMessage()
{
	//Display help message.
	std::cout << "The three required arguments are: --directory-one <DIRECTORY_PATH>' as the source, --directory-two <DIRECTORY_PATH>' as the destination, and '--operation-mode <OPERATION_MODE>' to specifiy the operation mode." << std::endl;
	std::cout << "The operation mode can either be 'contribute' that only copies files from directory one to directory two, 'echo' that makes directory two look like directory one." << std::endl;
	std::cout << "Detailed help can be found by using '--help' or utilizing the readme.md file: https://github.com/JadinHeaston/sync-application" << std::endl;
	system("PAUSE");
	writeDebugThreadPool.wait_for_tasks();
	return;
}

//Handles all incoming arguments.
void handleArguments(int& argc, char* argv[])
{
	//Default arguments that don't need stored in a configuration.
	bool useConfigurationFile = false;
	bool showHelpMessage = false;
	std::string configurationName;
	bool addToConfigFile = false; //Recieved from argument --add-to-config

	//Reading args
	if (argc == 1) //No arguments provided. Notify. Close program.
	{
		std::cout << "No arguments provided.\nUse the \"-h\" or \"--help\" switch to show the available options.\n" << std::endl;
		system("PAUSE");
		writeDebugThreadPool.wait_for_tasks();
		exit(1);
	}

	//Iterate through arguments for a help message, or a use configuration message.
	for (size_t i = 0; i < argc; i++) // Cycle through all arguments.
	{
		if (strcmp(argv[1], "--help") == 0) //Checking second argument for if it is "-h" or "-help".
			showHelpMessage = true;
		else if (strcmp(argv[i], "--use-config") == 0) //Use an external configuration
		{
			pathToConfigFile = formatFilePath(argv[i + 1]); //Getting provided configuration file location.
			useConfigurationFile = true;
		}
		else if ((strcmp(argv[i], "--configuration-name") == 0)) //Gets the provided configuration name.
			configurationName = argv[i + 1];
		else if (strncmp(argv[i], "-", 1) == 0) //Check for single dash.
		{
			for (size_t iterator = 1; iterator < sizeof(argv[i]); ++iterator) //Iterating through all characters, after the slash. (Starting at 1 to skip the initial dash)
				singleCharArguments[tolower(argv[i][iterator])] = 1; //Ensuring keys are lowercase for easy use later.

			if (singleCharArguments['h']) //Short help message.
				showHelpMessage = true;
		}
	}

	//Check results of previous iteration.
	if (showHelpMessage)
	{
		displayHelpMessage();
		exit(0);
	}
	else if (useConfigurationFile)
		readFromConfigurationFile(pathToConfigFile, argumentVariables, configurationName);
	else //There is no help message or configuration file being used.
	{
		singleCharArguments.clear(); //Clearing single character arguments for future use.

		//Defining default arguments.
		argumentVariables["internalObject"]["Check File Contents"] = false; ////Recieved from arg: --check-content | Defaults to false.
		argumentVariables["internalObject"]["Output Files"] = false; //Recieved from arg: --output-files | Defaults to false.
		argumentVariables["internalObject"]["Show Console"] = true; //Recieved from arg: --hide-console | defaults to false | Defines whether things are output to the console or not.

		argumentVariables["internalObject"]["Directory One"]["Recursive Search"] = true; //Recieved from arg: --no-recursive | defaults to true.
		argumentVariables["internalObject"]["Directory Two"]["Recursive Search"] = true; //Recieved from arg: --no-recursive | defaults to true.

		argumentVariables["internalObject"]["Verbose Debugging"] = false; //Defines if verbose debugging is enabled.
		argumentVariables["internalObject"]["Windows Max Path Bypass"] = false; //Determines whether "\\?\" is prepended to path and backslashes are used as directory separators.
		argumentVariables["internalObject"]["Show Warning"] = true; //Recieved from arg: --no-warning | defaults to true | Defines whether things are output to the console or not.
		argumentVariables["internalObject"]["Operation Mode"] = ""; //Holds operation mode to perform.


		//Verifying that no \ escaped " exist in the path string.
		for (size_t i = 0; i < argc; i++)
		{
			std::size_t found = std::string(argv[i]).find("\"");
			if (found != std::string::npos)
			{
				std::cout << "ERROR: Rogue quote found. Likely due to a \"\\\" placed before a double quote (\"). Please double check your input and try again." << std::endl;
				system("PAUSE");
				writeDebugThreadPool.wait_for_tasks();
				exit(1);
			}
		}

		for (size_t i = 0; i < argc; i++) // Cycle through all arguments.
		{
			//std::cout << argv[i] << " : " << strncmp(argv[i], "--", 2) << std::endl;

			//Check if the argument contains a single or double slash
			if (strncmp(argv[i], "--", 2) == 0) //Check for double slash
			{
				if ((strcmp(argv[i], "--add-to-config") == 0)) //Enable file output.
				{
					addToConfigFile = true;
					pathToConfigFile = formatFilePath(argv[i + 1]);
				}
				else if ((strcmp(argv[i], "--check-content") == 0) || (strncmp(argv[i], "--check-contents", 32) == 0)) //Enable file hashing.
					argumentVariables["internalObject"]["Check File Contents"] = true; //Set hashing to true.
				else if (strcmp(argv[i], "--directory-one") == 0) //Directory one path switch.
				{
					firstGivenDirectoryPath = formatFilePath(argv[i + 1]);

					if (firstGivenDirectoryPath.back() == '\\' || firstGivenDirectoryPath.back() == '/')
						firstGivenDirectoryPath.pop_back(); //Remove trailing slash.

					if (!std::filesystem::is_directory(firstGivenDirectoryPath)) //Verify path is real and valid.
					{
						if (std::filesystem::exists(firstGivenDirectoryPath))
						{
							std::cout << "The '--directory-one' path provided is NOT a directory, and a directory can not be created. Please try again. (" << firstGivenDirectoryPath << ")" << std::endl;
							system("PAUSE");
							writeDebugThreadPool.wait_for_tasks();
							exit(1);
						}

						std::cout << "The '--directory-one' path provided was NOT found. (" << firstGivenDirectoryPath << ")" << std::endl;
						std::cout << "Would you like to create this directory?" << std::endl;

						std::cin >> userInput[0]; //Awaiting user input...

						//Verify if the user is okay with continuing.
						if (toupper(userInput[0]) != 'Y') //The input is NOT a "Y".
							std::filesystem::create_directories(firstGivenDirectoryPath); //Creating directory.
						else //The user gave the okay. Continue.
						{
							std::cout << "No directory created. Program ending." << std::endl;
							system("PAUSE");
							writeDebugThreadPool.wait_for_tasks();
							exit(1);
						}
					}
					//Getting absolute path.
					firstGivenDirectoryPath = formatFilePath(std::filesystem::absolute(firstGivenDirectoryPath).string());

					//Double check that there is no slash.
					//This was added because running a drive letter ( such as "D:") through absolute adds a slash..
					if (firstGivenDirectoryPath.back() == '/' || firstGivenDirectoryPath.back() == '\\')
						firstGivenDirectoryPath.pop_back(); //Remove the slash.

					//Adding path to JSON.
					argumentVariables["internalObject"]["Directory One"]["Directory Path"] = firstGivenDirectoryPath;
				}
				else if (strncmp(argv[i], "--directory-two", 32) == 0) //Destination two path switch.
				{
					secondGivenDirectoryPath = formatFilePath(argv[i + 1]);

					if (secondGivenDirectoryPath.back() == '\\' || secondGivenDirectoryPath.back() == '/')
						secondGivenDirectoryPath.pop_back(); //Remove trailing slash

					if (!std::filesystem::is_directory(secondGivenDirectoryPath)) //Verify path is real and valid.
					{
						if (std::filesystem::exists(firstGivenDirectoryPath))
						{
							std::cout << "The '--directory-one' path provided is NOT a directory, and a directory can not be created. Please try again. (" << firstGivenDirectoryPath << ")" << std::endl;
							system("PAUSE");
							writeDebugThreadPool.wait_for_tasks();
							exit(1);
						}

						std::cout << "The '--directory-two' path provided was NOT found. (" << secondGivenDirectoryPath << ")" << std::endl;
						std::cout << "Would you like to create this directory?" << std::endl;

						std::cin >> userInput[0]; //Awaiting user input...

						//Verify if the user is okay with continuing.
						if (toupper(userInput[0]) == 'Y') //The input is a "Y".
							std::filesystem::create_directories(secondGivenDirectoryPath); //Creating directory.
						else //The does not want to create a directory. Ending program.
						{
							std::cout << "No directory created. Program ending." << std::endl;
							system("PAUSE");
							writeDebugThreadPool.wait_for_tasks();
							exit(1);
						}
					}
					//Getting absolute path.
					secondGivenDirectoryPath = formatFilePath(std::filesystem::absolute(secondGivenDirectoryPath).string());

					//Double check that there is no slash.
					//This was added because running a drive letter ( such as "D:") through absolute adds a slash..
					if (secondGivenDirectoryPath.back() == '/' || secondGivenDirectoryPath.back() == '\\')
						secondGivenDirectoryPath.pop_back(); //Remove the slash.


					//Adding path to JSON.
					argumentVariables["internalObject"]["Directory Two"]["Directory Path"] = secondGivenDirectoryPath;
				}
				else if (strncmp(argv[i], "--hide-console", 32) == 0) //Defines if anything is output to the console.
					argumentVariables["internalObject"]["Show Console"] = false;
				else if (strcmp(argv[i], "--no-recursive-one") == 0) //Disable recursive operation for directory one.
					argumentVariables["internalObject"]["Directory One"]["Recursive Search"] = false;
				else if (strcmp(argv[i], "--no-recursive-two") == 0) //Disable recursive operation for directory one.
					argumentVariables["internalObject"]["Directory Two"]["Recursive Search"] = false;
				else if (strcmp(argv[i], "--no-warning") == 0) //Disable deletion warning.
					argumentVariables["internalObject"]["Show Warning"] = false;
				else if (strcmp(argv[i], "--operation-mode") == 0) //Operation mode switch.
					argumentVariables["internalObject"]["Operation Mode"] = argv[i + 1];
				else if ((strcmp(argv[i], "--output-files") == 0)) //Enable file output.
					argumentVariables["internalObject"]["Output Files"] = true;
				else if (strcmp(argv[i], "--output-verbose-debug") == 0) //Output debug file in running directory.
				{
					argumentVariables["internalObject"]["Verbose Debugging"] = true; //Set global verbose debug variable to true.

					argumentVariables["internalObject"]["Debug File Path"] = formatFilePath(std::string(argv[i + 1])); //Get next argument.
					debugFilePath = formatFilePath(argv[i + 1]);
					if (debugFilePath.find("/")) //Search for a slash to determine if the given text is a full path or a name. If a slash is found, it is a path.
					{
						//Checking that a file name exists. Continuing with default name appended to the given path if it doesn't.
						if (debugFilePath.substr(debugFilePath.find_last_of("/") + 1, std::string::npos) != "")
						{
							debugFileName = debugFilePath.substr(debugFilePath.find_last_of("/") + 1, std::string::npos);
							debugFilePath = debugFilePath.substr(0, debugFilePath.find_last_of("/") + 1); //Remove filename from path.
						}
					}
					else if (debugFilePath.find("\\"))
					{
						//Checking that a file name exists. Continuing with default name appended to the given path if it doesn't.
						if (debugFilePath.substr(debugFilePath.find_last_of("\\") + 1, std::string::npos) != "")
						{
							debugFileName = debugFilePath.substr(debugFilePath.find_last_of("\\") + 1, std::string::npos);
							debugFilePath = debugFilePath.substr(0, debugFilePath.find_last_of("\\") + 1); //Remove filename from path.
						}
					}
					else //If there is no slash, then a name was given.
					{
						debugFileName = debugFilePath; //Set the given item to be the name.
						debugFilePath = ""; //Set the path to nothing. The name will be appended to this and cause the file to be created in the same location as the running application.
					}

					verboseDebugOutput.open(debugFilePath + debugFileName, std::ios::out | std::ios::binary | std::ios::app); //Open the file.
					if (!verboseDebugOutput.is_open())
					{
						std::cout << "Debug file path not usable: " + debugFilePath + debugFileName << std::endl;
						system("PAUSE");
						writeDebugThreadPool.wait_for_tasks();
						exit(1);
					}
					verboseDebugOutput.close();
				}
			}
			else if (strncmp(argv[i], "-", 1) == 0) //Check for single dash.
			{
				for (size_t iterator = 1; iterator < sizeof(argv[i]); ++iterator) //Iterating through all characters, after the slash. (Starting at 1 to skip the initial dash)
					singleCharArguments[tolower(argv[i][iterator])] = 1; //Ensuring keys are lowercase for easy use later.
			}

			//std::cout << argv[i] << std::endl; //*** Display all arguments given.
		}

		//Iterating through argument array and applying arguments.
		for (size_t iterator = 0; iterator < sizeof(singleCharArguments); ++iterator)
		{
			if (singleCharArguments['l']) //Windows Max Path Bypass
				argumentVariables["internalObject"]["Windows Max Path Bypass"] = true;
		}


		//Checking if backslashes need to be used internally to support the max path bypass (UNC)
		if (argumentVariables["internalObject"]["Windows Max Path Bypass"].get<bool>())
			directorySeparator = "\\"; //Set directory separator appropriately.
	}

	//MAX_PATH bypass.
	//Also ensuring that path is an absolute path.
	if (argumentVariables["internalObject"]["Windows Max Path Bypass"].get<bool>())
	{
		//Checking if the prefix already exists.
		firstGivenDirectoryPath = formatFilePath("\\\\?\\" + std::filesystem::absolute(firstGivenDirectoryPath).string());
		secondGivenDirectoryPath = formatFilePath("\\\\?\\" + std::filesystem::absolute(secondGivenDirectoryPath).string());
		
		//Double check that there is no slash.
		//This was added because running a drive letter ( such as "D:") through absolute adds a slash..
		if (firstGivenDirectoryPath.back() == '/' || firstGivenDirectoryPath.back() == '\\')
			firstGivenDirectoryPath.pop_back(); //Remove the slash.
		if (secondGivenDirectoryPath.back() == '/' || secondGivenDirectoryPath.back() == '\\')
			secondGivenDirectoryPath.pop_back(); //Remove the slash.
	}
		
	//Add arguments to a configuration file, if needed.
	if (addToConfigFile)
		addToConfigurationFile(pathToConfigFile, argumentVariables, configurationName);
}