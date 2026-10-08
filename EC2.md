# Deploying on a New AWS EC2 Instance

The C++ server supports HTTP by default and native HTTPS when configured with a TLS certificate and private key. This guide covers direct HTTPS on EC2 and the recommended Application Load Balancer (ALB) setup for ECS.

These steps assume **Amazon Linux 2023** and the default `ec2-user` account.

## 1. Create the EC2 instance

- Select Amazon Linux 2023.
- Allow inbound SSH on TCP port `22` from your own IP address.
- Allow inbound HTTP on TCP port `80` and HTTPS on TCP port `443` from `0.0.0.0/0` (and `::/0` if using IPv6). Port 80 is needed for Let's Encrypt certificate issuance and renewal.
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

You can now close SSH. The service keeps running. Without TLS environment variables, visit `http://YOUR_EC2_PUBLIC_IP/` in a browser. To enable HTTPS with a domain and certificate, follow the HTTPS section below.

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

The returned address should be the EC2 Elastic IP. Once TLS is configured, visit `https://adityaman.website/`.

## HTTPS on EC2

The server uses its built-in TLS support when both `TLS_CERT_PATH` and `TLS_KEY_PATH` are set. It defaults to port `443` in that mode; `PORT` can override it. Without both variables, it remains an HTTP server on port `80`.

First, point your domain's DNS `A` record at the EC2 Elastic IP and make sure inbound ports `80` and `443` are allowed. Install Certbot and issue a certificate. The standalone challenge needs port 80 to be free, so stop the existing HTTP service during issuance:

```bash
sudo dnf install -y certbot
sudo systemctl stop portfolio
sudo certbot certonly --standalone -d adityaman.website -d www.adityaman.website
```

The systemd service runs as `ec2-user`, while Certbot keeps its private key root-only. Copy the certificate into a restricted directory readable by that service account:

```bash
sudo install -d -o ec2-user -g ec2-user -m 700 /etc/portfolio/tls
sudo install -o ec2-user -g ec2-user -m 644 /etc/letsencrypt/live/adityaman.website/fullchain.pem /etc/portfolio/tls/fullchain.pem
sudo install -o ec2-user -g ec2-user -m 600 /etc/letsencrypt/live/adityaman.website/privkey.pem /etc/portfolio/tls/privkey.pem
```

Configure the service to use these copies. Replace the domain in the Certbot paths above if yours differs:

```bash
sudo systemctl edit portfolio
```

Add:

```ini
[Service]
Environment=PORT=443
Environment=HTTPS_HOST=adityaman.website
Environment=TLS_CERT_PATH=/etc/portfolio/tls/fullchain.pem
Environment=TLS_KEY_PATH=/etc/portfolio/tls/privkey.pem
```

Then restart and verify the service and HTTPS response:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now portfolio
sudo systemctl status portfolio
curl -I https://adityaman.website/
```

Certbot renews the certificate files, but the running C++ server loads them only on startup. Restart it after renewal:

```bash
sudo install -d /etc/letsencrypt/renewal-hooks/deploy
sudo tee /etc/letsencrypt/renewal-hooks/deploy/restart-portfolio.sh > /dev/null <<'EOF'
#!/bin/sh
install -o ec2-user -g ec2-user -m 644 /etc/letsencrypt/live/adityaman.website/fullchain.pem /etc/portfolio/tls/fullchain.pem
install -o ec2-user -g ec2-user -m 600 /etc/letsencrypt/live/adityaman.website/privkey.pem /etc/portfolio/tls/privkey.pem
systemctl restart portfolio
EOF
sudo chmod 755 /etc/letsencrypt/renewal-hooks/deploy/restart-portfolio.sh
sudo systemctl enable --now certbot-renew.timer
sudo certbot renew --dry-run
```

With TLS configured, the native C++ server listens for HTTPS on port 443 and redirects HTTP requests on port 80 to `HTTPS_HOST`, preserving the path, query, and request method. Set `HTTPS_HOST` to the canonical hostname only, without a scheme or trailing slash. Both ports must be allowed in the EC2 security group. Keep the TLS private key readable by the service account only as required by your systemd setup, and never commit either certificate file.

## ECS requirements

For ECS, use an ALB to terminate public HTTPS rather than putting a public certificate in each task. The browser connects to the ALB over HTTPS; the ALB forwards HTTP to the private ECS task. The C++ server needs no certificate in the task in this layout.

- Create an ECR repository and build/push a container image containing `build/server`, `data.json`, `frontend/`, and any public files from `DataBase/resume/`. This repository does not currently include a Dockerfile, so container image packaging is a required deployment step.
- Create an ECS cluster and a Fargate task definition using Linux `X86_64`. The bundled llama.cpp runtime is Ubuntu x64 and does not run on ARM/Graviton as currently supplied.
- Set the web container's `PORT=8080`; leave `TLS_CERT_PATH` and `TLS_KEY_PATH` unset. Configure the ALB target group for HTTP on container port `8080`, with a health check on `/`.
- Request or import a certificate in AWS Certificate Manager (ACM) for the domain in the same AWS region as the ALB. Add an ALB HTTPS listener on `443` using that certificate, and optionally an HTTP listener on `80` that redirects to HTTPS.
- Give the ALB security group inbound `80`/`443` access from the internet. Give the task security group inbound access to `8080` only from the ALB security group; do not expose the task or model port publicly.
- The current AI route calls `127.0.0.1:8081`. Run the model in a sidecar container in the same ECS task so the task's shared network namespace can use that loopback endpoint. Keep the sidecar bound to loopback and configure task startup/health checks so the web container does not receive traffic before the model is ready.
- Allocate at least `4 GiB` of task memory for the model and backend together; `8 GiB` is a more practical starting point, with at least `2 vCPU` for responsive CPU inference. Load-test and raise the task size based on concurrency. The model launcher downloads its model/runtime if missing, so provide outbound internet access (NAT for private subnets) or package/cache those assets in the image or persistent storage.
- Put `.env` secrets in AWS Secrets Manager or Systems Manager Parameter Store and map them into the task definition. Send container logs to CloudWatch Logs. Store resume files in the image for a small static site or move them to durable storage such as S3/EFS before scaling tasks; task-local filesystem changes are not durable across replacement.
- Add Route 53 alias records (or DNS records at your provider) to the ALB. The EC2 Elastic IP and EC2 Certbot steps above do not apply to this ECS/ALB setup.

An ECS deployment is not complete until the web and model images/task definition are built, the ALB health check passes, and both `https://YOUR_DOMAIN/` and the AI route have been tested through the ALB.

## Local AI backend (Gemma)

This section supplements the deployment steps above. The AI backend is a local `llama.cpp` process, not Google's hosted Gemini API. It runs Google Gemma 3 1B Instruct QAT (`IQ4_XS` GGUF) on the EC2 instance. No Gemini API key is required.

### Instance requirements and ports

- Use an x86_64 instance; `AI_backend/start-model.sh` downloads the pinned Ubuntu x64 llama.cpp runtime. The bundled runtime is not for Graviton/ARM instances.
- Use an x86_64 instance with at least 4 GiB of RAM; 8 GiB is preferable if other services run on the instance. The model file is about 681 MiB and uses a 1024-token context. Instances with 1-2 GiB of RAM may rely heavily on swap and respond slowly; a t3.nano is not suitable.
- Keep the EC2 security group open for SSH on port 22, the website on port 443, and port 80 for Let's Encrypt validation. The model binds to `127.0.0.1:8081`; do not expose port 8081 publicly.
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
