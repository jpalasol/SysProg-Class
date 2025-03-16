1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My implementation makes sure all child processes finish before the shell accepts new input by using waitpid after each fork. This prevents the shell from moving on too soon and avoids zombie processes.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

You need to close unused pipe ends after calling dup2 because the originals stay open. If left open, they can use up file descriptors and prevent the pipe from signaling that it's empty.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

The cd command is a built-in rather than an external command because it needs to change the shell's working directory. If implemented externally, it would run in a child process, leaving the parent shell’s directory unchanged.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To modify my implementation to allow that, I would need to allocate memory dynamically, using functions like realloc. This would make the program more flexible, allowing it to handle varying amounts of data, but it would also introduce challenges, as I would need to manage memory carefully to avoid leaks or inefficiencies.
