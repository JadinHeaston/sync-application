#pragma once
//This file contains all debugging related functions.
//This means the function that writes to the debug file and the function that writes content to the console.

//Writes to the the debug file.
void writeToDebug(std::chrono::system_clock::time_point givenTime, bool writeTime, std::wstring textToWrite)
{
    //Check if we are actually writing to the debug file.
    if (!argumentVariables["internalObject"]["Verbose Debugging"])
        return;

    //open the current debug file.
    std::ifstream temporaryDebugHandle(debugFilePath + debugFileName, std::ios::in | std::ios::binary | std::ios::ate); //Start the cursor at the very end.
    size_t fileSize = temporaryDebugHandle.tellg(); //Get the file size in bytes.
    temporaryDebugHandle.close(); //Close temporary handle reading 

    if (fileSize >= 102857600) //100 MB
    {
        //Change the file name.
        if (debugFileName.find(L"."))
            debugFileName = debugFileName.insert(debugFileName.find(L"."), L" - " + std::to_wstring(debugFileCount));
        else
            debugFileName = debugFileName.append(L" - " + std::to_wstring(debugFileCount));
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

        std::wstring timeValue = stringToWString(temporaryString); //Converting string to wstring.

        timeValue = timeValue + L": "; //Appending "delimiter".

        //Add the current time of writing.
        writeUnicodeToFile(verboseDebugOutput, timeValue + textToWrite + newLine); //Write this to the debug file.
    }
    else
    {
        writeUnicodeToFile(verboseDebugOutput, textToWrite + newLine); //Write this to the debug file.
    }

    verboseDebugOutput.close(); //Close file.
}

void displayConsoleMessage(std::wstring givenMessage)
{
    
    if (!argumentVariables["internalObject"]["Show Console"])
        return;

    std::lock_guard<std::mutex> coutLock(coutMutex);
    std::wcout << givenMessage << std::endl;
}

//This section ignores whether the console is shown or not.
void showWarningMessage()
{
    //Displaying warning information.
    std::wcout << L"-------------------------------------------------- WARNING --------------------------------------------------" << std::endl;
    if (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"echo")
    {
        std::wcout << L"The \"ECHO\" operation will take place. This will cause the second directory to look *IDENTICAL* to the first." << std::endl;
        std::wcout << L"When a file is found within both directories, the program will compare the size and last modified time. When a difference is found, the first directories file will overwrite the seconds. (Hashes are only used when files, size, and modification time are all the same and the --check-contents argument is provided.)" << std::endl;
        std::wcout << L"This option is best used when you are backing up data and want a second directory to match the directory you make changes in." << std::endl;
        std::wcout << L"Deletions and file overwrites are possible within the second directory." << std::endl;
        std::wcout << L"First Directory: " << firstGivenDirectoryPath << std::endl;
        std::wcout << L"Second Directory: " << secondGivenDirectoryPath << std::endl;
    }
    else if (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"synchronize" || argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"sync")
    {
        std::wcout << L"The \"SYNCHRONIZE\" operation will take place. This will cause the first and second directories to look identical to each other, with the newest files being kept." << std::endl;
        std::wcout << L"When a file is found within both directories, the one with the newest modification time is used and copied to replace the older version." << std::endl;
        std::wcout << L"This option is best used when changes can be made within both directories and you want them both to be synced with the newest versions from both." << std::endl;
        std::wcout << L"Deletions and file overwrites are possible within both directories." << std::endl;
        std::wcout << L"First Directory: " << firstGivenDirectoryPath << std::endl;
        std::wcout << L"Second Directory: " << secondGivenDirectoryPath << std::endl;
    }
    else if (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"contribute" || argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"cont")
    {
        std::wcout << L"The \"CONTRIBUTE\" operation will take place. This will cause the first directory to contribute any new files or changes to the second directory." << std::endl;
        std::wcout << L"When a file is found within both directories, the program will compare the size and last modified time. When a difference is found, the first directories file will overwrite the second directories file. If the file is not present within the second directory, it is copied over to it. (Hashes are only used when files, size, and modification time are all the same and the --check-contents argument is provided.)" << std::endl;
        std::wcout << "This option is best used when you are regularly archiving files and want to keep everything." << std::endl;
        std::wcout << L"No deletions are ever made. File overwrites are possible within the second directory." << std::endl;
        std::wcout << L"First Directory: " << firstGivenDirectoryPath << std::endl;
        std::wcout << L"Second Directory: " << secondGivenDirectoryPath << std::endl;
    }

    std::wcout << std::endl;
    std::wcout << L"(This warning can be disabled by adding the \"--no-warning\" argument)" << std::endl;
    std::wcout << L"File operations are permenant (ESPECIALLY DELETIONS). Do you wish to proceed, knowing what files are potentially at risk? (Y/N)" << std::endl;
    
    std::cin >> userInput[0]; //Awaiting user input...

    //Verify if the user is okay with continuing.
    if (toupper(userInput[0]) != 'Y') //The input is NOT a "Y".
    {
        std::cout << "Terminating Program. No changes have been made." << std::endl;
        exit(0);
        return;
    }
    else //The user gave the okay. Continue.
        system("CLS");  std::cout << "Permission to continue granted. Moving forward with program.\n" << std::endl;
}