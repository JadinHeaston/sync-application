#pragma once
//This file holds functions that handle the interactions with the configuration file.

void change_key(json& object, const std::string& old_key, const std::string& new_key);

void addToConfigurationFile(std::string pathToConfig, json givenArguments, std::string configurationName)
{
	if (configurationName == "")
	{
		std::cout << "No configuration name given." << std::endl;
		//Give option to specify configuration name or terminate program. *****
		std::cout << "Please resolve the issue and try again. Program terminating." << std::endl;
		system("PAUSE");
		exit(1);
	}
	
	json configurationFileJSON; //Holds the full JSON configuration from the file.
	size_t largestNumericID = NULL;
	std::vector<std::string> foundConfigIDs;

	//Open configuration file of reading.
	std::ifstream configFileReading(std::filesystem::u8path(pathToConfig));

	//Check that the file got opened properly. *****
	if (!configFileReading.good())
	{
		std::cout << "Configuration file failed to open: \"" << pathToConfig << "\"" << std::endl;
		//Maybe add a "Would you like to continue without adding this configuration to the file", or "Please specify a new path". *****
		std::cout << "Please resolve the issue and try again. Program terminating." << std::endl;
		system("PAUSE");
		exit(1);
	}

	//Fail is the ifstream can't be parsed.
	if (!json::accept(configFileReading))
	{
		//Error with JSON syntax. Notifying user.
		std::cout << "The given configuration file at: \"" << pathToConfig << "\"" << std::endl;
		std::cout << "Check your configuration to ensure it is valid JSON." << std::endl;
		//Maybe add a "Would you like to continue without adding this configuration to the file", or "Please specify a new path". *****
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

	//Iterating through all top level objects. These are the IDs of the configuration.
	for (json::iterator JSONiterator = configurationFileJSON.begin(); JSONiterator != configurationFileJSON.end(); ++JSONiterator)
	    foundConfigIDs.push_back(JSONiterator.key()); //Store all IDs found.

	//Check for duplicate keys. Error out if any are found.
	for (size_t iterator = 0; iterator < foundConfigIDs.size(); ++iterator)
	{
		if (configurationName == foundConfigIDs.at(iterator))
		{
			std::cout << "A duplicate top-level object key was found in the configuration file.";
			std::cout << "Please resolve the issue and try again. Program terminating." << std::endl;
		    system("PAUSE");
		    exit(1);
		}
	}

	change_key(givenArguments, "internalObject", configurationName); //Change the top level key to be the configuration ID.
	
	configurationFileJSON[configurationName] = givenArguments[configurationName]; //Appending internal configuration to the file version.

	//Outputting full altered configuration to configuration file.
	std::ofstream configFileWriting(std::filesystem::u8path(pathToConfig), std::ios::out | std::ios::binary);
	writeToFile(configFileWriting, configurationFileJSON.dump(5)); //Write the serialized json to the file. "5" indicates the indenting amount.
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

void readFromConfigurationFile(std::string pathToConfig, json givenArguments, std::string configurationName)
{
	if (configurationName == "")
	{
		std::cout << "No configuration name given." << std::endl;
		//Give option to specify configuration name or terminate program. *****
		std::cout << "Please resolve the issue and try again. Program terminating." << std::endl;
		system("PAUSE");
		exit(1);
	}

	json configurationFileJSON; //Holds the full JSON configuration from the file.
	std::vector<std::string> foundConfigIDs;

	//Open configuration file of reading.
	std::ifstream configFileReading(std::filesystem::u8path(pathToConfig));

	//Check that the file got opened properly. *****
	if (!configFileReading.good())
	{
		std::cout << "Configuration file failed to open: \"" << pathToConfig << "\"" << std::endl;
		//Maybe add a "Would you like to continue without adding this configuration to the file", or "Please specify a new path". *****
		std::cout << "Please resolve the issue and try again. Program terminating." << std::endl;
		system("PAUSE");
		exit(1);
	}

	//Fail is the ifstream can't be parsed.
	if (!json::accept(configFileReading))
	{
		//Error with JSON syntax. Notifying user.
		std::cout << "The given configuration file at: \"" << pathToConfig << "\"" << std::endl;
		std::cout << "Check your configuration to ensure it is valid JSON." << std::endl;
		//Maybe add a "Would you like to continue without adding this configuration to the file", or "Please specify a new path". *****
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
		//Maybe offer to delete all configurations using this name? *****
		system("PAUSE");
		exit(1);
	}

	change_key(configurationFileJSON, configurationName, "internalObject"); //Change the top level object key to be the internal version needed.

	givenArguments["internalObject"] = configurationFileJSON["internalObject"]; //Replacing the internal JSON object with the changed file version.
}
