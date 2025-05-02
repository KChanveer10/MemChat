# Shared Memory Chat in C

A simple project that mimics a chat application using shared memory in C. Built to explore inter-process communication and multithreading.

## Features

- Server creates shared memory for communication.
- Multiple clients can connect and write/read messages.
- Clients use threads to continuously read and print updates.
- Server tracks active clients and deletes shared memory when none remain.

## How to Run

1. **Compile**
   ```bash
   gcc shm_server.c -o server -lpthread
   gcc shm_client.c -o client -lpthread
   ```

2. **Start the server**
   ```bash
   ./server
   ```

3. **Start one or more clients (in separate terminals)**
   ```bash
   ./client
   ```

## Purpose

Started as a fun project to deeply learn about shared memory, threading, and C programming.

## Updates

More features will be added gradually.

## Feedback

Suggestions and contributions are always welcome!
