# ftqr

File Tansfer Quick Response (ftqr) is a command-line tool that lets you share files between devices without the need of a physical connection or a cloud service. Connections are initially made over a public relay server, but all actual file transfers occur peer-to-peer over webRTC.

<img width="960" height="496" alt="image" src="https://github.com/user-attachments/assets/88c689e3-1b2f-47bb-95d1-f94a3de3d4fc" />

The ftqr host program should be cross compilable to most operating systems. The ftqr host program is a purely command line tool, so it can be run on a headless server or a machine over ssh without a issue.

## Usage

To download or upload files to a ftqr host you just need any modern enough web browser. Precompiled binaries of the ftqr host program can be found in the releases tab. Eventually I hope it will be avilable through package managers for various linux dtstros.

To start a ftqr host program to share files, run the following command in your terminal:
```
ftqr <filename>
```

This will start the ftqr host program and print a qr code to the terminal. Scan this qr code with your phone or another device to open the web application that will let you download or upload files to the ftqr host. Next to the qr code the encoded url is also printed if you want to copy it manually.

To add or remove files from the ftqr host program, detach from the interface (ctrl + d) and run `ftqr add <filename>` or `ftqr remove <filename>`. You can then reattach to the interface with `ftqr`. Only one instance of the ftqr host can run per user at a time.

To stop the ftqr host you can terminate the program within the interface with ctrl + x or by running `ftqr stop`.

ftqr requires a "relay server" to establish the initial connection between the ftqr host and the web application. A free relay server is provided via github workers at www.ftqr.cc. Note that this is a free services provided by me, so if it runs out of resources you will be blocked from connecting. For running your own relay go to the `relay_src/` folder. It can be run entierly on the cloudflare free plan to no cost to you except for the domain name.

## Components

This repository contains the source code for all the different components that make up the ftqr system:
- `host_src/`: The ftqr host program, written in C++.
- `web_src/`: The web application that is used to download and upload files to a ftqr host. It is a static html page with some javascript.
- `relay_src/`: The relay server that is used to establish the initial connection between the ftqr host and the web application. Written in Node.js.
