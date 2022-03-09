#pragma once
//This file holds functions that handle the interactions with the configuration file.

void change_key(json& object, const std::string& old_key, const std::string& new_key);
std::string filetostring(std::ifstream& givenFile);

void addToConfigurationFile(std::wstring pathToConfig, json givenArguments, std::string configurationName)
{
    if (configurationName == "")
    {
        std::cout << "No configuration name given." << std::endl;
        system("PAUSE");
        exit(0);
    }

    json configurationFileJSON;
    size_t largestNumericID = NULL;
    std::vector<std::string> foundConfigIDs;
    //Open configuration file of reading.
    std::wifstream configFileReading(pathToConfig);
    //Reading each line and storing it in a single string.
    std::wstring currentLine;
    std::wstring finalString;

    finalString = readUnicodeFile(configFileReading);

    //std::wstring tempString = LR"()";
    configurationFileJSON = finalString;
    system("PAUSE");
    //if (!json::accept(configFileReading))
    //{
    //    std::cout << "NOT ACCEPTED" << std::endl;
    //}
    //else
    //{
    //    std::cout << "ACCEPTED" << std::endl;
    //}


    std::wstring test = L"ここにいﾋ (I'm Here) ft. rionos (Stephen Walking Remix)";
    std::wstring failFilePath = L"TEST.log";
    std::wofstream failFile(failFilePath, std::ios::out);
    writeUnicodeToFile(failFile, test);
    failFile.close();
    //writeUnicodeToFile(failFile, stringToWString(configurationFileJSON.dump(5)));
    system("PAUSE");
    exit(1);


    //std::cout << finalString << std::endl;
    //configurationFileJSON = configurationFileJSON.dump();
    //std::cout << givenArguments.dump(5) << std::endl;
    //system("PAUSE");
    //std::cout << configurationFileJSON.dump(5) << std::endl;
    //system("PAUSE");
    //change_key(configurationFileJSON, "cum", configurationName); //Change the top level key to be the configuration ID.
    //writeUnicodeToFile(failFile, stringToWString(givenArguments.dump(5)));
    //writeUnicodeToFile(failFile, stringToWString(configurationFileJSON.dump(5)));
    //
    //std::cout << configurationFileJSON["cum"] << std::endl;

    //Iterating through all top level objects. These are the IDs of the configuration.
    //for (json::iterator JSONiterator = configurationFileJSON.begin(); JSONiterator != configurationFileJSON.end(); ++JSONiterator)
    //{
    //    //configurationFileJSON = std::to_string(JSONiterator.value());
    //    std::wcout << configurationFileJSON.get<std::wstring>() << std::endl;
    //    system("PAUSE");
    //    //foundConfigIDs.push_back(JSONiterator.key()); //Store all IDs found.
    //}
    //std::cout << foundConfigIDs[0] << std::endl;
    std::cout << "YO" << std::endl;
    system("PAUSE");
    exit(1);
















    //for (size_t iterator = 0; iterator < foundConfigIDs.size(); ++iterator)
    //{
        //if (configurationName == foundConfigIDs.at(iterator))
        //{
        //    displayConsoleMessage(L"DUPLICATE CONFIGURATION ID FOUND.");
        //    system("PAUSE");
        //    exit(0);
        //}
    //}
    //std::cout << givenArguments.dump(5) << std::endl;
    //system("PAUSE");
    //change_key(givenArguments, "internalObject", configurationName); //Change the top level key to be the configuration ID.
    //change_key(configurationFileJSON, "cum", "newcum");
    //system("PAUSE");

    //Output to configuration file.
    //std::ofstream configFileWriting(pathToConfig, std::ios::out | std::ios::app | std::ios::binary);
    //writeUnicodeToFile(configFileWriting, stringToWString(givenArguments.dump(5))); //Write the serialized json to the file. "5" indicates the indenting amount.
}

void change_key(json& object, const std::string& old_key, const std::string& new_key)
{
    // get iterator to old key; TODO: error handling if key is not present
    json::iterator it = object.find(old_key);
    // create null value for new key and swap value from old key
    std::swap(object[new_key], it.value());
    // delete value at old key (cheap, because the value is null after swap)
    object.erase(it);
}

bool is_empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

void readFromConfigurationFile(std::wstring pathToConfig, json givenArguments, std::wstring configurationName)
{
    change_key(givenArguments, wstringToString(configurationName), "internalObject"); //Change the top level key to be the internal version needed.
}

std::string filetostring(std::ifstream& givenFile) 
{
    std::string fileStr;

    std::istreambuf_iterator<char> inputIt(givenFile), emptyInputIt;
    std::back_insert_iterator<std::string> stringInsert(fileStr);

    std::copy(inputIt, emptyInputIt, stringInsert);

    return fileStr;
}