# 🚀 High-Availability C++ Chat System

## 🖧 Overview

This project is a **highly resilient** and **efficient** chat system built in C++ using `sys/socket`. Designed for **stability** and **flexibility**, it implements a **Heartbeat mechanism** to swiftly detect server crashes and an intelligent **failover protocol** to ensure seamless connectivity. 💡

## 🛠 Features

- **🔄 Heartbeat Mechanism** – Instantly detects server failures to prevent unexpected disconnections.
- **⚡ Failover Protocol** – Automatically connects to alternative servers in case of failure.
- **📡 Multi-Server Support** – Clients can specify a prioritized list of server IPs for maximum reliability.
- **💬 Real-Time Messaging** – Efficient socket-based communication with minimal latency.
- **🛡 Robust & Scalable** – Designed with stability in mind for long-lasting connections.

## 🎯 How It Works

1. **Client-Side Configuration**

   - Users define a list of **server IPs** in their preferred order.
   - If the primary server crashes, the system **automatically** switches to the next available server.

2. **Heartbeat Mechanism**

   - The client constantly checks the server’s status.
   - If no response is received within a predefined time, it assumes failure and initiates a failover.

3. **Failover Protocol**

   - Seamlessly reconnects to the next prioritized server in the list.
   - Ensures minimal disruption to the chat experience.

## 🏗️ Technology Stack

- **C++** – Core implementation.
- **sys/socket** – Using sys/socket for Unix-like operating systems.
- **winsock** - Using winsock for Windows operating system.
- **Multi-threading** – Optimized for performance.
- **Thread Pooling**  - Efficient thread management&#x20;

## 🚀 Getting Started

### 🔧 Prerequisites

Ensure you have the following installed:

- A **C++ compiler** (`g++` recommended).
- A **Unix-based system** (Linux/macOS recommended, though it can be adapted for Windows).

### 🔨 Compilation & Execution

```sh
# Clone the repository 
git clone https://github.com/JustNOTRO/ApartiumChat.git

# Go to build directory
cd build/

# Compile the server
make server

# Compile the client
make client

# Run the server
./server

# Run the client (provide multiple server IPs as fallback)
./client <server_ip_1> <server_ip_2> <server_ip_3>
```

## 📜 License

This project is licensed under the **MIT License** – feel free to use, modify, and enhance it! 🚀

## 🤝 Contributing

Contributions are welcome! If you have improvements, submit a PR or open an issue. Let’s build a **rock-solid** chat system together! 🚀💬

