# UNIX-like-Shell-Implementation

 Description:
  This program implements a basic shell similar to the bash shell in Linux. 
  The shell reads user input, executes standard Linux commands using the exec system call, 
  and handles single pipes between commands. 
  It supports a set of built-in commands, including changing directories (cd) 
  and displaying command history (history). The shell continues to process commands 
  until the user inputs the "exit" command. 
  Errors are handled gracefully, displaying an "Invalid Command" message without crashing the shell.
 
  Objectives:
  1. Basic Shell Implementation: Display a prompt ("MTL458 >") and continuously process commands.
  2. Command Execution: Support execution of standard Linux commands.
  3. Pipes: Implement single pipe functionality between two commands.
  4. Built-in Commands: Implement "cd" for changing directories and "history" for displaying command history.
  5. User Interrupt: Terminate the shell program upon receiving the "exit" command.
  6. Error Handling: Handle incorrect arguments or command formats gracefully.
