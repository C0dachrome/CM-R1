import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("192.168.2.10", 5005))

while True:
    data, addr = sock.recvfrom(4096)
    line_in = data.decode('utf-8')
    if line_in.startswith("DATA:"):
        try:
            # Parse the CSV values
            str_values = line_in.replace("DATA:", "").strip().split(",")
            y_data = [float(v) for v in str_values]
            x_data = list(range(len(y_data)))
            
            # Update plot
            line.set_data(x_data, y_data)
            ax.set_xlim(0, len(y_data))
            
            plt.pause(0.01) # Small pause to allow the GUI to draw
        except ValueError:
            continue



