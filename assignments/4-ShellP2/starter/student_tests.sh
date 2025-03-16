#!/usr/bin/env bats

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF
ls
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Exit command terminates shell" {
    run ./dsh <<EOF
exit
EOF
    # Verify that the shell loop terminates with OK_EXIT (-7)
    echo "$output" | grep -q "cmd loop returned -7"
    [ "$status" -eq 0 ]
}

@test "Built-in cd with no args does not change directory" {
    current=$(pwd)
    run ./dsh <<EOF
cd
pwd
exit
EOF
    # The output from pwd should contain the current working directory.
    echo "$output" | grep -q "$current"
    [ "$status" -eq 0 ]
}

@test "Return code for failed command via rc" {
    run ./dsh <<EOF
nonexistentcommand
rc
exit
EOF
    # Filter out prompt lines (starting with "dsh2>"), remove blank lines, trim whitespace,
    # and then search for a line consisting solely of digits.
    rc_line=$(echo "$output" | grep -v '^dsh2>' | sed '/^[[:space:]]*$/d' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//' | grep -E '^[0-9]+$')
    [ -n "$rc_line" ]
    [ "$status" -eq 0 ]
}


@test "Eliminate duplicate spaces for non-quoted tokens" {
    run ./dsh <<EOF
echo    hello    world
exit
EOF
    # 'echo' should output tokens separated by a single space: "hello world"
    echo "$output" | grep -q "hello world"
    [ "$status" -eq 0 ]
}

@test "Empty command input prints warning" {
    run ./dsh <<EOF
     
exit
EOF
    # The shell should print a warning for no commands provided.
    echo "$output" | grep -q "warning: no commands provided"
    [ "$status" -eq 0 ]
}
