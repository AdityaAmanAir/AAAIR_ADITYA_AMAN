# Deploying on a New AWS EC2 Instance

This guide is for the current project version. The C++ server serves **HTTP on port 80**. HTTPS is not enabled by the current source code, so do not request a TLS certificate or expect `https://` to work with this build.

These steps assume **Amazon Linux 2023** and the default `ec2-user` account.

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
git clone https://github.com/AdityaAmanAir/AAAIR_ADITYA_AMAN.git
cd ~/AAAIR_ADITYA_AMAN
cmake -S . -B build && cmake --build build --parallel "$(nproc)"
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
cmake --build build --parallel "$(nproc)"
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
dig +short adityaman.website
dig +short www.adityaman.website
```

The returned address should be the EC2 Elastic IP. With the current HTTP-only application, visit `http://adityaman.website/`.

## HTTPS note

The current `backend/src/main.cpp` starts a plain HTTP server on port 80, and the current `CMakeLists.txt` does not enable TLS. A certificate alone does not make this program speak HTTPS. HTTPS requires changing the server implementation and build configuration to use TLS, or deploying a separate TLS endpoint. Do not run Certbot's standalone HTTP challenge while this server is occupying port 80; stop the service first if using that challenge after HTTPS support is configured.

## Local AI backend (Gemma)

This section supplements the deployment steps above. The AI backend is a local `llama.cpp` process, not Google's hosted Gemini API. It runs Google Gemma 3 1B Instruct QAT (`IQ4_XS` GGUF) on the EC2 instance. No Gemini API key is required.

### Instance requirements and ports

- Use an x86_64 instance; `AI_backend/start-model.sh` downloads the pinned Ubuntu x64 llama.cpp runtime. The bundled runtime is not for Graviton/ARM instances.
- Use an x86_64 instance with at least 4 GiB of RAM; 8 GiB is preferable if other services run on the instance. The model file is about 681 MiB and uses a 1024-token context. Instances with 1-2 GiB of RAM may rely heavily on swap and respond slowly; a t3.nano is not suitable.
- Keep the EC2 security group open for SSH on port 22 and the website on port 80 only. The model binds to `127.0.0.1:8081`; do not expose port 8081 publicly.
- The first model start downloads the model and llama.cpp runtime. The script verifies pinned SHA-256 checksums before launching. Subsequent starts reuse those files.

Check the instance architecture and memory before continuing:

```bash
uname -m
free -h
```

`uname -m` should print `x86_64`. If it prints `aarch64`, the current start script's prebuilt runtime will not run; build llama.cpp for that architecture and update the script before enabling the service.

### Install the model service

The model service runs as `ec2-user`, downloads missing files into the repository, and restarts if llama.cpp exits:

```bash
sudo tee /etc/systemd/system/portfolio-ai.service > /dev/null <<'EOF'
[Unit]
Description=Local Gemma model for portfolio chat
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=ec2-user
WorkingDirectory=/home/ec2-user/AAAIR_ADITYA_AMAN
ExecStart=/home/ec2-user/AAAIR_ADITYA_AMAN/AI_backend/start-model.sh
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF
```

The model binds only to loopback at `127.0.0.1:8081`. Its chat endpoint is `POST /v1/chat/completions`; it is an internal service called by the portfolio backend, not a public website endpoint.

### Start services in dependency order

The portfolio service must wait until the model health endpoint responds. Add a systemd drop-in without replacing the existing `portfolio.service`:

```bash
sudo mkdir -p /etc/systemd/system/portfolio.service.d
sudo tee /etc/systemd/system/portfolio.service.d/ai.conf > /dev/null <<'EOF'
[Unit]
Requires=portfolio-ai.service
After=portfolio-ai.service

[Service]
TimeoutStartSec=20min
ExecStartPre=/usr/bin/bash -lc 'for attempt in {1..600}; do /usr/bin/curl -fsS http://127.0.0.1:8081/health >/dev/null && exit 0; /usr/bin/sleep 2; done; exit 1'
EOF
```

Enable both services and start them. The readiness check lets the model finish its first download and load before the website backend starts:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now portfolio-ai
sudo systemctl enable --now portfolio
sudo systemctl restart portfolio
```

Check model health, service state, and logs:

```bash
curl -fsS http://127.0.0.1:8081/health
sudo systemctl status portfolio-ai portfolio
sudo journalctl -u portfolio-ai -n 100 --no-pager
sudo journalctl -u portfolio -n 100 --no-pager
```

The health response should be `{"status":"ok"}`. Test the public site at `http://YOUR_EC2_PUBLIC_IP/`. The model service is not reachable from outside the instance.

### AI request privacy

The AI chat endpoint receives only the current question. It does not read `data.json`, use portfolio details, or receive earlier chat messages. The browser replaces the displayed exchange whenever a new question is submitted. No transcript history is sent to the AI, and no `portfolio_context.json` snapshot is created. Normal portfolio APIs continue using `data.json` independently of the AI route.

### Updating the AI runtime

After pulling a change to the model launcher or AI backend, rebuild the C++ server if its source changed, then restart the model and website services:

```bash
cd ~/AAAIR_ADITYA_AMAN
git pull
cmake -S . -B build
cmake --build build --parallel "$(nproc)"
sudo systemctl restart portfolio-ai
sudo systemctl restart portfolio
```

If the model URL, checksum, or llama.cpp runtime changes, inspect `AI_backend/start-model.sh` and verify the new artifacts before restarting. Gemma's weights are subject to Google's Gemma Terms of Use.

```bash
cd ~/AAAIR_ADITYA_AMAN
cmake -S . -B build

(
set -euo pipefail
SWAPFILE=/swapfile

if [[ -e "$SWAPFILE" ]] || sudo swapon --show=NAME --noheadings | grep -Fxq "$SWAPFILE"; then
  echo "$SWAPFILE already exists or is active; leaving it untouched."
  exit 1
fi

cleanup() {
  result=$?
  trap - EXIT
  if sudo swapon --show=NAME --noheadings | grep -Fxq "$SWAPFILE"; then
    sudo swapoff "$SWAPFILE" || {
      echo "Could not disable swap; leaving $SWAPFILE in place." >&2
      exit 1
    }
  fi
  if [[ -e "$SWAPFILE" ]]; then
    sudo rm -f -- "$SWAPFILE"
  fi
  exit "$result"
}
trap cleanup EXIT

sudo fallocate -l 2G "$SWAPFILE"
sudo chmod 600 "$SWAPFILE"
sudo mkswap "$SWAPFILE"
sudo swapon "$SWAPFILE"

cmake --build build --parallel 1
)
```
