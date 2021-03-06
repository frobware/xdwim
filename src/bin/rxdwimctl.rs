extern crate xdwim;

use std::io::BufRead;
use std::io::BufReader;
use std::io::Write;
use std::os::unix::net::UnixStream;
use std::path::Path;
use std::process::Command;

fn main() {
    let socket = Path::new(xdwim::SOCKET_PATH);

    let mut stream = match UnixStream::connect(&socket) {
        Err(err) => {
            println!("cannot connect to {}: {:?}", xdwim::SOCKET_PATH, err);
            return;
        }
        Ok(stream) => stream,
    };

    let mut args = Vec::new();

    for arg in std::env::args().skip(1) {
	args.push(arg);
    }

    if args.len() < 2 {
        println!("insufficient arguments!");
        return;
    }

    match stream.write((args.join(" ") + "\n").as_bytes()) {
        Err(err) => panic!("couldn't send message: {}", err),
        Ok(_) => {}
    }

    let mut reader = BufReader::new(stream);
    let mut line = String::new();

    match reader.read_line(&mut line) {
        Err(err) => panic!("couldn't read message: {}", err),
        Ok(_) => {
            if !line.starts_with("success") {
                match Command::new(&args[1]).args(&args[2..]).spawn() {
                    Err(err) => panic!("couldn't spawn for client {}: {}", args[1], err),
                    Ok(_) => {}
                }
            }
        }
    }
}
