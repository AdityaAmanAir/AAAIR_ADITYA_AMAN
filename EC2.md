# Deploying on a New AWS EC2 Instance

This guide is for the current project version. The C++ server serves **HTTP on port 80**. HTTPS is not enabled by the current source code, so do not request a TLS certificate or expect `https://` to work with this build.

Replace `YOUR_GITHUB_REPOSITORY_URL` below with the repository's clone URL. These steps assume **Amazon Linux 2023** and the default `ec2-user` account.

## 1. Create the EC2 instance

- Select Amazon Linux 2023.
- Allow inbound SSH on TCP port `22` from your own IP address.
- Allow inbound HTTP on TCP port `80` from `0.0.0.0/0` (and `::/0` if using IPv6).
- An Elastic IP keeps the public address stable. Without one, the public IPv4 address can change after stopping and starting the instance.

Connect using the SSH command shown by the EC2 console. On the instance, check the OS if unsure:

```bash
cat /etc/os-release
```

## 2. Install build tools

For Amazon Linux 2023:

```bash
sudo dnf update -y
sudo dnf install -y git gcc-c++ cmake make openssl-devel
```

For Amazon Linux 2, use its `yum` packages and `cmake3` command:

```bash
sudo yum update -y
sudo yum install -y git gcc-c++ cmake3 make openssl-devel
```

For Ubuntu:

```bash
sudo apt update
sudo apt install -y git build-essential cmake libssl-dev
```

These provide Git to download the project, G++ to compile the C++ source, and CMake/Make to configure and run the build.

Check they are installed:

```bash
git --version
g++ --version
cmake --version
```

On Amazon Linux 2, use `cmake3` instead of `cmake` in the build commands below.

## 3. Download and build

```bash
cd ~
git clone YOUR_GITHUB_REPOSITORY_URL AAAIR_ADITYA_AMAN
cd ~/AAAIR_ADITYA_AMAN
cmake -S . -B build && cmake --build build --parallel 1
```

A successful build ends with `Built target server`. The executable is `build/server`.

The build requires OpenSSL development files. On Amazon Linux, these are provided by `openssl-devel`; on Ubuntu, install `libssl-dev`. If CMake reports that it cannot find `OPENSSL_CRYPTO_LIBRARY` or `OPENSSL_INCLUDE_DIR`, install the package for your operating system and rerun the build command. Configuration must succeed before CMake creates the build files, so a subsequent “No rule to make target 'Makefile'” error just means the build was attempted after configuration failed.

If the repository is private, configure Git access on EC2 before cloning. Do not put passwords or access tokens directly into the clone URL or this document.

## 4. Configure application data

The server reads `data.json` and serves files from `frontend/`, so run it with the project directory as its working directory. If the application needs environment values from `.env`, create that file on EC2; `.env` is intentionally not committed:

```bash
cd ~/AAAIR_ADITYA_AMAN
nano .env
```

Add the required `KEY=value` lines, save with `Ctrl+O`, press Enter, and exit with `Ctrl+X`. Keep secrets private and restrict access:

```bash
chmod 600 .env
```

The app can start without `.env`, but features requiring those values may not work.

## 5. Run in the background and start on boot

Create a `systemd` service. It runs in the background, starts after EC2 boots, and restarts if the program exits unexpectedly:

```bash
sudo tee /etc/systemd/system/portfolio.service > /dev/null <<'EOF'
[Unit]
Description=AA Portfolio C++ Server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=ec2-user
WorkingDirectory=/home/ec2-user/AAAIR_ADITYA_AMAN
ExecStart=/home/ec2-user/AAAIR_ADITYA_AMAN/build/server
Restart=on-failure
RestartSec=5
AmbientCapabilities=CAP_NET_BIND_SERVICE
CapabilityBoundingSet=CAP_NET_BIND_SERVICE

[Install]
WantedBy=multi-user.target
EOF
```

The two capability settings let the service bind to standard port 80 without running the whole application as root. Then enable and start it:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now portfolio
```

Check whether it is running:

```bash
sudo systemctl status portfolio
```

View its logs:

```bash
sudo journalctl -u portfolio -n 100 --no-pager
```

Follow new log messages live:

```bash
sudo journalctl -u portfolio -f
```

You can now close SSH. The service keeps running. Visit `http://YOUR_EC2_PUBLIC_IP/` in a browser.

## 6. Start, stop, and update

Start the service:

```bash
sudo systemctl start portfolio
```

Stop it:

```bash
sudo systemctl stop portfolio
```

Restart it:

```bash
sudo systemctl restart portfolio
```

After pulling code changes, rebuild and restart:

```bash
cd ~/AAAIR_ADITYA_AMAN
git pull
cmake -S . -B build
cmake --build build --parallel 1
sudo systemctl restart portfolio
```

## 7. Reboot or stop the EC2 instance

Reboot the operating system and let `systemd` start the service again:

```bash
sudo reboot
```

After it boots, reconnect and check:

```bash
sudo systemctl status portfolio
```

Stopping the EC2 instance from the AWS console powers off the machine, so the website is unavailable while it is stopped. Start the instance again in AWS; because the service was enabled, it starts automatically during boot. The public IP may change unless you use an Elastic IP. EBS storage and some networking resources can still incur charges while an instance is stopped; check AWS Billing for your account and region.

AWS Free Tier eligibility and included usage depend on your account age, region, and current AWS offer. Monitor Billing and set a budget alert; a running instance, storage, and public IPv4 can have separate charges.

## Domain name

In GoDaddy DNS, create an `A` record pointing your domain to the EC2 Elastic IP:

```text
Type: A
Name: @
Value: YOUR_EC2_ELASTIC_IP
```

Optionally add `www` as another `A` record to the same IP. Once DNS has propagated, test it from EC2:

```bash
dig +short yourdomain.com
dig +short www.yourdomain.com
```

The returned address should be the EC2 Elastic IP. With the current HTTP-only application, visit `http://yourdomain.com/`.

## HTTPS note

The current `backend/src/main.cpp` starts a plain HTTP server on port 80, and the current `CMakeLists.txt` does not enable TLS. A certificate alone does not make this program speak HTTPS. HTTPS requires changing the server implementation and build configuration to use TLS, or deploying a separate TLS endpoint. Do not run Certbot's standalone HTTP challenge while this server is occupying port 80; stop the service first if using that challenge after HTTPS support is configured.
