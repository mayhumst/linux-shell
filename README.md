# linux-shell
An interactive shell based in Linux that takes input, parses commands, and manages multiple processes. 

*Linux, C, Docker, Multiprocessing, Process Management*

This is an implementation of a Linux shell in C. I developed and tested this code in a Docker container environment.

While running, this shell takes user input and parses it to generate commands. When executing a command or program, it creates a child process that will execute and terminate so the shell stays active. The shell recognizes all basic Linux commands, such as ls and cd, as well as these key symbols: < > >> | The shell can redirect the input and output of these commands to/from external files instead of the commandline itself, and can pipe output into new commands.