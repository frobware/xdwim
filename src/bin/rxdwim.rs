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
use std::path::Path;
use std::ptr;
use x11::xlib::Window;

fn search_windows(xdo: *mut xdo_t, classname: &str) -> Vec<Window> {
    let cstr = CString::new(classname).expect("no nul bytes");
    let mut search = Struct_xdo_search::default();

    search.only_visible = 1;
    search.require = SEARCH_ANY;
    search.searchmask |= 1 << 4; // only visible
    search.searchmask |= 1 << 6; // classname
    search.winclassname = cstr.as_ptr();
    search.max_depth = -1;

    unsafe {
        let mut nwindows = { std::mem::MaybeUninit::uninit().assume_init() };
        let mut windows: *mut Window = { std::mem::MaybeUninit::uninit().assume_init() };
        if xdo_search_windows(xdo, &search, &mut windows, &mut nwindows) != 0 {
            libc::free(windows as *mut _);
            return Vec::new();
        }
        let v = std::slice::from_raw_parts(windows, nwindows as usize).to_vec();
        libc::free(windows as *mut _);
        return v;
    }
}

fn focus_window(xdo: *mut xdo_t, window: Window) -> bool {
    unsafe {
        if xdo_activate_window(xdo, window) == 0 {
            return xdo_focus_window(xdo, window) == 0;
        }
    }
    return false;
}

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

    let words: Vec<&str> = line.split(' ').collect();
    if words.len() == 0 {
        return;
    }

    let matched_windows = search_windows(xdo, words[0]);
    if matched_windows.len() == 0 {
        return;
    }

    unsafe {
        let mut active_window: Window = { std::mem::MaybeUninit::uninit().assume_init() };

        if xdo_get_active_window(xdo, &mut active_window) != 0 {
            return;
        }

        let mut topmost_window: Window = matched_windows[matched_windows.len() - 1];

        if topmost_window == active_window && matched_windows.len() > 1 {
            topmost_window = matched_windows[matched_windows.len() - 2];
        }

        if !focus_window(xdo, topmost_window) {
            return;
        }
    }

    match writer.write_all(b"success\n") {
        Err(err) => {
            println!("couldn't send message: {}", err);
            return;
        }
        Ok(_) => {}
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    if Path::new(xdwim::SOCKET_PATH).exists() {
        fs::remove_file(xdwim::SOCKET_PATH)?;
    }

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
