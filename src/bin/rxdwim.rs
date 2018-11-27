use std::thread;
use std::os::unix::net::{UnixStream, UnixListener};

extern crate xdwim;

fn handle_client(stream: UnixStream) {
    println!("handle_clientr: {:?}", stream);
}

fn main() {
    let listener = UnixListener::bind(xdwim::SOCKET_PATH).unwrap();

    // accept connections, spawning a new thread on each connection.

    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                // connection succeeded
                thread::spawn(|| handle_client(stream));
            }

            Err(err) => {
                println!("accept function failed: {:?}", err);
                break;
            }
        }
    }
}
