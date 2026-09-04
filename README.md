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

The server listens on `http://localhost:80`. Port 80 may require administrator privileges:

```bash
sudo ./build/server
```

Keep the terminal open while the server is running, then visit `http://localhost` in a browser.

## Clean Rebuild

To remove only generated CMake files and compile the project again:

```bash
rm -rf build
cmake -S . -B build
cmake --build build --parallel
```

The `build/` directory, compiled executable, and other CMake files are generated automatically and should not be committed to source control.
