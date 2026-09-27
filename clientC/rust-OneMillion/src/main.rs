use std::env;
use std::fs;
use std::io::{self, Error, ErrorKind, Read, Write};
use std::net::{TcpStream, ToSocketAddrs};
use std::os::fd::AsRawFd;
use std::thread;
use std::time::{Duration, Instant};

const HOST: &str = "im2ag-appolab.u-ga.fr";
const PORT: u16 = 443;
const BLOCK: usize = 100;
const LENGTH: usize = 1_000_000;
const CAPACITY: usize = 1 << 20;
const MASK: usize = CAPACITY - 1;
const MAX_RESPONSE: usize = 100_000;
const MAX_MESSAGE: usize = 128;
#[cfg(target_os = "linux")] // Used because I had the goal to execute it on school computers (
                            // Windows ones )to get
                            // better latency... Didn't had the chance.
const TCP_QUICKACK: i32 = 12;
const SUCCESS: &str = "Je savais que je pouvais compter sur toi";
const LOW_LATENCY_THRESHOLD: Duration = Duration::from_millis(4);

struct Appolab {
    stream: TcpStream,
}

impl Appolab {
    fn connect(address: &str) -> io::Result<Self> {
        let stream = TcpStream::connect(address)?;
        stream.set_nodelay(true)?;
        stream.set_read_timeout(Some(Duration::from_secs(60)))?;
        stream.set_write_timeout(Some(Duration::from_secs(60)))?;
        let mut client = Self { stream };
        client.read_welcome()?;
        client.send_packet_mode()?;
        Ok(client)
    }

    fn set_quick_ack(&self) -> io::Result<()> {
        #[cfg(target_os = "linux")]
        {
            let enabled: i32 = 1;
            let result = unsafe {
                setsockopt(
                    self.stream.as_raw_fd(),
                    IPPROTO_TCP,
                    TCP_QUICKACK,
                    &enabled as *const _ as *const core::ffi::c_void,
                    size_of::<i32>() as u32,
                )
            };
            if result != 0 {
                return Err(Error::last_os_error());
            }
        }
        #[cfg(not(target_os = "linux"))]
        let _ = self;
        Ok(())
    }

    fn read_quick(&mut self, buffer: &mut [u8]) -> io::Result<usize> {
        let count = self.stream.read(buffer)?;
        if count > 0 {
            self.set_quick_ack()?;
        }
        Ok(count)
    }

    fn read_exact_quick(&mut self, buffer: &mut [u8]) -> io::Result<()> {
        let mut filled = 0;
        while filled < buffer.len() {
            let count = self.read_quick(&mut buffer[filled..])?;
            if count == 0 {
                return Err(Error::new(ErrorKind::UnexpectedEof, "connection closed"));
            }
            filled += count;
        }
        Ok(())
    }

    fn read_u32(&mut self) -> io::Result<u32> {
        let mut header = [0; 4];
        self.read_exact_quick(&mut header)?;
        Ok(u32::from_be_bytes(header))
    }

    fn read_welcome(&mut self) -> io::Result<()> {
        let mut buffer = [0; 32_000];
        let mut welcome = Vec::new();
        loop {
            let count = self.read_quick(&mut buffer)?;
            if count == 0 {
                return Err(Error::new(ErrorKind::UnexpectedEof, "welcome ended early"));
            }
            welcome.extend_from_slice(&buffer[..count]);
            if welcome.ends_with(b"\n\n") {
                return Ok(());
            }
            if welcome.len() > MAX_RESPONSE {
                return Err(Error::new(ErrorKind::InvalidData, "welcome too long"));
            }
        }
    }

    fn send_packet_mode(&mut self) -> io::Result<()> {
        self.stream.write_all(&0xFFFF01CC_u32.to_be_bytes())?;
        self.read_u32()?;
        Ok(())
    }

    fn send_receive_into(&mut self, message: &[u8], response: &mut [u8]) -> io::Result<usize> {
        if message.len() + 4 > MAX_MESSAGE {
            return Err(Error::new(
                ErrorKind::InvalidInput,
                format!("message too long: {}", message.len()),
            ));
        }
        let mut packet = [0u8; MAX_MESSAGE];
        packet[..4].copy_from_slice(&(message.len() as u32).to_be_bytes());
        packet[4..4 + message.len()].copy_from_slice(message);
        self.stream.write_all(&packet[..4 + message.len()])?;

        let size = usize::try_from(self.read_u32()?)
            .map_err(|_| Error::new(ErrorKind::InvalidData, "invalid response size"))?;
        if size > MAX_RESPONSE {
            return Err(Error::new(
                ErrorKind::InvalidData,
                format!("server response too long: {size}"),
            ));
        }
        self.read_exact_quick(&mut response[..size])?;
        Ok(size)
    }
}

#[cfg(target_os = "linux")]
extern "C" {
    fn setsockopt(
        socket: std::os::fd::RawFd,
        level: i32,
        option_name: i32,
        option_value: *const core::ffi::c_void,
        option_len: u32,
    ) -> i32;
}

#[cfg(target_os = "linux")]
const IPPROTO_TCP: i32 = 6;

fn prepare_prefix(base: &[u8], ring: &mut [u16], plan: &mut [u16; BLOCK]) {
    let mut head: usize = 0;
    let mut length = BLOCK;
    ring[..BLOCK]
        .iter_mut()
        .enumerate()
        .for_each(|(index, value)| {
            *value = 256 + index as u16;
        });

    for index in (1..=LENGTH - BLOCK).rev() {
        let character = base[(index - 1) % BLOCK];
        let shifts = (character % 8) as usize;
        if shifts < length {
            for _ in 0..shifts {
                let last = ring[(head + length - 1) & MASK];
                head = head.wrapping_sub(1) & MASK;
                ring[head] = last;
            }
        }
        head = head.wrapping_sub(1) & MASK;
        ring[head] = character as u16;
        length += 1;
    }

    plan.iter_mut().enumerate().for_each(|(index, value)| {
        *value = ring[(head + index) & MASK];
    });
}
#[inline(always)]
fn apply_prefix(plan: &[u16; BLOCK], key: &[u8]) -> [u8; BLOCK] {
    let mut ring = [0u8; 128];
    let mut head: usize = 0;
    let mut length: usize = 0;

    for index in (1..=BLOCK).rev() {
        let character = key[index - 1];
        let shifts = (character % 8) as usize;
        if shifts < length {
            for _ in 0..shifts {
                let last = ring[(head + length - 1) & 127];
                head = head.wrapping_sub(1) & 127;
                ring[head] = last;
            }
        }
        head = head.wrapping_sub(1) & 127;
        ring[head] = character;
        length += 1;
    }

    let mut result = [0u8; BLOCK];
    result.iter_mut().zip(plan).for_each(|(output, tag)| {
        *output = if *tag < 256 {
            *tag as u8
        } else {
            ring[(head + (*tag as usize - 256)) & 127]
        };
    });
    result
}
fn decrypt(encrypted: &[u8]) -> Vec<u8> {
    let mut text = std::collections::VecDeque::with_capacity(encrypted.len());
    for &character in encrypted.iter().rev() {
        let mut shifts = (character % 8) as usize;
        if shifts > text.len() {
            shifts = text.len();
        }
        text.rotate_right(shifts);
        text.push_front(character);
    }
    text.into_iter().collect()
}

fn clean_line(response: &[u8]) -> &[u8] {
    response
        .split(|byte| *byte == b'\r' || *byte == b'\n')
        .next()
        .unwrap_or_default()
}

fn first_address() -> io::Result<String> {
    let host = env::var("APPOLAB_HOST").unwrap_or_else(|_| HOST.to_string());
    let port = env::var("APPOLAB_PORT")
        .ok()
        .and_then(|port| port.parse::<u16>().ok())
        .unwrap_or(PORT);
    format!("{host}:{port}")
        .to_socket_addrs()?
        .next()
        .map(|address| address.to_string())
        .ok_or_else(|| Error::new(ErrorKind::NotFound, "server address not found"))
}

#[inline]
fn run_attempt(address: &str, credentials: &str, attempt: usize) -> io::Result<()> {
    let mut client = Appolab::connect(address)?;
    let mut response = vec![0u8; MAX_RESPONSE];

    for line in credentials.split_inclusive('\n') {
        client.send_receive_into(line.as_bytes(), &mut response)?;
    }
    client.send_receive_into(b"load OneMillion", &mut response)?;
    client.send_receive_into(b"aide", &mut response)?;
    let base = clean_line(&response);
    let base = &base[..base.len().min(BLOCK)];
    if base.len() != BLOCK {
        return Err(Error::new(
            ErrorKind::InvalidData,
            format!("unexpected base string: {}", String::from_utf8_lossy(&base)),
        ));
    }

    let base = base.to_vec();
    let prefix_worker = thread::spawn(move || {
        let mut ring = vec![0u16; CAPACITY].into_boxed_slice();
        let mut plan = [0u16; BLOCK];
        prepare_prefix(&base, &mut ring, &mut plan);
        plan
    });

    let wait_low_latency = env::args().any(|argument| argument == "--wait-low-latency");
    if wait_low_latency {
        let mut consecutive_low_probes = 0;
        for probe in 1..=20 {
            let probe_start = Instant::now();
            client.send_receive_into(b"aide", &mut response)?;
            let probe_rtt = probe_start.elapsed();
            println!(
                "Attempt {attempt}: probe {probe:02} RTT {:.3} ms",
                probe_rtt.as_secs_f64() * 1_000.0,
            );
            if probe_rtt <= LOW_LATENCY_THRESHOLD {
                consecutive_low_probes += 1;
                if consecutive_low_probes == 2 {
                    break;
                }
            } else {
                consecutive_low_probes = 0;
                thread::sleep(Duration::from_millis(200));
            }
        }
    }

    let timed_start = Instant::now();
    let start_size = client.send_receive_into(b"start", &mut response)?;
    let start_exchange = timed_start.elapsed();
    let key = clean_line(&response);
    let key = &key[..key.len().min(BLOCK)];
    if key.len() != BLOCK {
        return Err(Error::new(
            ErrorKind::InvalidData,
            format!("unexpected key: {}", String::from_utf8_lossy(&key)),
        ));
    }
    let plan = prefix_worker
        .join()
        .map_err(|_| Error::new(ErrorKind::Other, "prefix worker panicked"))?;

    let solve_start = Instant::now();
    let result = apply_prefix(&plan, &key);
    let solve_time = solve_start.elapsed();

    let submit_start = Instant::now();
    let final_size = client.send_receive_into(&result, &mut response)?;
    let submit_exchange = submit_start.elapsed();
    let elapsed = timed_start.elapsed();

    let decoded = decrypt(&response[..final_size]);
    println!(
        "Attempt {}: total {:.3} ms | start {:.3} ms ({start_size} B) | solve {:.3} µs | submit {:.3} ms ({final_size} B)",
        attempt,
        elapsed.as_secs_f64() * 1_000.0,
        start_exchange.as_secs_f64() * 1_000.0,
        solve_time.as_secs_f64() * 1_000_000.0,
        submit_exchange.as_secs_f64() * 1_000.0,
    );
    let decoded = String::from_utf8_lossy(&decoded);
    if !decoded.contains(SUCCESS) {
        return Err(Error::new(
            ErrorKind::InvalidData,
            format!("unexpected server response: {decoded}"),
        ));
    }
    Ok(())
}

fn benchmark() {
    /// Used to realiably measure the time gain in pure computation independatly from network.
    let base: Vec<u8> = (0..BLOCK)
        .map(|index| ((index * 31 + 17) % 128) as u8)
        .collect();
    let key: Vec<u8> = (0..BLOCK)
        .map(|index| ((index * 37 + 91) % 128) as u8)
        .collect();
    let mut ring = vec![0u16; CAPACITY].into_boxed_slice();
    let mut plan = [0u16; BLOCK];

    let start = Instant::now();
    let mut result = [0u8; BLOCK];
    for _ in 0..1_000 {
        prepare_prefix(&base, &mut ring, &mut plan);
        result = apply_prefix(&plan, &key);
    }
    println!(
        "benchmark: {:.3} ms | checksum {}",
        start.elapsed().as_secs_f64() * 1_000.0,
        result.iter().map(|byte| *byte as u32).sum::<u32>(),
    ); // This is to avoid that compiler agressive optimization remove some of the work because it
       // has no observable effect.
}

fn main() {
    if env::args().any(|argument| argument == "--benchmark") {
        benchmark();
        return;
    }

    let mut arguments = env::args()
        .skip(1)
        .filter(|argument| argument != "--wait-low-latency");
    let attempts = match (arguments.next(), arguments.next()) {
        (None, None) => Ok(1),
        (Some(value), None) => value
            .parse::<usize>()
            .ok()
            .filter(|attempts| (1..=20).contains(attempts))
            .ok_or_else(|| value.clone()),
        _ => Err(String::new()),
    };
    let attempts = match attempts {
        Ok(attempts) => attempts,
        Err(_) => {
            eprintln!("Usage: client-onemillion [--wait-low-latency] [attempts: 1..20]");
            std::process::exit(1);
        }
    };

    let credentials = fs::read_to_string(".env")
        .map_err(|error| format!("read .env: {error}"))
        .and_then(|contents| {
            if contents.trim().is_empty() {
                Err("empty .env".to_string())
            } else {
                Ok(contents)
            }
        });
    let credentials = match credentials {
        Ok(credentials) => credentials,
        Err(error) => {
            eprintln!("OneMillion setup: {error}");
            std::process::exit(1);
        }
    };
    let address = match first_address() {
        Ok(address) => address,
        Err(error) => {
            eprintln!("resolve server: {error}");
            std::process::exit(1);
        }
    };

    for attempt in 1..=attempts {
        if let Err(error) = run_attempt(&address, &credentials, attempt) {
            eprintln!("Attempt {attempt}: {error}");
            std::process::exit(1);
        }
    }
}
