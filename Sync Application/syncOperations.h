#pragma once


//Uses unordered maps to compare directory lists and find matches.
void contributeCompareDirectories(std::vector<std::string>& firstGivenVectorDB, std::vector<std::string>& secondGivenVectorDB, std::vector<std::string>& hashActions, std::vector<std::string>& fileOpAction, std::string firstGivenPath, std::string secondGivenPath)
{
    //Storing ending iterator (vector size). Every "for loop" call will ask the vector for it's size otherwise. - https://articles.emptycrate.com/2008/10/26/c_loop_optimization.html
    size_t firstDBSize = firstGivenVectorDB.size();
    size_t secondDBSize = secondGivenVectorDB.size();


    //Creating unordered maps.
    std::unordered_map<std::string, size_t> DB2Map;

    size_t DB2Line; //DB1Map searching in DB2Map provides the line of DB2Match.

    //Holds 
    std::string iter1;
    std::string iter2;


    //Column variables to avoid streamline calls on vectors.
    std::string workingPath;
    std::string workingPathTwo;
    std::string workingSize;
    std::string workingSizeTwo;
    std::string workingDateMod;
    std::string workingDateModTwo;
    std::string workingHash;
    std::string workingHashTwo;

    //Iterating through directory two vector and inserting the file path and line location into the unordered map.
    //Key: path | Value: line location
    for (size_t iteratorTwo = 0; iteratorTwo < secondDBSize; ++iteratorTwo)
        DB2Map.insert(std::make_pair(secondGivenVectorDB[iteratorTwo].substr(secondGivenPath.length() + 1, nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 1) - secondGivenPath.length() - 1), iteratorTwo));

    //Iterate through firstDB.
    for (size_t iterator = 0; iterator < firstDBSize; ++iterator)
    {
        workingPath = firstGivenVectorDB[iterator].substr(firstGivenPath.length() + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - firstGivenPath.length() - 1); //Get path of object.

        if (DB2Map.count(workingPath)) //Search for match in DB2. This value can only return 0 or 1 as unordered maps cannot hold duplicate keys.
        {
            DB2Line = DB2Map[workingPath]; //Save the value.

            iter1 = iterator; //Convert the line of DB1 line to a string for saving.
            iter2 = DB2Line; //Convert newly found DB2 line to a string for saving.
            firstGivenVectorDB[iterator].insert(firstGivenVectorDB[iterator].length() - 1, "MATCHED" + delimitingCharacter + iter2); //Add match marker and line.
            secondGivenVectorDB[DB2Line].insert(secondGivenVectorDB[DB2Line].length() - 1, "MATCHED" + delimitingCharacter + iter1); //Add match marker and line.

            if (std::filesystem::is_directory(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)))) //If the path not a directory, skip the iteration.
                continue;

            workingDateMod = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 3) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - 1); //Third column
            workingDateModTwo = secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) + 1, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 3) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - 1); //Third column
            if (workingDateMod == workingDateModTwo) //If the file paths match, check the last modified times.
            {
                workingSize = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - 1); //Second column
                workingSizeTwo = secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) + 1, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) - 1); //Second column
                if (workingSize == workingSizeTwo) //Check if the file sizes match.
                {
                    if (argumentVariables["internalObject"]["Check File Contents"].get<bool>()) hashActions.push_back(workingPath + delimitingCharacter + iter1 + delimitingCharacter + iter2 + newLine); //If everything matches, these files need hashed and compared.
                }
                else
                    fileOpAction.push_back("COPY - Different file sizes" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy directory one file to directory two.
            }
            else //A matching file has been found, with differing last modified times.
                fileOpAction.push_back("COPY - Different last modified time" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy directory one file to directory two.
        }
        else
        {
            if (std::filesystem::is_directory(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1))) && std::filesystem::is_empty(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1))))
                fileOpAction.push_back("COPY - Empty directory present on source" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy directory one file to directory two.
            else
                fileOpAction.push_back("COPY - No destination found" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy directory one file to directory two.
        }
    }

    DB2Map.clear();

    //If matching files need to be hashed, do so.
    if (argumentVariables["internalObject"]["Check File Contents"].get<bool>())
    {
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Beginning hash process. " + std::to_string(hashActions.size() * 2) + " Files to be hashed..."); //FIX ME
        performHashActionFile(hashActions, firstGivenVectorDB, secondGivenVectorDB, firstGivenPath, secondGivenPath);
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Hashing finished!");
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Comparing file hashes...");
        compareHashes(firstGivenVectorDB, secondGivenVectorDB, fileOpAction, firstGivenPath, secondGivenPath);
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Hash comparison finished!"); //
    }


}


//Uses unordered maps to compare directory lists and find matches.
void echoCompareDirectories(std::vector<std::string>& firstGivenVectorDB, std::vector<std::string>& secondGivenVectorDB, std::vector<std::string>& hashActions, std::vector<std::string>& fileOpAction, std::string firstGivenPath, std::string secondGivenPath)
{
    //Storing ending iterator (vector size). Every "for loop" call will ask the vector for it's size otherwise. - https://articles.emptycrate.com/2008/10/26/c_loop_optimization.html
    size_t firstDBSize = firstGivenVectorDB.size();
    size_t secondDBSize = secondGivenVectorDB.size();


    //Creating unordered maps.
    std::unordered_map<std::string, size_t> DB2Map;

    size_t DB2Line; //DB1Map searching in DB2Map provides the line of DB2Match.

    //Holds 
    std::string iter1;
    std::string iter2;

    //Column variables to avoid streamline calls on vectors.
    std::string workingPath;
    std::string workingPathTwo;
    std::string workingSize;
    std::string workingSizeTwo;
    std::string workingDateMod;
    std::string workingDateModTwo;
    std::string workingHash;
    std::string workingHashTwo;

    //Iterating through directory two vector and inserting the file path and line location into the unordered map.
    //Key: path | Value: line location
    for (size_t iteratorTwo = 0; iteratorTwo < secondDBSize; ++iteratorTwo)
        DB2Map.insert(std::make_pair(secondGivenVectorDB[iteratorTwo].substr(secondGivenPath.length() + 1, nthOccurrence(secondGivenVectorDB[iteratorTwo], delimitingCharacter, 1) - secondGivenPath.length() - 1), iteratorTwo));

    //Iterate through firstDB.
    for (size_t iterator = 0; iterator < firstDBSize; ++iterator)
    {
        workingPath = firstGivenVectorDB[iterator].substr(firstGivenPath.length() + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - firstGivenPath.length() - 1); //Get path of object.

        if (DB2Map.count(workingPath)) //Search for match in DB2. This value can only return 0 or 1 as unordered maps cannot hold duplicate keys.
        {
            DB2Line = DB2Map[workingPath]; //Save the value.

            iter1 = iterator; //Convert newly found DB1 line to a string for saving.
            iter2 = DB2Line; //Convert newly found DB2 line to a string for saving.
            firstGivenVectorDB[iterator].insert(firstGivenVectorDB[iterator].length() - 1, "MATCHED" + delimitingCharacter + iter2); //Add match marker and line.
            secondGivenVectorDB[DB2Line].insert(secondGivenVectorDB[DB2Line].length() - 1, "MATCHED" + delimitingCharacter + iter1); //Add match marker and line.

            if (std::filesystem::is_directory(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)))) //If the path not a directory, skip the iteration.
                continue;

            workingDateMod = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 3) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - 1); //Third column
            workingDateModTwo = secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) + 1, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 3) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - 1); //Third column
            if (workingDateMod == workingDateModTwo) //If the file paths match, check the last modified times.
            {
                workingSize = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - 1); //Second column
                workingSizeTwo = secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) + 1, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) - 1); //Second column

                if (workingSize == workingSizeTwo) //Check if the file sizes match.
                {
                    if (argumentVariables["internalObject"]["Check File Contents"]) hashActions.push_back(workingPath + delimitingCharacter + iter1 + delimitingCharacter + iter2 + newLine); //If everything matches, these files need hashed and compared.
                }
                else
                    fileOpAction.push_back("COPY - Different file sizes" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy directory one file to directory two.
            }
            else //A matching file has been found, with differing last modified times.
                fileOpAction.push_back("COPY - Different last modified time" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy directory one file to directory two.
        }
        else
        {
            if (std::filesystem::is_directory(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1))) && std::filesystem::is_empty(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1))))
                fileOpAction.push_back("COPY - Empty directory present on source" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy directory one file to directory two.
            else
                fileOpAction.push_back("COPY - No destination found" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy directory one file to directory two.
        }
    }

    DB2Map.clear();


    writeConsoleMessagesPool.push_task(displayConsoleMessage, "Checking for directory two items that did not get matched..."); //***

    //Iterating through directory two list and checking against directory one list, comparing the matched values.
    for (size_t iterator = 0; iterator < secondDBSize; ++iterator)
    {
        if (secondGivenVectorDB[iterator].substr(nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 5) + 1, 7) != "MATCHED")
            fileOpAction.push_back("DELETE - No source found" + delimitingCharacter + secondGivenVectorDB[iterator].substr(0, nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 1)) + newLine); //Delete directory two file. No directory one file found that matches.
    }

    //If matching files need to be hashed, do so.
    if (argumentVariables["internalObject"]["Check File Contents"])
    {
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Beginning hash process. " + std::to_string(hashActions.size() * 2) + " Files to be hashed...");
        performHashActionFile(hashActions, firstGivenVectorDB, secondGivenVectorDB, firstGivenPath, secondGivenPath);
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Hashing finished!");
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Comparing file hashes...");
        compareHashes(firstGivenVectorDB, secondGivenVectorDB, fileOpAction, firstGivenPath, secondGivenPath);
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Hash comparison finished!"); //
    }


}

//Uses unordered maps to compare directory lists and find matches.
void synchronizeCompareDirectories(std::vector<std::string>& firstGivenVectorDB, std::vector<std::string>& secondGivenVectorDB, std::vector<std::string>& hashActions, std::vector<std::string>& fileOpAction, std::string firstGivenPath, std::string secondGivenPath)
{
    //Storing ending iterator (vector size). Every "for loop" call will ask the vector for it's size otherwise. - https://articles.emptycrate.com/2008/10/26/c_loop_optimization.html
    size_t firstDBSize = firstGivenVectorDB.size();
    size_t secondDBSize = secondGivenVectorDB.size();


    //Creating unordered maps.
    std::unordered_map<std::string, size_t> DB2Map;

    size_t DB2Line; //DB1Map searching in DB2Map provides the line of DB2Match.

    //Holds 
    std::string iter1;
    std::string iter2;


    //Column variables to avoid streamline calls on vectors.
    std::string workingPath;
    std::string workingPathTwo;
    std::string workingSize;
    std::string workingSizeTwo;
    std::string workingDateMod;
    std::string workingDateModTwo;
    std::string workingHash;
    std::string workingHashTwo;

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

            iter1 = iterator; //Convert newly found DB1 line to a string for saving.
            iter2 = DB2Line; //Convert newly found DB2 line to a string for saving.
            firstGivenVectorDB[iterator].insert(firstGivenVectorDB[iterator].length() - 1, "MATCHED" + delimitingCharacter + iter2); //Add match marker and line.
            secondGivenVectorDB[DB2Line].insert(secondGivenVectorDB[DB2Line].length() - 1, "MATCHED" + delimitingCharacter + iter1); //Add match marker and line.

            if (std::filesystem::is_directory(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)))) //If the path is a directory, then skip the iteration.
                continue;

            workingDateMod = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 3) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - 1); //Third column
            workingDateModTwo = secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) + 1, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 3) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - 1); //Third column
            if (workingDateMod == workingDateModTwo) //Check if the last modified times are equal.
            {
                workingSize = firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - 1); //Second column
                workingSizeTwo = secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) + 1, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) - 1); //Second column
                if (workingSize == workingSizeTwo) //Check if the file sizes match.
                {
                    if (argumentVariables["internalObject"]["Check File Contents"].get<bool>()) hashActions.push_back(workingPath + delimitingCharacter + iter1 + delimitingCharacter + iter2 + newLine); //If everything matches, these files need hashed and compared.
                }
                else //If the file sizes are different, we don't know which is newer. Alert the user.
                {
                    //*****
                    //Output conflict file.
                }
            }
            else if (workingDateMod > workingDateModTwo) //If the directory one file is newer than the directory two file.
                fileOpAction.push_back("COPY - Directory one file has a newer last modified time" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy directory one file to directory two.
            else //The directory two file must be the newer file. Copy it to directory one.
                fileOpAction.push_back("COPY - Directory two file has a newer last modified time" + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + newLine); //Copy directory two file to directory one.
        }
        else //No matching file from directory one found in directory two. Copy the file over to directory two.
        {
            if (std::filesystem::is_directory(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1))) && std::filesystem::is_empty(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1))))
                fileOpAction.push_back("COPY - Empty directory present on directory one and not directory two" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy first directory file to second directory.
            else
                fileOpAction.push_back("COPY - No match from directory one found in directory two" + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + newLine); //Copy first directory file to second directory.
        }
    }

    DB2Map.clear(); //Clear the map to save some memory.


    writeConsoleMessagesPool.push_task(displayConsoleMessage, "Checking for directory two items that did not get matched...");

    //Iterating through directory two list and checking against matched list.
    for (size_t iterator = 0; iterator < secondDBSize; ++iterator) //Needs to start at "1" if headers are added before this.
    {
        if (secondGivenVectorDB[iterator].substr(nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 5) + 1, 7) != "MATCHED")
        {
            //Getting workingPath
            workingPath = secondGivenVectorDB[iterator].substr(0, nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 1));
            workingPath = workingPath.erase(workingPath.find(secondGivenPath), secondGivenPath.length() + 1);

            if (std::filesystem::is_directory(secondGivenVectorDB[iterator].substr(0, nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 1))) && std::filesystem::is_empty(secondGivenVectorDB[iterator].substr(0, nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 1))))
                fileOpAction.push_back("COPY - Empty directory present on directory two and not directory one" + delimitingCharacter + secondGivenPath + directorySeparator + workingPath + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + newLine); //Copy empty directory found in directory two that is not present in directory one.
            else
                fileOpAction.push_back("COPY - No match from directory two found in directory one" + delimitingCharacter + secondGivenVectorDB[iterator].substr(0, nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 1)) + delimitingCharacter + firstGivenPath + directorySeparator + workingPath + newLine); //Copy file from directory two that is not present in the directory one.
        }
    }


    //If matching files need to be hashed, do so.
    if (argumentVariables["internalObject"]["Check File Contents"].get<bool>())
    {
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Beginning hash process. " + std::to_string(hashActions.size() * 2) + " Files to be hashed...");
        performHashActionFile(hashActions, firstGivenVectorDB, secondGivenVectorDB, firstGivenPath, secondGivenPath);
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Hashing finished!");
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Comparing file hashes...");
        compareHashes(firstGivenVectorDB, secondGivenVectorDB, fileOpAction, firstGivenPath, secondGivenPath);
        writeConsoleMessagesPool.push_task(displayConsoleMessage, "Hash comparison finished!");
    }
}
