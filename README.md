# ftqr

File Tansfer Quick Response (ftqr) is a command-line tool that lets you share files between devices without the need of a physical connection or a cloud service. Connections are initially made over a public relay server, but all actual file transfers occur peer-to-peer over webRTC.

The ftqr host program should be cross compilable to most operating systems. The ftqr host program is a purely command line tool, so it can be run on a headless server or a machine over ssh without a issue.

To download or upload files to a ftqr host you just need any modern enough web browser.

This repository contains the source code for all the different components that make up the ftqr system:
- `host_src/`: The ftqr host program, written in C++.
- `web_src/`: The web application that is used to download and upload files to a ftqr host. It is a static html page with some javascript.
- `relay_src/`: The relay server that is used to establish the initial connection between the ftqr host and the web application. Written in Node.js.