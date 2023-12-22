use config::Config;
use md5::{Digest, Md5};
use rayon::iter::IntoParallelRefMutIterator;
use rayon::prelude::*;
use std::collections::HashMap;
use std::fs;
use std::io::Write;
use std::os::windows::fs::MetadataExt;
use std::path::Path;
use std::path::PathBuf;
// use std::process::exit;
use std::sync::Mutex;

// const THREAD_COUNT: usize = 4;
const DIRECTORY_KEYS: [&str; 2] = ["Source", "Destination"];
const CHECK_FILE_CONTENTS_KEY: &str = "Check File Contents";
const SKIP_FILE_OPERATIONS_KEY: &str = "Skip File Operations";
const CONFIGURATION_SELECTION: &str = "TEST_CONFIG";

// const CONFIG_KEY_THREAD_COUNT: &str = "Threads";

#[derive(Clone, Debug, PartialEq, Eq)]
enum Action {
    Copy,
    Delete,
    NoChange,
}

#[derive(Clone, Debug)]
struct File {
    root_path: PathBuf,
    relative_path: PathBuf,
    hash: Option<[u8; 16]>,
    metadata: std::fs::Metadata,
    action: Option<Action>,
}

mod config_options;

fn main() {
    let mut debug_file = fs::File::create("debug.log").unwrap();

    use std::time::Instant;
    let now = Instant::now();

    println!("{:.2?} Started - Loading configuration", now.elapsed());
    let config = Config::builder()
        .add_source(config::File::with_name("config"))
        .build()
        .unwrap()
        .get_table(CONFIGURATION_SELECTION)
        .unwrap();
    let mut directory_configs: HashMap<&str, HashMap<String, config::Value>> = HashMap::new();

    for directory_key in DIRECTORY_KEYS {
        directory_configs.insert(
            directory_key,
            config
                .get(directory_key)
                .unwrap()
                .clone()
                .into_table()
                .unwrap(),
        );
    }
    println!("{:.2?} Finished - Loading configuration", now.elapsed());
    // let thread_count = config
    //     .get(CONFIG_KEY_THREAD_COUNT)
    //     .unwrap()
    //     .clone()
    //     .into_uint128()
    //     .unwrap();

    // std::env::set_var("RAYON_THREAD_COUNT", thread_count.to_string().clone());

    println!("{:.2?} Started - Reading directories", now.elapsed());

    let temporary_directory_files: Mutex<HashMap<&str, HashMap<PathBuf, File>>> =
        Mutex::new(HashMap::new());

    //Reading directories.
    DIRECTORY_KEYS.par_iter().for_each(|directory_key| {
        let directory_config = directory_configs.get(directory_key).unwrap().clone();
        let directory = directory_config
            .get("Path")
            .unwrap()
            .clone()
            .into_string()
            .unwrap();
        let recursive_flag = directory_config
            .get("Recursive")
            .unwrap()
            .clone()
            .into_bool()
            .unwrap();

        temporary_directory_files.lock().unwrap().insert(
            directory_key,
            get_files_in_directory(directory.clone(), recursive_flag).unwrap(),
        );
    });
    let mut directory_files = temporary_directory_files.into_inner().unwrap();
    println!("{:.2?} Finished - Reading directories", now.elapsed());

    let hash_files = config
        .get(CHECK_FILE_CONTENTS_KEY)
        .unwrap()
        .clone()
        .into_bool()
        .unwrap();

    println!(
        "{:.2?} Starting - Performing initial directory comparison (Hashing: {})",
        now.elapsed(),
        hash_files
    );

    //Comparing directory files.
    compare_directories(&mut directory_files, hash_files);

    println!(
        "{:.2?} Finished - Performing initial directory comparison (Hashing: {})",
        now.elapsed(),
        hash_files
    );

    if config
        .get(SKIP_FILE_OPERATIONS_KEY)
        .unwrap()
        .clone()
        .into_bool()
        .unwrap()
        == false
    {
        println!("{:.2?} Starting - File Operations", now.elapsed());
        perform_file_operations(
            &directory_files,
            config
                .get("Destination")
                .unwrap()
                .clone()
                .into_table()
                .unwrap()
                .get("Path")
                .unwrap()
                .clone()
                .into_string()
                .unwrap()
                .into(),
        );
        println!("{:.2?} Finished - File Operations", now.elapsed());
    } else {
        println!(
            "{:.2?} Skipping file operations. (Config - \"Skip File Operations\")",
            now.elapsed()
        );
    }

    let elapsed = now.elapsed();
    println!("Total elapsed: {:.2?}", elapsed);
    writeln!(debug_file, "formatted {:#?}", directory_files).unwrap();
}

fn get_files_in_directory(
    directory: String,
    subdirectories: bool,
) -> Result<HashMap<PathBuf, File>, std::io::Error> {
    let mut file_objects: HashMap<PathBuf, File> = HashMap::new();
    if subdirectories.clone() == true {
        for entry in walkdir::WalkDir::new(&directory)
            .follow_links(true)
            .into_iter()
            .filter_map(|e| e.ok())
        {
            file_objects.insert(
                entry.path().strip_prefix(&directory).unwrap().to_path_buf(),
                File {
                    root_path: directory.clone().into(),
                    relative_path: entry.path().strip_prefix(&directory).unwrap().to_path_buf(),
                    hash: None,
                    metadata: std::fs::metadata(entry.path()).unwrap(),
                    action: None,
                },
            );
        }
    } else {
        for entry in Path::new(&directory).read_dir().unwrap() {
            let entry = entry.unwrap();
            file_objects.insert(
                entry.path().strip_prefix(&directory).unwrap().to_path_buf(),
                File {
                    root_path: directory.clone().into(),
                    relative_path: entry.path().strip_prefix(&directory).unwrap().to_path_buf(),
                    hash: None,
                    metadata: std::fs::metadata(entry.path()).unwrap(),
                    action: None,
                },
            );
        }
    }
    return Ok(file_objects);
}

fn compare_directories(
    directory_files: &mut HashMap<&str, HashMap<PathBuf, File>>,
    hash_files: bool,
) {
    let source_directory = Mutex::new(directory_files.get("Source").unwrap().clone());
    let mut source_directory_guard = source_directory.lock().unwrap();
    let destination_directory = Mutex::new(directory_files.get("Destination").unwrap().clone());
    let mut destination_directory_guard = destination_directory.lock().unwrap();

    source_directory_guard
        .par_iter_mut()
        .for_each(|(source_key, source_object)| {
            let destination_object = destination_directory_guard.get(source_key);

            if destination_object.is_some() {
                let destination_object = destination_object.unwrap();
                //Comparing metadata
                if source_object.metadata.len() == destination_object.metadata.len()
                    && source_object.metadata.last_write_time()
                        == destination_object.metadata.last_write_time()
                // && source_file.metadata.last_access_time()
                //     == destination_object.metadata.last_access_time()
                {
                    //Metadata matched and hashing is enabled.
                    if hash_files == true {
                        //Hashing the files.
                        source_object.hash = Some(hash_file(
                            &source_object.root_path.join(&source_object.relative_path),
                        ));

                        let destination_object_hash = Some(hash_file(
                            &destination_object
                                .root_path
                                .join(&destination_object.relative_path),
                        ));

                        //Comparing the hashes
                        if source_object.hash.unwrap() == destination_object_hash.unwrap() {
                            source_object.action = Some(Action::NoChange);
                            // destination_object.action = Some(Action::NoChange);
                        } else {
                            source_object.action = Some(Action::Copy);
                            // destination_object.action = Some(Action::Copy);
                        }
                    } else {
                        source_object.action = Some(Action::NoChange);
                        // destination_object.action = Some(Action::NoChange);
                    }
                } else {
                    source_object.action = Some(Action::Copy);
                    // destination_object.action = Some(Action::NoChange);
                }
            } else {
                source_object.action = Some(Action::Copy);
            }
        });

    //Iterating through destination directory and removing any files that weren't found in the source directory.
    destination_directory_guard
        .par_iter_mut()
        .for_each(|(destination_key, destination_file)| {
            let source_file = source_directory_guard.get(destination_key);

            if source_file.is_none() {
                destination_file.action = Some(Action::Delete);
            }
        });

    //Updating given directory files with the new values.
    directory_files.insert("Source", source_directory_guard.clone());
    directory_files.insert("Destination", destination_directory_guard.clone());
}

fn perform_file_operations(
    directory_items: &HashMap<&str, HashMap<PathBuf, File>>,
    destination_root_path: PathBuf,
) {
    let source_directory = directory_items.get("Source").unwrap();
    let destination_directory = directory_items.get("Destination").unwrap();

    //Iterating through destination and performing deletions first to ensure space is available for future items.
    for (_key, object) in destination_directory.iter() {
        if object.action.is_some() && object.action == Some(Action::Delete) {
            if object.metadata.is_file() {
                let removal_result = fs::remove_file(
                    &object
                        .root_path
                        .join(&object.relative_path)
                        .canonicalize()
                        .unwrap(),
                );

                if removal_result.is_err() {
                    println!(
                        "Error removing file ({}) - {}",
                        &removal_result.unwrap_err(),
                        &object
                            .root_path
                            .join(&object.relative_path)
                            .canonicalize()
                            .unwrap()
                            .display()
                    );
                };
            } else if object.metadata.is_dir() {
                let removal_result = fs::remove_dir_all(
                    &object
                        .root_path
                        .join(&object.relative_path)
                        .canonicalize()
                        .unwrap(),
                );

                if removal_result.is_err() {
                    println!(
                        "Error removing directory ({}) - {}",
                        &removal_result.unwrap_err(),
                        &object
                            .root_path
                            .join(&object.relative_path)
                            .canonicalize()
                            .unwrap()
                            .display()
                    );
                };
            }
        }
    }
    //Copying items.
    for (_key, object) in source_directory.iter() {
        if object.action.is_some() && object.action == Some(Action::Copy) {
            let source_path = object
                .root_path
                .join(&object.relative_path)
                .canonicalize()
                .unwrap();
            let destination_path = destination_root_path
                .canonicalize()
                .unwrap()
                .join(&object.relative_path);
            // println!(
            //     "Copying file: {} --> {}",
            //     &file
            //         .root_path
            //         .join(&file.relative_path)
            //         .canonicalize()
            //         .unwrap()
            //         .display(),
            //     &destination_path.display()
            // );
            //Creating directories, if needed.
            fs::create_dir_all(&destination_path.parent().unwrap()).unwrap();

            //Removing readonly flag.
            source_path
                .metadata()
                .unwrap()
                .permissions()
                .set_readonly(false);
            if destination_path.is_dir() || destination_path.is_file() {
                destination_path
                    .metadata()
                    .unwrap()
                    .permissions()
                    .set_readonly(false);
            }

            //Copying object.
            let copy_result = fs::copy(&source_path, &destination_path);

            if copy_result.is_err() {
                println!(
                    "Error copying file ({}) - {} --> {}",
                    &copy_result.unwrap_err(),
                    &object
                        .root_path
                        .join(&object.relative_path)
                        .canonicalize()
                        .unwrap()
                        .display(),
                    &destination_path.display()
                );
            }
        }
    }
}

fn hash_file(file_path: &PathBuf) -> [u8; 16] {
    let mut hasher = Md5::new();
    hasher.update(std::fs::read(file_path).unwrap());
    let hash = hasher.clone().finalize();
    return hash.into();
}
