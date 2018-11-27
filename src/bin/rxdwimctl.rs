use std::env;
use std::os::unix::net::UnixStream;
use std::path::Path;
use std::io::Write;
//use std::io::Read;

extern crate xdwim;

fn main() {
    let args: Vec<String> = env::args().collect();

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

    let joined = &args[1..].join(" ");

    match stream.write(joined.as_bytes ()) {
        Err(err) => panic!("couldn't send message: {}", err),
        Ok(_) => {},
    }

    let mut buffer = vec![0; 10];
    stream.read_to_end(&mut buffer).unwrap();
    println!("{:?}", buffer.as_slice());
}
