#pragma once


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
    if (argumentVariables["internalObject"]["Recursive Search"])
    {
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
                //writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"2 Is not a directory: " + formatFilePath(current_file));
                testStream << std::filesystem::file_size(current_file) << delimitingCharacter;
                //writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"3 Got File Size: " + formatFilePath(current_file));
                //Getting last modified time. In seconds from 1970 EPOCH format.
                lastModifiedTime = boost::filesystem::last_write_time(current_file);
                testStream << lastModifiedTime << delimitingCharacter;

                //writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"4 Got last write time: " + formatFilePath(current_file));
                //Getting date created time. In seconds from 1970 EPOCH format.
                dateCreatedTime = boost::filesystem::creation_time(current_file);
                //writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"5 Got creation time: " + formatFilePath(current_file));
                testStream << dateCreatedTime << delimitingCharacter + delimitingCharacter + newLine; //Adding an additional delimiter, since the hash is not added yet but the matching stuff will be.

                //writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"6 Data stored for file: " + formatFilePath(current_file));
            }
            else if (std::filesystem::is_directory(current_file))
                testStream << delimitingCharacter + delimitingCharacter + delimitingCharacter + delimitingCharacter + newLine;


            //writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"7 Pushing data back: " + formatFilePath(current_file));
            //Append to DB.
            givenVectorDB.push_back(testStream.str());
            //writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"8 Reseting streamstring: " + formatFilePath(current_file));
            //Clearing stringstream for next iteration.
            testStream.str(std::wstring());
            //writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"9 finished streamstring: " + formatFilePath(current_file));

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
    std::wstring destinationDirectoriesPath = givenDestinationPath.substr(0, givenDestinationPath.find_last_of(directorySeparator) + 1); //Obtaining path of the destination until the last backslash.

    if (!std::filesystem::exists(destinationDirectoriesPath)) //If the directory does not exist, then create it.
        std::filesystem::create_directories(destinationDirectoriesPath);

    while (!std::filesystem::exists(destinationDirectoriesPath)) //Wait for directories to be created.
    {
        //V O I D
    }

    //*****
    if (!std::filesystem::is_directory(givenDestinationPath)) //Don't bother dealing with directories right now.
    {
        std::error_code ec; //Create error handler.
        std::filesystem::copy(givenSourcePath, givenDestinationPath, std::filesystem::copy_options::overwrite_existing, ec); //Copying the file. - If a directory is being looked at, it would have already been made above. This will do nothing.

        if (ec.value() == 5) //If error value is 5, it is access denied.
            writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"ERROR. ACCESS DENIED: " + givenSourcePath + L" - " + givenDestinationPath); //Log.
    }

}

//Moving file.
void moveFile(std::wstring givenSourcePath, std::wstring givenDestinationPath)
{
    //Creating files themselves.
    std::wstring destinationDirectoriesPath = givenDestinationPath.substr(0, givenDestinationPath.find_last_of(directorySeparator) + 1); //Obtaining path of the destination until the last backslash.

    if (!std::filesystem::exists(destinationDirectoriesPath)) //If the directory does not exist, then create it.
        std::filesystem::create_directories(destinationDirectoriesPath);

    while (!std::filesystem::exists(destinationDirectoriesPath)) //Wait for directories to be created.
    {
        //V O I D
    }

    //*****
    if (!std::filesystem::is_directory(givenDestinationPath)) //Don't bother dealing with directories right now
    {
        std::error_code ec; //Create error handler.
        std::filesystem::copy(givenSourcePath, givenDestinationPath, std::filesystem::copy_options::overwrite_existing, ec); //Copying the file. - If a directory is being looked at, it would have already been made above. This will do nothing.

        if (ec.value() == 5) //If error value is 5, it is access denied.
            writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, L"ERROR. ACCESS DENIED: " + givenSourcePath + L" - " + givenDestinationPath); //Log.

        removeObject(givenSourcePath, false); //Now that the file is copied, remove the source file.
    }


}
