# Vehicle Cloud Gateway

A dedicated communication bridge between the vehicle's internal systems (like the main application, sensors, Autoware) and the external cloud backend. This gateway module handles bidirectional communication, enabling telemetry data transmission from the vehicle to the cloud and command reception from the cloud to the vehicle.

## Overview

The Vehicle Cloud Gateway serves as a critical middleware component in the Yaquod autonomous vehicle system. It provides secure and reliable data transmission between the vehicle and the cloud, protocol translation, message routing, and ensures that commands and telemetry are delivered efficiently and safely.

## Prerequisites

### Ubuntu

```bash
# Update package list and install all dependencies
sudo apt-get update && sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    ninja-build \
    libssl-dev \
    zlib1g-dev \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler-grpc \
    protobuf-compiler \
    libpaho-mqtt-dev \
    libpaho-mqttpp-dev \
    libboost-system-dev \
    libboost-thread-dev
```

### Fedora

```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    pkg-config \
    ninja-build \
    openssl-devel \
    zlib-devel \
    grpc-devel \
    grpc-plugins \
    protobuf-devel \
    boost-devel \
    paho-c-devel \
    paho-cpp-devel
```

## Building the Project

### 1. Clone the Repository

Clone the repository with submodules (required for third-party dependencies):

```bash
git clgit clone --recursive https://github.com/Yaquod/iv-cloud-gateway.git
one --recursive https://github.com/Yaquod/iv-cloud-gateway.git
cd iv-cloud-gateway
```

If you've already cloned without `--recursive`, initialize the submodules:

```bash
git submodule update --init --recursive
```

### 2. Create Build Directory

```bash
mkdir -p build
cd build
```

### 3. Configure with CMake

```bash
cmake ..
```

#### Build Options

- `BUILD_TESTS`: Build unit tests (default: ON)
- `ENABLE_COVERAGE`: Enable code coverage reporting (default: OFF)
- `ENABLE_SANITIZERS`: Enable sanitizers for debugging (default: OFF)

Example with options:

```bash
cmake -DBUILD_TESTS=ON -DENABLE_SANITIZERS=OFF ..
```

### 4. Build

```bash
make -j$(nproc)
```

Or for a specific target:

```bash
make vehicle_gateway
```

### 5. Run Tests (Optional)

```bash
ctest --output-on-failure
```

Or directly:

```bash
make test
```

## Running the Gateway

After building, the executable will be located in the `build/bin` directory:

```bash
./build/bin/vehicle_gateway
```

## Development

### Adding New Features

1. Implement your feature in the appropriate module (`http_client/`, `mqtt_client/`, etc.)
2. Add corresponding tests in the `tests/` directory
3. Update the CMakeLists.txt if adding new source files
4. Run tests to ensure everything works

## License

See the [LICENSE](LICENSE) file for details.

## Authors

- Yaquod Project Team - 2025-2026
