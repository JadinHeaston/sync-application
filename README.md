# Sync Application

(No. The name is not finalized.)

**RUST CONVERSION WiP - NOT SAFE TO USE**

Sync Application is a command-line driven project created out of frustration with other free file backup tools. I found that other free tools wouldn't consistently copy files, or folders, and weren't byte-for-byte copies of the data I provided. I was also dissatisfied by the performance of these tools, with backups sometimes taking 12+ hours when dealing with only ~500GB of data.  

Additionally, I found that some features that I desired simply did not exist.  
&nbsp;&nbsp;&nbsp;&nbsp;I will eventually be adding those features to this application.  

Ideally, you can run this program twice and see no changes occurring for the second run.

## Table of Contents

1. [Sync Application](#sync-application)
	1. [Table of Contents](#table-of-contents)
	2. [Requirements](#requirements)
	3. [Installation](#installation)
	4. [What's New?](#whats-new)
		1. [Features](#features)
		2. [Bug Fixes](#bug-fixes)
		3. [Future Plan](#future-plan)
	5. [Options](#options)
		1. [List of Arguments](#list-of-arguments)
		2. [Operations](#operations)
			1. [Contribute (Cont)](#contribute-cont)
			2. [Echo](#echo)
	6. [Configuration File](#configuration-file)
	7. [Sample Usage](#sample-usage)
	8. [Software Shout-outs](#software-shout-outs)
	9. [Licensing](#licensing)
		1. [Sync Application License](#sync-application-license)
		2. [What is required?](#what-is-required)
		3. [Other Used Software Licenses](#other-used-software-licenses)
	10. [Contact](#contact)
		 1. [Website](#website)

This is a passion project, and it's only a plus if others can find some use from it.  
My main focus is making something that works for myself, and learning new stuff as I go about it.  
<br/>
All future tasks are generally tracked within the [TODO.todo](TODO.todo) file.  

## Requirements

This application is compiled to include as many dependencies as possible.
As of right now, I am aware of issues running on machines that do not have the [vc_redist_x64 redistributable package](https://docs.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170).

## Installation

You can download the pre-configured release binaries from this GitHub page. As of right now, they are only verified to work on Windows 10 with limited testing.
You can also compile your own binaries by downloading the source files, opening the .sln file, and building it.
(Eventually, I will provide a better in-depth walk-through.)

## What's New?

### Features

* The synchronize operation was removed indefinitely.
  * It could return some day, but the primary focus of this application is to make backups easier. There are other utilities for synchronizing folders.
* [Configuration files](config.md).
  * Allows you to add, use, and export configurations from a JSON file.
  * You can also "clean" a configuration file and remove any properties that aren't in use using "--clean-config".
* Proper UTF-8 Support. Woo!
  * This may have also significantly decreased memory usage, but I can't confirm this.
* Changed how arguments are handled (twice). 
	* This mostly just effects the maintainability of the code.
	* The JSON config data-type is now used internally.
	* Single letter switches (when more are added) can now be specified like "-abc..." and work as intended.
* You can now specify where the internal database files are output to.
  * This is a small addition that moves the project closer to being able to output/input specific files.
* Recursive-ness can now be specified on a per-directory basis.
* Thread pool size can now be manually determined using "--threads"

### Bug Fixes

* If the program fails to remove a file/folder due to being read-only, it makes five attempts to change the permissions and remove the object.
* When going between NTFS and ExFAT, Last modification times were often being different due to implicit rounding (ExFAT rounds to the nearest 2 seconds sometimes). This has been remedied by adding the "--modify-window" argument, which allows you to specify how different the times can be before being marked as "different".
  * If you find that running the program consecutively copies the same files repeatedly, look into utilizing this argument.
* You can no longer provide a directory that is nested within the other specified directory.
  * I'm not entirely sure how this would act, but it's not necessary.
* Forward slashes were used internally when using the -l Windows Max Path Bypass. (So having "\\?\" at the start and forward slashes everywhere else). This caused issues and backslashes are now used internally, which seems to have fixed the issue.

### Future Plan

* Add individual file exporting/importing.
* Implement other protocols using libcurl (SFTP being one of the first.)
* Add other hashing algorithms.
  * I view MD5 as the best for this use-case and it will continue being the default. Despite that, I believe it would be easy to add a few others.

The [TODO](TODO.todo) file is what I use to track what needs to be done.

## Options

### List of Arguments

**NOTE**: *Paths should NOT end with a back slash ("\\") as quotation marks could be escaped. Usage of forward slashes ("/") is encouraged and what is used internally, when possible, to avoid conflicts like this.*
```
--add-to-config <PATH_TO_CONFIG_FILE>
	This requires a fully functional set of arguments, as well as a --name <NAME> to identify the configuration.
	This should also likely be used with the "--configuration-name" argument to avoid having gross numbered configurations.
	
--check-content(s)
	Enables hashing of files when the last modified time, relative location, and size are all the same.

--clean-config <PATH_TO_CONFIG_FILE>
	Note: Please create a backup your configuration prior to doing this.
	Removes all properties that are not legitimate. This can be useful to do when using a newer version of this software as arguments change over time.

--configuration-name <NAME>
	This is only necessary when using the "--add-to-configuration" argument.
	Provides a configuration ID that is used for using this configuration.
	If a configuration name is not provided, then a number ID will be assigned one higher than the largest number ID found. 

--directory-one <DIRECTORY_PATH>
	The first directory. ("Left")
--directory-two <DIRECTORY_PATH>
	The second directory. ("Right")

-h <ARGUMENT>
	Displays a short help message about the argument provided.

--help
	Displays extended help. This will likely end up being this document section.

--hide-console
	Hides console.

-l
	Bypasses Windows MAX_PATH limit of 260 characters. It appends "\\?\", which requires utilizing backslashes for directory separators in the backend. (Thanks, Windows!)

--list-config <PATH_TO_CONFIG_FILE>
	Lists all configurations present in the provided configuration file.
	A configuration name can be provided with this to output the configuration requested.

--no-file-operations
	Skips the file operation stage.

--no-recursive-one/two
	Defines where a recursive process should NOT be used for the respective directory.
	Add either "one" or "two" at the end, to correspond with which directory shouldn't be recursive.
	You can either use "--no-recursive-one" or "--no-recursive-two".

--no-warning
	Disables the warning that explains the operation and outlines what files are potentially at risk. This should probably only be used when automating the run process.

--modify-window <SECONDS>
	Determines how different last modified times can be before being marked as different.
	File systems store times differently, so transferring a file from NTFS to ExFAT typically results in the ExFAT time getting rounded up to the nearest 2 seconds (even number).
	Use this setting if you find that consecutively running the program results in the copying of the same files.
	(This works identically to rsync)

--operation-mode <OPERATION_MODE>
	Operation Mode specifies how the program decides which files go where. The available options being "contribute" or "cont", and "echo".

--output-files
	Dumps all internal database vectors to .log files in the specified directory.

--output-verbose-debug <OUTPUT_LOCATION>
	Outputs log as the program runs to assist with debugging. If a log is present, new data is appended to prevent debugging data loss.

--threads <THREAD_COUNT>
	Manually sets how many threads the internal thread pool has to use. This will default to the system's thread count.
	This is best used by someone who knows what they are doing and why they want to do it.
	For example, I have personally found benefits in limiting the thread pool to 1 thread when using two older hard drives as it causes less random IO to happen on the disk.
	It could also be used if you want to keep the program from taking all of your processing power, although I tend to find that there are other bottlenecks first. (Storage)
	(No harm in playing around!)

--use-config <PATH_TO_CONFIG_FILE>
	If multiple configurations are within the same file, a --configuration-name <NAME> MUST be specified for the program to know which configuration to use.
```

### Operations

#### Contribute (Cont)

* Contribute will copy any new changes from directory one to directory two.  
* No deletions will NEVER occur within directory two.  
* Renaming a file on directory one will result in a new copy of the file within directory two.  
* A modified file on directory one will cause the matching file on directory two to be overwritten.

#### Echo

* Echo will echo any changes made in directory one to directory two.  
* Changes will only occur within directory two.  

## [Configuration File](config.md)
Multiple pre-made operations can be stored in a configuration file for easy access.  
More information can be found in the [config.md](config.md)


## Sample Usage

Create a backup copy of an external drive that can be run with a batch file. This requires disabling all console input and enabling logging.

	"Sync Application.exe" --operation-mode "echo" --directory-one "D:" --directory-two "T:\Archives\SSD Backup" --hide-console --no-warning -l --output-verbose "T:\Archives\debug.txt" --output-files "T:\Archives"	
Alternatively, we can save this as a configuration and do this:

	"Sync Application.exe" --use-config "CONFIG_PATH" --configuration-name "CONFIG_NAME"
More documentation for configuration files can be found in the [config.md](config.md) file.


## Software Shout-outs

## Licensing

### Sync Application License

This software is under the [GPLv3 license](https://www.gnu.org/licenses/gpl-3.0.html).  
GPLv3 is a strong copyleft license that is intended to protect the rights of software users by ensuring that any derivative work stays free and available.

All software licenses and related information can be found in the LICENSES folder.  

### What is required?

While the GPLv3 license allows you to modify, redistribute, and use this software for any reason, your software is required to have the GPLv3 license.  
This means that you must:  
* Include a copy of the full license text.
	* The GPLv3 license  
* State all significant changes made to the original software.
* Make available the original source code when you distribute any binaries based on the licensed work.
* Include a copy of the original copyright notice.
	* In this case, a copyright notice is within the licenses folder. 
		* This will eventually be directly in the source code.

### Other Used Software Licenses

## Contact

Please reach out if you run into any bugs or issues. - jadinheaston@jadinheaston.com  
I can not promise that I will implement anything, or respond, but I am more likely to if I am aware of the problem to begin with.

### [Website](https://www.jadinheaston.com/)

Eventually, all information regarding this project will also be available on my website.
