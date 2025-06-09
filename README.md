# From-Scratch HTTP Server in C

## Overview

This project implements a minimal HTTP server in C that handles basic GET routes and returns HTML responses. The server uses a custom router built with a hash table and linear probing for efficient route lookup.
Features

- Simple HTTP server handling only GET requests (for now).
- Returns HTML responses for registered routes.
- Router implemented as a hash table with linear probing for collision handling.
- No external dependencies — relies solely on standard C libraries and POSIX sockets.

## How to Build and Run

Build the server:

```bash
make
```

Run the server:

```bash
./server <port_number>
```

Replace <port_number> with the port number you want the server to listen on (e.g., 8080).

## Usage

Routes are registered in the main function by associating HTTP paths with controller functions.
These controller functions point to the html pages in the `pages` directory.

## Stats

Over 1000 concurrent "/" requests
```
Average Time (ms)   : 0.180
Min Time (ms)       : 0.127
Max Time (ms)       : 1.229
```
