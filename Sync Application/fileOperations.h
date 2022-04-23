#pragma once

//Creates a list of all files and directories within a given directory. Places each entry in a delimited format into the given wstring vector.
void createDirectoryMapDB(std::vector<std::string>& givenVectorDB, std::string givenStartPath, bool recursiveSearch)
{
	//Making the given path an actual usable path. idk why
	std::filesystem::path dirPath(givenStartPath);
	std::filesystem::directory_iterator end_itr;


	//Creating a stringstream to hold the file size
	std::stringstream temporaryStringStreamFileAndDirSearch;

	std::string current_file;
	std::stringstream testStream;

	std::filesystem::file_time_type lastModifiedTime;


	//Checking whether to search recursively or not
	if (recursiveSearch)
	{
		//RECURSIVE
		for (std::filesystem::recursive_directory_iterator end, dir(givenStartPath); dir != end; ++dir)
		{
			//Setting current file equal to the full path of the file.
			current_file = dir->path().u8string();

			//Putting path into array.
			testStream << formatFilePath(current_file) << delimitingCharacter;

			// If it's not a directory, list it. If you want to list directories too, just remove this check.
			if (std::filesystem::is_regular_file(std::filesystem::u8path(current_file)))
			{
				testStream << std::filesystem::file_size(std::filesystem::u8path(current_file)) << delimitingCharacter;
				
				//Getting last modified time. In 'seconds from 1970 EPOCH' format.
				lastModifiedTime = std::filesystem::last_write_time(std::filesystem::u8path(current_file));
				testStream << std::chrono::duration_cast<std::chrono::seconds>(lastModifiedTime.time_since_epoch()).count() << delimitingCharacter;
				
				testStream << delimitingCharacter + delimitingCharacter + newLine;
			}
			else
				testStream << delimitingCharacter + delimitingCharacter + delimitingCharacter + delimitingCharacter + newLine;

			//Append to DB.
			givenVectorDB.push_back(testStream.str());

			//Clearing stringstream for next iteration.
			testStream.str(std::string());
		}
	}
	else
	{
		//NON-RECURSIVE
		// cycle through the directory
		for (std::filesystem::directory_iterator itr(givenStartPath); itr != end_itr; ++itr)
		{
			//Setting current file equal to the full path of the file.
			current_file = itr->path().u8string();

			//Putting path into array.
			testStream << formatFilePath(current_file) << delimitingCharacter;

			// If it's not a directory, list it. If you want to list directories too, just remove this check.
			if (std::filesystem::is_regular_file(std::filesystem::u8path(current_file)))
			{
				testStream << std::filesystem::file_size(std::filesystem::u8path(current_file)) << delimitingCharacter;
				
				//Getting last modified time. In 'seconds from 1970 EPOCH' format.
				lastModifiedTime = std::filesystem::last_write_time(std::filesystem::u8path(current_file));
				testStream << std::chrono::duration_cast<std::chrono::seconds>(lastModifiedTime.time_since_epoch()).count() << delimitingCharacter;
				
				testStream << delimitingCharacter + delimitingCharacter + newLine;
			}
			else
				testStream << delimitingCharacter + delimitingCharacter + delimitingCharacter + delimitingCharacter + newLine;

			//Append to DB.
			givenVectorDB.push_back(testStream.str());

			//Clearing stringstream for next iteration.
			testStream.str(std::string());
		}
	}

}

//Functions asks for a path to directory and will return number of files. - Also asks for a T/F bool determining whether a search should be recursive.
size_t countFiles(std::string pathToDir, bool recursiveLookup)
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
size_t countDir(std::string pathToDir, bool recursiveLookup)
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
void removeObject(std::string destinationFilePath, bool recursiveRemoval)
{
	if (!std::filesystem::exists(std::filesystem::u8path(destinationFilePath))) //Verify it is a normal file. I don't know what other types there are, but I'll avoid deleting them until I know.
		return;


	writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, "DELETING: " + destinationFilePath);


	std::error_code ec; //Create error handler.

	size_t count = 0;

	while (count < 10) //give up after 5 attempts.
	{
		if (recursiveRemoval) //Determine which remove method we are using.
			std::filesystem::remove_all(std::filesystem::u8path(destinationFilePath), ec); //Attempt to remove the object.
		else
			std::filesystem::remove(std::filesystem::u8path(destinationFilePath), ec); //Attempt to remove the object.


		if (ec.value() == 5) //If access is denied, sleep for a short period, change the permissions to allow writing.
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2500)); //Give a small pause to prevent actions overlapping.
			std::filesystem::permissions(std::filesystem::u8path(destinationFilePath), std::filesystem::perms::owner_write);
			count++;
		}
		else
			break;
	}

	//After 5 deletion attempts, the file still exists Notify user.
	if (ec.value() == 5 && std::filesystem::exists(std::filesystem::u8path(destinationFilePath))) //If error value is 5, it is access denied.
		writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, "ERROR. ACCESS DENIED. FILE NOT DELETED: " + destinationFilePath); //Log.
	
	ec.clear();
}

//Copying file.
void copyFile(std::string givenSourcePath, std::string givenDestinationPath)
{
	writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, "COPYING: " + givenSourcePath + " - " + givenDestinationPath); //Log.

	//Creating files themselves.
	std::string destinationDirectoriesPath = givenDestinationPath.substr(0, givenDestinationPath.find_last_of(directorySeparator) + 1); //Obtaining path of the destination until the last backslash.

	if (!std::filesystem::exists(std::filesystem::u8path(destinationDirectoriesPath))) //If the directory does not exist, then create it.
		std::filesystem::create_directories(std::filesystem::u8path(destinationDirectoriesPath));



	while (!std::filesystem::exists(std::filesystem::u8path(destinationDirectoriesPath))) //Wait for directories to be created.
	{
		//V O I D
	}


	std::error_code ec; //Create error handler.
	std::filesystem::copy(std::filesystem::u8path(givenSourcePath), std::filesystem::u8path(givenDestinationPath), std::filesystem::copy_options::overwrite_existing, ec); //Copying the file. - If a directory is being looked at, it would have already been made above. This will do nothing.
	
	if (ec.value() == 5) //If error value is 5, it is access denied.
		writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, "ERROR. ACCESS DENIED: " + givenSourcePath + " - " + givenDestinationPath); //Log.
}

//Moving file.
void moveFile(std::string givenSourcePath, std::string givenDestinationPath)
{
	//Creating files themselves.
	std::string destinationDirectoriesPath = givenDestinationPath.substr(0, givenDestinationPath.find_last_of(directorySeparator) + 1); //Obtaining path of the destination until the last backslash.

	if (!std::filesystem::exists(std::filesystem::u8path(destinationDirectoriesPath))) //If the directory does not exist, then create it.
		std::filesystem::create_directories(std::filesystem::u8path(destinationDirectoriesPath));

	while (!std::filesystem::exists(std::filesystem::u8path(destinationDirectoriesPath))) //Wait for directories to be created.
	{
		//V O I D
	}

	//*****
	if (!std::filesystem::is_directory(std::filesystem::u8path(givenDestinationPath))) //Don't bother dealing with directories right now
	{
		std::error_code ec; //Create error handler.
		std::filesystem::copy(std::filesystem::u8path(givenSourcePath), std::filesystem::u8path(givenDestinationPath), std::filesystem::copy_options::overwrite_existing, ec); //Copying the file.

		if (ec.value() == 5) //If error value is 5, it is access denied.
			writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, "ERROR. ACCESS DENIED: " + givenSourcePath + " - " + givenDestinationPath); //Log.

		removeObject(givenSourcePath, false); //Now that the file is copied, remove the source file.
	}


}


void importExternalFile(std::string sourceFilePath, std::vector<std::string>& destinationVector)
{
	//Check if path to the file is legitimate.
	//Check that the file can be read.

	//Wipe the destination vector, just to be safe.
	
	//Iterate through file lines and write it to the vector.
}

void exportVectorFile(std::vector<std::string>& givenVector, std::string destinationFilePath)
{
	std::string currentReadLine;
	size_t vectorSize = givenVector.size(); //Pre-calculating size.
	std::ofstream outputFileStream(destinationFilePath, std::ios::out | std::ios::binary);

	for (size_t iterator = 0; iterator < vectorSize; ++iterator)
	{
		currentReadLine = givenVector[iterator]; //Grab item
		writeToFile(outputFileStream, currentReadLine); //Write it
	}

	outputFileStream.close();
}