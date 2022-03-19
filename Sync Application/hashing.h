#pragma once


//Converts char array of MD5 value to a "human readable" hex string.
std::string convertMD5ToHex(unsigned char* givenDigest)
{
    //Create and zero out buffer to hold hex output. - 32 characters given.
    char hexBuffer[32];
    memset(hexBuffer, 0, 32);

    //Creating string to hold final hash output.
    std::string outputHexString;

    //Convert the 128-bit hash to hex.
    for (size_t i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(hexBuffer, "%02x", givenDigest[i]);
        outputHexString.append(hexBuffer);
    }

    return outputHexString; //Output hash
}

//Multithreadable hash given file that also handles writing the result to the givenVector.
//Asks for the path to the file to hash, a vector to store output in, and line location in that vector to modify.
void MThashGivenFile(std::string givenFilePath, std::vector<std::string>& givenVector, std::string lineLocation)
{
    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, "HASHING STARTED: " + givenFilePath);

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
        writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, "ERROR - FAIL FLAG BEFORE HASHING: " + givenFilePath);
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
            writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, "ERROR - BAD FLAG DURING HASHING: " + givenFilePath);
            return;
        }

    }

    //Finish the MD5, place it in the digest.
    size_t MD5Result = MD5_Final(digest, &md5Context);

    //Verify hash completed successfully.
    if (MD5Result == 0) // Hash failed.
        std::cout << "HASH FAILED. 0 RETURNED." + givenFilePath << std::endl;

    //Close file.
    fileHandle.close();

    writeDebugThreadPool.push_task(writeToDebug, std::chrono::system_clock::now(), true, "HASHING COMPLETED: " + givenFilePath);

    //Write the result to the vector.
    givenVector[std::stoi(lineLocation.c_str())].insert(nthOccurrence(givenVector[std::stoi(lineLocation.c_str())], delimitingCharacter, 4) + 1, convertMD5ToHex(digest));
    return;
}
