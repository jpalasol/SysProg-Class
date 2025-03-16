1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**: this is a good choice compared to a different option like fscan() because with fgets() it includes the entirety of 
    the users input leaving in spaces and \n. Even though we have to code removing it ourselves we know we aren't missing anything.
    We can also give it a maximum character limit to prevent buffer overflow. Overall it is better for processing input.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  we needed to use malloc() instead of a fixed-size array because we need to be flexible in case the user input is very large.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  trimming ensures we have the exact command to the how ever many arguments. If our shell can not identify the command or arg
    there can be errors in what is an argument or errors in executing the command. Either way not being read right leading to our shell breaking.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**: 3 redirection examples to implement are:
    1. Redirecting the output to a file. cat < files.txt
        Some challenges is creating a .txt file to hold the output as well as being able to open the file and write the output
    2. Redirect errors to a file. grep "sort" files.txt 2> error.log
        The same as redirecting the output, file management is a challenge.
    3. Redirecting the input as well to a file. sort < listtosort.txt
        When handling the files contents with the buffer problems I had that will be a challenge
        to make sure it is being redirected into the file correctly.


- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  the differences between the two is that redirection uses files while piping does not. Piping has IPC or inter-process communication.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  It is easier to differentiate the two in debugging if they were seperate since they are similar.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  it should show the output as well as what the error was (both STDOUT and STDERR) seperately. If we wanted to merge them then we would use redirection or pipes.