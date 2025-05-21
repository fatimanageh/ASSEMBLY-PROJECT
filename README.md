#  Cache Performance Simulator – CSCE230301

A project for the **Computer Organization and Assembly Language Programming** course at **Spring 2025**, focused on simulating and analyzing the performance of an **n-way set-associative cache** under various configurations.

---

##  Project Overview

Modern processors are much faster than main memory, causing a performance bottleneck during memory accesses. To mitigate this, caches are used to store frequently accessed data closer to the CPU. This project simulates how different cache configurations affect cache performance, particularly hit ratios, using various synthetic memory access patterns.

---

##  Files Included

| File         | Description                                                                 |
|--------------|-----------------------------------------------------------------------------|
| `main.cpp`   | The main simulator implementation. Models an n-way set-associative cache with configurable line sizes and associativity. Includes six memory access generators and runs experiments with 1,000,000 memory accesses per configuration. |
| `Assembly Project 2 Report.pdf` | The final project report including introduction, design, experiment setup, results (with graphs), analysis, and conclusions. To be added before submission. |

---

##  Simulation Details

### Cache Configuration
- **Total Cache Size**: 64 KB
- **Variable Line Sizes**: 16, 32, 64, 128 bytes
- **Associativity (Ways)**: 1, 2, 4, 8, 16
- **Main Memory Size**: 64 MB

### 🔁 Memory Generators
Six different memory access patterns were used via `memGen1()` through `memGen6()`. Each simulates a different memory workload ranging from sequential access to fully random access.

### 📊 Experiments
- **Experiment 1**: Fixed number of sets, varied line size
- **Experiment 2**: Fixed line size (64 bytes), varied associativity

Each configuration simulates **1,000,000 memory accesses**, tracking **hit/miss ratios**.
