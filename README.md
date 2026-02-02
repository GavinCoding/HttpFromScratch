# HTTP Server From Scratch

A lightweight HTTP server written in modern **C++ (C++20)**, built entirely from scratch with no external web frameworks. This project was created to deeply understand how web servers work at a low level — from handling raw TCP connections to parsing HTTP requests and returning valid responses.

🚀 **This server is currently running live on my Raspberry Pi**, serving real HTTP traffic.

---

## Overview

The goal of this project is educational and systems-focused rather than production-scale. It demonstrates:

* Manual socket programming
* Parsing HTTP requests by hand
* Serving static HTML files
* Handling basic HTTP routing logic
* Running a long-lived server process on Linux

Everything is intentionally kept simple and explicit so the control flow and data handling are easy to reason about.

---

## Features

* Written in **C++20**
* Uses **POSIX sockets** directly (no libraries like Boost or frameworks)
* Serves static files (e.g. `index.html`)
* Integrates Multi-client concurrency
* Proper HTTP response formatting
* Runs as a standalone binary
* Deployed on a **Raspberry Pi**

---

## Why This Project

Most web development abstracts away networking details. This project intentionally does the opposite.

By building everything manually, I wanted to:

* Understand how browsers actually talk to servers
* Learn how HTTP works at the byte level
* Get comfortable with Linux networking APIs
* Practice writing robust, low-level C++ code
* Deploy and run a custom service on real hardware

---

## Running the Server

The server is compiled as a single binary and designed to run on Linux-based systems.

It listens on a configurable port and responds to incoming HTTP requests from any browser or HTTP client.

This project is **actively running on my Raspberry Pi**, acting as a live self-hosted web service.

---

## Technologies Used

* C++20
* Linux / POSIX
* TCP sockets
* HTTP/1.1 (basic implementation)
* Raspberry Pi (deployment target)

---

## Status

This is an actively maintained learning project. Future improvements may include:

* Better HTTP compliance
* MIME type handling
* Logging and error handling
* Basic routing or templating

---

## Author

**Gavin Roake**
Software Engineer

---

If you’re interested in low-level networking, operating systems, or how the web works under the hood, this project is meant to be readable, hackable, and educational.
