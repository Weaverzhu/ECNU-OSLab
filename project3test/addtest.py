filename = "project3b.py"

s = open(filename, "r")
len = s.__len__()
for i in range(s):
    if (s[i] == '#' and s[i+1] == '='):
        print(i)