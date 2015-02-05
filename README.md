# latorendszer
Vision system for firewire camera (linux)

To compile:
in project root( where CMakeLists.txt is )
cmake .
then:
make

The built binary is called server.
It can be run by:
./server

There is an example client in socket_example, called client.cpp
compile with:

g++ -o client client.cpp

run it with:
socket_example/client localhost 8888


There is also an example that listens for UDP datagarams server_udp.c.
Compile with:

gcc -o server_udp server_udp.c

run with:
socket_example/server_udp 8080