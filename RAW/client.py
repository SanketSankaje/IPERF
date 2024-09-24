import fcntl
import socket
import struct
import sys
import threading
import time as t



def send_frame(ifname, dstmac, eth_type, payload, cnt1):
# Open raw socket and bind it to network interface.
	s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.IPPROTO_IP)
	s.bind((ifname, 5201))

# Get source interface's MAC address.
	info = fcntl.ioctl(s.fileno(),
			0x8927,
			struct.pack('256s', bytes(ifname, 'utf-8')[:15]))
	srcmac = ':'.join('%02x' % b for b in info[18:24])

# Build Ethernet frame
	payload_bytes = payload.encode('utf-8')
	assert len(payload_bytes) <= 1500  # Ethernet MTU

	frame = human_mac_to_bytes(dstmac) + \
		human_mac_to_bytes(srcmac) + \
		eth_type + \
		payload_bytes

# Send Ethernet frame
	while cnt1 > 0:
		s.send(frame)
		cnt1 -= 1
		#print('Send packet count {cnt}'.format(cnt = count))
	return

def human_mac_to_bytes(addr):
	return bytes.fromhex(addr.replace(':', ''))

def main():
	ifname = sys.argv[1]
	dstmac = sys.argv[2]
	payload = sys.argv[3]
	if (len(sys.argv) -1) > 3:
		cnt = int(sys.argv[4])
	else:
		cnt = 0
	if (len(sys.argv) - 1) > 4:
		tcnt = int(sys.argv[5])
	else:
		tcnt = 1
	if cnt < tcnt:
		if int (cnt / 2) > 0:
			tcnt = cnt
		else:
			tcnt = 1
	if tcnt > 128:
		tcnt = 128
	ethtype = b'\x7A\x05'  # arbitrary, non-reserved
	#count = 0
	Threads = []
	T_count = tcnt
	while T_count > 0:
		Threads.append(threading.Thread(target=send_frame, args=(ifname, dstmac, ethtype, payload, int(cnt / tcnt))))
		T_count -= 1
	while T_count < tcnt:
		Threads[T_count].start()
		T_count += 1
	T_count -= 1
	while T_count > 0:
		Threads[T_count].join()
		T_count -= 1
	Threads.clear()

if __name__ == "__main__":
	main()
