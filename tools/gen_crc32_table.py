POLY = 0xEDB88320

table = []
for i in range(256):
    c = i
    for _ in range(8):
        c = (c >> 1) ^ POLY if c & 1 else c >> 1
    table.append(c)

with open("src/crc32_table.h", "w") as f:
    f.write("#ifndef CRC32_TABLE_H\n#define CRC32_TABLE_H\n\n")
    f.write("#include <stdint.h>\n\n")
    f.write("/* Genere par tools/gen_crc32_table.py - ne pas modifier */\n")
    f.write("static const uint32_t crc32_table[256] = {\n")
    for i in range(0, 256, 4):
        f.write("    " + ", ".join(f"0x{v:08X}UL" for v in table[i:i+4]) + ",\n")
    f.write("};\n\n#endif\n")

print("src/crc32_table.h genere")