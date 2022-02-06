# Sync Application
(No. The name is not finalized.)

- [Description](#description)
- [Requirements](#requirements)
- [Installation](#installation)
- [Options](#options)
- [Website](#website)
- [Contact](#contact)

# Description
Sync Application is a command-line driven project created out of frustration with other free file backup tools. I found that other free tools wouldn't consistently copy files, or folders, and weren't byte-for-byte copies of the data I provided. I was also dissatisfied by the performance of these tools, with backups sometimes taking 12+ hours when dealing with only ~500GB of data.
Additionally, I found that some features that I desired simply did not exist.
    I will eventually be adding those features to this application.


This is a passion tool, and it's only a plus if others can find some use from it.

# Requirements
This application is compiled to include as many dependencies as possible.
As of right now, I am aware of issues running on machines that do not have the [vc_redist_x64 redistributable package](https://docs.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170).

# Installation
You can download the pre-configured release binaries from this GitHub page. As of right now, they are only verified to work on Windows 10 with limited testing.
You can also compile your own binaries by downloading the source files, opening the .sln file, and building it.
(Eventually, I will provide a better in-depth walk-through.)

# Options
"Sync Application.exe" [OPTIONS] -o <OPERATION_MODE> -s <DIRECTORY_PATH> -d <DIRECTORY_PATH>
## List of Arguments
(An asterisk indicates a required input)

**NOTE**: *Paths should NOT end with a back slash ("\\") as quotation marks could be escaped. Usage of forward slashes ("/") is encouraged and what is used internally to avoid conflicts.*
```
--check-content(s)
* --directory-one <DIRECTORY_PATH>
* --directory-two <DIRECTORY_PATH>
-h - Displays a short help message.
--help - Displays extended help.
--hide-console - Hides console.
-l - Bypasses Windows MAX_PATH limit of 260 characters. It appends "\\?\", which requires utilizing backslashes for directory separators in the backend. (Thanks, Windows!)
* --operation-mode <OPERATION_MODE> | Operation Mode is defined as a string. The available options being "echo", "sync" or "synchronize", and "cont" or "contribute".
--no-recursive <OPERATION_INTEGER> | Defines where a recursive process should not be used. Represented with an integer: 0 being no recursive on neither, 1 being not on directory-one, 2 being no on directory-two.
--no-warning | Disables warning that explains the operation and outlines what files are potentially at risk. This should probably only be used when automating the run process.
--output-files | Dumps all internal database vectors to .log files in the same directory as the application.
--output-verbose-debug <OUTPUT_LOCATION> | Outputs log as the program runs to assist with debugging. If a log is present, new data is appended to prevent debugging data loss.
--perform-sync-conflict-resolution <RESOLUTION_FILEPATH> | When a sync operation encounters conflicts that it cannot resolve, a conflict file is created that shows the 
```
## Sync Operations

### Contribute (Cont)
Contribute will copy any new changes from directory one to directory two.  
No deletions will NEVER occur within directory two.  
Renaming a file on directory one will result in a new copy of the file within directory two.  
A modified file on directory one will cause the matching file on directory two to be overwritten.
### Echo
Echo will echo any changes made in directory one to directory two.  
Changes will only occur within directory two.  

### Synchronize (Sync)
Synchronize will take the newest version from a directory and copy it to the other directory.  
This results in deletions and changes within BOTH directories.  
When a conflict is encountered that it cannot resolve the program will output a "conflicts file" that shows which provides a list of file pairs that need to be manually decided.  
The conflicts can be resolved by removing one of the two paths. **The path kept is assumed to be the newer file to keep. The other file will be overwritten.**  
Conflicts occur when the last modified times are identical, but the file size or hashes differ. There is no way to know which version is the "new" one, and thus no safe changes can be made.  
Once conflicts have been resolve, you can utilize the "--perform-sync-conflict-resolution <RESOLUTION_FILEPATH>" argument to perform the changes. The "--directory-one <DIRECTORY_PATH>" and "--directory-two <DIRECTORY_PATH>" arguments are still required.  

# Contact
Please reach out if you run into any bugs or issues. - jadinheaston@jadinheaston.com  
I can not promise that I will implement anything, or respond, but I am more likely to if I am aware of the problem to begin with.

## [Website](https://www.jadinheaston.com/)
Eventually, all information regarding this project will also be available on my website.
