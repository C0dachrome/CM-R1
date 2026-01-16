import socket

# Create socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Explicitly bind to the Pluto-facing IP if you know it, otherwise 0.0.0.0
sock.bind(("192.168.2.10", 5005))

print("Receiver started. Waiting for data...")

while True:
    data, addr = sock.recvfrom(4096)
    print(f"GOT DATA from {addr}: {data[:20]}...")