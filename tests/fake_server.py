import socket

PORT = 7033
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('127.0.0.1', PORT))
print(f"Fake SLOW server listening on UDP port {PORT} at 127.0.0.1...")

print(f"Fake SLOW server listening on UDP port {PORT}...")

while True:
    data, addr = sock.recvfrom(2048)
    print(f"Received {len(data)} bytes from {addr}")
    print(f"Dados recebidos: {data.hex()}")
    
    # --- Parse the incoming CONNECT packet to get its seqnum for acknum ---
    # Assuming the incoming CONNECT packet also follows the SLOWPacket structure
    # For a CONNECT packet, seqnum is 0, so acknum in reply will be 0.
    # The peripheral's seqnum should be used if it was a DATA packet, but for CONNECT it's 0.
    # Let's extract peripheral's seqnum to make the fake_server more robust
    # In a real scenario, you'd deserialize the whole packet.
    
    # SLOWPacket header size is 32 bytes
    if len(data) >= 32:
        # Extract the sequence number from the incoming packet (bytes 20-24, little-endian)
        # For a CONNECT packet, the peripheral sends seqnum = 0.
        incoming_seqnum = int.from_bytes(data[20:24], 'little')
        
        # Extract SID from incoming packet to use in response if needed (optional for this specific fix)
        incoming_sid = data[0:16]

    # Simula resposta SLOW com SID = 16 bytes 'A', TTL = 30000, flag ACCEPT
    response = bytearray(32)
    
    # Use the incoming SID if you want to be more spec-compliant for the SETUP packet
    # Or generate a new one if the server is truly creating the session ID.
    # The spec says "sid: UUID da sessão, deve ser usado daqui em diante" for Setup 
    # So, for the Setup (Accept) packet, the server should generate the SID.
    # For now, let's keep it simple with 'A' * 16 for testing.
    response[0:16] = b'A' * 16  # SID (Server-generated)

    # sttl: 30000, flags: ACCEPT (0b01000)
    # Combine sttl (27 bits) and flags (5 bits)
    # sttl << 5 | flags
    # The spec says sttl is "uso exclusivo do central. valores deste campo passados do peripheral para o central serão ignorados" 
    # So, the central sets its own sttl.
    sttl_value = 30000
    flags_value = 0b01000 # ACCEPT
    sttl_flags_combined = (sttl_value << 5) | flags_value
    response[16:20] = sttl_flags_combined.to_bytes(4, 'little') # Change to 'little'

    # seqnum: "primeiro seqnum da sessão, deve ser usado e incrementado daqui em diante" 
    # The server sets the initial sequence number. Let's start with 1.
    server_initial_seqnum = 1
    response[20:24] = server_initial_seqnum.to_bytes(4, 'little') # Change to 'little'
    
    # acknum: For Setup, the spec says "acknum: 0" 
    # This acknowledges the peripheral's CONNECT packet (which had seqnum 0).
    response[24:28] = (0).to_bytes(4, 'little')  # Change to 'little'

    # window: "tamanho restante no buffer de recebimento" 
    # Let's put a dummy window size, e.g., 4096 (2 bytes)
    response[28:30] = (4096).to_bytes(2, 'little') # Change to 'little'

    response[30] = 0  # fid (1 byte)
    response[31] = 0  # fo (1 byte)

    sock.sendto(response, addr)
    print(f"Sent {len(response)} bytes response to {addr}. Hex: {response.hex()}")