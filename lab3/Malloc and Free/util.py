from sys import argv

WORD_LIST = [
    ["Mem_Init", "mem_init"],
    ["Mem_Free", "mem_free"],
    ["Mem_Alloc", "mem_alloc"],
    ["Mem_Dump", "mem_dump"],
    ["FIRSTFIT", "M_FIRSTFIT"],
    ["BESTFIT", "M_BESTFIT"],
    ["WORSTFIT", "M_WORSTFIT"]
]

STAMP = "/*has been converted*/"

def main():
    n = argv.__len__()
    if n != 1:
        print("Usage: python util.py <filename>")
    filename = argv[1]
    f = open(filename, "r")
    content = f.read()
    print(content)
    f.close()
    times = content.count(STAMP)
    if times == 0:
        for tp in WORD_LIST:
            content = content.replace(tp[0], tp[1])
        content = STAMP + "\n" + content
    f = open(filename, "w")
    f.write(content)
    f.close()

if __name__ == "__main__":
    main()