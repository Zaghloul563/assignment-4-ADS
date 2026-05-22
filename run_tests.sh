#!/usr/bin/env bash
set -euo pipefail

run_case() {
    local name="$1"
    local input="$2"
    local expected="$3"

    output="$(printf "%b" "$input" | ./assignment4)"

    if grep -Fq "$expected" <<< "$output"; then
        printf "PASS: %s\n" "$name"
    else
        printf "FAIL: %s\n" "$name"
        printf "Expected to find: %s\n" "$expected"
        printf "%s\n" "$output"
        exit 1
    fi
}

run_case "Boyer-Moore overlapping matches" "2\nAAAAAA\n3\nAAA\n6\n" "Match at index: 3"
run_case "Rabin-Karp normal match" "2\nDATA STRUCTURE COURSE\n4\nSTRUCTURE\n6\n" "Highlighted text:"
run_case "Compare algorithms agree" "2\nMISSISSIPPI\n5\nISSI\n6\n" "Both algorithms found the same match indexes."
run_case "File loading works" "1\ninput.txt\n3\nDATA\n6\n" "Loaded"

printf "All tests passed.\n"
