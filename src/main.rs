use config::Config;
use hex_literal::hex;
use md5::{Digest, Md5};
use std::{fmt, path::PathBuf};

mod config_options;

fn main() {
    println!("Loading configuration...");
    let config = Config::builder()
        .add_source(config::File::with_name("config"))
        .build()
        .unwrap()
        .get_table("TEST_CONFIG")
        .unwrap();
    println!("{:?}", config);
    println!("Configuration loaded!");

    let directory_one_config = config
        .get_key_value("Directory One")
        .unwrap()
        .1
        .clone()
        .into_table()
        .unwrap();

    println!("{:#?}", directory_one_config);
    let directory_one_files = get_files_in_directory(
        &directory_one_config
            .get_key_value("Path")
            .unwrap()
            .1
            .clone()
            .into_string()
            .unwrap(),
        &directory_one_config
            .get_key_value("Recursive")
            .unwrap()
            .1
            .clone()
            .into_bool()
            .unwrap(),
    );
    println!("Files in directory: {:#?}", directory_one_files);

    for file in directory_one_files {
        let hash = hash_file(&file);
        println!("{:#?}", hash);
    }
}

fn get_files_in_directory(directory: &str, subdirectories: &bool) -> Vec<std::path::PathBuf> {
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
    return file_list;
}

fn hash_file(file_path: &PathBuf) -> String {
    let mut hasher = Md5::new();
    hasher.update(std::fs::read(file_path).unwrap());
    let hash = hasher.clone().finalize();
    return fmt::format(format_args!("{:x}", hash));
}
