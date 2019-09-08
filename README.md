# Protobuf Tests

## Contents

* `docs` folder : contains the [WASM client](https://github.juangburgos.com/ProtobufTests/).

* `libs` folder : contains the dependencies of the project as git submodules.

* `tests` folder : contains the test projects.

## Build

Requires `Qt 5.12` and `C++11`. 

Clone this repo **recursively**, i.e.

```bash
git clone https://github.com/juangburgos/ProtobufTests.git ./ProtobufTests.git
cd ProtobufTests.git
git submodule update --init --recursive
```

Run `qmake` on root project:

```bash
# Windows
qmake -r -tp vc tests.pro
msbuild tests.sln
# Linux
qmake -r vc tests.pro
make all
```

## Description

* `00_proto_test` : project used to test a wrapper around the auto generated `c++` classes for the [./libs/qlistproto/qlistproto.proto](./libs/qlistproto/qlistproto.proto) file.

* `01_ws_server` : a `websocket` server console application that once executed, it provides a console menu to configure and interact with the server.

* `02_ws_client` : a `websocket` client console appliation that once executed, it provides a console menu to configure and interact with the client. Should be ran in conjunction with the `01_ws_server` project.

* `03_gui_client` : a `websocket` client GUI appliation that once executed, it displays a user interface that allows to request and display data from the server.Should be ran in conjunction with the `01_ws_server` project.

## Usage

Start by running the `01_ws_server` server application. Use the `start` command to make the websocket server start listening for incoming connections (port `8080` by default). The server will automatically stop listening if no incoming request comes within 1 minute.

Then run the `02_ws_client` client application and use the `start` command to connect to the server. Use the `name` command to request the server for the name assigned to this client. If all succeeds, proceed with the `routine` server to start sending periodic requests to the server.

The `03_gui_client` client application can be used in parallel to connect to the server and keep track of the progress made by the `routine` command executed by the `02_ws_client` application.

Alternativelly, use the WASM client (web application) to connect to the server. The WASM client can be accessed in the link below:

* <https://github.juangburgos.com/ProtobufTests/>

