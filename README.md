# 🖼️ Image Compression Tool in C

This project is part of my **Software Project Lab-1 (SPL-1)** course.  
It implements an **Image Compression Tool** using the **C programming language**, designed to reduce image file sizes using **lossless compression algorithms** like **Run-Length Encoding (RLE)**, **LZW**, and **Huffman Coding**.

---

## 🚀 Overview
The main goal of this project is to compress and decompress image files efficiently without losing quality.  
It supports simple image formats such as **PGM (grayscale)** and **BMP (bitmap)**.

Users can:
- Choose from multiple compression algorithms.
- Compress an input image file.
- Decompress it back to verify correctness.

---

## ⚙️ Features
✅ **Lossless Compression** — Restores the exact original image after decompression.  
✅ **Multiple Algorithms** — RLE, LZW, and Huffman Coding modules implemented from scratch.  
✅ **Command-Line Interface** — Easy to use and modular.  
✅ **Structured Codebase** — Separate modules for image reading, compression, and decompression.  
✅ **Performance Testing** — Allows comparing compression ratio and execution time.

---

## 📂 Project Structure


---

## 🧩 Algorithms Implemented

| Algorithm | Type | Description |
|------------|------|-------------|
| **RLE (Run-Length Encoding)** | Lossless | Replaces consecutive identical pixels with a single count-value pair. |
| **LZW (Lempel–Ziv–Welch)** | Lossless | Dictionary-based algorithm for efficient pattern compression. |
| **Huffman Coding** | Lossless | Bit-level compression using frequency-based encoding. |

---

## 🧠 Motivation
Digital images consume significant storage and bandwidth.  
This project demonstrates how **classic compression techniques** can be implemented in **C** to achieve efficient storage while learning how bit-level operations, file I/O, and data structures interact in real systems.

---

## 🧰 Technologies Used
- **Language:** C  
- **Concepts:** File handling, dynamic memory, bit manipulation, data structures (trees, dictionaries)  
- **Environment:** GCC compiler, Linux/Windows terminal

---

## 💻 How to Run

1. **Clone the repository**
   ```bash
   git clone https://github.com/Farhana-Faruque/SPL-1.git
   cd SPL-1
