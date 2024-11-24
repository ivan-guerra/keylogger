use clap::{Args, Parser, Subcommand};
use std::net::IpAddr;

#[derive(Parser)]
#[command(version, about, long_about = None)]
#[command(propagate_version = true)]
struct Cli {
    #[command(subcommand)]
    command: Commands,

    #[arg(
        short = 'n',
        long,
        default_value_t = 8,
        value_parser = clap::value_parser!(u32).range(1..=1_000_000),
        help = "max number of keystrokes buffered before recording to the target"
    )]
    keystroke_threshold: u32,
}

#[derive(Subcommand)]
enum Commands {
    Net(NetArgs),
    File(FileArgs),
}

#[derive(Args)]
#[command(about = "log keystrokes to a remote server")]
struct NetArgs {
    #[arg(
        help = "target ipv4 address",
        value_parser = clap::value_parser!(IpAddr))
    ]
    ipv4_addr: IpAddr,

    #[arg(
        value_parser = clap::value_parser!(u16),
        help = "target port")
    ]
    port: u16,
}

#[derive(Args)]
#[command(about = "log keystrokes to a file")]
struct FileArgs {
    #[arg(help = "file that will buffer keystrokes")]
    recorder_file: std::path::PathBuf,
}

fn main() {
    let cli = Cli::parse();
    match &cli.command {
        Commands::Net(args) => {
            println!("ipv4_addr: {:?}", args.ipv4_addr);
            println!("port: {:?}", args.port);
        }
        Commands::File(args) => {
            println!("recorder_file: {:?}", args.recorder_file);
        }
    }
}
