# Vehicle Cloud Gateway

A dedicated communication bridge between the vehicle's internal systems (like the main application, sensors, Autoware) and the external cloud backend. This gateway module handles bidirectional communication, enabling telemetry data transmission from the vehicle to the cloud and command reception from the cloud to the vehicle.

## Overview

The Vehicle Cloud Gateway serves as a critical middleware component in the Yaquod autonomous vehicle system. It provides.

## Prerequisites

### Ubuntu

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    zlib1g-dev \
    libspdlog-dev \
    libyaml-cpp-dev
```

### Fedora

```bash
sudo dnf install spdlog-devel openssl-devel zlib-devel yaml-cpp-devel
```

## Building the Project

### 1. Clone the Repository

```bash
git clone <repository-url>
cd iv-cloud-gateway
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
