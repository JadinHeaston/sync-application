//Useful debugging functions

fn debug_hash_file(file_path: &PathBuf) -> String {
    let mut hasher = Md5::new();
    hasher.update(std::fs::read(file_path).unwrap());
    let hash = hasher.clone().finalize();
    return fmt::format(format_args!("{:x}", hash));
}
