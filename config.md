# Configuration File <!-- omit in toc -->

A configuration file can be created that contains several configurations that can be accessed with identifiable names.  
The topics below will help document the system and how to utilize built-in commands to manage configurations, while also providing a better understanding of how the internal code works.

**Note:** There is no issue with manually editing a configuration file. It is important to ensure that the JSON formatting is valid and that there is no duplicate configuration names. If you are manually editing the configuration file, I recommend using an online JSON validator to check your work.


## Table of Contents <!-- omit in toc -->
1. [Available Properties](#available-properties)
		1. [Text Version](#text-version)
	1. [Default Template](#default-template)
2. [Utilizing Built-In Commands](#utilizing-built-in-commands)
	1. [--add-to-config \<PATH\_TO\_CONFIG\_FILE\>](#--add-to-config-path_to_config_file)
	2. [--clean-config \<PATH\_TO\_CONFIG\_FILE\>](#--clean-config-path_to_config_file)
	3. [--export-config-to-command \<PATH\_TO\_CONFIG\_FILE\>](#--export-config-to-command-path_to_config_file)
	4. [--list-config \<PATH\_TO\_CONFIG\_FILE\>](#--list-config-path_to_config_file)
	5. [--use-config \<PATH\_TO\_CONFIG\_FILE\>](#--use-config-path_to_config_file)

## Formatting
The configuration file uses the [JSON standard](https://www.json.org/json-en.html).  
For simplicity and consistency, this way of handling arguments is also used internally by using [JSON by Niels Lohmann](https://github.com/nlohmann/json).  
**NOTE:** The entire configuration file must be treated as a JSON object, not an array.
# Available Properties
**NOTE:** Any omitted properties will have their default values used. The program will notify you and terminate if a required property is missing.

### Text Version
	Check File Contents - [true] | [false]
	Debug File Path - [file_path]
	Directory One
		Directory Path - [file_path]
		Recursive Search - [true] | [false]
	Directory Two
		Directory Path - [file_path]
		Recursive Search - [true] | [false]
	Modify Window - <NUMBER>
	No File Operations - [true] | [false]
	Operation Mode - [echo] | [cont] | [contribute]
	Output Files - [file_path]
	Show Console - [true] | [false]
	Show Warning - [true] | [false]
	Thread Assignment - <NUMBER>
	Verbose Debugging - [true] | [false]
	Windows Max Path Bypass - [true] | [false]

### Default Template
	{
		"CONFIGURATION_NAME": {
			"Check File Contents": false,
			"Debug File Path": "",
			"Directory One": {
				"Directory Path": "",
				"Recursive Search": true
			},
			"Directory Two": {
				"Directory Path": "",
				"Recursive Search": true
			},
			"Modify Window": 0,
			"No File Operations": false,
			"Operation Mode": "",
			"Output Files": "",
			"Show Console": true,
			"Show Warning": true,
			"Thread Assignment": "",
			"Verbose Debugging": false,
			"Windows Max Path Bypass": false
		}
	}

## Utilizing Built-In Commands
Note: Each command shown below requires "--configuration-name <NAME>" to be used as an identifier for which config to utilize in the file.

### --add-to-config <PATH_TO_CONFIG_FILE>  
When adding a configuration to the file, you must specify the full argument that you want stored.
The argument will go through most, if not all, of the programs validation before allowing it to be stored.
### --clean-config <PATH_TO_CONFIG_FILE>  
Removes all invalid/unused properties.
This could be useful to run after changing software versions.
Note: I highly encourage creating a backup of the file before using this since it destructively edits the configuration file. It is also recommended to manually verify that everything seems to be configured as intended.
### --export-config-to-command <PATH_TO_CONFIG_FILE> 
This will convert your configuration to a command that can be run. 
Note: The resulting command might need some tweaking, especially in regards to file path formatting.
### --list-config <PATH_TO_CONFIG_FILE>
Lists all configurations present in the provided configuration file.
A configuration name can be provided with this to output the configuration requested.
### --use-config <PATH_TO_CONFIG_FILE>  
As stated, it will use a configuration from the file.