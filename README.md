# latorendszer
Vision system for firewire camera (linux)

To compile:
in project root( where CMakeLists.txt is )
cmake .
then:
make

The built binary is called server.
It can be run by ./server

There is an example client in socket_example, called client.cpp
compile with:
g++ -o client client.cpp
run it with:
socket_example/client
