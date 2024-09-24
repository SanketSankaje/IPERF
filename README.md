# IPERF

## custom_iperf
An simple library for communication between multiple systems in an network.

- `make` will build the library and compile the test application and creat 'test' executable.

- `./test -s/-c interface_name -tcp/-udp dest_ip[incase of client]`
	-s to run test as server.
	-c to run test as client
	-tcp for tcp protocol
	-udp for udp protocol


## RAW
An simple python file which can be used to control number of packets to send.

Command to run this application:
`sudo python3 client.py interface_name dst_mac 'DATA' no_of_packets number_of_threads`




