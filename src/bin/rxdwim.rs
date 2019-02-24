extern crate xdwim;
extern crate libxdo_sys;

use std::io::BufRead;
use std::io::BufReader;
use std::os::unix::net::UnixStream;
use std::os::unix::net::UnixListener;
use std::ptr;

use libxdo_sys::xdo_free;
use libxdo_sys::xdo_new;
//use libxdo_sys::xdo_search_windows;
use libxdo_sys::xdo_t;

// pub fn xdo_search_windows(xdo: *const xdo_t, search: *const xdo_search_t,
//                           windowlist_ret: *mut *mut Window,
//                           nwindows_ret: *mut ::libc::c_uint)

fn switch_to_window(_xdo: *mut xdo_t, client: &str) {
    println!("lookig for X11 client: {}", client);
}

fn handle_client(xdo: *mut xdo_t, stream: UnixStream) {
    let mut reader = BufReader::new(stream);
    let mut line = String::new();

    match reader.read_line(&mut line) {
        Err(err) => println!("couldn't read message: {}", err),
        Ok(_) => {
            let v: Vec<&str> = line.split(' ').collect();

            if v.len() > 0 {
                println!("lookig for X11 client: {}", v[0]);
                //xdo_search_windows(xdo);
                switch_to_window(xdo, v[0]);
            }
        }
    }
}

fn main() -> Result<(), Box<std::error::Error>> {
    let xdo: *mut xdo_t;

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
