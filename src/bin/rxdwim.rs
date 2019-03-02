extern crate libc;
extern crate libxdo_sys;
extern crate x11;
extern crate xdwim;

use libxdo_sys::*;
use std::ffi::*;
use std::fs;
use std::io::BufRead;
use std::io::BufReader;
use std::os::unix::net::UnixListener;
use std::os::unix::net::UnixStream;
use std::ptr;
use x11::xlib::Window;

//#[link(name = "libxdo")]

// unsafe fn switch_to_window(_xdo: *mut xdo_t, client: &str) -> bool {
//     println!("looking for X11 client: {}", client);
//     return true;
// }

fn handle_client(xdo: *mut xdo_t, stream: UnixStream) {
    let mut reader = BufReader::new(stream);
    let mut line = String::new();

    match reader.read_line(&mut line) {
        Err(err) => println!("couldn't read message: {}", err),
        Ok(_) => {}
    }

    let v: Vec<&str> = line.split(' ').collect();

    if v.len() > 0 {
        let mut search = Struct_xdo_search::default();

        println!("{:?}", CString::new(v[0]).expect("no nul bytes"));

        let cstr = CString::new(v[0]).expect("no nul bytes");

        search.only_visible = 1;
        search.require = SEARCH_ANY;

        search.searchmask |= 1 << 6;
        search.winclassname = cstr.as_ptr();
        search.max_depth = -1;

        unsafe {
            //let windowlist_ret: *mut *mut Window = std::mem::uninitialized();
            //let nwindows_ret: ::libc::c_uint = std::ptr::null_mut();
            let mut v = std::mem::uninitialized();
            let mut p: *mut Window = std::mem::uninitialized();
            let res = xdo_search_windows(xdo, &search, &mut p, &mut v);

            println!("xdo_search_windows: {}, count={}", res, v);
        }
    }
}

fn main() -> Result<(), Box<std::error::Error>> {
    fs::remove_file(xdwim::SOCKET_PATH)?;

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
