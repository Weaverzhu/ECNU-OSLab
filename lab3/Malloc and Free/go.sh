#!/bin/sh
compile() {
    # Usage: compile <source file> <obj pathname>
    if [ -f $2 ]; then
        a=`stat -c %Y $1`
        b=`stat -c %Y $2`

        if [ $a -gt $b ]; then
            echo compiling...
            gcc $1 -o $2
            suc=$?
            if [ $suc -eq 0 ]; then
                echo compile completed
            else
                echo Verdict: CE
                exit 0
            fi
        fi
    else
        echo compiling...
        gcc $1 -o $2
        suc=$?
        if [ $suc -eq 0 ]; then
            echo compile completed
        else
            echo Verdict: CE
            exit 0
        fi
    fi
}

run() {
    #Usage run <obj file> [input file] [output file]
    echo running $1
    if [ $# -gt 2 ]; then
        echo log: to files
        time $1 < $2 > $3
    elif [ $# -gt 1 ]; then
        echo ===========OUTPUT===========
        time $1 < $2
        echo ============END=============
    else
        echo ===========OUTPUT===========
        time $1
        echo ============END=============
    fi
}

echo ============================
echo .
echo . Weaver_zhu\'s OSLab run shell
echo . v1.0
echo . Powered by linux bash
echo . Easy to run single C source file
echo .
echo 
if [ $# -eq 0 ]; then
    echo 'Usage: ./go.sh <option>'
elif [ $1 = "test" ]; then
    echo Running test!
    compile "./test.c" "./bin/test"
    run "./bin/test"
elif [ $1 = "ex" ]; then
    echo Running example!
    compile "./ex.c" "./bin/ex"
    run "./bin/ex"
elif [ $1 = "python" ]; then
    echo Running python!
    python3 "./main.py" < "./input/in.txt"
else
    echo 'Usage: ./go.sh <option>'
fi