use config::Config;
use md5::{Digest, Md5};
use std::collections::HashMap;
use std::{fmt, path::PathBuf};
// use threadpool::ThreadPool;

// const THREAD_COUNT: usize = 4;
const DIRECTORY_KEYS: [&str; 2] = ["Directory One", "Directory Two"];

mod config_options;

fn main() {
    println!("Started - Loading configuration");
    let config = Config::builder()
        .add_source(config::File::with_name("config"))
        .build()
        .unwrap()
        .get_table("TEST_CONFIG")
        .unwrap();
    let mut directory_configs: HashMap<&str, HashMap<String, config::Value>> = HashMap::new();

    for directory_key in DIRECTORY_KEYS {
        directory_configs.insert(
            directory_key,
            config
                .get_key_value(directory_key)
                .unwrap()
                .1
                .clone()
                .into_table()
                .unwrap(),
        );
    }
    println!("Finished - Loading configuration");

    println!("Reading directories...");
    // let thread_pool = ThreadPool::new(THREAD_COUNT);
    let mut directory_files: HashMap<&str, Vec<std::path::PathBuf>> = HashMap::new();
    // let directory_files = {directory_keys[0]}
    for directory_key in DIRECTORY_KEYS {
        directory_files.insert(
            directory_key,
            get_files_in_directory(
                &directory_configs
                    .get_key_value(directory_key)
                    .unwrap()
                    .1
                    .get_key_value("Path")
                    .unwrap()
                    .1
                    .clone()
                    .into_string()
                    .unwrap(),
                &directory_configs
                    .get_key_value(&directory_key)
                    .unwrap()
                    .1
                    .get_key_value("Recursive")
                    .unwrap()
                    .1
                    .clone()
                    .into_bool()
                    .unwrap(),
            )
            .unwrap(),
        );
    }
    println!("Files in directory: {:#?}", directory_files);

    // println!("Starting - Performing initial comparison");
    // println!("Finished - Performing initial comparison");

    for (directory, files) in directory_files.into_iter() {
        for file in files {
            let hash = hash_file(&file);
            println!("{:#?}", hash);
        }
    }
}

fn get_files_in_directory(
    directory: &str,
    subdirectories: &bool,
) -> Result<Vec<std::path::PathBuf>, std::io::Error> {
    let mut file_list: Vec<std::path::PathBuf> = Vec::new();
    if subdirectories.clone() == true {
        for entry in walkdir::WalkDir::new(directory)
            .follow_links(true)
            .into_iter()
            .filter_map(|e| e.ok())
        {
            if entry.path().is_file() {
                file_list.push(entry.path().to_path_buf());
            }
        }
    } else {
        for entry in std::path::Path::new(directory).read_dir().unwrap() {
            let entry = entry.unwrap();
            if entry.path().is_file() {
                file_list.push(entry.path());
            }
        }
    }
    return Ok(file_list);
}

fn hash_file(file_path: &PathBuf) -> String {
    let mut hasher = Md5::new();
    hasher.update(std::fs::read(file_path).unwrap());
    let hash = hasher.clone().finalize();
    return fmt::format(format_args!("{:x}", hash));
}
