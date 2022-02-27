#pragma once
//This file holds functions that handle the interactions with the configuration file.

void addToConfigurationFile(std::wstring pathToConfig, json givenArguments, std::wstring configurationID = L"")
{
    json configurationFileJSON;
    size_t largestNumericID = NULL;
    //Check what configurations IDs exist in the configuration file.
        //Throw a BIG error if a duplicate ID is found.
    //Open configuration file of reading.
    std::ifstream configFileReading(pathToConfig, std::ios::in | std::ios::binary);
    //Read the file.
    configFileReading >> configurationFileJSON;
    
    //Iterating through all top level objects. These are the IDs of the configuration.
        //If a single object is found. Just use that.

    //Check if an ID was given. If not, add 1 to the largestNumericID found and use that.
    if (configurationID == L"")
    {
        if (largestNumericID != NULL)
            configurationID = std::to_wstring(largestNumericID++);
        else
            configurationID = L"0";
    }
    else
    {
        //Check that ID is not a duplicate.

        //If it is, offer three options:
        //Overwrite existing ID.
        //Use a generated ID.
        //Cancel program.
    }

    
    //Output to configuration file.
    std::ofstream configFileWriting(pathToConfig, std::ios::out | std::ios::binary);
    writeUnicodeToFile(configFileWriting, stringToWString(givenArguments.dump(5))); //Write the serialized json to the file. "5" indicates the indenting amount.
}

void readFromConfigurationFile(std::wstring pathToConfig, json givenArguments, std::wstring configurationID)
{

}