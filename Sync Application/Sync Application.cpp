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



//ARGUMENT DEFAULTS:
bool checkContents = false; ////Recieved from arg: --check-content | Defaults to false.
bool outputFiles = false; //Recieved from arg: --output-files | Defaults to false.
bool recursiveSearch = true; //Recieved from arg: --no-recursive | defaults to true.
bool showConsole = true; //Recieved from arg: --hide-console | defaults to false | Defines whether things are output to the console or not.
bool verboseDebug = false; //Defines if verbose debugging is enabled.
bool showWarning = true; //Recieved from arg: --no-warning | defaults to true | Defines whether things are output to the console or not.
bool dataLossProtection = false; //Recieved from arg: --data-protection | defaults to false;
bool windowsMaxPathBypass = false; //Determines whether "\\?\" is prepended to path and backslashes are used as directory separators.

//Sets global delimiter used for reading and writing DB files. Tilde typically works well. (CONSIDER USING MULTIPLE CHARACTER DELIMITER FOR SAFETY)
std::wstring delimitingCharacter = L"▼";
//Simple newline dude.
std::wstring newLine = L"\n";
//Buffer size for hashing
const int hashBufferSize = 4096;

//Creating threadpools.
thread_pool threadPool(std::thread::hardware_concurrency()); //"Default" pool

thread_pool hashingThreadPool(std::thread::hardware_concurrency());

thread_pool fileOperationThreadPool(std::thread::hardware_concurrency()); //Specific to 

thread_pool writeDebugThreadPool(1); //Dedicated to writing to the debug file.


//Holds provided directory given from args. "Super Parents"
std::wstring firstGivenDirectoryPath;
std::wstring secondGivenDirectoryPath;

//Verbose debugging variables
std::ofstream verboseDebugOutput; //Hold potential future file handle if verbose debugging is enabled.
std::wstring debugFilePath = L"";
std::wstring debugFileName = L"debug.log";
int debugFileCount = 1;

//Holds an array of single letter arguments that need to be applied.
std::unordered_map<char, int> singleCharArguments;


//FUNCTION PROTOTYPES
void writeUnicodeToFile(std::ofstream& outputFile, std::wstring inputWstring); //Does as said.
std::wstring readUnicodeFile(std::wifstream& givenWideFile); //CURRENTLY UNUSED.
std::wstring charToWString(char* givenCharArray); //Used during beginning argument search to integrate provided char switches to fit in internally used wide strings.
std::wstring stringToWString(const std::string& s); //Does as said. Found here: https://forums.codeguru.com/showthread.php?193852-How-to-convert-string-to-wstring
void createDirectoryMapDB(std::vector<std::wstring>& givenVectorDB, std::wstring givenStartPath); //Creates database of given directory. Providing file names with their size, date mod, and date created values.
size_t countFiles(std::wstring pathToDir, bool recursiveLookup); //CURRENTLY UNUSED.
size_t countDir(std::wstring pathToDir, bool recursiveLookup); //CURRENTLY UNUSED.
size_t nthOccurrence(std::wstring& givenString, std::wstring delimitingCharacter, size_t nth); //Provides character location of nthOccurrence of a given character in a given string.
std::wstring formatFilePath(std::wstring givenFile); //Used to change \\ to /
void MThashGivenFile(std::wstring givenFilePath, std::vector<std::wstring>& givenVector, std::wstring lineLocation); //Multithreaded version that writes the outputs to the given vectors.
std::string convertMD5ToHex(unsigned char* givenDigest); //Used BY hashGivenFile to format ahsh into "human readable" hex value.
void sortDirectoryDatabases(std::vector<std::wstring>& givenVectorDB); //Created to allow multithreading. Simple std::sort on databases. Originally (and currently) only used for directory DB's.
void performFileOpActionFile(std::vector<std::wstring>& fileOpAction); //Goes through File Operation Actions vector. Interprets data and assigns task to thread pool.
void performHashActionFile(std::vector<std::wstring>& hashActions, std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::wstring firstGivenPath, std::wstring secondGivenPath); //
void compareHashes(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath); //
void echoCompareDirectories(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& hashActions, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath);
void syncCompareDirectories(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& hashActions, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath);
void contCompareDirectories(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& hashActions, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath);
void removeObject(std::wstring destinationFilePath, bool recursiveRemoval); //Removes given object.
void copyFile(std::wstring source, std::wstring destination); //Copies file.
void writeToDebug(std::chrono::system_clock::time_point givenTime, bool writeTime, std::wstring textToWrite); //Writes to the debug file.

void syncCompareDirectories(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& hashActions, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath)
{

}
void contCompareDirectories(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& hashActions, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath)
{

}

int main(int argc, char* argv[])
{
    ::ShowWindow(::GetConsoleWindow(), SW_SHOW); //Hiding console immediately.

    std::chrono::time_point start = std::chrono::steady_clock::now(); //START TIMER.

    std::wstring operationMode; //Holds sync operation mode. - May want to make this a global variable later.

    //Creating vectors to hold directory maps.
    std::vector<std::wstring> directoryOneDB;
    std::vector<std::wstring> directoryTwoDB;

    //Creating action vectors.
    std::vector<std::wstring> hashActions;
    std::vector<std::wstring> fileOpActions;

    //Verifying that no \ escaped " exist in the path string.
    for (int i = 0; i < argc; i++)
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

    for (int i = 0; i < argc; i++) // Cycle through all arguments.
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
                checkContents = true; //Set hashing to true.
            if (strcmp(argv[i], "--directory-one") == 0) //Directory one path switch.
            {
                firstGivenDirectoryPath = formatFilePath(charToWString(argv[i + 1]));

                if (firstGivenDirectoryPath.back() == L'\\')
                    firstGivenDirectoryPath.pop_back(); //Remove the slash.

                if (!std::filesystem::is_directory(firstGivenDirectoryPath)) //Verify path is real and valid.
                {
                    std::wcout << "--directory-one path provided was NOT found. (" << firstGivenDirectoryPath << ")" << std::endl;
                    system("PAUSE");
                    return 0;
                }
            }
            else if (strcmp(argv[i], "--directory-two") == 0) //Destination two path switch.
            {
                secondGivenDirectoryPath = formatFilePath(charToWString(argv[i + 1]));

                if (secondGivenDirectoryPath.back() == L'\\')
                    secondGivenDirectoryPath.pop_back(); //Remove the slash

                if (!std::filesystem::is_directory(secondGivenDirectoryPath)) //Verify path is real and valid.
                {
                    std::wcout << "The '--second-directory' path provided was NOT found. (" << secondGivenDirectoryPath << ")" << std::endl;
                    std::cout << "Would you like to create this directory?" << std::endl;
                    //*****
                    system("PAUSE");
                    return 0;
                }
            }
            else if (strcmp(argv[i], "--hide-console") == 0) //Defines if anything is output to the console.
                showConsole = false;
            else if (strcmp(argv[i], "--no-recursive") == 0) //Disable recursive operation.
                recursiveSearch = false;
            else if (strcmp(argv[i], "--no-warning") == 0) //Disable deletion warning.
                showWarning = false;
            else if (strcmp(argv[i], "--operation-mode") == 0) //Operation mode switch.
                operationMode = formatFilePath(charToWString(argv[i + 1]));
            else if ((strcmp(argv[i], "--output-files") == 0)) //Enable file output.
                outputFiles = true;
            else if (strcmp(argv[i], "--output-verbose-debug") == 0) //Output debug file in running directory.
            {
                verboseDebug = true; //Set global verbose debug variable to true.

                debugFilePath = formatFilePath(charToWString(argv[i + 1])); //Get next argument.

                if (debugFilePath.find(L"/")) //Search for a slash to determine if the given text is a full path or a name. If a slash is found, it is a path.
                {
                    //Checking that a file name exists. Continuing with default name appended to the given path if it doesn't.
                    if (debugFilePath.substr(debugFilePath.find_last_of(L"/") + 1, std::wstring::npos) != L"")
                    {
                        debugFileName = debugFilePath.substr(debugFilePath.find_last_of(L"/") + 1, std::wstring::npos);
                        debugFilePath = debugFilePath.substr(0, debugFilePath.find_last_of(L"/") + 1); //Remove filename from path.
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
            for (int iterator = 1; iterator < sizeof(argv[i]); ++iterator) //Iterating through all characters, after the slash. (Starting at 1 to skip the initial dash)
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
        if (singleCharArguments['l']) //Short help message.
            windowsMaxPathBypass = true;
    }
    //ARGS FINISHED.


    //If debugging is enabled, write the "new application line"
    if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), false, L"-------------------------------------------------- NEW APPLICATION INSTANCE --------------------------------------------------"); //Stating new application instance.

    //MAX_PATH bypass.
    //Also ensuring that path is an absolute path.
    windowsMaxPathBypass = true;
    if (windowsMaxPathBypass)
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

    if (operationMode != L"")
    {
        std::transform(operationMode.begin(), operationMode.end(), operationMode.begin(), towlower); //Convert to lowercase for easy comparison.
            
        //Check that it is a legitimate value.
        if (operationMode != L"echo" && (operationMode != L"synchronize" && operationMode != L"sync") && (operationMode != L"contribute" && operationMode != L"cont"))
        {
            if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Error-----");
            std::wcout << L"Invalid Operation Mode: " << operationMode << std::endl;
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
    if (showWarning)
    {
        //Displaying warning information.
        std::wcout << L"-------------------------------------------------- WARNING --------------------------------------------------" << std::endl;
        if (operationMode == L"echo")
        {
            std::wcout << L"The \"ECHO\"operation will take place. This will cause the second directory to look *IDENTICAL* to the first." << std::endl;
            std::wcout << L"When a file is found within both directories, the program will compare the size and last modified time. When a difference is found, the first directories file will overwrite the seconds. (Hashes are only used when files, size, and modification time are all the same and the --check-contents argument is provided.)" << std::endl;
            std::wcout << L"This option is best used when you are backing up data and want a second directory to match the directory you make changes in." << std::endl;
            std::wcout << L"Deletions and file overwrites are possible within the second directory." << std::endl;
            std::wcout << L"First Directory: " << firstGivenDirectoryPath << std::endl;
            std::wcout << L"Second Directory: " << secondGivenDirectoryPath << std::endl;
        }
        else if (operationMode == L"synchronize" || operationMode == L"sync")
        {
            std::wcout << L"The \"SYNCHRONIZE\"operation will take place. This will cause the first and second directories to look identical to each other, with the newest files being kept." << std::endl;
            std::wcout << L"When a file is found within both directories, the one with the newest modification time is used and copied to replace the older version." << std::endl;
            std::wcout << L"This option is best used when changes can be made within both directories and you want them both to be synced with the newest versions from both." << std::endl;
            std::wcout << L"Deletions and file overwrites are possible within both directories." << std::endl;
            std::wcout << L"First Directory: " << firstGivenDirectoryPath << std::endl;
            std::wcout << L"Second Directory: " << secondGivenDirectoryPath << std::endl;
        }
        else if (operationMode == L"contribute" || operationMode == L"cont")
        {
            std::wcout << L"The \"CONTRIBUTE\"operation will take place. This will cause the first directory to contribute any new files or changes to the second directory." << std::endl;
            std::wcout << L"When a file is found within both directories, the program will compare the size and last modified time. When a difference is found, the first directories file will overwrite the second directories file. If the file is not present within the second directory, it is copied over to it. (Hashes are only used when files, size, and modification time are all the same and the --check-contents argument is provided.)" << std::endl;
            std::wcout << "This option is best used when you are regularly archiving files and want to keep everything." << std::endl;
            std::wcout << L"No deletions are ever made. File overwrites are possible within the second directory." << std::endl;
            std::wcout << L"First Directory: " << firstGivenDirectoryPath << std::endl;
            std::wcout << L"Second Directory: " << secondGivenDirectoryPath << std::endl;
        }

        std::wcout << std::endl;
        std::wcout << L"(This warning can be disabled by adding the \"--no-warning\" argument)" << std::endl;
        std::wcout << L"File operations are permenant (ESPECIALLY DELETIONS). Do you wish to proceed, knowing what files are potentially at risk? (Y/N)" << std::endl;

        char userInput[1]; //Holds user input character.
        std::cin >> userInput[0]; //Awaiting user input...

        //Verify if the user is okay with continuing.
        if (toupper(userInput[0]) != 'Y') //The input is NOT a "Y".
        {
            std::cout << "Termination Program. No changes have been made." << std::endl;
            return 0;
        }
        else //The user gave the okay. Continue.
            std::cout << "Permission to continue granted. Moving forward with program." << std::endl;
    }

    if (!showConsole) //If the console should be displayed, then show it.
        ::ShowWindow(::GetConsoleWindow(), SW_HIDE);

    

    //Displaying file locations.
    if (showConsole) std::wcout << L"First Directory: " << firstGivenDirectoryPath << std::endl;
    if (showConsole) std::wcout << L"Second Directory: " << secondGivenDirectoryPath << std::endl;

    if (verboseDebug) //Debug beginning program information.
    {
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Operation Mode - " + operationMode);
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"First Given Path - " + firstGivenDirectoryPath);
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Second Given Path - " + secondGivenDirectoryPath);
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Arg --check-content - " + std::to_wstring(checkContents));
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Arg --output-files - " + std::to_wstring(outputFiles));
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Arg --output-verbose-debug - " + std::to_wstring(verboseDebug));
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Arg --no-recursive - " + std::to_wstring(!recursiveSearch));
    }



    if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- DIRECTORY CRAWLING -----");
    if (showConsole) std::cout << "Creating directory maps..." << std::endl; //***

    //Creating initial directory map.
    threadPool.push_task(createDirectoryMapDB, std::ref(directoryOneDB), std::ref(firstGivenDirectoryPath));
    threadPool.push_task(createDirectoryMapDB, std::ref(directoryTwoDB), std::ref(secondGivenDirectoryPath));
    threadPool.wait_for_tasks();

    if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* DIRECTORY CRAWLING -----");
    if (showConsole) std::cout << "Directory maps created..." << std::endl; //***


    if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- SORTING DIRECTORY LISTS -----");
    if (showConsole) std::cout << "Sorting lists..." << std::endl; //***

    //Semi-Sorting directories. This may be changed to be a natural sorting later. (to make it more human-readable)
    threadPool.push_task(sortDirectoryDatabases, std::ref(directoryOneDB));
    threadPool.push_task(sortDirectoryDatabases, std::ref(directoryTwoDB));
    threadPool.wait_for_tasks();

    if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* SORTING DIRECTORY LISTS -----");
    if (showConsole) std::cout << "Sorting finished..." << std::endl; //***



    //FUTURE FEATURE: CHECK WHAT OPERATION IS BEING DONE.
    if (operationMode == L"echo")
    {
        if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- DIRECTORY COMPARISON - " + operationMode + L" -----");
        if (showConsole) std::cout << "Beginning directory comparison function." << std::endl; //***
        echoCompareDirectories(directoryOneDB, directoryTwoDB, hashActions, fileOpActions, firstGivenDirectoryPath, secondGivenDirectoryPath); //Handles a LOT of stuff. Includes the process of hashing files before ending.
        if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* DIRECTORY COMPARISON - " + operationMode + L" -----");
        if (showConsole) std::cout << "Directory comparing finished..." << std::endl; //***
    }
    else if (operationMode == L"synchronize" || operationMode == L"sync")
    {
        if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- DIRECTORY COMPARISON - " + operationMode + L" -----");
        if (showConsole) std::cout << "Beginning directory comparison function for synchronization mode." << std::endl; //***
        syncCompareDirectories(directoryOneDB, directoryTwoDB, hashActions, fileOpActions, firstGivenDirectoryPath, secondGivenDirectoryPath); //Includes the process of hashing files and comparing again before ending.
        if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* DIRECTORY COMPARISON - " + operationMode + L" -----");
        if (showConsole) std::cout << "Directory comparing finished..." << std::endl; //***
    }
    else if (operationMode == L"contribute" || operationMode == L"cont")
    {
        if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- DIRECTORY COMPARISON - " + operationMode + L" -----");
        if (showConsole) std::cout << "Beginning directory comparison function for synchronization mode." << std::endl; //***
        contCompareDirectories(directoryOneDB, directoryTwoDB, hashActions, fileOpActions, firstGivenDirectoryPath, secondGivenDirectoryPath); //Includes the process of hashing files and comparing again before ending.
        if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* DIRECTORY COMPARISON - " + operationMode + L" -----");
        if (showConsole) std::cout << "Directory comparing finished..." << std::endl; //***
    }
    
    //Performing file operations.
    if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- FILE OPERATIONS -----");
    if (showConsole) std::cout << "Beginning File Operations..." << std::endl;
    performFileOpActionFile(fileOpActions); //Regardless of the type of operation, a file operation check should occur.
    if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- *COMPLETED* FILE OPERATIOSN -----");
    if (showConsole) std::cout << "File Operations finished!..." << std::endl;




    //Outputting files!
    if (outputFiles == true)
    {
        if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"----- OUTPUTING INTERNAL FILES -----");
        //Holds output of hashAction reading, allows for manipulation.
        std::wstring currentReadLine;

        if (showConsole) std::cout << "Shifting arrays to show headers..." << std::endl; //***

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
        for (int testIter = 0; testIter < directoryOneDB.size(); ++testIter)
        {
            currentReadLine = directoryOneDB[testIter]; //Grab item
            writeUnicodeToFile(firstFileStream, currentReadLine); //Write it
        }
        //Directory two.
        for (int testIter = 0; testIter < directoryTwoDB.size(); ++testIter)
        {
            currentReadLine = directoryTwoDB[testIter]; //Grab item
            writeUnicodeToFile(secondFileStream, currentReadLine); //Write it
        }
        //Hash actions.
        for (int testIter = 0; testIter < hashActions.size(); ++testIter)
        {
            currentReadLine = hashActions[testIter]; //Grab item
            writeUnicodeToFile(hashActionFile, currentReadLine); //Write it
        }
        //File operation actions.
        for (int testIter = 0; testIter < fileOpActions.size(); ++testIter)
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
    if (verboseDebug)
    {
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
    }

    //Console displaying final stats.
    if (showConsole)
    {
        std::cout << "FINISHED! - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl; //Display clock results.


        std::cout << "Directory One size: " << directoryOneDB.size() << std::endl;
        std::cout << "Directory One capacity: " << directoryOneDB.capacity() << std::endl;

        std::cout << "Directory Two size: " << directoryTwoDB.size() << std::endl;
        std::cout << "Directory Two capacity: " << directoryTwoDB.capacity() << std::endl;

        std::cout << "Hash Action size: " << hashActions.size() << std::endl;
        std::cout << "Hash Action capacity: " << hashActions.capacity() << std::endl;

        std::cout << "File Operation size: " << fileOpActions.size() << std::endl;
        std::cout << "File Operation capacity: " << fileOpActions.capacity() << std::endl;
    }
    if (writeDebugThreadPool.get_tasks_queued() != 0) //Check if debug thread pool is finished writing.
    {
        if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"Waiting for debug to finish writing...");

        if (showConsole) std::cout << "Waiting for debug to finish writing..." << std::endl;
        writeDebugThreadPool.wait_for_tasks(); //Wait for it to finish.

        if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), false, L"-------------------------------------------------- APPLICATION INSTANCE FINISHED --------------------------------------------------");
        writeDebugThreadPool.wait_for_tasks(); //Wait for it to finish.
    }


    //ENDING OF PROGRAM
    if (showConsole) std::cout << "Program finished." << std::endl;
    if (showConsole) system("PAUSE");

    return 0;

}

//Writing unicode to file. Stolen from https://cppcodetips.wordpress.com/2014/09/16/reading-and-writing-a-unicode-file-in-c/
void writeUnicodeToFile(std::ofstream& outputFile, std::wstring inputWstring)
{
    outputFile.write((char*)inputWstring.c_str(), inputWstring.length() * sizeof(wchar_t));
}

//Converts character arrays to wstring.
std::wstring charToWString(char* givenCharArray)
{
    std::string intermediaryString = givenCharArray;
    int wchars_num = MultiByteToWideChar(65001, 0, intermediaryString.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[wchars_num];
    MultiByteToWideChar(65001, 0, intermediaryString.c_str(), -1, wstr, wchars_num);

    return wstr;
}

//Converts string to WString.
std::wstring stringToWString(const std::string& s)
{
    std::wstring temp(s.length(), L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp;
}

//This seems as though it would work. I have not tried it yet.
std::wstring readUnicodeFile(std::wifstream& givenWideFile)
{
    std::stringstream wss;
    wss << givenWideFile.rdbuf();
    std::string  const& str = wss.str();
    std::wstring wstr;
    wstr.resize(str.size() / sizeof(wchar_t));
    std::memcpy(&wstr[0], str.c_str(), str.size()); // copy data into wstring
    return wstr;
}

//Functions asks for a path to directory and will return number of files. - Also asks for a T/F bool determining whether a search should be recursive.
size_t countFiles(std::wstring pathToDir, bool recursiveLookup)
{
    size_t fileCounter = 0;

    //Making path a path it likes.
    std::filesystem::path dirPath(pathToDir);

    //Checking if user wanted a recursive lookup.
    if (recursiveLookup)
    {
        //Performing recursive searching...
        for (std::filesystem::recursive_directory_iterator end, dir(pathToDir); dir != end; ++dir)
        {
            //Verify item is a file.
            if (std::filesystem::is_regular_file(dir->path()))
                fileCounter++; //Count files specifically.
        }
    }
    else
    {
        std::filesystem::directory_iterator end_itr;
        //Cycle through the GIVEN directory. No deeper. Any directories are just directories directly viewable.
        for (std::filesystem::directory_iterator itr(dirPath); itr != end_itr; ++itr)
        {
            //Verify item is a file.
            if (std::filesystem::is_regular_file(itr->path()))
                fileCounter++; //Count files specifically.
        }
    }
    return fileCounter; //Return files!
}

//Functions asks for a path to a directory and will return number of files. - Also asks for a T/F bool determining whether a search should be recursive.
size_t countDir(std::wstring pathToDir, bool recursiveLookup)
{
    size_t directoryCounter = 0;

    //Making path a path it likes.
    std::filesystem::path dirPath(pathToDir);

    //Checking if user wanted a recursive lookup.
    if (recursiveLookup)
    {
        //Performing recursive searching...
        for (std::filesystem::recursive_directory_iterator end, dir(pathToDir); dir != end; ++dir)
        {
            //Verifying the item is a directory.
            if (std::filesystem::is_directory(dir->path()))
                directoryCounter++;
        }
    }
    else
    {
        std::filesystem::directory_iterator end_itr;
        //Cycle through the GIVEN directory. No deeper. Any directories are just directories directly viewable.
        for (std::filesystem::directory_iterator itr(dirPath); itr != end_itr; ++itr)
        {
            //Verifying the item is a directory.
            if (std::filesystem::is_directory(itr->path()))
                directoryCounter++; //Count directories specifically.
        }
    }
    return directoryCounter; //Return files!
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
std::wstring formatFilePath(std::wstring givenString)
{
    if (givenString.find(L"\\\\?\\") != std::wstring::npos) //If the windows max_path bypass is in the path, then all separators must be backslashes.
    {
        //Formating givenFile to have the slashes ALL be \.
        for (int i = 0; i < (int)givenString.length(); ++i)
        {
            if (givenString[i] == '/')
                givenString[i] = '\\';
        }
    }
    else
    {
        //Formating givenFile to have the slashes ALL be /.
        for (int i = 0; i < (int)givenString.length(); ++i)
        {
            if (givenString[i] == '\\')
                givenString[i] = '/';
        }
    }


    return givenString;
}

//Multithreadable hash given file that also handles writing the result to the givenVector.
//Asks for the path to the file to hash, a vector to store output in, and line location in that vector to modify.
void MThashGivenFile(std::wstring givenFilePath, std::vector<std::wstring>& givenVector, std::wstring lineLocation)
{
    if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"HASHING STARTED: " + givenFilePath);

    //https://www.quora.com/How-can-I-get-the-MD5-or-SHA-hash-of-a-file-in-C
    //Define relevant variables.
    unsigned char digest[MD5_DIGEST_LENGTH];
    char BUFFER[hashBufferSize];
    //Open file to hash for reading.
    std::ifstream fileHandle(givenFilePath, std::ios::binary);

    std::error_code ec; //Create error handler.

    //Create MD5 handle
    MD5_CTX md5Context;

    //Init the MD5 process.
    MD5_Init(&md5Context);



    if (fileHandle.fail())
    {
        if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"ERROR - FAIL FLAG BEFORE HASHING: " + givenFilePath);
        return;
    }

    //Read until the file is ended.
    while (fileHandle.good())
    {
        //Read. Read again. Read.
        fileHandle.read(BUFFER, sizeof(BUFFER));
        //Update the MD5 process. gcount is vital here.
        MD5_Update(&md5Context, BUFFER, fileHandle.gcount());

        if (fileHandle.bad())
        {
            if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"ERROR - BAD FLAG DURING HASHING: " + givenFilePath);
            return;
        }

    }

    //Finish the MD5, place it in the digest.
    int MD5Result = MD5_Final(digest, &md5Context);

    //Verify hash completed successfully.
    if (MD5Result == 0) // Hash failed.
        std::wcout << L"HASH FAILED. 0 RETURNED." + givenFilePath << std::endl;

    //Close file.
    fileHandle.close();

    if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"HASHING COMPLETED: " + givenFilePath);

    //Write the result to the vector.
    givenVector[_wtoi(lineLocation.c_str())].insert(nthOccurrence(givenVector[_wtoi(lineLocation.c_str())], delimitingCharacter, 4) + 1, stringToWString(convertMD5ToHex(digest)));
    return;
}

//Converts char array of MD5 value to a "human readable" hex string.
std::string convertMD5ToHex(unsigned char* givenDigest)
{
    //Create and zero out buffer to hold hex output. - 32 characters given.
    char hexBuffer[32];
    memset(hexBuffer, 0, 32);

    //Creating string to hold final hash output.
    std::string outputHexString;

    //Convert the 128-bit hash to hex.
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(hexBuffer, "%02x", givenDigest[i]);
        outputHexString.append(hexBuffer);
    }

    return outputHexString; //Output hash
}

//Allows you to multi-thread the process of doign a simple sort of a vector.
void sortDirectoryDatabases(std::vector<std::wstring>& givenVectorDB)
{
    std::sort(givenVectorDB.begin(), givenVectorDB.end());
}

//Creates a list of all files and directories within a given directory. Places each entry in a delimited format into the given wstring vector.
void createDirectoryMapDB(std::vector<std::wstring>& givenVectorDB, std::wstring givenStartPath)
{
    //Making the given path an actual usable path. idk why
    std::filesystem::path dirPath(givenStartPath);
    std::filesystem::directory_iterator end_itr;


    //Creating a stringstream to hold the file size
    std::stringstream temporaryStringStreamFileAndDirSearch;

    std::wstring current_file;
    std::wstringstream testStream;


    time_t lastModifiedTime;
    time_t dateCreatedTime;

    //Checking whether to search recursively or not
    if (recursiveSearch) {
        //RECURSIVE
        for (std::filesystem::recursive_directory_iterator end, dir(givenStartPath); dir != end; ++dir)
        {
            //Setting current file equal to the full path of the file.
            current_file = std::filesystem::absolute(dir->path().native());

            //Putting path into array.
            testStream << formatFilePath(current_file) << delimitingCharacter;

            // If it's not a directory, list it. If you want to list directories too, just remove this check.
            if (std::filesystem::is_regular_file(dir->path()))
            {
                    //if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"2 Is not a directory: " + formatFilePath(current_file));
                    testStream << std::filesystem::file_size(current_file) << delimitingCharacter;
                    //if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"3 Got File Size: " + formatFilePath(current_file));
                    //Getting last modified time. In seconds from 1970 EPOCH format.
                    testStream << boost::filesystem::last_write_time(current_file) << delimitingCharacter;

                    //if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"4 Got last write time: " + formatFilePath(current_file));
                    //Getting date created time. In seconds from 1970 EPOCH format.
                    //if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"5 Got creation time: " + formatFilePath(current_file));
                    testStream << boost::filesystem::creation_time(current_file) << delimitingCharacter + delimitingCharacter + newLine; //Adding an additional delimiter, since the hash is not added yet but the matching stuff will be.

                    //if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"6 Data stored for file: " + formatFilePath(current_file));
            }
            else if (std::filesystem::is_directory(current_file))
                testStream << delimitingCharacter + delimitingCharacter + delimitingCharacter + delimitingCharacter + newLine;
                    

            //if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"7 Pushing data back: " + formatFilePath(current_file));
            //Append to DB.
            givenVectorDB.push_back(testStream.str());
            //if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"8 Reseting streamstring: " + formatFilePath(current_file));
            //Clearing stringstream for next iteration.
            testStream.str(std::wstring());
            //if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"9 finished streamstring: " + formatFilePath(current_file));
            
        }
    }
    else
    {
        // cycle through the directory
        for (std::filesystem::directory_iterator itr(givenStartPath); itr != end_itr; ++itr)
        {
            // If it's not a directory, list it. If you want to list directories too, just remove this check.
            if (std::filesystem::is_regular_file(itr->path()))
            {
                //Setting current file equal to the full path of the file.
                current_file = std::filesystem::absolute(itr->path().native());

                //Putting path into array.
                testStream << current_file << delimitingCharacter;

                //Getting filesize (in bytes).
                testStream << std::filesystem::file_size(current_file) << delimitingCharacter;

                //Getting last modified time. In seconds from 1970 EPOCH format.
                time_t firstDirectoryModifiedTime = boost::filesystem::last_write_time(current_file);
                testStream << firstDirectoryModifiedTime << delimitingCharacter;

                //Getting date created time. In seconds from 1970 EPOCH format.
                time_t firstDirectorydateCreatedTime = boost::filesystem::creation_time(current_file);
                testStream << firstDirectorydateCreatedTime << delimitingCharacter + newLine;

                //Append to DB.
                givenVectorDB.push_back(testStream.str());

                //Clearing stringstream for next iteration.
                testStream.str(std::wstring());
            }

        }
    }

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

    if (showConsole) std::cout << "Assigning hash thread tasks." << std::endl;
    for (int iterator = 0; iterator < hashActionSize; ++iterator) //Iterating through hashAction file.
    {
        //Grab item...
        currentReadLine = hashActions[iterator]; //Reading the full line.
        directoryOneFile = firstGivenPath + L"/" + currentReadLine.substr(0, nthOccurrence(currentReadLine, delimitingCharacter, 1)); //Up until first delimiter, adding first dir path.
        directoryTwoFile = secondGivenPath + L"/" + currentReadLine.substr(0, nthOccurrence(currentReadLine, delimitingCharacter, 1)); //Up until first delimiter, adding second dir path.
        directoryOneVectorLocation = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 1) + 1, nthOccurrence(currentReadLine, delimitingCharacter, 2)); //Between first and second delimiter.
        directoryTwoVectorLocation = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 2) + 1, currentReadLine.length() - 1); //Going up until newline.


        hashingThreadPool.push_task(MThashGivenFile, directoryOneFile, std::ref(firstGivenVectorDB), directoryOneVectorLocation); //Creating task, assigning it to main pool. Directory One file.
        hashingThreadPool.push_task(MThashGivenFile, directoryTwoFile, std::ref(secondGivenVectorDB), directoryTwoVectorLocation); //Creating task, assigning it to main pool. Directory Two file.
    }
    if (showConsole) std::cout << "Tasks assigned. Waiting for hash tasks to finish..." << std::endl;

    //*****
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
                    if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"HASHING TASK UPDATE - Total | Queued | Running - " + std::to_wstring(hashingThreadPool.get_tasks_total()) + L" | " + std::to_wstring(hashingThreadPool.get_tasks_queued()) + L" | " + std::to_wstring(hashingThreadPool.get_tasks_running()));
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


    if (showConsole) std::cout << "Assigning file operation thread tasks." << std::endl;
    for (int iterator = 0; iterator < fileOpSize; ++iterator) //Iterating through hashAction file.
    {
        currentReadLine = fileOpAction[iterator]; //Reading line.

        currentReadLine.erase(std::remove(currentReadLine.begin(), currentReadLine.end(), '\n'), currentReadLine.end()); //Removing newline character from end of line.
        requestedAction = currentReadLine.substr(0, nthOccurrence(currentReadLine, L" - ", 1)); //Reading requested action. Action word is before the " - ".


        //Assigning task.
        if (requestedAction == L"DELETE")
        {
            destination = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 1) + 1, nthOccurrence(currentReadLine, delimitingCharacter, 2) - nthOccurrence(currentReadLine, delimitingCharacter, 1) - 1); //Reading destination. When deleting, it is after the second delimiter to end of string. We also remove the "\n" characters at the end.
            if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"DELETING: " + destination);
            fileOperationThreadPool.push_task(removeObject, destination, true); //Creating deletion task, assigning it to main pool.
        }

        else if (requestedAction == L"COPY")
        {
            source = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 1) + 1, nthOccurrence(currentReadLine, delimitingCharacter, 2) - nthOccurrence(currentReadLine, delimitingCharacter, 1) - 1); //Reading source. Between first and second delimiter.  
            destination = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 2) + 1, -1); //Reading destination. Second delimiter to end of string, removing the "\n" characters at the end.
            if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"COPYING: " + source + L" - " + destination);
            fileOperationThreadPool.push_task(copyFile, source, destination); //Creating copying task, assigning it to main pool. Directory One file.
        }
        else if (requestedAction == L"MOVE") //Future use for other sync methods. *****
        {
            source = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 1) + 1, nthOccurrence(currentReadLine, delimitingCharacter, 2) - nthOccurrence(currentReadLine, delimitingCharacter, 1) - 1); //Reading source. Between first and second delimiter.  
            destination = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 2) + 1, -1); //Reading destination. Second delimiter to end of string, removing the "\n" characters at the end.
            if (verboseDebug) writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"MOVE: " + source + L" - " + destination); //Log.
            //***** THIS NEEDS WORK. THE OBJECT WILL LIKELY BE REMOVED BEFORE COPYING IS COMPLETED.
            fileOperationThreadPool.push_task(copyFile, source, destination); //Creating copying task, assigning it to main pool. Directory One file.
            fileOperationThreadPool.push_task(removeObject, source, false); //Removing source AFTER copying.
        }

        //Initializing variables.
        source = L"";
        destination = L"";
    }


    if (showConsole) std::cout << "Tasks assigned. Waiting for file operations to finish..." << std::endl;
    fileOperationThreadPool.wait_for_tasks(); //Waiting for tasks to finish.
}

//Performs "filesystem::remove_all" on given path.
void removeObject(std::wstring destinationFilePath, bool recursiveRemoval)
{
    if (!std::filesystem::exists(destinationFilePath)) //Verify it is a normal file. I don't know what other types there are, but I'll avoid deleting them until I know.
        return;

    std::error_code ec; //Create error handler.

    //if (std::filesystem::is_directory(destinationFilePath))
    //    return; //do nothing

    if (recursiveRemoval) //Determine which remove method we are using.
        std::filesystem::remove_all(destinationFilePath, ec); //Removing all.
    else
        std::filesystem::remove(destinationFilePath, ec); //Removing.

    //Sending errors to that error_code seems to fix some problems?
    //An error was occuring sometimes when deleting destination empty directories, but adding this error part make it just work.
    if (ec.value() == 5) //If error value is 5, it is access denied.
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"ERROR. ACCESS DENIED: " + destinationFilePath); //Log.

}

//Copying file.
void copyFile(std::wstring givenSourcePath, std::wstring givenDestinationPath)
{

    //Creating files themselves.
    std::wstring destinationDirectoriesPath = givenDestinationPath.substr(0, givenDestinationPath.find_last_of(L"/") + 1); //Obtaining path of the destination until the last backslash.



    if (!std::filesystem::exists(destinationDirectoriesPath)) //If the directory does not exist, then create it.
        std::filesystem::create_directories(destinationDirectoriesPath);


    while (!std::filesystem::exists(destinationDirectoriesPath)) //Wait for directories to be created.
    {
        //V O I D
    }

    //***
    if (!std::filesystem::is_directory(givenDestinationPath)) //Don't bother dealing with directories right now *****
    {
        std::error_code ec; //Create error handler.
        std::filesystem::copy(givenSourcePath, givenDestinationPath, std::filesystem::copy_options::overwrite_existing, ec); //Copying the file. - If a directory is being looked at, it would have already been made above. This will do nothing.

        if (ec.value() == 5) //If error value is 5, it is access denied.
            writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"ERROR. ACCESS DENIED: " + givenSourcePath + L" - " + givenDestinationPath); //Log.
    }

}

//Uses unordered maps to compare directory lists and find matches.
void echoCompareDirectories(std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::vector<std::wstring>& hashActions, std::vector<std::wstring>& fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath)
{
    //Storing ending iterator (vector size). Every "for loop" call will ask the vector for it's size otherwise. - https://articles.emptycrate.com/2008/10/26/c_loop_optimization.html
    size_t firstDBSize = firstGivenVectorDB.size();
    size_t secondDBSize = secondGivenVectorDB.size();


    //Creating unordered maps.
    //std::unordered_map<size_t, std::wstring> DB1Map;
    std::unordered_map<std::wstring, size_t> DB2Map;

    size_t DB2Line; //DB1Map searching in DB2Map provides the line of DB2Match.

    //Holds 
    std::wstring iter1;
    std::wstring iter2;


    //Column variables to avoid streamline calls on vectors.
    std::wstring workingPath;
    std::wstring workingPathTwo;
    std::wstring workingSize;
    std::wstring workingSizeTwo;
    std::wstring workingDateMod;
    std::wstring workingDateModTwo;
    std::wstring workingDateCreated;
    std::wstring workingDateCreatedTwo;
    std::wstring workingHash;
    std::wstring workingHashTwo;

    //Iterating through second directory vector.
    for (size_t iteratorTwo = 0; iteratorTwo < secondDBSize; ++iteratorTwo) //Needs to start at "1" if headers are added before this.
        DB2Map.insert(std::make_pair(secondGivenVectorDB[iteratorTwo].substr(secondGivenPath.length() + 1, nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 1) - secondGivenPath.length() - 1), iteratorTwo));

    //Iterate through firstDB.
    for (size_t iterator = 0; iterator < firstDBSize; ++iterator)
    {
        workingPath = firstGivenVectorDB[iterator].substr(firstGivenPath.length() + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - firstGivenPath.length() - 1); //Get path of object.

        if (DB2Map.count(workingPath)) //Search for match in DB2.
        {
            DB2Line = DB2Map[workingPath]; //Save the value.

            iter1 = std::to_wstring(iterator); //Convert the line of DB1 to a wstring, to allow saving.
            iter2 = std::to_wstring(DB2Line); //Convert newly found DB2 line to wstring for saving.
            firstGivenVectorDB[iterator].insert(firstGivenVectorDB[iterator].length() - 1, L"MATCHED" + delimitingCharacter + iter2); //Add match marker and line.
            secondGivenVectorDB[DB2Line].insert(secondGivenVectorDB[DB2Line].length() - 1, L"MATCHED" + delimitingCharacter + iter1); //Add match marker and line.


            if (!std::filesystem::is_directory(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1))))
            {
                workingDateMod = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 3) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - 1); //Third column
                workingDateModTwo = secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) + 1, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 3) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - 1); //Third column
                if (workingDateMod == workingDateModTwo) //If the file paths match, check the last modified times.
                {
                    workingSize = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - 1); //Second column
                    workingSizeTwo = secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) + 1, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) - 1); //Second column
                    if (workingSize == workingSizeTwo) //If last modified times match, check that the file sizes match.
                    {
                        if (checkContents)
                            hashActions.push_back(workingPath + delimitingCharacter + iter1 + delimitingCharacter + iter2 + newLine); //If everything matches, these files need hashed and compared.
                    }
                    else
                    {
                        fileOpAction.push_back(L"COPY - Different file sizes" + delimitingCharacter + firstGivenPath + L"/" + workingPath + delimitingCharacter + secondGivenPath + L"/" + workingPath + newLine); //Copy first directory file to second directory.
                        //fileOpAction.push_back(L"COPY - Different file sizes" + delimitingCharacter + firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)) + delimitingCharacter + secondGivenPath + L"\\" + workingPath + newLine); //Copy first directory file to second directory.
                    }
                }
                else //A matching file has been found, with differing last modified times.
                {
                    fileOpAction.push_back(L"COPY - Different last modified time" + delimitingCharacter + firstGivenPath + L"/" + workingPath + delimitingCharacter + secondGivenPath + L"/" + workingPath + newLine); //Copy first directory file to second directory.
                    //fileOpAction.push_back(L"COPY - Different last modified time" + delimitingCharacter + firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)) + delimitingCharacter + secondGivenPath + L"\\" + workingPath + newLine); //Copy first directory file to second directory.
                }
            }
        }
        else
        {
            if (std::filesystem::is_directory(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1))) && std::filesystem::is_empty(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1))))
                fileOpAction.push_back(L"COPY - Empty directory present on source" + delimitingCharacter + firstGivenPath + L"/" + workingPath + delimitingCharacter + secondGivenPath + L"/" + workingPath + newLine); //Copy first directory file to second directory.
            else
                fileOpAction.push_back(L"COPY - No destination found" + delimitingCharacter + firstGivenPath + L"/" + workingPath + delimitingCharacter + secondGivenPath + L"/" + workingPath + newLine); //Copy first directory file to second directory.
        }
    }

    //DB1Map.clear();
    DB2Map.clear();


    if (showConsole) std::cout << "Checking for second directory items that did not get matched..." << std::endl; //***

    //Iterating through secondary directory list and checking against matched list.
    for (size_t iterator = 0; iterator < secondDBSize; ++iterator) //Needs to start at "1" if headers are added before this.
    {
        if (secondGivenVectorDB[iterator].substr(nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 5) + 1, 7) != L"MATCHED")
            fileOpAction.push_back(L"DELETE - No source found" + delimitingCharacter + secondGivenVectorDB[iterator].substr(0, nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 1)) + newLine); //Delete second directory file. No source file found that matches.
    }


    //If matching files need to be hashed, do so.
    if (checkContents)
    {
        if (showConsole) std::cout << "Beginning hash process. " << hashActions.size() * 2 << " Files to be hashed..." << std::endl; //***
        performHashActionFile(hashActions, firstGivenVectorDB, secondGivenVectorDB, firstGivenPath, secondGivenPath);
        if (showConsole) std::cout << "Hashing finished!" << std::endl; //***
        if (showConsole) std::cout << "Comparing file hashes..." << std::endl; ///***
        compareHashes(firstGivenVectorDB, secondGivenVectorDB, fileOpAction, firstGivenPath, secondGivenPath);
        if (showConsole) std::cout << "Hash comparison finished!" << std::endl; //***
    }


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
                std::wstring currentDB1FilePath = firstGivenVectorDB[iterator].substr(firstGivenPath.length() + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - firstGivenPath.length() - 1); //Getting path of file.
                fileOpAction.push_back(L"COPY - Different hashes" + delimitingCharacter + firstGivenPath + L"/" + currentDB1FilePath + delimitingCharacter + secondGivenPath + L"/" + currentDB1FilePath + newLine); //Copy first directory file to second directory.
            }
        }

    }

}

//Writes to the the debug file.
void writeToDebug(std::chrono::system_clock::time_point givenTime, bool writeTime, std::wstring textToWrite)
{
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