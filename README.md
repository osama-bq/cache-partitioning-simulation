# 🖥️ Cache Simulation Project

This project simulates cache memory operations with and without **cache partitioning**.  
It models CPU cores, RAM, and cache, and compares performance metrics such as **cache hits/misses** and **energy consumption**.

---

## 🚀 Features
- Multi-core CPU simulation
- Cache memory with and without partitioning
- Random process generation with instruction/data split
- Parallel process execution (using threads)
- Cache/RAM statistics (hits, misses, reads, writes)
- Energy estimation model
- Summary report with formatted output

---

## 📂 Project Structure

├── bin
│   └── main
├── include
│   ├── Cache.h
│   ├── Constants.h
│   ├── Core.h
│   ├── CPU.h
│   ├── HardwareComponent.h
│   ├── Instruction.h
│   ├── OS.h
│   ├── Process.h
│   ├── RAM.h
├── Makefile
└── src
    ├── Cache.cpp
    ├── Core.cpp
    ├── CPU.cpp
    ├── HardwareComponent.cpp
    ├── Instruction.cpp
    ├── main.cpp
    ├── OS.cpp
    ├── Process.cpp
    └── RAM.cpp

---

## ⚡ How to Build & Run
```bash
# Clone this repo
git clone https://github.com/osama-bq/cache-partitioning-simulation.git
cd cache-simulation

# Build
make

# Run
./bin/main
```
---

## 📊 Sample Output

```
================= Simulation Summary =================
Cores: 4 | RAM blocks: 512 | Cache blocks: 64 | Ways: 2

                        No Partitioning     Partitioning
--------------------------------------------------------
Cache Hits              132                 184
Cache Misses            56                  42
Hit Rate                70.20%              81.40%
RAM Reads               40                  28
RAM Writes              16                  14
Energy (arbitrary)      3100.00             2470.00
======================================================
```

---

## 📜 License

This project is for **academic purposes**.
You’re free to use or adapt it with proper credit.

---
