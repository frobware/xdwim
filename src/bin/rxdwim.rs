extern crate xdwim;
extern crate libxdo_sys;

use std::io::BufRead;
use std::io::BufReader;
use std::os::unix::net::UnixStream;
use std::os::unix::net::UnixListener;
use std::ptr;

use libxdo_sys::xdo_free;
use libxdo_sys::xdo_new;
use libxdo_sys::xdo_t;

fn handle_client(_xdo: *mut xdo_t, stream: UnixStream) {
    let mut reader = BufReader::new(stream);
    let mut line = String::new();

    match reader.read_line(&mut line) {
        Err(err) => println!("couldn't read message: {}", err),
        Ok(_) => {
            println!("You said: {}", line);
        }
    }
}

fn main() -> Result<(), Box<std::error::Error>> {
    let xdo;

    unsafe {
        xdo = xdo_new(ptr::null());

        if xdo.is_null() {
            panic!("Failed to initialise libxdo.");
        }
    }

    let listener = UnixListener::bind(xdwim::SOCKET_PATH)?;

    for stream in listener.incoming() {
        match stream {
            Err(err) => {
                println!("accept function failed: {:?}", err);
                break;
            }
            Ok(stream) => {
                handle_client(xdo, stream);
            }
        }
    }

    unsafe {
        xdo_free(xdo);
    }

    Ok(())
}
