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
	size_t workingSize;
	size_t workingSizeTwo;
	size_t workingDateMod;
	size_t workingDateModTwo;
	size_t modifyWindow = argumentVariables["internalObject"]["Modify Window"].get<size_t>();
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

			iter1 = std::to_string(iterator); //Convert the line of DB1 line to a string for saving.
			iter2 = std::to_string(DB2Line); //Convert newly found DB2 line to a string for saving.
			firstGivenVectorDB[iterator].insert(firstGivenVectorDB[iterator].length() - 1, "MATCHED" + delimitingCharacter + iter2); //Add match marker and line.
			secondGivenVectorDB[DB2Line].insert(secondGivenVectorDB[DB2Line].length() - 1, "MATCHED" + delimitingCharacter + iter1); //Add match marker and line.

			if (std::filesystem::is_directory(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)))) //If the path not a directory, skip the iteration.
				continue;

			workingDateMod = stoull(firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 3) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - 1)); //Third column
			workingDateModTwo = stoull(secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) + 1, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 3) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - 1)); //Third column
			if (workingDateMod % workingDateModTwo <= modifyWindow || workingDateModTwo % workingDateMod <= modifyWindow) //If the file paths match, check the last modified times.
			{
				workingSize = stoull(firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) + 1, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - 1)); //Second column
				workingSizeTwo = stoull(secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) + 1, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) - 1)); //Second column
				if (workingSize == workingSizeTwo) //Check if the file sizes match.
				{
					if (argumentVariables["internalObject"]["Check File Contents"].get<bool>()) 
						hashActions.push_back(workingPath + delimitingCharacter + iter1 + delimitingCharacter + iter2 + newLine); //If everything matches, these files need hashed and compared.
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

	//Column variables to avoid streamline calls on vectors.
	std::string workingPath;
	std::string workingPathTwo;
	size_t workingSize;
	size_t workingSizeTwo;
	size_t workingDateMod;
	size_t workingDateModTwo;
	size_t modifyWindow = argumentVariables["internalObject"]["Modify Window"].get<size_t>();
	std::string workingHash;
	std::string workingHashTwo;

	//Iterating through directory two vector and inserting the relative/working file path and line location into the unordered map.
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

			//Adding corresponding line number and a "MATCHED" marker for future use.
			firstGivenVectorDB[iterator].insert(firstGivenVectorDB[iterator].length() - 1, "MATCHED" + delimitingCharacter + std::to_string(DB2Line));
			secondGivenVectorDB[DB2Line].insert(secondGivenVectorDB[DB2Line].length() - 1, "MATCHED" + delimitingCharacter + std::to_string(iterator));

			//If the path not a directory, skip this iteration. This prevents errors trying to get data that isn't present in the DBs.
			if (std::filesystem::is_directory(std::filesystem::u8path(firstGivenVectorDB[iterator].substr(0, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1)))))
				continue;

			workingDateMod = stoull(firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) + 3, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 3) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - 3)); //Third column
			workingDateModTwo = stoull(secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) + 3, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 3) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - 3)); //Third column
			if (workingDateMod % workingDateModTwo <= modifyWindow || workingDateModTwo % workingDateMod <= modifyWindow)
			{
				workingSize = stoull(firstGivenVectorDB[iterator].substr(nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) + 3, nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 2) - nthOccurrence(firstGivenVectorDB[iterator], delimitingCharacter, 1) - 3)); //Second column
				workingSizeTwo = stoull(secondGivenVectorDB[DB2Line].substr(nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) + 3, nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 2) - nthOccurrence(secondGivenVectorDB[DB2Line], delimitingCharacter, 1) - 3)); //Second column
				if (workingSize == workingSizeTwo)
				{
					if (argumentVariables["internalObject"]["Check File Contents"].get<bool>())
						hashActions.push_back(workingPath + delimitingCharacter + std::to_string(iterator) + delimitingCharacter + std::to_string(DB2Line) + newLine); //If everything matches, these files need hashed and compared.
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
		//Checking for a match.
		if (secondGivenVectorDB[iterator].substr(nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 5) + 3, 7) != "MATCHED")
			fileOpAction.push_back("DELETE - No source found" + delimitingCharacter + secondGivenVectorDB[iterator].substr(0, nthOccurrence(secondGivenVectorDB[iterator], delimitingCharacter, 1)) + newLine); //Delete directory two file. No directory one file found that matches.
	}
}
