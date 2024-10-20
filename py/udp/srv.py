import socket

# Create a UDP server socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to a specific port and IP
server_address = ('localhost', 12345)
server_socket.bind(server_address)

print(f"UDP server is up and listening on {server_address}...")

while True:
    # Wait for data from the client
    data, client_address = server_socket.recvfrom(1024)
    
    print(f"Received message: {data.decode()} from {client_address}")
    
    # Respond to the client
    response = "Message received!"
    server_socket.sendto(response.encode(), client_address)
