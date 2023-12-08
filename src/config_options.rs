use config::Config;

// #[derive(Debug, Deserialize, Clone)]
#[derive()]
pub struct Configuration {
    check_file_contents: bool, //false
    debug_file_path: Option<String>,
    directory_one: Option<DirectoryConfiguration>,
    directory_two: Option<DirectoryConfiguration>,
    operation_mode: Option<String>, //Echo |
    output_files: Option<String>,
    show_console: bool,            //true
    show_warning: bool,            //true
    verbose_debugging: bool,       //true
    windows_max_path_bypass: bool, //true
}
// impl Default for Configuration {
//     fn default() -> Configuration {
//         Configuration {
//             check_file_contents: false,
//             show_console: true,
//             debug_file_path: None,
//             show_warning: true,
//             directory_one: None,
//             directory_two: None,
//             operation_mode: None,
//             output_files: None,
//             verbose_debugging: true,
//             windows_max_path_bypass: false,
//         }
//     }
// }

struct DirectoryConfiguration {
    directory_two_path: std::path::PathBuf,
    recursive_two_search: bool, //true
}

struct ThreadConfiguration {
    directory_parsing: u128,
    directory_comparison: u128,
    file_operations: u128,
}

fn build_configuration(config: Config) -> Configuration {
    Configuration {
        check_file_contents: config.get_bool("check_file_contents").unwrap_or(false),
        show_console: config.get_bool("show_console").unwrap_or(true),
        debug_file_path: config.get_string("debug_file_path").ok(),
        show_warning: config.get_bool("show_warning").unwrap_or(true),
        directory_one: build_directory_configuration(config.get_string("directory_one").ok()),
        directory_two: build_directory_configuration(config.get_string("directory_two").ok()),
        operation_mode: config.get_string("operation_mode").ok(),
        output_files: config.get_string("output_files").ok(),
        verbose_debugging: config.get_bool("verbose_debugging").unwrap_or(true),
        windows_max_path_bypass: config.get_bool("windows_max_path_bypass").unwrap_or(false),
    }
}

fn build_directory_configuration(directory_path: Option<String>) -> Option<DirectoryConfiguration> {
    if let Some(directory_path) = directory_path {
        let mut path = std::path::PathBuf::from(directory_path);

        path = std::env::current_dir().unwrap().join(path);
        // if path.is_relative() {
        //     path = std::env::current_dir().unwrap().join(path);
        // } else {
        //     // if cfg!(windows) {
        //     //     if path.to_str().unwrap().len() > 259 {
        //     //         path = std::env::current_dir().unwrap().join(path);
        //     //         println!("Warning: Windows maximum path length exceeded, bypassing path length check.");
        //     //         println!("Warning: Using path: {}", path.to_str().unwrap());
        //     //         println!("Warning: You may experience errors.");
        //     //         println!("Warning: To bypass this warning, set the windows_max_path_bypass configuration option to true.");
        //     //         println!("Warning: This warning will not be shown again.");
        //     //     }
        //     // }
        // }

        if path.is_dir() {
            Some(DirectoryConfiguration {
                directory_two_path: path,
                recursive_two_search: true,
            })
        } else {
            None
        }
    } else {
        None
    }
}
