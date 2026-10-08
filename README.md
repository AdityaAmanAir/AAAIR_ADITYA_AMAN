# AA Portfolio

A C++17 portfolio server with a vanilla JavaScript frontend.

## Requirements

Install these tools before building:

- CMake 3.16 or newer
- A C++17 compiler, such as GCC or Clang
- pthreads, usually provided by the operating system

On Ubuntu/Debian, install the compiler and CMake with:

```bash
sudo apt update
sudo apt install build-essential cmake
```

## Build

Run these commands from the project root, the directory containing `CMakeLists.txt`:

```bash
cmake -S . -B build
cmake --build build --parallel
```

What these commands do:

- `cmake -S . -B build` configures the project and creates build files in `build/`.
- `cmake --build build --parallel` compiles the C++ source files using those build files.

The compiled executable is `build/server`.

## Run

Start the server from the project root:

```bash
./build/server
```

The server listens on `http://localhost:80` by default. Port 80 may require administrator
privileges:

```bash
sudo ./build/server
```

Keep the terminal open while the server is running, then visit `http://localhost` in a browser.

To serve HTTPS directly, provide a certificate chain and private key and use port 443 (or set another `PORT`):

```bash
TLS_CERT_PATH=/path/to/fullchain.pem TLS_KEY_PATH=/path/to/privkey.pem ./build/server
```

Both TLS variables are required. When both are set and `PORT` is omitted, the server listens on `https://localhost:443`.

To run on an unprivileged local port instead of port 80:

```bash
PORT=8000 ./build/server
```

## Resume Documents

Put public PDF or DOCX files directly in `DataBase/resume/`. The server exposes this folder under `/resume/`; files placed here are publicly accessible, so do not put private documents in it. Document files in this folder are ignored by Git; copy them to the same path on the server when deploying or adding files. `index.html` and `viewer.css` remain tracked.

Open a specific document in the shared preview by passing its filename:

```text
http://localhost/resume/?file=AdityaAman_AmazonML2026_Resume-1.pdf
http://localhost/resume/?file=CSE3001_DBMS_Lab_Experiments_1_to_11.docx
```

When running locally on port `8000`, use `http://localhost:8000/resume/?file=FILENAME.pdf`. On the deployed site, use `https://adityaman.website/resume/?file=FILENAME.pdf` when TLS is configured. The viewer does not list every file; open each one by filename. DOCX preview uses browser libraries from jsDelivr and requires internet access; downloading the original file does not.

## Clean Rebuild

To remove only generated CMake files and compile the project again:

```bash
rm -rf build
cmake -S . -B build
cmake --build build --parallel
```

The `build/` directory, compiled executable, and other CMake files are generated automatically and should not be committed to source control.
