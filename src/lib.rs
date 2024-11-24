use std::io::Write;
use std::net::IpAddr;

pub trait Log {
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
        writeln!(file, "{}", data)?;
        Ok(())
    }
}

pub fn run(logger: Box<dyn Log>) -> Result<(), Box<dyn std::error::Error>> {
    logger.log("Hello, world!")?;
    Ok(())
}
