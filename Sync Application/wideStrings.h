#pragma once

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