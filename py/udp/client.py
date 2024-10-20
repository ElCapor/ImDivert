import socket

# Create a UDP client socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = ('localhost', 12345)

# Send a message to the server
message = "Hello, UDP server!"
client_socket.sendto(message.encode(), server_address)

# Wait for a response from the server
response, _ = client_socket.recvfrom(1024)
print(f"Server says: {response.decode()}")

# Close the client socket
client_socket.close()
