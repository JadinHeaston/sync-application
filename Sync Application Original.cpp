//This application is meant to do what SyncToy does, but better for myself.
//Initially, this will be a console (command line) driven utility. It could maybe get UI elements later(?)

//Three asterisks *** Indicate something done for debugging only. These will likely be removed later, or moved to output to some kind of debug file.
//Five asterisks are generally used to bring attention to an area. Maybe to indicate that something is currently being worked on, or is troublesome.

//LIBRARIES
#include <iostream> //Yeah.
#include <boost/filesystem.hpp> //Used for getting file date metadata (date modified and date created). Struggled to switch to the built in C++ filesystem for this.
#include <fstream> //File input/output.
#include <string> //Strings!
#include <sstream> //Stringstream
#include <openssl/md5.h> //Used for MD5 hashing. Pretty good!
#include <filesystem> //Great for ease and function overloading things that allow wide character sets compared to Boost.
//Thread stuff
#include <thread>
#include <mutex>
#include <chrono> //Time tracking.
#include <vector> //vector.


//GLOBAL VARIABLES
//Is the file search recursive?
bool recursiveSearch = true;
bool checkContents = false;

//Sets global delimiter used for reading and writing DB files. Tilde typically works well. (CONSIDER USING MULTIPLE CHARACTER DELIMITER FOR SAFETY)
std::wstring delimitingCharacter = L"▼";
//Simple newline dude. - OFTEN DOESN'T WORK.
std::wstring newLine = L"\n";
//Buffer size for hashing
const int hashBufferSize = 4096;

//Mutex lock
std::mutex g_lock;
//Creating threads.
const int threadCount = 12;
std::thread processingThreads[threadCount];



//FUNCTION PROTOTYPES
void echoCompareDirectories(std::vector<std::wstring> &firstGivenVectorDB, std::vector<std::wstring> &secondGivenVectorDB, std::vector<std::wstring> &hashActions, std::vector<std::wstring> &fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath);
void writeUnicodeToFile(std::ofstream& outputFile, std::wstring inputWstring); //Does as said.
void writeUnicodeToFile(std::ofstream& outputFile, std::string inputString); //Removing this breaks the other writeUnicodeToFile. I'm not sure why :(
void writeNonUnicodeToFile(std::ofstream& outputFile, std::string inputString); //Does as said.
std::wstring readUnicodeFile(std::wifstream& givenWideFile); //CURRENTLY UNUSED.
std::wstring charToWString(char* givenCharArray); //Used during beginning argument search to integrate provided char switches to fit in internally used wide strings.
std::wstring stringToWString(const std::string& s); //Does as said. Found here: https://forums.codeguru.com/showthread.php?193852-How-to-convert-string-to-wstring
std::wstring stringToWString(const std::string& s, std::wstring& returnString); //Same as above, but allows you to return the string THROUGH the function args. Used for multiple thread stuff.
void createDirectoryMapDB(std::vector<std::wstring> &givenVectorDB, std::wstring givenStartPath); //Creates database of given directory. Providing file names with their size, date mod, and date created values.
size_t countFiles(std::wstring pathToDir, bool recursiveLookup); //Used to 
size_t countDir(std::wstring pathToDir, bool recursiveLookup); //CURRENTLY UNUSED.
size_t nthOccurrence(std::wstring& givenString, std::wstring delimitingCharacter, int nth);
std::wstring formatFilePath(std::wstring givenFile); //Used
std::string hashGivenFile(std::wstring givenFilePath, std::wstring& outputWString); //Used. Allows for multithreaded operation.
std::string convertMD5ToHex(unsigned char* givenDigest); //Used BY hashGivenFile to format ahsh into "human readable" hex value.
void sortDirectoryDatabases(std::vector<std::wstring>& givenVectorDB); //Created to allow multithreading. Simple std::sort on databases. Originally (and currently) only used for directory DB's.
void performHashActionFile(std::vector<std::wstring>& hashActions, std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::wstring firstGivenPath, std::wstring secondGivenPath); //WiP
void pushBackToVector(std::vector<std::wstring>& givenVector, std::wstring givenMessage); //Created to allow multithreaded work pushing back data to vectors. - IN TESTING.
void insertMatchInfoIntoVector(std::vector<std::wstring>& givenVector, size_t vectorItem, std::wstring matchingLine); //Created for multithreading the match process - IN TESTING.
void multiThreadedDirectoryCompare();



int main(int argc, char* argv[])
{
    //START TIMER.
    std::chrono::time_point start = std::chrono::steady_clock::now();
    
    //Creating vectors to hold directory maps.
    std::vector<std::wstring> directoryOneDB;
    std::vector<std::wstring> directoryTwoDB;

    //Creating action vectors.
    std::vector<std::wstring> hashActions;
    std::vector<std::wstring> fileOpActions;

    //Holds provided directory given from args.
    std::wstring firstGivenDirectoryPath;
    std::wstring secondGivenDirectoryPath;

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
        std::cout << "No arguments provided.\nUse the \"-h\" or \"-help\" switch to show the available options.\n(-s and -d are required for operation)" << std::endl;
        system("PAUSE");
        return 0;
    }
    else if (argc == 2) //Checking for help message.
    {
        std::cout << "ARG 1: " << argv[1] << std::endl;
        if (strncmp(argv[1], "-h", 3) == 0 || strcmp(argv[1], "-help") == 0) //Checking second argument for if it is "-h" or "-help".
        {
            //Display help
            std::cout << "HELP PROVIDED. GET FUCKED" << std::endl;
            system("PAUSE");
            return 0;
        }
        else //No arguments provided. Notify. Close program.
        {
            std::cout << "Use the \"-h\" or \"-help\" switch to show the available options.\n(-s and -d are required for operation)" << std::endl;
            system("PAUSE");
            return 0;
        }
    }
    for (int i = 0; i < argc; i++) // Cycle through all arguments.
    {

        if (strncmp(argv[i], "-s", 2) == 0) //Source path switch.
        {
            firstGivenDirectoryPath = formatFilePath(charToWString(argv[i + 1]));
            if (firstGivenDirectoryPath.back() == L'\\')
            {
                secondGivenDirectoryPath.pop_back(); //Remove the slash.
            }
            if (!std::filesystem::is_directory(firstGivenDirectoryPath)) //Verify path is real and valid.
            {
                std::wcout << "-s path provided was NOT found. (" << firstGivenDirectoryPath << ")" << std::endl;
                system("PAUSE");
                return 0;
            }
            //std::wcout << firstGivenDirectoryPath << std::endl;

        }
        else if (strncmp(argv[i], "-d", 2) == 0) //Destination path switch.
        {
            secondGivenDirectoryPath = formatFilePath(charToWString(argv[i + 1]));

            if (secondGivenDirectoryPath.back() == L'\\')
            {
                secondGivenDirectoryPath.pop_back(); //Remove the slash
            }

            if (!std::filesystem::is_directory(secondGivenDirectoryPath)) //Verify path is real and valid.
            {
                std::wcout << "-d path provided was NOT found. (" << secondGivenDirectoryPath << ")" << std::endl;
                system("PAUSE");
                return 0;
            }
            //std::wcout << secondGivenDirectoryPath << std::endl;
        }
        else if (0)
        {

        }



        //std::cout << argv[i] << std::endl;
    }




    //Determines whether files are synced with newest, echo'd, only copied to destination. etc.
    //Eventually tied to input args. Echo is used for now.
    std::string operationMode = "echo";

    //Recieved from arg. WILL DEFAULT TO FALSE IN THE FUTURE.
    bool createFileOuptut = true;



    //Displaying file locations.
    std::wcout << L"first dir: " << firstGivenDirectoryPath << std::endl;
    std::wcout << L"second dir: " << secondGivenDirectoryPath << std::endl;

    //Pre-allocating vector space.
    //***** This may need work to find an optimal size of some of these. Otherwise, excess memory is used.
    
    //std::cout << "Pre-allocating vector space..." << std::endl; //***
    //size_t fileCountDirOne = countFiles(firstGivenDirectoryPath, recursiveSearch);
    //size_t fileCountDirTwo = countFiles(secondGivenDirectoryPath, recursiveSearch);
    //directoryOneDB.resize(fileCountDirOne);
    //directoryTwoDB.resize(fileCountDirTwo);
    //hashActions.resize(fileCountDirOne + fileCountDirTwo);
    //fileOpActions.resize(fileCountDirOne + fileCountDirTwo);

    //std::cout << "Finished pre-allocating vector space!" << std::endl; //***



    //auto end = std::chrono::steady_clock::now(); // Stop the clock!
    //std::cout << "Creating directory maps... - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl; //***
    std::cout << "Creating directory maps..." << std::endl; //***

    //Creating initial directory map.
    processingThreads[0] = std::thread(createDirectoryMapDB, std::ref(directoryOneDB), std::ref(firstGivenDirectoryPath));
    processingThreads[1] = std::thread(createDirectoryMapDB, std::ref(directoryTwoDB), std::ref(secondGivenDirectoryPath));
    processingThreads[0].join();
    processingThreads[1].join();

    //std::chrono::time_point end = std::chrono::steady_clock::now(); // Stop the clock!
    //std::cout << "Directory maps created... - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl; //***
    //std::chrono::time_point end = std::chrono::steady_clock::now(); // Stop the clock!
    //std::cout << "Sorting lists... - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl; //***
    std::cout << "Directory maps created..." << std::endl; //***

    //*****
    //Pre-allocating 
    //hashActions.resize(directoryOneDB.size()*2);
    //fileOpActions.resize(directoryOneDB.size() + directoryTwoDB.size());
    //std::cout << "Initial Hash Action size: " << hashActions.size() << std::endl;
    //std::cout << "Initial Hash Action capacity: " << hashActions.capacity() << std::endl;

    //std::cout << "Initial File Operation size: " << fileOpActions.size() << std::endl;
    //std::cout << "Initial File Operation capacity: " << fileOpActions.capacity() << std::endl;


    std::cout << "Sorting lists..." << std::endl; //***

    //Semi-Sorting directories. This may be changed to be a natural sorting later. (to make it more human-readable)
    processingThreads[0] = std::thread(sortDirectoryDatabases, std::ref(directoryOneDB));
    processingThreads[1] = std::thread(sortDirectoryDatabases, std::ref(directoryTwoDB));
    processingThreads[0].join();
    processingThreads[1].join();

    //auto end = std::chrono::steady_clock::now(); // Stop the clock!
    //std::cout << "Sorting finished... - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl; //***
    std::cout << "Sorting finished..." << std::endl; //***
    


    //FUTURE FEATURE: CHECK WHAT OPERATION IS BEING DONE.
    if (operationMode == "echo")
    {
        //ECHO (LEFT TO RIGHT) IS ALL THAT IS DONE.
        //std::chrono::time_point end = std::chrono::steady_clock::now(); // Stop the clock!
        //std::cout << "Comparing directories... - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl; //***
        std::cout << "Comparing directories..." << std::endl; //***
        echoCompareDirectories(directoryOneDB, directoryTwoDB, hashActions, fileOpActions, firstGivenDirectoryPath, secondGivenDirectoryPath);
        //std::chrono::time_point end = std::chrono::steady_clock::now(); // Stop the clock!
        //std::cout << "Comparing completed... - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl; //***
        std::cout << "Comparing completed..." << std::endl; //***


        //If matching files need to be hashed, do so.
        if (checkContents)
        {
            std::cout << "Beginning hash process. " << hashActions.size() * 2 << " Files to be hashed..." << std::endl; //***
            performHashActionFile(hashActions, directoryOneDB, directoryTwoDB, firstGivenDirectoryPath, secondGivenDirectoryPath);
            std::cout << "Hashing finished!" << std::endl; //***

            //Comparison of hashs needs to occur!

        }
    }
    else
    {
        std::cout << "No operation mode provided." << std::endl;
        return 0;
    }

    //performFileOpActionFile(fileOpActionFileCreationPath);




    //Outputting files!
    if (createFileOuptut == true)
    {


        //Holds output of hashAction reading, allows for manipulation.
        std::wstring currentReadLine;

        ////HASH IT FOR TESTING TIME STUFF.
        //std::cout << "HASHING FILES START..." << std::endl;
        ////START TIMER.
        //std::chrono::time_point start2 = std::chrono::steady_clock::now();

        ////Pre-determining hashActionVector size to reduce calls during for loop iterations.
        //size_t hashActionSize = hashActions.size();


        ////Hash actions.
        //for (int iterator = 0; iterator < hashActionSize; ++iterator)
        //{
        //    //Grab item...
        //    currentReadLine = hashActions[iterator];

        //    hashGivenFile(firstGivenDirectoryPath + L"\\" + currentReadLine);
        //    //hashGivenFile(secondGivenPath + L"\\" + currentReadLine);
        //}

        //std::chrono::time_point end2 = std::chrono::steady_clock::now(); // Stop the clock!
        //std::cout << "FINISHED! TIME (MS) TAKEN: " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count() << std::endl; //Display clock results.
        //std::cout << "HASHING FILES ENDED - FINISHING WRITING TO FILES..." << std::endl;
        // 
        //std::chrono::time_point end = std::chrono::steady_clock::now(); // Stop the clock!
        //std::cout << "Shifting arrays to show headers... - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl; //***
        std::cout << "Shifting arrays to show headers..." << std::endl; //***

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

        std::wstring firstDirectoryTMPDB = L"DB1.txt";
        std::wstring secondDirectoryTMPDB = L"DB2.txt";
        //If files are placed in the exe's running location, ensure that these debug files contain what file paths belong to what instead of just "DB1" and "DB2" ***

        //Creating files themselves.
        std::ofstream firstFileStream(firstDirectoryTMPDB, std::ios::out | std::ios::binary);
        std::ofstream secondFileStream(secondDirectoryTMPDB, std::ios::out | std::ios::binary);

        //Creating hash action file
        //Contains a list of file paths of files that need to be hashed.
        std::wstring hashActionFileCreationPath = L"hashActionFile.txt";
        std::ofstream hashActionFile(hashActionFileCreationPath, std::ios::out | std::ios::binary);

        //Creating file operations action file.
        //Contains a list of operations, and paths to do so, of files.
        //Such as "Copy this file here". "delete this file".
        std::wstring fileOpActionFileCreationPath = L"fileOpActionFile.txt";
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
    std::cout << "FINISHED! - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl; //Display clock results.


    std::cout << "Directory One size: " << directoryOneDB.size() << std::endl;
    std::cout << "Directory One capacity: " << directoryOneDB.capacity() << std::endl;

    std::cout << "Directory Two size: " << directoryTwoDB.size() << std::endl;
    std::cout << "Directory Two capacity: " << directoryTwoDB.capacity() << std::endl;

    std::cout << "Hash Action size: " << hashActions.size() << std::endl;
    std::cout << "Hash Action capacity: " << hashActions.capacity() << std::endl;

    std::cout << "File Operation size: " << fileOpActions.size() << std::endl;
    std::cout << "File Operation capacity: " << fileOpActions.capacity() << std::endl;

    //ENDING OF PROGRAM
    system("PAUSE");

    return 0;

}



//Writing unicode to file. Stolen from https://cppcodetips.wordpress.com/2014/09/16/reading-and-writing-a-unicode-file-in-c/
void writeUnicodeToFile(std::ofstream& outputFile, std::wstring inputWstring)
{
    outputFile.write((char*)inputWstring.c_str(), inputWstring.length() * sizeof(wchar_t));
}
void writeUnicodeToFile(std::ofstream& outputFile, std::string inputString)
{
    outputFile.write((char*)inputString.c_str(), inputString.length() * sizeof(wchar_t));
    //std::cout << "CUM" << std::endl;
}

std::wstring charToWString(char* givenCharArray)
{
    std::string intermediaryString = givenCharArray;
    int wchars_num = MultiByteToWideChar(65001, 0, intermediaryString.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[wchars_num];
    MultiByteToWideChar(65001, 0, intermediaryString.c_str(), -1, wstr, wchars_num);

    return wstr;
}

std::wstring stringToWString(const std::string& s)
{
    std::wstring temp(s.length(), L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp;
}

std::wstring stringToWString(const std::string& s, std::wstring& returnString)
{
    std::wstring temp(s.length(), L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    returnString = temp;
    return temp;
}

void writeNonUnicodeToFile(std::ofstream& outputFile, std::string inputString)
{
    //I stole this from: https://stackoverflow.com/questions/6693010/how-do-i-use-multibytetowidechar
    //65001 = CP_UTF8
    int wchars_num = MultiByteToWideChar(65001, 0, inputString.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[wchars_num];
    MultiByteToWideChar(65001, 0, inputString.c_str(), -1, wstr, wchars_num);
    writeUnicodeToFile(outputFile, wstr);
    // do whatever with wstr
    delete[] wstr;
}

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
    if (recursiveLookup) {
        //Performing recursive searching...
        for (std::filesystem::recursive_directory_iterator end, dir(pathToDir); dir != end; ++dir) {
            // If it's not a directory, list it. If you want to list directories too, just remove this check.
            if (std::filesystem::is_regular_file(dir->path()))
            {
                //Count files specifically.
                fileCounter++;
            }
        }
    }
    else
    {
        std::filesystem::directory_iterator end_itr;
        //Cycle through the GIVEN directory. No deeper. Any directories are just directories directly viewable.
        for (std::filesystem::directory_iterator itr(dirPath); itr != end_itr; ++itr)
        {
            // If it's not a directory, list it. If you want to list directories too, just remove this check.
            if (std::filesystem::is_regular_file(itr->path()))
            {
                //Count files specifically.
                fileCounter++;
            }
        }
    }
    //Return files!
    return fileCounter;
}

//Functions asks for a path to directory and will return number of files. - Also asks for a T/F bool determining whether a search should be recursive.
size_t countDir(std::wstring pathToDir, bool recursiveLookup)
{

    size_t fileCounter = 0;
    size_t directoryCounter = 0;

    //Making path a path it likes.
    std::filesystem::path dirPath(pathToDir);

    //Checking if user wanted a recursive lookup.
    if (recursiveLookup) {
        //Performing recursive searching...
        for (std::filesystem::recursive_directory_iterator end, dir(pathToDir); dir != end; ++dir) {
            //Count all items.
            directoryCounter++;
            // If it's not a directory, list it. If you want to list directories too, just remove this check.
            if (std::filesystem::is_regular_file(dir->path())) {
                //Count files specifically.
                fileCounter++;
            }
        }
    }
    else {
        std::filesystem::directory_iterator end_itr;
        //Cycle through the GIVEN directory. No deeper. Any directories are just directories directly viewable.
        for (std::filesystem::directory_iterator itr(dirPath); itr != end_itr; ++itr)
        {
            //Count all items.
            directoryCounter++;
            // If it's not a directory, list it. If you want to list directories too, just remove this check.
            if (std::filesystem::is_regular_file(itr->path())) {
                //Count files specifically.
                fileCounter++;
            }
        }
    }
    //Finding difference between ALL items, and literal files. Leaves directories.
    directoryCounter = directoryCounter - fileCounter;
    //Return files!
    return directoryCounter;
}

//returns the position of the nth occurance of a given character. - This could easily be made a string.
//Asks for a string to be searched, character to find, and what count you desire.
size_t nthOccurrence(std::wstring& givenString, std::wstring delimitingCharacter, int nth)
{
    size_t stringPosition = 0;
    int count = 0;

    while (count != nth)
    {
        stringPosition += 1;
        stringPosition = givenString.find(delimitingCharacter, stringPosition);
        if (stringPosition == std::wstring::npos)
            return -1;
        count++;
    }
    return stringPosition;
}

//Takes a string and removes "/" and places "\\".
std::wstring formatFilePath(std::wstring givenFile)
{
    //Formating givenFile to have the slashes ALL be \ instead of mixed with / and \.
    for (int i = 0; i < (int)givenFile.length(); ++i)
    {
        if (givenFile[i] == '/')
            givenFile[i] = '\\';
    }

    return givenFile;
}

std::string hashGivenFile(std::wstring givenFilePath, std::wstring& outputWString)
{
    //https://www.quora.com/How-can-I-get-the-MD5-or-SHA-hash-of-a-file-in-C
    //Define relevant variables.
    unsigned char digest[MD5_DIGEST_LENGTH];
    char BUFFER[hashBufferSize];

    //Open file to hash for reading.
    std::ifstream fileHandle(givenFilePath, std::ios::binary);

    //Create MD5 handle
    MD5_CTX md5Context;

    //Init the MD5 process.
    MD5_Init(&md5Context);
    //Read until the file is ended.
    while (!fileHandle.eof())
    {
        //Read. Read again. Read.
        fileHandle.read(BUFFER, sizeof(BUFFER));
        //Update the MD5 process. gcount is vital here.
        MD5_Update(&md5Context, BUFFER, fileHandle.gcount());
    }
    //Finish the MD5, place it in the digest.
    int MD5Result = MD5_Final(digest, &md5Context);
    //Verify hash completed successfully.
    if (MD5Result == 0) // Hash failed.
        std::wcout << L"HASH FAILED. 0 RETURNED." + givenFilePath << std::endl;

    //Close file.
    fileHandle.close();

    outputWString = stringToWString(convertMD5ToHex(digest));
    return ""/*convertMD5ToHex(digest)*/;
}

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
        //system("PAUSE");
        sprintf(hexBuffer, "%02x", givenDigest[i]);
        //sprintf(hexBuffer, "%02x", digest[i]);
        outputHexString.append(hexBuffer);
        //outputHexString = (hexBuffer[0], hexBuffer[30]);
    }
    
    //Output hash
    return outputHexString;
}

//Function created to allow 2 threads to work at the same time.
void sortDirectoryDatabases(std::vector<std::wstring>& givenVectorDB)
{
    std::sort(givenVectorDB.begin(), givenVectorDB.end());
}

void createDirectoryMapDB(std::vector<std::wstring> &givenVectorDB, std::wstring givenStartPath)
{

    //These are temporary strings holding the returned MD5 Hex hash from files. Each directory has it's own to avoid collisions/waiting.
    std::string temporaryFileHash;


    //Making the given path an actual usable path. idk why
    std::filesystem::path dirPath(givenStartPath);
    std::filesystem::directory_iterator end_itr;


    //Creating a stringstream to hold the file size
    std::stringstream temporaryStringStreamFileAndDirSearch;

    std::wstring current_file;
    std::wstringstream testStream;
    //Checking whether to search recursively or not
    if (recursiveSearch) {
        //RECURSIVE
        for (std::filesystem::recursive_directory_iterator end, dir(givenStartPath); dir != end; ++dir)
        {
            // If it's not a directory, list it. If you want to list directories too, just remove this check.
            if (std::filesystem::is_regular_file(dir->path()))
            {
                //Setting current file equal to the full path of the file.
                current_file = std::filesystem::absolute(dir->path().native());
                
                //Putting path into array.
                testStream << current_file << delimitingCharacter;

                //Getting filesize (in bytes).
                testStream << std::filesystem::file_size(current_file) << delimitingCharacter;

                //Getting last modified time. In seconds from 1970 EPOCH format.
                time_t firstDirectoryModifiedTime = boost::filesystem::last_write_time(current_file);
                testStream << firstDirectoryModifiedTime << delimitingCharacter;

                //Getting date created time. In seconds from 1970 EPOCH format.
                time_t firstDirectorydateCreatedTime = boost::filesystem::creation_time(current_file);
                testStream << firstDirectorydateCreatedTime << delimitingCharacter + delimitingCharacter + newLine; //Adding an additional delimiter, since the hash is not added yet but the matching stuff will be.

                //Append to DB.
                givenVectorDB.push_back(testStream.str());

                //Clearing stringstream for next iteration.
                testStream.str(std::wstring());
            }
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

void echoCompareDirectories(std::vector<std::wstring> &firstGivenVectorDB, std::vector<std::wstring> &secondGivenVectorDB, std::vector<std::wstring> &hashActions, std::vector<std::wstring> &fileOpAction, std::wstring firstGivenPath, std::wstring secondGivenPath)
{
    //Storing ending iterator (vector size). Every "for loop" call will ask the vector for it's size otherwise. - https://articles.emptycrate.com/2008/10/26/c_loop_optimization.html
    size_t firstDBSize = firstGivenVectorDB.size();
    size_t secondDBSize = secondGivenVectorDB.size();


    //Used for comparing the strings.
    //These aren't NEEDED, but it makes reading and future work with this code easier if used.
    //***** Maybe this system needs removed, and direct calls should be used, to optimize further.
    //Removing these would likely create more total calls.
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

    //Keeps track of whether a first directory file was found or not.
    bool notFound;



    //*** Debug output.
    //std::wstring echoDebugHandle = L"echoCompareDebug.txt";
    //int comparisonOutput;

    ////Creating files themselves.
    //std::ofstream echoDebug(echoDebugHandle, std::ios::out | std::ios::binary);


    std::wstring iter1;
    std::wstring iter2;

    //Iterating through first directory vector.
    for (size_t iterator = 0; iterator < firstDBSize; ++iterator) //Needs to start at "1" if headers are added before this.
    {
        //Placing parts of the string into their variables.
        workingPath = firstGivenVectorDB[iterator].substr(firstGivenPath.length() + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - firstGivenPath.length() - 1); //Removing first provided paths to make them both comparable.
        workingSize = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)+1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - 1); //Second column
        workingDateMod = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2)+1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 3) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - 1); //Third column
        //workingDateCreated = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 3)+1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 4) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 3) - 1); //Fourth column
        //workingHash = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 4), std::wstring::npos); //Fifth column - Special work, since the DB doesn't end with a delimiter. Reading until end of string.
        //workingMatch = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 5)+1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 6) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 5) - 1); //Fifth column - Special work, since the DB doesn't end with a delimiter. Reading until end of string.
        //workingLineFound = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 6), std::wstring::npos); //Fifth column - Special work, since the DB doesn't end with a delimiter. Reading until end of string.
        
        //Init notFound to be false.
        notFound = true;








        //Iterating through second directory vector.
        for (size_t iteratorTwo = 0; iteratorTwo < secondDBSize; ++iteratorTwo) //Needs to start at "1" if headers are added before this.
        {
            //Placing parts of the string into their variables.
            //workingDateCreatedTwo = secondGivenVectorDB[iteratorTwo].substr(nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 3) + 1, nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 4) - nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 3) - 1); //Fourth column
            //workingHashTwo = secondGivenVectorDB[iteratorTwo].substr(nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 4), std::wstring::npos); //Fifth column - Special work, since the DB doesn't end with a delimiter. Reading until end of string.

            if (secondGivenVectorDB[iteratorTwo].substr(nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 5) + 1, 7) != L"MATCHED") //Ignore if previously matched.
            {
                workingPathTwo = secondGivenVectorDB[iteratorTwo].substr(secondGivenPath.length() + 1, nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 1) - secondGivenPath.length() - 1); //Removing first provided paths to make them both comparable.
                //comparisonOutput = wcscmp(firstGivenVectorDB[iterator].c_str(), secondGivenVectorDB[iteratorTwo].c_str());
                //std::cout << comparisonOutput << std::endl;
                ////system("PAUSE");
                //if (comparisonOutput >= 0) //Check if the first string is alphabetically larger than the second. If it is, then we have passed the location it could be, and can break. This assumes that both vectors are SORTED prior. - Benefits would be seen when source items are not in the destination.
                //{
                    if (workingPath == workingPathTwo) //Check if the paths match.
                    {
                        notFound = false; //Keeping track that the file has been found.
                        iter1 = std::to_wstring(iterator);
                        iter2 = std::to_wstring(iteratorTwo);
                        firstGivenVectorDB[iterator].insert(firstGivenVectorDB[iterator].length() - 1, L"MATCHED" + delimitingCharacter + iter2); //Add match marker and line.
                        secondGivenVectorDB[iteratorTwo].insert(secondGivenVectorDB[iteratorTwo].length() - 1, L"MATCHED" + delimitingCharacter + iter1); //Add match marker and line.


                        //secondGivenVectorDB[iteratorTwo].erase(secondGivenVectorDB[iteratorTwo].length() - 1, 1); //Removing newline to allow the matched marker, and first directory line to be added.
                        //secondGivenVectorDB[iteratorTwo].append(L"MATCHED" + delimitingCharacter + iter1 + newLine); //Second Directory: Marking that the entry has had a match found and providing first directory line of match.
                        //firstGivenVectorDB[iterator].erase(firstGivenVectorDB[iterator].length()-1, 1); //Removing newline to allow the matched marker, and second directory line to be added.
                        //firstGivenVectorDB[iterator].append(L"MATCHED" + delimitingCharacter + iter2 + newLine); //First Directory: Marking that the entry has had a match found and providing second directory line of match.

                        //processingThreads[0] = std::thread(insertMatchInfoIntoVector, std::ref(firstGivenVectorDB), iterator, iter2);
                        //processingThreads[1] = std::thread(insertMatchInfoIntoVector, std::ref(secondGivenVectorDB), iteratorTwo, iter1);


                        //***** These need to include the 
                        workingDateModTwo = secondGivenVectorDB[iteratorTwo].substr(nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 2) + 1, nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 3) - nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 2) - 1); //Third column
                        if (workingDateMod == workingDateModTwo) //If the file paths match, check the last modified times.
                        {
                            workingSizeTwo = secondGivenVectorDB[iteratorTwo].substr(nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 1) + 1, nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 2) - nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 1) - 1); //Second column
                            if (workingSize == workingSizeTwo) //If last modified times match, check that the file sizes match.
                            {
                                hashActions.push_back(workingPath + delimitingCharacter + iter1 + delimitingCharacter + iter2 + newLine); //If everything matches, these files need hashed and compared.
                                break; //Exiting directory two iterating against directory one file. Proceed to next directory one file.
                            }
                            else
                            {
                                fileOpAction.push_back(L"COPY - Different file sizes" + delimitingCharacter + firstGivenPath + L"\\" + workingPath + delimitingCharacter + secondGivenPath + L"\\" + workingPath + newLine); //Copy first directory file to second directory.
                                //fileOpAction.push_back(L"COPY - Different file sizes" + delimitingCharacter + firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)) + delimitingCharacter + secondGivenPath + L"\\" + workingPath + newLine); //Copy first directory file to second directory.
                                break; //Exiting directory two iterating against directory one file. Proceed to next directory one file.
                            }
                        }
                        else //A matching file has been found, with differing last modified times.
                        {
                            fileOpAction.push_back(L"COPY - Different last modified time" + delimitingCharacter + firstGivenPath + L"\\" + workingPath + delimitingCharacter + secondGivenPath + L"\\" + workingPath + newLine); //Copy first directory file to second directory.
                            //fileOpAction.push_back(L"COPY - Different last modified time" + delimitingCharacter + firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)) + delimitingCharacter + secondGivenPath + L"\\" + workingPath + newLine); //Copy first directory file to second directory.
                            break; //Exiting directory two iterating against directory one file. Proceed to next directory one file.
                        }
                    }
                //else
                //{
                //    writeUnicodeToFile(echoDebug, workingPath + delimitingCharacter + workingPathTwo + delimitingCharacter + std::to_wstring(comparisonOutput) + newLine);
                //    notFound = true;
                //    break; //Exit the loop. The second path is larger than the first alphabetically and can't be in the remaining options.
                //}
            }
            else //Path not found during this cycle. Could be found in future ones? NO! It should break out of the loop, if found.
            {
                notFound = true; //Directory one file not found within directory two. It should break out of the loop, if found.
            }
        } //End of second directory iterating loop






        if (notFound == true) //If the file was not found in the second directory, create a file operation to copy (create) it there.
            fileOpAction.push_back(L"COPY - No destination file found" + delimitingCharacter + firstGivenPath + L"\\" + workingPath + delimitingCharacter + secondGivenPath + L"\\" + workingPath + newLine); //Copy first directory file to second directory.
        //fileOpAction.push_back(L"COPY - No destination file found" + delimitingCharacter + firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)) + delimitingCharacter + secondGivenPath + L"\\" + workingPath + newLine); //Copy first directory file to second directory.
        //Go to the next first directory file.
    }


    //std::cout << "Checking matched list against second directory..." << std::endl; //***
    std::cout << "Checking for second directory items that did not get matched..." << std::endl; //***

    //Iterating through secondary directory list and checking against matched list.
    for (size_t iterator = 0; iterator < secondDBSize; ++iterator) //Needs to start at "1" if headers are added before this.
    {
        if (secondGivenVectorDB[iterator].substr(nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 5) + 1, 7) != L"MATCHED")
            fileOpAction.push_back(L"DELETE - No source file found" + delimitingCharacter + secondGivenVectorDB[iterator].substr(0, nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 1)) + newLine); //Delete second directory file. No source file found that matches.
    }

    //***** 
    //std::cout << "Shrinking vectors" << std::endl; //***
    ////Resizing vectors to ensure that memory is not being wasted. They shouldn't be getting larger/smaller after this point.
    //firstGivenVectorDB.resize(firstGivenVectorDB.size());
    //secondGivenVectorDB.resize(secondGivenVectorDB.size());
    //hashActions.resize(hashActions.size());
    //fileOpAction.resize(fileOpAction.size());

    //echoDebug.close(); //***



    //TODO *****
    //The below solutions needs changed. It theoretically doubles the time/processing required if everything is identical between directories. It gets more efficient as less things were matched above.
    //Ideas for solutions:
    //1. This could be solved by adding another column and marking the second database when a match is found. This list needs gone through once, and everything without that mark needs to be deleted.
        //1A. Maybe instead of a column, the entire second directory can be output to the file (debugging/exporting) before this step, and Matching entries can be deleted??? - NO. Matching entries are needed for hashing later.
        //1AA. Matched files can be immediately ignored when doing the earlier comparisons between dir 1 and 2. This should help speed things up?
    //2. A forum suggests a "hash map". - https://softwareengineering.stackexchange.com/questions/280361/list-comparing-techniques-for-faster-performance
        //An unordered map seems VERY similar to the "folder deepeness" idea mentioned below. 
    //That's the best I have right now...


    //Additionally, this entire process (echoCompareDirectories) could potentially be multi-threaded by doing chunks of 10000 (or 1000) files (The exact number may needed testing, but a large number)
        //Potentially every 10000 files gets a thread, each with it's own COPY (not passed by reference) of the vector data.
        //Maybe each thread stores a vector of data containing matches that is applied to the master/original at the end.
        //How would this matches vector avoid the problem? Eventually every entry of the "matching lists" will be checked against the master list.
        //A good suggestion I found mentioned creating a "queueing system" and dedicating a thread to that. That seems like it would eliminate most issues... - https://articles.emptycrate.com/2009/06/10/optimizing_massively_multithreaded_c_applications__watch_for_hidden_mutexes.html
            //2 threads could be handling the comparisons and reporting matches to the queue, where 2 more are checking those and determining what is missing.
    //Potentially the deepness of a file (how many folders deep) is stored in the database, and this can be referenced first to sift through the majority of files that differ.
    //Another idea: Create a list of folders that should be searched for and compared for. If the folder is missing, we automatically know that every item in the folder needs copied over (or deleted, depending on which side is missing the folder)


    //***** CURRENTLY IMPLEMENTED. WILL BE TESTED TO SEE IF PERFORMANCE IS IMPROVED/HARMED.
    //Count files in the directory prior to the scan, and pre-allocate the expected vector space needed.





    ////Iterating through secondary directory list and checking against matched list.
    //for (int iterator = 0; iterator < secondDBSize; ++iterator) //Needs to start at "1" if headers are added before this.
    //{
    //    //Finding file path of the string into their variables.
    //    workingPathTwo = secondGivenVectorDB[iterator].substr(secondGivenPath.length() + 1, nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 1) - secondGivenPath.length() - 1); //Removing first provided paths to make them both comparable.

    //    //Iterating through second directory vector to check against the matched vector list.
    //    for (int iteratorTwo = 0; iteratorTwo < matchingFiles.size(); ++iteratorTwo)
    //    {
    //        //wprintf(L"%s \n", workingPathTwo.c_str());
    //        //wprintf(L"%s \n", matchingFiles[iteratorTwo].c_str());

    //        //system("PAUSE");
    //        //Checking matching file list to second directory list.
    //        if (workingPathTwo + newLine == matchingFiles[iteratorTwo])
    //        {
    //            notFound = false; //Keeping track that the file has been found.
    //            break; //Exit loop. A matched 
    //        }
    //        else //Not matched.
    //        {
    //            notFound = true;
    //        }
    //        

    //    }
    //    if (notFound == true) //If the file was never found earlier and had a matching comparison AND the matching list, it ends up here. DELETED.
    //    {
    //        fileOpAction.push_back(L"DELETE - No source file found" + delimitingCharacter + secondGivenVectorDB[iterator].substr(0, nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 1)) + newLine); //Delete second directory file. No source file found that matches.
    //    }
    //}
}

//Goes through hash action file and hashes files on each side.
void performHashActionFile(std::vector<std::wstring>& hashActions, std::vector<std::wstring>& firstGivenVectorDB, std::vector<std::wstring>& secondGivenVectorDB, std::wstring firstGivenPath, std::wstring secondGivenPath)
{
    //Pre-determining hashActionVector size to reduce calls during for loop iterations.
    size_t hashActionSize = hashActions.size();
    //Holds output of hashAction reading, allows for manipulation.
    std::wstring currentReadLine;

    std::wstring directoryOneFile;
    std::wstring directoryOneVectorLocation;
    std::wstring directoryTwoVectorLocation;
    std::wstring directoryTwoFile;
    std::wstring dirOneHash;
    std::wstring dirTwoHash;





    //Hash actions. - Multi-threaded.
    //processingThreads[1] = std::thread(hashGivenFile, directoryOneFile, std::ref(dirOneHash));
    //processingThreads[2] = std::thread(hashGivenFile, directoryTwoFile, std::ref(dirTwoHash));
    //std::wstring fileToHash;
    //std::wstring vectorLocation;
    //bool directoryOne = true;
    //bool threadControl = true;
    //int threadIterator = 0;
    //for (int iterator = 0; iterator < hashActionSize; ++iterator)
    //{
    //    if (directoryOne)
    //    {

    //        currentReadLine = hashActions[iterator]; //Reading the full line.
    //        directoryOneFile = firstGivenPath + L"\\" + currentReadLine.substr(0, nthOccurrence(currentReadLine, delimitingCharacter, 1)); //Up until first delimiter, adding first dir path.
    //        vectorLocation = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 1) + 1, nthOccurrence(currentReadLine, delimitingCharacter, 2)); //Between first and second delimiter.
    //    }
    //    else
    //    {
    //        currentReadLine = hashActions[iterator]; //Reading the full line.
    //        fileToHash = secondGivenPath + L"\\" + currentReadLine.substr(0, nthOccurrence(currentReadLine, delimitingCharacter, 1)); //Up until first delimiter, adding second dir path.
    //        vectorLocation = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 2) + 1, currentReadLine.length() - 1); //Going up until newline.
    //    }

    //    processingThreads[threadIterator] = std::thread(hashGivenFile, fileToHash, std::ref(dirOneHash)); //Assign task to thread.

    //    threadControl = true;
    //    while (threadControl)
    //    {
    //        for (int threadIterator = 0; threadIterator <= threadCount; ++threadIterator)
    //        {
    //            if (processingThreads[threadIterator].joinable()) //Check if joinable.
    //            {
    //                processingThreads[threadIterator].join(); //Join the thread.
    //                break; //Leave for loop and 
    //            }
    //        }
    //    }
    //}



    //Hash actions. - Single threaded.
    for (int iterator = 0; iterator < hashActionSize; ++iterator)
    {

        //Grab item...
        currentReadLine = hashActions[iterator]; //Reading the full line.
        directoryOneFile = firstGivenPath + L"\\" + currentReadLine.substr(0, nthOccurrence(currentReadLine, delimitingCharacter, 1)); //Up until first delimiter, adding first dir path.
        directoryTwoFile = secondGivenPath + L"\\" + currentReadLine.substr(0,nthOccurrence(currentReadLine, delimitingCharacter, 1)); //Up until first delimiter, adding second dir path.
        directoryOneVectorLocation = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 1) + 1, nthOccurrence(currentReadLine, delimitingCharacter, 2)); //Between first and second delimiter.
        directoryTwoVectorLocation = currentReadLine.substr(nthOccurrence(currentReadLine, delimitingCharacter, 2) + 1, currentReadLine.length()-1); //Going up until newline.
        
        hashGivenFile(directoryOneFile, dirOneHash);
        hashGivenFile(directoryTwoFile, dirTwoHash);

        firstGivenVectorDB[_wtoi(directoryOneVectorLocation.c_str())].insert(nthOccurrence(firstGivenVectorDB[_wtoi(directoryOneVectorLocation.c_str())], delimitingCharacter, 4) + 1, dirOneHash);
        secondGivenVectorDB[_wtoi(directoryTwoVectorLocation.c_str())].insert(nthOccurrence(secondGivenVectorDB[_wtoi(directoryTwoVectorLocation.c_str())], delimitingCharacter, 4) + 1, dirTwoHash);
    }
}
void pushBackToVector(std::vector<std::wstring>& givenVector, std::wstring givenMessage)
{
    givenVector.push_back(givenMessage);
}
void insertMatchInfoIntoVector(std::vector<std::wstring>& givenVector, size_t vectorItem, std::wstring matchingLine)
{
    givenVector[vectorItem].insert(givenVector[vectorItem].length() - 1, L"MATCHED" + delimitingCharacter + matchingLine);
}