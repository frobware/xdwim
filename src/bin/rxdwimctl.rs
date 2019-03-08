extern crate xdwim;

use std::env;
use std::io::BufRead;
use std::io::BufReader;
use std::io::Write;
use std::os::unix::net::UnixStream;
use std::path::Path;
use std::process::Command;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() <= 2 {
        println!("insufficient arguments!");
        return;
    }

    let socket = Path::new(xdwim::SOCKET_PATH);

    let mut stream = match UnixStream::connect(&socket) {
        Err(err) => {
            println!("cannot connect to {}: {:?}", xdwim::SOCKET_PATH, err);
            return;
        }
        Ok(stream) => stream,
    };

    let joined = &args[1..].join(" ");

    match stream.write(joined.as_bytes()) {
        Err(err) => panic!("couldn't send message: {}", err),
        Ok(_) => {}
    }

    match stream.write("\n".as_bytes()) {
        Err(err) => panic!("couldn't send message: {}", err),
        Ok(_) => {}
    }

    let mut reader = BufReader::new(stream);
    let mut line = String::new();

    match reader.read_line(&mut line) {
        Err(err) => panic!("couldn't read message: {}", err),
        Ok(_) => {
            if !line.starts_with("success") {
                match Command::new(&args[2]).args(&args[3..]).spawn() {
                    Err(err) => panic!("couldn't spawn for client {}: {}", args[1], err),
                    Ok(_) => {}
                }
            }
        }
    }
}
