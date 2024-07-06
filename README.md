## TCP over UDP

- A Codebase built in C that implements reliable transport (like TCP) using the base functionality of UDP.
- The end to end functionality includes the transfer of a .txt file from a sender on one host to a receiver on another host reliably with a transfer speed of 200Mbps.
- Some of the implemented features include reliable connection setup and teardown, flow control, congestion control, sliding window mechanism with go-back-N ARQ.

## Codebase Overview

- The `rsend()` function in sender.c is responsible for the logic to read bytes for a file and break it up into segments for transport.
- The sender will first establish a connection with the receiver using a 2 Way SYN -> SYN-ACK handshake with the receiver.
- It will then begin reading bytes from the file and send them to the receiver based on a window size.
- Congestion Control:The sender window starts with size 1 and increases additively with 2 while there are no lost ACKS and in case of incorrect / lost packets, the window size is halved (multiplicative decrease)
- Once all the `bytesToSend` are sent successfully, the sender will initiate a 2 Way FIN -> FUN-ACK handshake with the receiver to close the connection.

- The `rrecv()` function in receiver.c is responsible for the logic to package the received segments in order and write the bytes correctly to the output file.
- The receiver will begin receiving and handling received packets from the sender once it ACKS the SYN from the sender at the establish connection stage.
- It will keep track of the sequence numbers in each received segment and reply with the corresponding ACK. It will also write bytes to the output file once its buffer is full.
- Once it receives a FIN from the sender, it will flush the remaining bytes in the buffer to the output file, respond with a FIN-ACK, and close the socket.

This project comprises several files aimed at facilitating communication between a sender and a receiver using TCP. Each file encapsulates specific functionalities crucial for this communication protocol. Below is a brief description of each file:

## Instructions to Run

`cd` into top level directory and run the following commands:

```bash
make
```

Open two terminals and run the following commands in each terminal.
Terminal 1:

```bash
./receiver <UDP_port> <filename_to_write>
```

Terminal 2:

```bash
./sender <receiver_hostname> <receiver_port> <filename_to_xfer> <bytes_to_xfer>
```

Both the executables will terminate after the file transfer is complete.
