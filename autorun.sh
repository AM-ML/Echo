#!/bin/bash

# File to monitor
FILE_TO_WATCH="./src/echo.c"

# Compilation and execution settings
GCC_WARNING_FLAGS="-Wunused-variable -Wshadow -Wconversion -Wuninitialized -Wfloat-equal"
GCC_OPTIMIZATION_FLAGS="-oFast -O3 -march=native -mtune=native -funroll-loops -finline-functions -fomit-frame-pointer -fprefetch-loop-arrays -ffast-math"
OUTPUT_BINARY="./bin/echo"

# Infinite loop to monitor file changes
while true; do
    # Wait for changes to the file
    inotifywait -e close_write "$FILE_TO_WATCH"

    # Clear the terminal
    clear
    echo "File changed. Rebuilding..."

    # Measure compilation time
    start=$(date +%s%N)
    gcc $GCC_WARNING_FLAGS "$FILE_TO_WATCH" -o "$OUTPUT_BINARY"
    end=$(date +%s%N)
    compile_time_ms=$(( (end - start) / 1000000 ))

    # Check if compilation succeeded
    if [[ $? -eq 0 ]]; then

        # Measure runtime
        start2=$(date +%s%N)
        "$OUTPUT_BINARY"
        end2=$(date +%s%N)
        runtime_ms=$(( (end2 - start2) / 1000000 ))

        echo "-------------------------"
        echo -e "\033[1;96mCompilation: \033[1;93m${compile_time_ms}ms\033[0;0m..."
        echo -e "\033[1;94m----\033[1;96mRuntime: \033[1;93m${runtime_ms}ms\033[0;0m..."
        echo "-------------------------"
    else
        echo "Compilation failed. Fix errors and save the file again."
    fi
done

