awk '
    {
        for (i=1; i<=NF; i=i+1) {
            line[NR, i] = $i
        }
        # store the data in a 2d array
    }
    
    END {
        for (i=1; i<=NF; i=i+1) {
            for (j=1; j<=NR; j=j+1) {
                printf("%s ", line[j,i])
            }
            print ""
        }
        # print the elements just like we program in C
    }' input
