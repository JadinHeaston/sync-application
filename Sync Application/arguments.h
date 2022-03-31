#pragma once

bool cleanConfig = false;

//Handles all incoming arguments.
void handleArguments(int& argc, char* argv[])
{
	//Default arguments that don't need stored in a configuration.
	bool useConfigurationFile = false;
	bool showHelpMessage = false;
	std::string configurationName; // --configuration-name
	std::string pathToConfigFile = ""; //Received from argument --add-to-config

	//Holds an array of single letter arguments that need to be applied.
	std::unordered_map<char, size_t> singleCharArguments;

	//Reading args
	if (argc == 1) //No arguments provided. Notify. Close program.
	{
		std::cout << "No arguments provided.\nUse the \"-h\" or \"--help\" switch to show the available options.\n" << std::endl;
		system("PAUSE");
		writeDebugThreadPool.wait_for_tasks();
		exit(1);
	}
	else
	{
		//Verifying that no backslash (\) escaped a quotation mark (").
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
	}

	//Iterate through arguments for a help message, or a use configuration message.
	for (size_t i = 0; i < argc; i++) // Cycle through all arguments.
	{
		if (strcmp(argv[1], "--help") == 0) //Checking second argument for if it is "-h" or "-help".
			displayHelpMessage(true, argc, argv);
		else if (strcmp(argv[i], "--use-config") == 0) //Use an external configuration
		{
			checkArgumentValue(i, argc, argv);
			useConfigurationFile = true;
			pathToConfigFile = formatFilePath(argv[i + 1]); //Getting provided configuration file location.
		}
		else if ((strcmp(argv[i], "--configuration-name") == 0)) //Gets the provided configuration name.
		{
			checkArgumentValue(i, argc, argv);
			configurationName = argv[i + 1];
		}
		else if (strncmp(argv[i], "-", 1) == 0) //Check for single dash.
		{
			for (size_t iterator = 1; iterator < sizeof(argv[i]); ++iterator) //Iterating through all characters, after the slash. (Starting at 1 to skip the initial dash)
				singleCharArguments[tolower(argv[i][iterator])] = 1; //Ensuring keys are lowercase for easy use later.

			if (singleCharArguments['h']) //Short help message.
				displayHelpMessage(false, argc, argv);
		}
	}

	//Determining whether we are getting arguments from a configuration file.
	if (useConfigurationFile)
		readFromConfigurationFile(pathToConfigFile, argumentVariables, configurationName);

	//Setting default arguments, if they are not provided:
	if (argumentVariables["internalObject"]["Check File Contents"].is_null())
		argumentVariables["internalObject"]["Check File Contents"] = false; ////Received from arg: --check-content | Defaults to false.
	if (argumentVariables["internalObject"]["Debug File Path"].is_null())
		argumentVariables["internalObject"]["Debug File Path"] = "";
	if (argumentVariables["internalObject"]["Show Console"].is_null())
		argumentVariables["internalObject"]["Show Console"] = true; //Received from arg: --hide-console | defaults to false | Defines whether things are output to the console or not.
	if (argumentVariables["internalObject"]["Directory One"]["Directory Path"].is_null())
		argumentVariables["internalObject"]["Directory One"]["Directory Path"] = "";
	if (argumentVariables["internalObject"]["Directory Two"]["Directory Path"].is_null())
		argumentVariables["internalObject"]["Directory Two"]["Directory Path"] = "";
	if (argumentVariables["internalObject"]["Directory One"]["Recursive Search"].is_null())
		argumentVariables["internalObject"]["Directory One"]["Recursive Search"] = true; //Received from arg: --no-recursive | defaults to true.
	if (argumentVariables["internalObject"]["Directory Two"]["Recursive Search"].is_null())
		argumentVariables["internalObject"]["Directory Two"]["Recursive Search"] = true; //Received from arg: --no-recursive | defaults to true.
	if (argumentVariables["internalObject"]["Verbose Debugging"].is_null())
		argumentVariables["internalObject"]["Verbose Debugging"] = false; //Defines if verbose debugging is enabled.
	if (argumentVariables["internalObject"]["Windows Max Path Bypass"].is_null())
		argumentVariables["internalObject"]["Windows Max Path Bypass"] = false; //Determines whether "\\?\" is prepended to path and backslashes are used as directory separators.
	if (argumentVariables["internalObject"]["Show Warning"].is_null())
		argumentVariables["internalObject"]["Show Warning"] = true; //Received from arg: --no-warning | defaults to true | Defines whether things are output to the console or not.
	if (argumentVariables["internalObject"]["Output Files"].is_null())
		argumentVariables["internalObject"]["Output Files"] = ""; //Received from arg: --output-files | Defaults to false.
	if (argumentVariables["internalObject"]["Operation Mode"].is_null())
		argumentVariables["internalObject"]["Operation Mode"] = ""; //Holds operation mode to perform.
	if (argumentVariables["internalObject"]["Windows Max Path Bypass"].is_null())
		argumentVariables["internalObject"]["Windows Max Path Bypass"] = false;

	//Reading for additional arguments.
	readArguments(argc, argv, pathToConfigFile);

	//Processing the arguments, regardless of where they came from.
	processArguments(argc, argv, pathToConfigFile);

	//Add arguments to a configuration file, if needed.
	if (pathToConfigFile != "")
		addToConfigurationFile(pathToConfigFile, argumentVariables, configurationName);

	return;
}

void readArguments(int& argc, char* argv[], std::string& pathToConfigFile)
{
	pathToConfigFile = "";

	//Holds an array of single letter arguments that need to be applied.
	std::unordered_map<char, bool> singleCharArguments;

	for (size_t i = 0; i < argc; i++) // Cycle through all arguments.
	{
		//Check if the argument contains a single or double slash
		if (strncmp(argv[i], "--", 2) == 0) //Check for double slash
		{
			if ((strcmp(argv[i], "--add-to-config") == 0)) //Enable file output.
			{
				checkArgumentValue(i, argc, argv);
				pathToConfigFile = argv[i + 1];
			}
			else if ((strcmp(argv[i], "--check-content") == 0) || (strncmp(argv[i], "--check-contents", 32) == 0)) //Enable file hashing.
				argumentVariables["internalObject"]["Check File Contents"] = true; //Set hashing to true.
			else if ((strcmp(argv[i], "--clean-config") == 0)) //Removes unused properties.
			{
				checkArgumentValue(i, argc, argv);
				cleanConfig = true;
				pathToConfigFile = argv[i + 1];
			}
			else if (strcmp(argv[i], "--directory-one") == 0) //Directory one path switch.
			{
				checkArgumentValue(i, argc, argv);
				argumentVariables["internalObject"]["Directory One"]["Directory Path"] = argv[i + 1];
			}
			else if (strncmp(argv[i], "--directory-two", 32) == 0) //Destination two path switch.
			{
				checkArgumentValue(i, argc, argv);
				argumentVariables["internalObject"]["Directory Two"]["Directory Path"] = argv[i + 1];
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
			{
				checkArgumentValue(i, argc, argv);
				argumentVariables["internalObject"]["Operation Mode"] = argv[i + 1];
			}
			else if ((strcmp(argv[i], "--output-files") == 0)) //Enable file output.
			{
				argumentVariables["internalObject"]["Output Files"] = ""; //Setting it to true by default, regardless of if a path is provided after.
				
				if (!checkArgumentValue(i, argc, argv, true)) //Seeing if next argument is potentially a path.
					argumentVariables["internalObject"]["Output Files"] = argv[i + 1]; //Adding path.
			}
			else if (strcmp(argv[i], "--output-verbose-debug") == 0) //Output debug file in running directory.
			{
				checkArgumentValue(i, argc, argv);
				argumentVariables["internalObject"]["Verbose Debugging"] = true; //Set global verbose debug variable to true.
				argumentVariables["internalObject"]["Debug File Path"] = argv[i + 1]; //Get next argument.
			}
		}
		else if (strncmp(argv[i], "-", 1) == 0) //Check for single dash.
		{
			for (size_t iterator = 0; iterator < sizeof(argv[i]); ++iterator) //Iterating through all characters, after the slash. (Starting at 1 to skip the initial dash)
				singleCharArguments[tolower(argv[i][iterator])] = true; //Ensuring keys are lowercase for easy use later.
		}

		//std::cout << argv[i] << std::endl; //*** Display all arguments given.
	}

	//Iterating through argument array and applying arguments.
	for (size_t iterator = 0; iterator < sizeof(singleCharArguments); ++iterator)
	{
		if (singleCharArguments['l']) //Windows Max Path Bypass
			argumentVariables["internalObject"]["Windows Max Path Bypass"] = true;
	}
}

void processArguments(int& argc, char* argv[], std::string& pathToConfigFile)
{
	//MAX_PATH bypass. Setting directory separator.
	if (argumentVariables["internalObject"]["Windows Max Path Bypass"].get<bool>())
		directorySeparator = '\\'; //Set directory separator appropriately.

	//Double check that there is no ending slash.
	//This was added because running a drive letter ( such as "D:") through absolute adds a slash..
	if (argumentVariables["internalObject"]["Directory One"]["Directory Path"].back() == directorySeparator)
		argumentVariables["internalObject"]["Directory One"]["Directory Path"].get<std::string>().pop_back(); //Remove the slash.
	if (argumentVariables["internalObject"]["Directory Two"]["Directory Path"].back() == directorySeparator)
		argumentVariables["internalObject"]["Directory Two"]["Directory Path"].get<std::string>().pop_back(); //Remove the slash.

	//Specific Argument Processing

	// ----- //
	//Add to Config
	pathToConfigFile = formatFilePath(pathToConfigFile);

	// ----- //
	//Clean Config
	if (cleanConfig)
	{
		cleanConfigurationFile(pathToConfigFile, argumentVariables);
		exit(0);
	}

	// ----- //
	//Directory One
	firstGivenDirectoryPath = argumentVariables["internalObject"]["Directory One"]["Directory Path"].get<std::string>();
	firstGivenDirectoryPath = formatFilePath(firstGivenDirectoryPath);

	if (firstGivenDirectoryPath.back() == directorySeparator)
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

		std::cin >> userInput; //Awaiting user input...

		//Verify if the user is okay with continuing.
		if (toupper(userInput) != 'Y') //The input is NOT a "Y".
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
	if (firstGivenDirectoryPath.back() == directorySeparator)
		firstGivenDirectoryPath.pop_back(); //Remove the slash.

	//Adding path to JSON.
	argumentVariables["internalObject"]["Directory One"]["Directory Path"] = firstGivenDirectoryPath;

	// ----- //
	//Directory TWo
	secondGivenDirectoryPath = argumentVariables["internalObject"]["Directory Two"]["Directory Path"].get<std::string>();
	secondGivenDirectoryPath = formatFilePath(secondGivenDirectoryPath);

	if (secondGivenDirectoryPath.back() == directorySeparator)
		secondGivenDirectoryPath.pop_back(); //Remove trailing slash

	if (!std::filesystem::is_directory(secondGivenDirectoryPath)) //Verify path is real and valid.
	{
		if (std::filesystem::exists(firstGivenDirectoryPath))
		{
			std::cout << "The '--directory-two' path provided is NOT a directory, and a directory can not be created. Please try again. (" << secondGivenDirectoryPath << ")" << std::endl;
			system("PAUSE");
			writeDebugThreadPool.wait_for_tasks();
			exit(1);
		}

		std::cout << "The '--directory-two' path provided was NOT found. (" << secondGivenDirectoryPath << ")" << std::endl;
		std::cout << "Would you like to create this directory?" << std::endl;

		std::cin >> userInput; //Awaiting user input...

		//Verify if the user is okay with continuing.
		if (toupper(userInput) == 'Y') //The input is a "Y".
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
	if (secondGivenDirectoryPath.back() == directorySeparator)
		secondGivenDirectoryPath.pop_back(); //Remove the slash.

	//Adding path to JSON.
	argumentVariables["internalObject"]["Directory Two"]["Directory Path"] = secondGivenDirectoryPath;


	//MAX_PATH bypass. Adding prefixes.
	//Also ensuring that path is an absolute path.
	if (argumentVariables["internalObject"]["Windows Max Path Bypass"].get<bool>())
	{
		//Checking if the prefix already exists for both paths. Adding it, if it is missing.
		if (argumentVariables["internalObject"]["Directory One"]["Directory Path"].get<std::string>().find("\\\\?\\") == std::string::npos)
		{
			firstGivenDirectoryPath = formatFilePath("\\\\?\\" + std::filesystem::absolute(argumentVariables["internalObject"]["Directory One"]["Directory Path"].get<std::string>()).string());

			if (firstGivenDirectoryPath.back() == directorySeparator)
				firstGivenDirectoryPath.pop_back(); //Remove the slash.

			argumentVariables["internalObject"]["Directory One"]["Directory Path"] = firstGivenDirectoryPath;
		}
		
		if (argumentVariables["internalObject"]["Directory Two"]["Directory Path"].get<std::string>().find("\\\\?\\") == std::string::npos)
		{
			secondGivenDirectoryPath = formatFilePath("\\\\?\\" + std::filesystem::absolute(argumentVariables["internalObject"]["Directory Two"]["Directory Path"].get<std::string>()).string());

			if (secondGivenDirectoryPath.back() == directorySeparator)
				secondGivenDirectoryPath.pop_back(); //Remove the slash.

			argumentVariables["internalObject"]["Directory Two"]["Directory Path"] = secondGivenDirectoryPath;
		}
	}
	else
	{
		//Checking if the prefix already exists for both paths. Removing it if it does.
		if (argumentVariables["internalObject"]["Directory One"]["Directory Path"].get<std::string>().find("\\\\?\\") != -1) //Removing prefix.
		{
			firstGivenDirectoryPath = formatFilePath(std::filesystem::absolute(argumentVariables["internalObject"]["Directory One"]["Directory Path"].get<std::string>().erase(argumentVariables["internalObject"]["Directory One"]["Directory Path"].get<std::string>().find("\\\\?\\"), std::string("\\\\?\\").length())).string());

			if (firstGivenDirectoryPath.back() == directorySeparator)
				firstGivenDirectoryPath.pop_back(); //Remove the slash.
			
			argumentVariables["internalObject"]["Directory One"]["Directory Path"] = firstGivenDirectoryPath;
		}
		
		if (argumentVariables["internalObject"]["Directory Two"]["Directory Path"].get<std::string>().find("\\\\?\\") != -1) //Removing prefix.
		{
			secondGivenDirectoryPath = formatFilePath(std::filesystem::absolute(argumentVariables["internalObject"]["Directory Two"]["Directory Path"].get<std::string>().erase(argumentVariables["internalObject"]["Directory Two"]["Directory Path"].get<std::string>().find("\\\\?\\"), std::string("\\\\?\\").length())).string());
			
			if (secondGivenDirectoryPath.back() == directorySeparator)
				secondGivenDirectoryPath.pop_back(); //Remove the slash.

			argumentVariables["internalObject"]["Directory Two"]["Directory Path"] = secondGivenDirectoryPath;
		}
	}

	//Checking that the two given paths do not overlap. That one is not within another.
	if (firstGivenDirectoryPath.find(secondGivenDirectoryPath + directorySeparator) != std::string::npos)
	{
		std::cout << "The given directories are nested within eachother." << std::endl;
		std::cout << firstGivenDirectoryPath << std::endl;
		std::cout << secondGivenDirectoryPath << std::endl;
		std::cout << "Please fix this and try again." << std::endl;
		system("PAUSE");
		writeDebugThreadPool.wait_for_tasks();
		exit(1);
	}
	else if (secondGivenDirectoryPath.find(firstGivenDirectoryPath + directorySeparator) != std::string::npos)
	{
		std::cout << "The given directories are nested within eachother." << std::endl;
		std::cout << firstGivenDirectoryPath << std::endl;
		std::cout << secondGivenDirectoryPath << std::endl;
		std::cout << "Please fix this and try again." << std::endl;
		system("PAUSE");
		writeDebugThreadPool.wait_for_tasks();
		exit(1);
	}

	// ----- //
	//Debug Files
	debugFilePath = formatFilePath(argumentVariables["internalObject"]["Debug File Path"]);
	if (debugFilePath.find(directorySeparator)) //Search for a slash to determine if the given text is a full path or a name. If a slash is found, it is a path.
	{
		//Checking that a file name exists. Continuing with default name appended to the given path if it doesn't.
		if (debugFilePath.substr(debugFilePath.find_last_of(directorySeparator) + 1, std::string::npos) != "")
		{
			debugFileName = debugFilePath.substr(debugFilePath.find_last_of("/") + 1, std::string::npos);
			debugFilePath = debugFilePath.substr(0, debugFilePath.find_last_of("/") + 1); //Remove filename from path.
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


	// ----- //
	//Output Files
	argumentVariables["internalObject"]["Output Files"] = formatFilePath(argumentVariables["internalObject"]["Output Files"]);
}

bool checkArgumentValue(size_t& position, int& argc, char* argv[], bool failSafe)
{
	if (position + 1 <= argc && argv[position + 1][0] == '-')
	{
		if (failSafe)
			return true;
		std::cout << "Error parsing arguments: " << argv[position] << " found another argument following it (" << argv[position + 1] << ")" << std::endl;
		system("PAUSE");
		exit(1);
	}

	return false;
}