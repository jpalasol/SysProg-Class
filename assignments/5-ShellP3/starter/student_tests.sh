#!/usr/bin/env bats

@test "Check ls runs without errors" {
    run ./dsh <<EOF
ls
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Built-in cd with no args does not change directory" {
    current=$(pwd)
    run ./dsh <<EOF
cd
pwd
exit
EOF
    echo "$output" | grep -q "$current"
    [ "$status" -eq 0 ]
}

@test "Redirection: echo to file and cat the file" {
    run ./dsh <<EOF
echo "hello world" > test_output.txt
cat test_output.txt
exit
EOF
    # Check that output contains "hello world"
    echo "$output" | grep -q "hello world"
    [ "$status" -eq 0 ]
}

@test "Piping: ls piped to grep" {
    run ./dsh <<EOF
ls | grep dshlib.h
exit
EOF
    # Verify that the output of ls piped to grep contains 'dshlib.h'
    echo "$output" | grep -q "dshlib.h"
    [ "$status" -eq 0 ]
}

@test "Handle quoted strings preserving internal spaces" {
    run ./dsh <<EOF
echo "hello,    world"
exit
EOF
    # Expect the exact quoted string with spaces preserved
    echo "$output" | grep -q "hello,    world"
    [ "$status" -eq 0 ]
}
