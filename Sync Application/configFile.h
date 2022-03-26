#pragma once
//This file holds functions that handle the interactions with the configuration file.

void addToConfigurationFile(std::string pathToConfig, json& givenArguments, std::string configurationName)
{
	if (configurationName == "")
	{
		std::cout << "No configuration name given." << std::endl;

		//Asking user to provide a configuration name or terminate the program.
		std::cout << "Please enter a configuration name. (Leave this blank to terminate the program with no changes.)" << std::endl;
		std::getline(std::cin, configurationName); //Getting user input.

		//User didn't provide an input. Terminating program.
		if (configurationName == "")
			exit(1);
	}
	
	json configurationFileJSON; //Holds the full JSON configuration from the file.
	size_t largestNumericID = NULL;
	std::vector<std::string> foundConfigIDs;

	//Open configuration file for reading.
	std::ifstream configFileReading(std::filesystem::u8path(pathToConfig));

	//Check that the file got opened properly.
	if (!configFileReading.good())
	{
		std::cout << "Configuration file failed to open: \"" << pathToConfig << "\"" << std::endl;
		std::cout << "Please specify a new path to a configuration file to use, or enter nothing to terminate the program." << std::endl;
		std::getline(std::cin, pathToConfig); //Getting user input.
		if (pathToConfig == "")
			exit(1);

		//Trying to open configuration file for reading again.
		std::ifstream configFileReading(std::filesystem::u8path(pathToConfig));

		if (!configFileReading.good())
		{
			std::cout << "File failed to open again: " << std::endl;
			std::cout << "Please try again. Program terminating." << std::endl;
			exit(1);
		}
	}

	//Fail is the ifstream can't be parsed.
	if (!json::accept(configFileReading))
	{
		//Error with JSON syntax. Notifying user.
		std::cout << "JSON syntax error: \"" << pathToConfig << "\"" << std::endl;
		std::cout << "Check your configuration to ensure it is valid JSON." << std::endl;
		std::cout << "Please resolve the issue and try again. Program terminating." << std::endl;
		system("PAUSE");
		exit(1);
	}
	else
	{
		//Resetting file seek head for the next parsing.
		configFileReading.clear();
		configFileReading.seekg(0);
	}

	configurationFileJSON = json::parse(configFileReading); //Parsing JSON from the file.

	configFileReading.close(); //Closing input file.

	if (configurationFileJSON.count(configurationName) > 0)
	{
		if (configurationFileJSON.count(configurationName) == 1) //A configuration already exists. Ask user if they want to update/overwrite it.
		{
			std::cout << "A duplicate configuration named \"" << configurationName << "\" key was found in the configuration file." << std::endl;
			std::cout << "Would you like to overwrite the configuration? (Y/N)" << std::endl;
			std::cin >> userInput; //Awaiting user input...

			if (toupper(userInput) != 'Y') //The input is not "Y". The user did not say yes.
			{
				std::cout << "Would you like to terminate the program? (Y/N)" << std::endl;
				std::cin >> userInput; //Awaiting user input...

				if (toupper(userInput) == 'Y') //The input is not "Y". The user did not say yes.
					exit(0);
			}
		}
		else
		{
			std::cout << "Duplicate top-level configurations named \"" << configurationName << "\" were found in the configuration file.";
			std::cout << "Please resolve the issue and try again. Program terminating." << std::endl;
			system("PAUSE");
			exit(1);
		}
	}

	configurationFileJSON["internalObject"] = givenArguments["internalObject"]; //Appending internal configuration to the file version.

	change_key(configurationFileJSON, "internalObject", configurationName); //Change the newly added top level internalObject key to be the configuration ID.
	
	//Outputting full altered configuration to configuration file.
	std::ofstream configFileWriting(std::filesystem::u8path(pathToConfig), std::ios::out | std::ios::binary);
	writeToFile(configFileWriting, configurationFileJSON.dump(4)); //Write the serialized json to the file. "5" indicates the indenting amount.
	configFileWriting.close();
}

void change_key(json& object, const std::string& old_key, const std::string& new_key)
{
	//Get iterator to old key; TODO: error handling if key is not present
	json::iterator it = object.find(old_key);
	//Create null value for new key and swap value from old key
	std::swap(object[new_key], it.value());
	//Delete value at old key (cheap, because the value is null after swap)
	object.erase(it);
}

void readFromConfigurationFile(std::string pathToConfig, json& givenArguments, std::string configurationName)
{
	if (configurationName == "") //No configuration name was given.
	{
		std::cout << "No configuration name given." << std::endl;
		
		//Asking user to provide a configuration name or terminate the program.
		std::cout << "Please enter a configuration name. (Leave this blank to terminate the program with no changes.)" << std::endl;
		std::string userInput;

		std::getline(std::cin, configurationName); //Getting user input.

		//User didn't provide an input. Terminating program.
		if (configurationName == "")
			exit(1);
	}

	json configurationFileJSON; //Holds the full JSON configuration from the file.
	std::vector<std::string> foundConfigIDs;

	//Open configuration file for reading.
	std::ifstream configFileReading(std::filesystem::u8path(pathToConfig));

	//Check that the file got opened properly.
	if (!configFileReading.good())
	{
		std::cout << "Configuration file failed to open: \"" << pathToConfig << "\"" << std::endl;
		std::cout << "Please specify a new path to a configuration file to use, or enter nothing to terminate the program." << std::endl;
		std::getline(std::cin, pathToConfig); //Getting user input.
		if (pathToConfig == "")
			exit(1);

		//Trying to open configuration file for reading again.
		std::ifstream configFileReading(std::filesystem::u8path(pathToConfig));

		if (!configFileReading.good())
		{
			std::cout << "File failed to open again: " << std::endl;
			std::cout << "Please try again. Program terminating." << std::endl;
			exit(1);
		}
	}

	//Fail is the ifstream can't be parsed.
	if (!json::accept(configFileReading))
	{
		//Error with JSON syntax. Notifying user.
		std::cout << "JSON syntax error: \"" << pathToConfig << "\"" << std::endl;
		std::cout << "Check your configuration to ensure it is valid JSON." << std::endl;
		std::cout << "Please resolve the issue and try again. Program terminating." << std::endl;
		system("PAUSE");
		exit(1);
	}
	else
	{
		//Resetting file seek head for the next parsing.
		configFileReading.clear();
		configFileReading.seekg(0);
	}

	configurationFileJSON = json::parse(configFileReading); //Parsing JSON from the file.

	configFileReading.close(); //Closing input file.

	//No configuration exists.
	if (configurationFileJSON.count(configurationName) == 0)
	{

		std::cout << "Configuration \"" << configurationName << "\" not found in: " << pathToConfig << std::endl;
		std::cout << "You can use --add-to-config <PATH_TO_CONFIG> to add a configuration. Alternatively, you can manually add an entry yourself." << std::endl;
		std::cout << "Program terminating." << std::endl;
		system("PAUSE");
		exit(1);
	}

	if (configurationFileJSON.count(configurationName) > 1)
	{
		std::cout << "Multiple configurations using the name \"" << configurationName << "\" have been found in the configuration file." << std::endl;
		std::cout << "Please resolve the issue and try again. Program terminating." << std::endl;
		system("PAUSE");
		exit(1);
	}

	change_key(configurationFileJSON, configurationName, "internalObject"); //Change the top level object key to be the internal version needed.
	
	givenArguments["internalObject"] = configurationFileJSON["internalObject"]; //Replacing the internal JSON object with the changed file version.
}

std::string convertJSONtoCommand(json& givenArguments)
{
	std::string finalString = ""; //Initializing string.

	//Creating associative array of JSON values and the corresponding command.
	json JSONArguments;
	JSONArguments["Check File Contents"] = "--check-file-content"; //T|F
	JSONArguments["Directory One"]["Directory Path"] = "--directory-one";
	JSONArguments["Directory Two"]["Directory Path"] = "--directory-two";
	JSONArguments["Directory One"]["Recursive Search"] = "--no-recursive-one";
	JSONArguments["Directory Two"]["Recursive Search"] = "--no-recursive-two";
	JSONArguments["Debug File Path"] = "--output-verbose-debug";
	JSONArguments["Output Files"] = "--output-files";
	JSONArguments["Output Location"] = "--output-location";
	JSONArguments["Operation Mode"] = "--operation-mode";
	JSONArguments["Show Console"] = "--hide-console"; //T|F
	JSONArguments["Show Warning"] = "--no-warning"; //T|F
	JSONArguments["Verbose Debugging"] = "";
	JSONArguments["Windows Max Path Bypass"] = "-l";

	//Iterating through the current argument JSON.
	for (json::iterator iterator = givenArguments["internalObject"].begin(); iterator != givenArguments["internalObject"].end(); ++iterator)
	{

		//if (!givenArguments.contains(JSONArguments))
		//{
		//	std::cout << "Argument not populated!" << std::endl;
		//}


		std::cout << iterator.key() << " " << iterator.value() << std::endl;
		if (iterator.key() == "Directory One" || iterator.key() == "Directory Two")
		{
			for (json::iterator directorySpecificIterator = givenArguments["internalObject"][iterator.key()].begin(); directorySpecificIterator != givenArguments["internalObject"][iterator.key()].end(); ++directorySpecificIterator)
			{
				if (givenArguments["internalObject"][iterator.key()][directorySpecificIterator.key()].is_boolean())
				{
					finalString.append(" " + JSONArguments[iterator.key()][directorySpecificIterator.key()].get<std::string>()); //Get the argument.
				}
				else
				{
					finalString.append(" " + JSONArguments[iterator.key()][directorySpecificIterator.key()].get<std::string>()); //Get the argument.
					finalString.append(" \"" + directorySpecificIterator.value().get<std::string>()); //Get the value.
					finalString.append("\"");
				}
			}
			
		}
		else if (givenArguments["internalObject"][iterator.key()].is_boolean()) //Handles boolians.
		{
			if (JSONArguments[iterator.key()].get<std::string>() != "")
			{
				//system("PAUSE");
				//std::cout << iterator.key() << std::endl;
				//std::cout << JSONArguments[iterator.key()] << std::endl;
				//std::cout << JSONArguments[iterator.key()].get<std::string>() << std::endl;

				finalString.append(" " + JSONArguments[iterator.key()].get<std::string>()); //Get the argument.
				//system("PAUSE");
			}
		}
		else //Handles all strings.
		{
			if (JSONArguments[iterator.key()].get<std::string>() != "")
			{
				finalString.append(" " + JSONArguments[iterator.key()].get<std::string>()); //Get the argument.
				finalString.append(" \"" + iterator.value().get<std::string>()); //Get the value.
				finalString.append("\"");
			}
		}
	}
	
	finalString = finalString.substr(1, std::string::npos); //Removing leading space.

	return finalString;
}