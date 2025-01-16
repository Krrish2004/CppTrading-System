
# Performance Optimization of a C++ System Interacting with the Deribit API

## Project Overview
This project involves the creation of a high-frequency trading system in C++ that interacts with the Deribit API. The primary focus was on achieving optimal performance, scalability, and resource utilization.

### Key Features
- **Memory Management**: Efficient memory allocation and deallocation.
- **Network Communication**: Utilized CURL for HTTP/2 protocol interactions.
- **Data Structures**: Implemented optimal data structures to improve processing speed.
- **Thread Management**: Enhanced multi-threading to handle high concurrency.
- **CPU Optimization**: Reduced CPU usage for maximum efficiency.

### Performance Analysis
Comprehensive testing was conducted to identify bottlenecks and optimize performance. The system now handles higher volumes of requests with reduced latency, ensuring faster order placements and data retrieval.

### Tools and Technologies
- **C++**: Core development language.
- **CURL**: For robust network communication.
- **JSON**: Data interchange format.
- **HTTP/2 Protocol**: Ensured fast and efficient communication.

### Conclusion
The system is now a scalable, efficient, and responsive platform for high-frequency trading, capable of handling modern trading demands.

## Running Command
```sh
g++ main.cpp -o trading_system -lcurl -I include

# Required Libraries

## libcurl4-openssl-dev (cURL Development Library)
```sh
sudo apt-get install libcurl4-openssl-dev

## nlohmann-json3-dev (JSON for Modern C++)
```sh
sudo apt-get install nlohmann-json3-dev

## g++ (C++ Compiler)
```sh 
sudo apt-get install g++



