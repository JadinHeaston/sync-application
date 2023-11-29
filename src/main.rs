use config::Config;
use md5::{Digest, Md5};
use rayon::prelude::*;
use std::collections::HashMap;
use std::sync::Mutex;
use std::{fmt, path::PathBuf};

// const THREAD_COUNT: usize = 4;
const DIRECTORY_KEYS: [&str; 2] = ["Directory One", "Directory Two"];

#[derive(Clone, Debug)]
struct File {
    path: PathBuf,
    hash: Option<[u8; 16]>,
    metadata: std::fs::Metadata,
}

mod config_options;

fn main() {
    use std::time::Instant;
    let now = Instant::now();
	
    println!("{:.2?} Started - Loading configuration", now.elapsed());
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
    println!("{:.2?} Finished - Loading configuration", now.elapsed());

    println!("{:.2?} Started - Reading directories", now.elapsed());
    let directory_files: Mutex<HashMap<&str, Vec<File>>> = Mutex::new(HashMap::new());

    //Reading directories.
    DIRECTORY_KEYS.par_iter().for_each(|directory_key| {
        let directory_config = directory_configs
            .get_key_value(directory_key)
            .unwrap()
            .1
            .clone();
        let directory = directory_config
            .get_key_value("Path")
            .unwrap()
            .1
            .clone()
            .into_string()
            .unwrap();
        let recursive_flag = directory_config
            .get_key_value("Recursive")
            .unwrap()
            .1
            .clone()
            .into_bool()
            .unwrap();

        let files = get_files_in_directory(directory.clone(), recursive_flag).unwrap();

        directory_files.lock().unwrap().insert(directory_key, files);
    });
    println!("{:.2?} Finished - Reading directories", now.elapsed());

    // println!("Starting - Performing initial comparison");
	
    // println!("Finished - Performing initial comparison");
	
    println!("{:.2?} Starting - Hashing", now.elapsed());
    directory_files
        .lock()
        .unwrap()
        .par_iter_mut()
        .for_each(|(_, files)| {
            for file in files {
				file.hash = Some(hash_file(&file.path));
				
            }
        });
	println!("{:.2?} Finished - Hashing", now.elapsed());
	println!("{:#?}", directory_files);

    let elapsed = now.elapsed();
    println!("Total elapsed: {:.2?}", elapsed);
}

fn get_files_in_directory(
    directory: String,
    subdirectories: bool,
) -> Result<Vec<File>, std::io::Error> {
    let mut file_list: Vec<File> = Vec::new();
    if subdirectories.clone() == true {
        for entry in walkdir::WalkDir::new(directory)
            .follow_links(true)
            .into_iter()
            .filter_map(|e| e.ok())
        {
            if entry.path().is_file() {
                file_list.push(File {
                    path: entry.path().to_path_buf(),
                    hash: None,
                    metadata: std::fs::metadata(entry.path()).unwrap(),
                });
            }
        }
    } else {
        for entry in std::path::Path::new(&directory).read_dir().unwrap() {
            let entry = entry.unwrap();
            if entry.path().is_file() {
                file_list.push(File {
                    path: entry.path().to_path_buf(),
                    hash: None,
                    metadata: std::fs::metadata(entry.path()).unwrap(),
                });
            }
        }
    }
    return Ok(file_list);
}

fn debug_hash_file(file_path: &PathBuf) -> String {
    let mut hasher = Md5::new();
    hasher.update(std::fs::read(file_path).unwrap());
    let hash = hasher.clone().finalize();
    return fmt::format(format_args!("{:x}", hash));
}

fn hash_file(file_path: &PathBuf) -> [u8; 16] {
    let mut hasher = Md5::new();
    hasher.update(std::fs::read(file_path).unwrap());
    let hash = hasher.clone().finalize();
    return hash.into();
}
