# Memory Inspector: Linux Kernel Module for Virtual Memory Analysis

## 🧠 Overview

This project implements a Linux Kernel Module (LKM) written in C to analyze and traverse the **virtual memory structure** of a running user-space process. It allows developers to inspect how virtual addresses are resolved within a process's page table hierarchy and print the results to the kernel log.

## 👨‍💻 Author
Aryan Shah

## 📌 Key Features

- Inspects memory layout of a given process using its PID.
- Traverses the complete page table levels: **PGD → P4D → PUD → PMD → PTE**.
- Prints debug information about each memory level and translation state to `dmesg`.
- Helps understand Linux virtual memory mechanics and kernel data structures like `mm_struct`.

## 🧠 Concept Overview

Linux uses a multi-level page table to manage virtual memory:
- **PGD** (Page Global Directory)
- **P4D** (Page 4th-level Directory)
- **PUD** (Page Upper Directory)
- **PMD** (Page Middle Directory)
- **PTE** (Page Table Entry)

This module walks through these layers to resolve a given virtual address passed as input.

## 🚀 How to Use

### 🛠️ Prerequisites

- Linux OS (Kernel version supporting 5-level paging)
- GCC and `make`
- Root access (`sudo`) to insert/load kernel modules

### 🔧 Compile the Module

```bash
cd source_code
make
```

### ⚙️ Load the Module
Replace `<PID>` and `<VIRTUAL_ADDRESS>` with actual values.

```bash
sudo insmod memory_manager.ko pid=<PID> addr=<VIRTUAL_ADDRESS>
```

### 📜 View Output
```bash
dmesg | grep Memory
```

### 🧹 Unload the Module
```bash
sudo rmmod memory_manager
```

### 🧪 Example Usage
``` bash
sudo insmod memory_manager.ko pid=1234 addr=0x7ffd9b0ae000
dmesg | tail -n 20
```
Outputs will show address translation progress through the page table layers.

### 🔒 Permissions
This module requires root privileges. Only use it on test environments or VMs. Do not use in production systems.

