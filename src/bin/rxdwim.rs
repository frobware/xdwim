extern crate libc;
extern crate libxdo_sys;
extern crate x11;
extern crate xdwim;

use libxdo_sys::*;
use std::ffi::*;
use std::fs;
use std::io::BufRead;
use std::io::BufReader;
use std::io::BufWriter;
use std::io::Write;
use std::os::unix::net::UnixListener;
use std::os::unix::net::UnixStream;
use std::ptr;
use x11::xlib::Window;

///#[link(name = "libxdo")]

// unsafe fn switch_to_window(_xdo: *mut xdo_t, client: &str) -> bool {
//     println!("looking for X11 client: {}", client);
//     return true;
// }

fn handle_client(xdo: *mut xdo_t, stream: UnixStream) {
    let mut reader = BufReader::new(&stream);
    let mut writer = BufWriter::new(&stream);
    let mut line = String::new();

    match reader.read_line(&mut line) {
        Err(err) => {
            println!("couldn't read message: {}", err);
        }
        Ok(_) => {}
    }

    let v: Vec<&str> = line.split(' ').collect();

    if v.len() == 0 {
        return;
    }

    let mut search = Struct_xdo_search::default();
    let cstr = CString::new(v[0]).expect("no nul bytes");

    search.only_visible = 1;
    search.require = SEARCH_ANY;
    search.searchmask |= 1 << 6;
    search.winclassname = cstr.as_ptr();
    search.max_depth = -1;

    unsafe {
        let mut nwindows = std::mem::uninitialized();
        let mut windows: *mut Window = std::mem::uninitialized();

        if xdo_search_windows(xdo, &search, &mut windows, &mut nwindows) != 0 || nwindows < 1 {
            return;
        }

        let mut active_window: Window = std::mem::uninitialized();

        if xdo_get_active_window(xdo, &mut active_window) != 0 {
            return;
        }

        let slice = std::slice::from_raw_parts(windows, nwindows as usize);
        let mut topmost_window = slice[0];

        if topmost_window == active_window && nwindows >= 1 {
            topmost_window = slice[1];
        }

        if topmost_window != active_window {
            xdo_activate_window(xdo, topmost_window);
            xdo_focus_window(xdo, topmost_window);
        }

        match writer.write(b"success\n") {
            Err(err) => {
                println!("couldn't send message: {}", err);
                return;
            }
            Ok(_) => {}
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
                println!("accept failed: {:?}", err);
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
