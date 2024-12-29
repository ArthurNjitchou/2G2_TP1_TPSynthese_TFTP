# 2G2_TP1_TPSynthese_TFTP
## Authors: NJITCHOU Arthur & HAZAN Justine

## Description
This work implements a Trivial File Transfer Protocol (TFTP) client in C, a client program capable of exchanging files using the Trivial File Transfer Protocol (TFTP). It allows users to download (GET) and upload (PUT) files to a TFTP server over UDP. The client supports optional block size configuration for data transfer.

## Features

- **File Downloading**: Use the 'gettftp' command to download files from a TFTP server.
- **File Uploading**: Use the 'puttftp' command to upload files to a TFTP server.
- **Block Size Option**: Specify block size for data transfer.
- **UDP Communication**: Utilizes UDP sockets for communication with the TFTP server.

## Prerequisites

- A C compiler (e.g., 'gcc').
- A TFTP server to connect to.
- Basic knowledge of C programming and command line usage.

## Getting Started

### Installation

1. Clone the repository:
```bash
   git clone <our-repository-url>
   cd <repository-directory>
```
   

2. Compile the code:
```bash
   gcc -o function function.c
```
   

### Usage

Run the TFTP client with the following commands:

#### Downloading a File

To download a file from a TFTP server:
```bash
./function gettftp <server-ip> <file-name> [-blocksize <value>]
```
- `<server-ip>`: The IP address of the TFTP server.
- `<file-name>`: The name of the file to download.
- `-blocksize <value>`: (Optional) Specify the block size for the transfer.

#### Uploading a File

To upload a file to a TFTP server:
```bash
./function puttftp <server-ip> <file-name>
```
- `<server-ip>`: The IP address of the TFTP server.
- `<file-name>`: The name of the file to upload.

### Example

To download a file named `example.txt` from a TFTP server at IP `192.168.1.1`:
```bash
./function gettftp 192.168.1.1 example.txt
```

To upload a file named `example.txt` to the same server:
```bash
./function puttftp 192.168.1.1 example.txt
```

## Code Structure

- **Main Functionality**: The main function handles command-line arguments, socket creation, and manages file transfer operations.
- **Read Request (RRQ)**: Constructs a read request packet and handles incoming data.
- **Write Request (WRQ)**: Constructs a write request packet and manages file uploads.



## Acknowledgments

- This project is inspired by the TFTP protocol specifications defined in [RFC 1350](https://tools.ietf.org/html/rfc1350).

