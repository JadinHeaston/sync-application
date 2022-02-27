//This application is meant to do what SyncToy does, but better for myself.
//Initially, this will be a console (command line) driven utility. It could maybe get UI elements later(?)

//Three asterisks *** Indicate something done for debugging only. These will likely be removed later, or moved to output to some kind of debug file.
//Five asterisks are generally used to bring attention to an area. Maybe to indicate that something is currently being worked on, or is troublesome.
#include <Windows.h> //Allows me to show/hide the console window.



//LIBRARIES
#include <iostream> //Yeah.
#include <boost/filesystem.hpp> //Used for getting file date metadata (date modified and date created). Struggled to switch to the built in C++ filesystem for this.
#include <fstream> //File input/output.
#include <string> //Strings!
#include <sstream> //Stringstream
#include <openssl/md5.h> //Used for MD5 hashing. Pretty good!
#include <filesystem> //Great for ease and function overloading things that allow wide character sets compared to Boost.
#include <chrono> //Time tracking.
#include <vector> //vector.
#include <unordered_map> //Map.
#include "thread_pool.hpp" //Thread pool stuff.

#include "curl/curl.h"

#include "json.hpp"

std::mutex coutMutex; //Lock for wcout to prevent errors display console messages.

using json = nlohmann::json; //Setting json namespace to a simgler term.
json argumentVariables;


std::wstring directorySeparator = L"/";
std::wstring pathToConfigFile;

char userInput[1]; //Holds user input, when needed.

//Sets global delimiter used for reading and writing DB files. Tilde typically works well. (CONSIDER USING MULTIPLE CHARACTER DELIMITER FOR SAFETY)
std::wstring delimitingCharacter = L"▼";
//Simple newline dude.
std::wstring newLine = L"\n";
//Buffer size for hashing
const size_t hashBufferSize = 4096;

//Creating threadpools.
thread_pool threadPool(std::thread::hardware_concurrency()); //"Default" pool

thread_pool hashingThreadPool(std::thread::hardware_concurrency());

thread_pool fileOperationThreadPool(std::thread::hardware_concurrency()); //Specific to 

thread_pool writeDebugThreadPool(1); //Dedicated to writing to the debug file.
thread_pool writeConsoleMessagesPool(1);

//Holds provided directory given from args. "Super Parents"
std::wstring firstGivenDirectoryPath;
std::wstring secondGivenDirectoryPath;

//Verbose debugging variables
std::ofstream verboseDebugOutput; //Hold potential future file handle if verbose debugging is enabled.
std::wstring debugFilePath = L"";
std::wstring debugFileName = L"debug.log";
size_t debugFileCount = 1;

//Holds an array of single letter arguments that need to be applied.
std::unordered_map<char, size_t> singleCharArguments;


//FUNCTION PROTOTYPES
size_t nthOccurrence(std::wstring& givenString, std::wstring delimitingCharacter, size_t nth); //Provides character location of nthOccurrence of a given character in a given string.
std::wstring formatFilePath(std::wstring givenFile, std::wstring givenDirectorySeparator = L""); //Used to change \\ to /
void sortVector(std::vector<std::wstring>& givenVectorDB); //Created to allow multithreading. Simple std::sort on databases. Originally (and currently) only used for directory DB's.
void performFileOpActionFile(std::vector<std::wstring>& fileOpAction); //Goes through File Operation Actions vector. Interprets data and assigns task to thread pool.
void performHashActionFile(std::vector<std::wstring>& hashActions, std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::wstring firstGivenPath, std::wstring secondGivenPath); //
void compareHashes(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath); //
void echoCompareDirectories(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& hashActions, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath);
void synchronizeCompareDirectories(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& hashActions, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath);
void contributeCompareDirectories(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& hashActions, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath);

//Internal header files:
#include "wideStrings.h"
#include "configFile.h"
#include "debugging.h"
#include "hashing.h"
#include "syncOperations.h"
#include "fileOperations.h"

int main(int argc, char* argv[])
{
    ::ShowWindow(::GetConsoleWindow(), SW_SHOW); //Hiding console immediately.

    std::chrono::time_point start = std::chrono::steady_clock::now(); //START TIMER.


    //Default arguments that don't need stored in a configuration.
    bool useConfigurationFile = false;
    bool showHelpMessage = false;


    //Iterate through arguments for a help message, or a use configuration message.
    //Check results of previous iteration.
    if (showHelpMessage)
    {

    }
    else if (useConfigurationFile)
    {

    }
    else //There is no help message or configuration file being used.
    {
        //Defining default arguments.
        argumentVariables["internalObject"]["Check File Contents"] = false; ////Recieved from arg: --check-content | Defaults to false.
        argumentVariables["internalObject"]["Output Files"] = false; //Recieved from arg: --output-files | Defaults to false.
        argumentVariables["internalObject"]["Show Console"] = true; //Recieved from arg: --hide-console | defaults to false | Defines whether things are output to the console or not.
        argumentVariables["internalObject"]["Recursive Search"] = true; //Recieved from arg: --no-recursive | defaults to true.
        argumentVariables["internalObject"]["Verbose Debugging"] = false; //Defines if verbose debugging is enabled.
        argumentVariables["internalObject"]["Windows Max Path Bypass"] = false; //Determines whether "\\?\" is prepended to path and backslashes are used as directory separators.
        argumentVariables["internalObject"]["Show Warning"] = true; //Recieved from arg: --no-warning | defaults to true | Defines whether things are output to the console or not.
        argumentVariables["internalObject"]["Operation Mode"] = L""; //Holds operation mode to perform.


        //Default arguments that don't need stored in a configuration.
        //bool dataLossProtection = false; //Recieved from arg: --data-protection | defaults to false;
        bool addToConfigFile = false; //Recieved from arugment --add-to-config

        //Verifying that no \ escaped " exist in the path string.
        for (size_t i = 0; i < argc; i++)
        {
            std::size_t found = std::string(argv[i]).find("\"");
            if (found != std::string::npos)
            {
                std::cout << "ERROR: Rogue quote found. Likely due to a \"\\\" placed before a double quote (\"). Please double check your input and try again." << std::endl;
                system("PAUSE");
                return 0;
            }
        }
        //Reading args
        if (argc == 1) //No arguments provided. Notify. Close program.
        {
            std::cout << "No arguments provided.\nUse the \"-h\" or \"--help\" switch to show the available options.\n" << std::endl;
            system("PAUSE");
            return 0;
        }

        for (size_t i = 0; i < argc; i++) // Cycle through all arguments.
        {
            //std::cout << argv[i] << " : " << strncmp(argv[i], "--", 2) << std::endl;

            //Check if the argument contains a single or double slash
            if (strncmp(argv[i], "--", 2) == 0) //Check for double slash
            {
                if (strcmp(argv[1], "--help") == 0) //Checking second argument for if it is "-h" or "-help".
                {
                    //Display help
                    std::cout << "Defaults:" << std::endl;
                    std::cout << "--check-content - F | --output-files - F | --output-verbose-debug <FILEPATH> - NULL | --no-recursive - T" << std::endl;
                    std::cout << "HELP PROVIDED. GET FUCKED" << std::endl;

                    system("PAUSE");
                    return 0;
                }

                if ((strcmp(argv[i], "--check-content") == 0) || (strncmp(argv[i], "--check-contents", 32) == 0)) //Enable file hashing.
                    argumentVariables["internalObject"]["Check File Contents"] = true; //Set hashing to true.
                if (strcmp(argv[i], "--directory-one") == 0) //Directory one path switch.
                {
                    firstGivenDirectoryPath = formatFilePath(charToWString(argv[i + 1]));

                    if (firstGivenDirectoryPath.back() == L'\\' || firstGivenDirectoryPath.back() == L'/')
                        firstGivenDirectoryPath.pop_back(); //Remove trailing slash.

                    if (!std::filesystem::is_directory(firstGivenDirectoryPath)) //Verify path is real and valid.
                    {
                        if (std::filesystem::exists(firstGivenDirectoryPath))
                        {
                            std::wcout << L"The '--directory-one' path provided is NOT a directory, and a directory can not be created. Please try again. (" << firstGivenDirectoryPath << ")" << std::endl;
                            system("PAUSE");
                            return 0;
                        }

                        std::wcout << "The '--directory-one' path provided was NOT found. (" << firstGivenDirectoryPath << ")" << std::endl;
                        std::cout << "Would you like to create this directory?" << std::endl;

                        std::cin >> userInput[0]; //Awaiting user input...

                        //Verify if the user is okay with continuing.
                        if (toupper(userInput[0]) != 'Y') //The input is NOT a "Y".
                            std::filesystem::create_directories(firstGivenDirectoryPath); //Creating directory.
                        else //The user gave the okay. Continue.
                        {
                            std::cout << "No directory created. Program ending." << std::endl;
                            system("PAUSE");
                            return 0;
                        }
                    }
                }
                else if (strncmp(argv[i], "--directory-two", 32) == 0) //Destination two path switch.
                {
                    secondGivenDirectoryPath = formatFilePath(charToWString(argv[i + 1]));

                    if (secondGivenDirectoryPath.back() == L'\\' || secondGivenDirectoryPath.back() == L'/')
                        secondGivenDirectoryPath.pop_back(); //Remove trailing slash

                    if (!std::filesystem::is_directory(secondGivenDirectoryPath)) //Verify path is real and valid.
                    {
                        if (std::filesystem::exists(firstGivenDirectoryPath))
                        {
                            std::wcout << L"The '--directory-one' path provided is NOT a directory, and a directory can not be created. Please try again. (" << firstGivenDirectoryPath << ")" << std::endl;
                            system("PAUSE");
                            return 0;
                        }

                        std::wcout << "The '--directory-two' path provided was NOT found. (" << secondGivenDirectoryPath << ")" << std::endl;
                        std::cout << "Would you like to create this directory?" << std::endl;

                        std::cin >> userInput[0]; //Awaiting user input...

                        //Verify if the user is okay with continuing.
                        if (toupper(userInput[0]) == 'Y') //The input is a "Y".
                            std::filesystem::create_directories(secondGivenDirectoryPath); //Creating directory.
                        else //The user gave the okay. Continue.
                        {
                            std::cout << "No directory created. Program ending." << std::endl;
                            system("PAUSE");
                            return 0;
                        }
                    }
                }
                else if (strncmp(argv[i], "--hide-console", 32) == 0) //Defines if anything is output to the console.
                    argumentVariables["internalObject"]["Show Console"] = false;
                else if (strcmp(argv[i], "--no-recursive") == 0) //Disable recursive operation.
                    argumentVariables["internalObject"]["No Recursive"] = false;
                else if (strcmp(argv[i], "--no-warning") == 0) //Disable deletion warning.
                    argumentVariables["internalObject"]["Show Warning"] = false;
                else if (strcmp(argv[i], "--operation-mode") == 0) //Operation mode switch.
                    argumentVariables["internalObject"]["Operation Mode"] = charToWString(argv[i + 1]);
                else if ((strcmp(argv[i], "--output-files") == 0)) //Enable file output.
                    argumentVariables["internalObject"]["Output Files"] = true;
                else if ((strcmp(argv[i], "--add-to-config") == 0)) //Enable file output.
                {
                    addToConfigFile = true;
                    pathToConfigFile = formatFilePath(charToWString(argv[i + 1]));
                }
                else if (strcmp(argv[i], "--output-verbose-debug") == 0) //Output debug file in running directory.
                {
                    argumentVariables["internalObject"]["Verbose Debugging"] = true; //Set global verbose debug variable to true.

                    argumentVariables["internalObject"]["Debug File Path"] = formatFilePath(charToWString(argv[i + 1])); //Get next argument.
                    debugFilePath = formatFilePath(charToWString(argv[i + 1]));
                    if (debugFilePath.find(L"/")) //Search for a slash to determine if the given text is a full path or a name. If a slash is found, it is a path.
                    {
                        //Checking that a file name exists. Continuing with default name appended to the given path if it doesn't.
                        if (debugFilePath.substr(debugFilePath.find_last_of(L"/") + 1, std::wstring::npos) != L"")
                        {
                            debugFileName = debugFilePath.substr(debugFilePath.find_last_of(L"/") + 1, std::wstring::npos);
                            debugFilePath = debugFilePath.substr(0, debugFilePath.find_last_of(L"/") + 1); //Remove filename from path.
                        }
                    }
                    else if (debugFilePath.find(L"\\"))
                    {
                        //Checking that a file name exists. Continuing with default name appended to the given path if it doesn't.
                        if (debugFilePath.substr(debugFilePath.find_last_of(L"\\") + 1, std::wstring::npos) != L"")
                        {
                            debugFileName = debugFilePath.substr(debugFilePath.find_last_of(L"\\") + 1, std::wstring::npos);
                            debugFilePath = debugFilePath.substr(0, debugFilePath.find_last_of(L"\\") + 1); //Remove filename from path.
                        }
                    }
                    else //If there is no slash, then a name was given.
                    {
                        debugFileName = debugFilePath; //Set the given item to be the name.
                        debugFilePath = L""; //Set the path to nothing. The name will be appended to this and cause the file to be created in the same location as the running application.
                    }

                    verboseDebugOutput.open(debugFilePath + debugFileName, std::ios::out | std::ios::binary | std::ios::app); //Open the file.
                    if (!verboseDebugOutput.is_open())
                    {
                        std::wcout << L"Debug file path not usable: " + debugFilePath + debugFileName << std::endl;
                        system("PAUSE");
                        return 0;
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
            //std::cout << singleCharArguments['h'] << std::endl;
            if (singleCharArguments['h']) //Short help message.
            {
                //Display help message.
                std::cout << "The three required arguments are: --directory-one <DIRECTORY_PATH>' as the source, --directory-two <DIRECTORY_PATH>' as the destination, and '--operation-mode <OPERATION_MODE>' to specifiy the operation mode." << std::endl;
                std::cout << "The operation mode can either be 'contribute' that only copies files from directory one to directory two, 'echo' that makes directory two look like directory one, or 'synchronize' that will use the newest version from either directory to keep both up to date and in sync." << std::endl;
                std::cout << "Detailed help can be found by using '--help' or utilizing the readme.md file: https://github.com/JadinHeaston/sync-application" << std::endl;
                system("PAUSE");
                return 0;
            }
            if (singleCharArguments['l']) //Windows Max Path Bypass
            {
                argumentVariables["internalObject"]["Windows Max Path Bypass"] = true; directorySeparator = L"\\";
            }
        }
        //ARGS FINISHED.



        //Add arguments to a configuration file, if needed.
        if (addToConfigFile)
            addToConfigurationFile(pathToConfigFile, argumentVariables);

    }

    //Creating vectors to hold directory maps.
    std::vector<std::wstring> directoryOneDB;
    std::vector<std::wstring> directoryTwoDB;

    //Creating action vectors.
    std::vector<std::wstring> hashActions;
    std::vector<std::wstring> fileOpActions;


    //If debugging is enabled, write the "new application line"
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), false, L"-------------------------------------------------- NEW APPLICATION INSTANCE --------------------------------------------------"); //Stating new application instance.

    //MAX_PATH bypass.
    //Also ensuring that path is an absolute path.
    if (argumentVariables["internalObject"]["Windows Max Path Bypass"])
    {
        firstGivenDirectoryPath = formatFilePath(L"\\\\?\\" + std::filesystem::absolute(firstGivenDirectoryPath).wstring());
        secondGivenDirectoryPath = formatFilePath(L"\\\\?\\" + std::filesystem::absolute(secondGivenDirectoryPath).wstring());
    }
    else
    {
        firstGivenDirectoryPath = formatFilePath(std::filesystem::absolute(firstGivenDirectoryPath).wstring());
        secondGivenDirectoryPath = formatFilePath(std::filesystem::absolute(secondGivenDirectoryPath).wstring());
    }

    //Double check that there is no slash.
    //This was added because running a drive letter ("D:") through absolute adds a slash..
    if (firstGivenDirectoryPath.back() == L'/' || firstGivenDirectoryPath.back() == L'\\')
        firstGivenDirectoryPath.pop_back(); //Remove the slash.
    if (secondGivenDirectoryPath.back() == L'/' || secondGivenDirectoryPath.back() == L'\\')
        secondGivenDirectoryPath.pop_back(); //Remove the slash.

    if (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() != L"")
    {
        //std::wstring operationMode = argumentVariables["internalObject"]["Operation Mode"];
        std::transform(argumentVariables["internalObject"]["Operation Mode"].begin(), argumentVariables["internalObject"]["Operation Mode"].end(), argumentVariables["internalObject"]["Operation Mode"].begin(), towlower); //Convert to lowercase for easy comparison.
            
        //Check that it is a legitimate value.
        if (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() != L"echo" && (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() != L"synchronize" && argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() != L"sync") && (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() != L"contribute" && argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() != L"cont"))
        {
            writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Error-----");
            std::wcout << L"Invalid Operation Mode: " << stringToWString(argumentVariables["internalObject"]["Operation Mode"]) << std::endl;
            return 0;
        }
    }
    else
    {
        std::cout << "No operation mode provided. (--operation-mode)" << std::endl;
        std::cout << "Use --help for more information." << std::endl;
        system("PAUSE");
        return 0;
    }

    //Show warning before continuing.
    if (argumentVariables["internalObject"]["Show Warning"]) showWarningMessage();

    if (!argumentVariables["internalObject"]["Show Console"]) //If the console should be displayed, then show it.
        ::ShowWindow(::GetConsoleWindow(), SW_HIDE);


    //Displaying file locations.
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"First Directory: " + firstGivenDirectoryPath);
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Second Directory: " + secondGivenDirectoryPath);

    
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), false, stringToWString(argumentVariables.dump(5)));
    
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- DIRECTORY CRAWLING -----");
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Creating directory maps...");

    //Creating initial directory map.
    threadPool.push_task(createDirectoryMapDB, std::ref(directoryOneDB), std::ref(firstGivenDirectoryPath));
    threadPool.push_task(createDirectoryMapDB, std::ref(directoryTwoDB), std::ref(secondGivenDirectoryPath));
    threadPool.wait_for_tasks();

    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* DIRECTORY CRAWLING -----");
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Directory maps created...");


    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- SORTING DIRECTORY LISTS -----");
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Sorting lists...");

    //Sorting directories. This may be changed to be a natural sorting later. (to make it more human-readable)
    threadPool.push_task(sortVector, std::ref(directoryOneDB));
    threadPool.push_task(sortVector, std::ref(directoryTwoDB));
    threadPool.wait_for_tasks();

    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* SORTING DIRECTORY LISTS -----");
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Sorting finished...");


    //FUTURE FEATURE: CHECK WHAT OPERATION IS BEING DONE.
    if (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"echo")
    {
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- DIRECTORY COMPARISON - " + argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() + L" -----");
        writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Beginning directory comparison function.");
        echoCompareDirectories(directoryOneDB, directoryTwoDB, hashActions, fileOpActions, firstGivenDirectoryPath, secondGivenDirectoryPath); //Handles a LOT of stuff. Includes the process of hashing files before ending.
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* DIRECTORY COMPARISON - " + argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() + L" -----");
        writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Directory comparing finished...");
    }
    else if (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"synchronize" || argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"sync")
    {
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- DIRECTORY COMPARISON - " + argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() + L" -----");
        writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Beginning directory comparison function for synchronization mode.");
        synchronizeCompareDirectories(directoryOneDB, directoryTwoDB, hashActions, fileOpActions, firstGivenDirectoryPath, secondGivenDirectoryPath); //Includes the process of hashing files and comparing again before ending.
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* DIRECTORY COMPARISON - " + argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() + L" -----");
        writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Directory comparing finished..."); //***
    }
    else if (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"contribute" || argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"cont")
    {
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- DIRECTORY COMPARISON - " + argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() + L" -----");
        writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Beginning directory comparison function for synchronization mode.");
        contributeCompareDirectories(directoryOneDB, directoryTwoDB, hashActions, fileOpActions, firstGivenDirectoryPath, secondGivenDirectoryPath); //Includes the process of hashing files and comparing again before ending.
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* DIRECTORY COMPARISON - " + argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() + L" -----");
        writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Directory comparing finished...");
    }
    
    //Performing file operations.
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- FILE OPERATIONS -----");
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Beginning File Operations...");
    performFileOpActionFile(fileOpActions); //Regardless of the type of operation, a file operation check should occur.
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* FILE OPERATIONS -----");
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"File Operations finished!");

    //Outputting files!
    if (argumentVariables["internalObject"]["Output Files"])
    {
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- OUTPUTING INTERNAL FILES -----");
        //Holds output of hashAction reading, allows for manipulation.
        std::wstring currentReadLine;

        writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Shifting arrays to show headers...");

        //Add headers to both directory files.
        std::wstring columnsLine = L"PATH" + delimitingCharacter + L"FILE_SIZE" + delimitingCharacter + L"DATE_MODIFIED" + delimitingCharacter + L"DATE_CREATED" + delimitingCharacter + L"MD5_HASH" + delimitingCharacter + L"MATCHED_CHECK" + delimitingCharacter + L"LINE_FOUND" + newLine;

        //Adding path information to debug file.
        directoryOneDB.push_back(L"PATH: " + firstGivenDirectoryPath + newLine);
        directoryTwoDB.push_back(L"PATH: " + secondGivenDirectoryPath + newLine);

        //Adding column headers for easy reading.
        directoryOneDB.push_back(columnsLine);
        directoryTwoDB.push_back(columnsLine);

        //Rotate all elements to the right two, to allow the header and path information to be at the top/start.
        std::rotate(directoryOneDB.rbegin(), directoryOneDB.rbegin() + 2, directoryOneDB.rend());
        std::rotate(directoryTwoDB.rbegin(), directoryTwoDB.rbegin() + 2, directoryTwoDB.rend());

        std::wstring firstDirectoryDB = L"DB1.log";
        std::wstring secondDirectoryDB = L"DB2.log";
        //If files are placed in the exe's running location, ensure that these debug files contain what file paths belong to what instead of just "DB1" and "DB2" ***

        //Creating files themselves.
        std::ofstream firstFileStream(firstDirectoryDB, std::ios::out | std::ios::binary);
        std::ofstream secondFileStream(secondDirectoryDB, std::ios::out | std::ios::binary);

        //Creating hash action file
        //Contains a list of file paths of files that need to be hashed.
        std::wstring hashActionFileCreationPath = L"hashActionFile.log";
        std::ofstream hashActionFile(hashActionFileCreationPath, std::ios::out | std::ios::binary);

        //Creating file operations action file.
        //Contains a list of operations, and paths to do so, of files.
        //Such as "Copy this file here". "delete this file".
        std::wstring fileOpActionFileCreationPath = L"fileOpActionFile.log";
        std::ofstream fileOpActionFile(fileOpActionFileCreationPath, std::ios::out | std::ios::binary);


        //std::wstring currentReadLine;

        //Directory one.
        for (size_t testIter = 0; testIter < directoryOneDB.size(); ++testIter)
        {
            currentReadLine = directoryOneDB[testIter]; //Grab item
            writeUnicodeToFile(firstFileStream, currentReadLine); //Write it
        }
        //Directory two.
        for (size_t testIter = 0; testIter < directoryTwoDB.size(); ++testIter)
        {
            currentReadLine = directoryTwoDB[testIter]; //Grab item
            writeUnicodeToFile(secondFileStream, currentReadLine); //Write it
        }
        //Hash actions.
        for (size_t testIter = 0; testIter < hashActions.size(); ++testIter)
        {
            currentReadLine = hashActions[testIter]; //Grab item
            writeUnicodeToFile(hashActionFile, currentReadLine); //Write it
        }
        //File operation actions.
        for (size_t testIter = 0; testIter < fileOpActions.size(); ++testIter)
        {
            currentReadLine = fileOpActions[testIter]; //Grab item
            writeUnicodeToFile(fileOpActionFile, currentReadLine); //Write it
        }

        //Closing files.
        firstFileStream.close();
        secondFileStream.close();
        hashActionFile.close();
        fileOpActionFile.close();
    }

    std::chrono::time_point end = std::chrono::steady_clock::now(); // Stop the clock!

    //Outputting statistic information.
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* OUTPUTING INTERNAL FILES -----");
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- STATISTICS -----");


    //Total time.
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Total Time Taken: " + std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + L"ms");


    //Vector information.
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"DB1 Vector Size - " + std::to_wstring(directoryOneDB.size()));
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"DB1 Vector Capacity - " + std::to_wstring(directoryOneDB.capacity()));
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"DB2 Vector Size - " + std::to_wstring(directoryTwoDB.size()));
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"DB2 Vector Capacity - " + std::to_wstring(directoryTwoDB.capacity()));
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Hash Actions Vector Size - " + std::to_wstring(hashActions.size()));
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Hash Actions Vector Capacity - " + std::to_wstring(hashActions.capacity()));
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"File Operation Actions Vector Size - " + std::to_wstring(fileOpActions.size()));
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"File Operation Actions Vector Capacity - " + std::to_wstring(fileOpActions.capacity()));

    //Console displaying final stats.
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"FINISHED! - " + std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count())); //Display clock results.


    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Directory One size: " + std::to_wstring(directoryOneDB.size()));
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Directory One capacity: " + std::to_wstring(directoryOneDB.capacity()));

    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Directory Two size: " + std::to_wstring(directoryTwoDB.size()));
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Directory Two capacity: " + std::to_wstring(directoryTwoDB.capacity()));

    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Hash Action size: " + std::to_wstring(hashActions.size()));
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Hash Action capacity: " + std::to_wstring(hashActions.capacity()));

    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"File Operation size: " + std::to_wstring(fileOpActions.size()));
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"File Operation capacity: " + std::to_wstring(fileOpActions.capacity()));

    if (writeDebugThreadPool.get_tasks_queued() != 0) //Check if debug thread pool is finished writing.
    {
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Waiting for debug to finish writing...");

        writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Waiting for debug to finish writing...");
        writeDebugThreadPool.wait_for_tasks(); //Wait for it to finish.

        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), false, L"-------------------------------------------------- APPLICATION INSTANCE FINISHED --------------------------------------------------");
        writeDebugThreadPool.wait_for_tasks(); //Wait for it to finish.
    }


    //ENDING OF PROGRAM
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Program finished.");
    writeConsoleMessagesPool.wait_for_tasks();
    if (argumentVariables["internalObject"]["Show Console"]) system("PAUSE");

    return 0;

}

//returns the position of the nth occurance of a given character. - This could easily be made a string.
//Asks for a string to be searched, character to find, and what count you desire.
size_t nthOccurrence(std::wstring& givenString, std::wstring delimitingCharacter, size_t nth)
{
    size_t stringPosition = 0;
    size_t count = 0;

    while (count != nth)
    {
        stringPosition += 1;
        stringPosition = givenString.find(delimitingCharacter, stringPosition);
        if (stringPosition == std::wstring::npos)
            return -1;

        count++; //Iterate count.
    }
    return stringPosition;
}

//Uniformly sets directory separators.
std::wstring formatFilePath(std::wstring givenString, std::wstring givenDirectorySeparator)
{

    
    if (givenDirectorySeparator == L"\\" || givenString.find(L"\\\\?\\") != std::wstring::npos) //If the windows max_path bypass is in the path, then all separators must be backslashes.
    {
        //Formating givenFile to have the slashes ALL be \.
        for (size_t i = 0; i < (size_t)givenString.length(); ++i)
        {
            if (givenString[i] == '/')
                givenString[i] = '\\';
        }
    }
    else
    {
        //Formating givenFile to have the slashes ALL be /.
        for (size_t i = 0; i < (size_t)givenString.length(); ++i)
        {
            if (givenString[i] == '\\')
                givenString[i] = '/';
        }
    }


    return givenString;
}

//Goes through hash action file and hashes files on each side.
void performHashActionFile(std::vector<std::wstring>& hashActions, std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::wstring firstGivenPath, std::wstring secondGivenPath)
{
    //Pre-determining hashActionVector size to reduce calls during for loop iterations.
    size_t hashActionSize = hashActions.size();
    //Holds output of hashAction reading, allows for manipulation.
    std::wstring currentReadLine;

    std::wstring directoryOneFile;
    std::wstring directoryTwoFile;
    std::wstring directoryOneVectorLocation;
    std::wstring directoryTwoVectorLocation;

    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Assigning hash thread tasks.");
    for (size_t iterator = 0; iterator < hashActionSize; ++iterator) //Iterating through hashAction file.
    {
        //Grab item...
        currentReadLine = hashActions[iterator]; //Reading the full line.
        directoryOneFile = firstGivenPath + directorySeparator + currentReadLine.substr(0, nthOccurrence(currentReadLine, delimitingCharacter, 1)); //Up until first delimiter, adding first dir path.
        directoryTwoFile = secondGivenPath + directorySeparator + currentReadLine.substr(0, nthOccurrence(currentReadLine, delimitingCharacter, 1)); //Up until first delimiter, adding second dir path.
        directoryOneVectorLocation = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 1) + 1, nthOccurrence(currentReadLine, delimitingCharacter, 2)); //Between first and second delimiter.
        directoryTwoVectorLocation = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 2) + 1, currentReadLine.length() - 1); //Going up until newline.


        hashingThreadPool.push_task(MThashGivenFile, directoryOneFile, std::ref(firstGivenVectorDB), directoryOneVectorLocation); //Creating task, assigning it to main pool. Directory One file.
        hashingThreadPool.push_task(MThashGivenFile, directoryTwoFile, std::ref(secondGivenVectorDB), directoryTwoVectorLocation); //Creating task, assigning it to main pool. Directory Two file.
    }
    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Tasks assigned. Waiting for hash tasks to finish...");


    //The timers below allow a status update to the hashing process every five minutes.
    //START TIMER.
    std::chrono::time_point start = std::chrono::steady_clock::now();

    std::chrono::time_point recordedTime = std::chrono::steady_clock::now();

    bool readyForNext = true;

    while (hashingThreadPool.get_tasks_total())
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() % 30000 == 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() > 29999)
        {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(recordedTime - start).count() != std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count())
            {
                if (readyForNext)
                {
                    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"HASHING TASK UPDATE - Total | Queued | Running - " + std::to_wstring(hashingThreadPool.get_tasks_total()) + L" | " + std::to_wstring(hashingThreadPool.get_tasks_queued()) + L" | " + std::to_wstring(hashingThreadPool.get_tasks_running()));
                    recordedTime = std::chrono::steady_clock::now();
                    readyForNext = false;
                }
            }
        }
        else
            readyForNext = true;
    }
    hashingThreadPool.wait_for_tasks(); //Waiting for tasks to finish.
}

//Goes through file operation action file and assigns the appropriate tasks.
void performFileOpActionFile(std::vector<std::wstring>& fileOpAction)
{
    size_t fileOpSize = fileOpAction.size(); //Pre-allocate size of vector.

    std::wstring currentReadLine;
    std::wstring requestedAction; //Holds what the action is. (Copying or deleting)
    std::wstring source, destination; //Source is between the first and second delimiter | Destonation between the second and end of wstring.


    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Assigning file operation thread tasks.");
    for (size_t iterator = 0; iterator < fileOpSize; ++iterator) //Iterating through hashAction file.
    {
        currentReadLine = fileOpAction[iterator]; //Reading line.

        currentReadLine.erase(std::remove(currentReadLine.begin(), currentReadLine.end(), '\n'), currentReadLine.end()); //Removing newline character from end of line.
        requestedAction = currentReadLine.substr(0, nthOccurrence(currentReadLine, L" - ", 1)); //Reading requested action. Action word is before the " - ".

        //Assigning task.
        if (requestedAction == L"DELETE")
        {
            destination = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 1) + 1, nthOccurrence(currentReadLine, delimitingCharacter, 2) - nthOccurrence(currentReadLine, delimitingCharacter, 1) - 1); //Reading destination. When deleting, it is after the second delimiter to end of string. We also remove the "\n" characters at the end.
            writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"DELETING: " + destination);
            fileOperationThreadPool.push_task(removeObject, destination, true); //Creating deletion task, assigning it to main pool.
        }

        else if (requestedAction == L"COPY")
        {
            source = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 1) + 1, nthOccurrence(currentReadLine, delimitingCharacter, 2) - nthOccurrence(currentReadLine, delimitingCharacter, 1) - 1); //Reading source. Between first and second delimiter.  
            destination = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 2) + 1, - 1); //Reading destination. Second delimiter to end of string, removing the "\n" characters at the end.
            writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"COPYING: " + source + L" - " + destination);
            fileOperationThreadPool.push_task(copyFile, source, destination); //Creating copying task, assigning it to main pool. Directory One file.
        }
        else if (requestedAction == L"MOVE") //Future use for other sync methods. *****
        {
            source = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 1) + 1, nthOccurrence(currentReadLine, delimitingCharacter, 2) - nthOccurrence(currentReadLine, delimitingCharacter, 1) - 1); //Reading source. Between first and second delimiter.  
            destination = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 2) + 1, - 1); //Reading destination. Second delimiter to end of string, removing the "\n" characters at the end.
            writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"MOVE: " + source + L" - " + destination); //Log.
            fileOperationThreadPool.push_task(moveFile, source, destination); //Creating move task, assigning it to main pool. Directory one file is moved to directory two.
        }

        //Initializing variables.
        source = L"";
        destination = L"";
    }


    writeConsoleMessagesPool.push_task(displayConsoleMessage, L"Tasks assigned. Waiting for file operations to finish...");
    fileOperationThreadPool.wait_for_tasks(); //Waiting for tasks to finish.
}

//Iterates through vectors and compares hashes. Creates file operation entries when differences are found.
void compareHashes(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath)
{
    size_t firstDBSize = firstGivenVectorDB.size(); //Pre-allocate size of vector.

    //Holds hashes that are compared against eachother.
    std::wstring DB1Hash;
    std::wstring DB2Hash;

    std::wstring DB1Match; //Holds match check value.
    std::wstring DB1MatchingLineNumber; //Holds DB2 line of other match.

    for (size_t iterator = 0; iterator < firstDBSize; ++iterator)
    {
        //Get matched value of DB1.
        DB1Match = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 5) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 6));
        //Check if the object has been matched. If so, continue to the next iteration and do nothing.
        if (DB1Match != L"MATCHED")
            continue;
        //Get DB1 hash and corresponding line for DB2.
        DB1Hash = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 4) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 5));

        //Get DB1 "Matching Line Number".
        DB1MatchingLineNumber = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 6) + 1, std::wstring::npos);

        //Get DB2 hash using given line from 
        DB2Hash = secondGivenVectorDB[_wtoi(DB1MatchingLineNumber.c_str())].substr(nthOccurrence(secondGivenVectorDB[_wtoi(DB1MatchingLineNumber.c_str())], delimitingCharacter, 4) + 1, nthOccurrence(secondGivenVectorDB[_wtoi(DB1MatchingLineNumber.c_str())], delimitingCharacter, 5));

        if (DB1Hash != L"" || DB2Hash != L"") //Do nothing. A hash is missing.
        {
            if (DB1Hash != DB2Hash) //Hashes do NOT match. 
            {
                if (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"echo" || argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"contribute" || argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"cont")
                {
                    std::wstring currentDB1FilePath = firstGivenVectorDB[iterator].substr(firstGivenPath.length() + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - firstGivenPath.length() - 1); //Getting path of file.
                    fileOpAction.push_back(L"COPY - Different hashes" + delimitingCharacter + firstGivenPath + directorySeparator + currentDB1FilePath + delimitingCharacter + secondGivenPath + directorySeparator + currentDB1FilePath + newLine); //Copy first directory file to second directory.
                }
                else if (argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"synchronize" || argumentVariables["internalObject"]["Operation Mode"].get<std::wstring>() == L"sync")
                {
                    //Alert user that hashes differ, and we do not know which to keep //*****
                }
            }
        }
    }

}


//Allows you to multi-thread the process of doign a simple sort of a vector.
void sortVector(std::vector<std::wstring>& givenVectorDB)
{
    std::sort(givenVectorDB.begin(), givenVectorDB.end());
}