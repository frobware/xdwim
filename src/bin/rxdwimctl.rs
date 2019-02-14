use std::env;
use std::os::unix::net::UnixStream;
use std::path::Path;
use std::io::Write;
use std::io::BufReader;
use std::io::BufRead;

// https://docs.w3cub.com/rust/std/os/unix/net/struct.unixlistener/

extern crate xdwim;

fn main() {
    let mut args: Vec<String> = env::args().collect();
    
    if args.len() <= 1 {
        println!("insufficient arguments");
        return
    }

    let socket = Path::new(xdwim::SOCKET_PATH);

    let mut stream = match UnixStream::connect(&socket) {
        Err(err) => {
            println!("cannot connect to {}: {:?}", xdwim::SOCKET_PATH, err);
            return;
        }
        Ok(stream) => stream,
    };

    args.push("\n".to_string());
        
    let joined = &args[1..].join(" ");

    match stream.write(joined.as_bytes()) {
        Err(err) => panic!("couldn't send message: {}", err),
        Ok(_) => {},
    }

    // match stream.write(b'\n'.as_bytes()) {
    //     Err(err) => panic!("couldn't send message: {}", err),
    //     Ok(_) => {},
    // }

    // let mut buf = vec![];
    let mut reader = BufReader::new(stream);
    let mut line = String::new();
    let len = reader.read_line(&mut line);
    println!("First line is {:?} bytes long", len);

// match reader.read_line(&mut line) {
//     Err(err) => panic!("couldn't read message: {}", err),
//     Ok(len) => {
//         println!("First line is {} bytes long", len);
//     },
// }

//println!("First line is {} bytes long", len);

// match rdr.read_line(&mut buf) {
//     Err(err) => panic!("couldn't read message: {}", err),
//     Ok(0) => {},
//     Ok(_) => {},
// }
}
