/*
 *  ****************************************************
 *  *                                                  *
 *  *        Name : Shashwat Raj                       *
 *  *        Entry Number: 2021MT10259                 *
 *  *                                                  *
 *  ****************************************************
 */

/*
 * Assignment 1: Building a Basic Shell
 *
 * Description:
 * This program implements a basic shell similar to the bash shell in Linux. 
 * The shell reads user input, executes standard Linux commands using the exec system call, 
 * and handles single pipes between commands. 
 * It supports a set of built-in commands, including changing directories (cd) 
 * and displaying command history (history). The shell continues to process commands 
 * until the user inputs the "exit" command. 
 * Errors are handled gracefully, displaying an "Invalid Command" message without crashing the shell.
 *
 * Objectives:
 * 1. Basic Shell Implementation: Display a prompt ("MTL458 >") and continuously process commands.
 * 2. Command Execution: Support execution of standard Linux commands.
 * 3. Pipes: Implement single pipe functionality between two commands.
 * 4. Built-in Commands: Implement "cd" for changing directories and "history" for displaying command history.
 * 5. User Interrupt: Terminate the shell program upon receiving the "exit" command.
 * 6. Error Handling: Handle incorrect arguments or command formats gracefully.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Global variable to store the previous directory
// This variable is used to store the directory path before a 'cd' command is executed,
// allowing the user to switch back to the previous directory with the 'cd -' command.
char prev_dir[1024] = "";

// Function to execute commands
// This function takes an array of command arguments and a flag indicating whether the command
// should be run in the background. It forks a child process to execute the command using execvp().
void execute_command(char **args, int background) {
    pid_t pid = fork();  // Create a new process
    if (pid == 0) {  // Child process
        // Replace the current process image with a new one, specified by the command and its arguments.
        if (execvp(args[0], args) == -1) {  // Execute command
            // If execvp() fails, it returns -1, and an error message is printed.
            perror("Invalid Command");
        }
        exit(EXIT_FAILURE);  // Exit child process if execvp fails
    } else if (pid < 0) {  // Fork failed
        // If fork() fails, an error message is printed.
        perror("fork failed");
    } else {  // Parent process
        // If the command is not meant to be run in the background, the parent process waits
        // for the child process to finish execution.
        if (!background) {  // Wait for child process to finish if not in background mode
            wait(NULL);
        }
    }
}

// Function to change the current directory
// This function handles changing directories, including special cases like 'cd ~' for the home directory
// and 'cd -' to switch to the previous directory.
void change_directory(char *path) {
    char current_dir[1024];
    char temp_dir[1024];
    char *home_dir = getenv("HOME");

    // Remove quotes from the path if present
    // This handles cases where the user might input a path with surrounding quotes.
    if (path[0] == '"' && path[strlen(path) - 1] == '"') {
        path[strlen(path) - 1] = '\0';  // Remove the ending quote
        path++;  // Move the pointer to skip the starting quote
    }

    // Store the current directory before changing
    // getcwd() gets the current working directory and stores it in current_dir.
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd failed");
        return;
    }

    // Handle 'cd ~' which changes to the home directory
    if (strcmp(path, "~") == 0) {
        path = home_dir;
    }
    // Handle 'cd -' which changes to the previous directory
    else if (strcmp(path, "-") == 0) {
        // If no previous directory is stored, print a message.
        if (strlen(prev_dir) == 0) {
            printf("No previous directory\n");
            return;
        }
        // Temporarily store the previous directory to switch back.
        strcpy(temp_dir, prev_dir);  // Temporarily store previous directory
        strcpy(prev_dir, current_dir);  // Update previous directory
        path = temp_dir;  // Set path to previous directory
    } else {
        // For normal 'cd' commands, update the previous directory.
        strcpy(prev_dir, current_dir);  // Update previous directory for normal cd commands
    }

    // Change the directory
    // chdir() changes the current working directory to the specified path.
    if (chdir(path) != 0) {  // Change the current directory
        perror("Invalid Command");
    }
}

// Function to handle pipes between commands
// This function takes an input string containing two commands separated by a pipe (|).
// It creates a pipe, forks two child processes to execute the commands, and connects them.
void handle_pipe(char *input) {
    int pipefd[2];  // File descriptors for pipe
    char *command1 = strtok(input, "|");  // First command
    char *command2 = strtok(NULL, "|");  // Second command

    // Check if the pipe syntax is correct, i.e., both commands are provided.
    if (command1 == NULL || command2 == NULL) {
        printf("Invalid pipe syntax\n");
        return;
    }

    char *args1[100];  // Arguments for the first command
    char *args2[100];  // Arguments for the second command
    char *token;
    int i = 0;

    // Tokenize and store arguments for the first command
    token = strtok(command1, " \n");
    while (token != NULL) {
        args1[i++] = token;
        token = strtok(NULL, " \n");
    }
    args1[i] = NULL;  // End of arguments list

    i = 0;
    // Tokenize and store arguments for the second command
    token = strtok(command2, " \n");
    while (token != NULL) {
        args2[i++] = token;
        token = strtok(NULL, " \n");
    }
    args2[i] = NULL;  // End of arguments list

    pipe(pipefd);  // Create a pipe

    // Fork the first child process to execute the first command
    pid_t pid1 = fork();  

    if (pid1 == 0) {  // Child process 1
        // Redirect the standard output to the write end of the pipe.
        dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to pipe write end
        close(pipefd[0]);  // Close unused read end of the pipe
        close(pipefd[1]);  // Close write end of the pipe
        execvp(args1[0], args1);  // Execute first command
        perror("Invalid Command");
        exit(EXIT_FAILURE);  // Exit if execution fails
    } else if (pid1 < 0) {  // Fork failed
        perror("fork failed");
        return;
    }

    // Fork the second child process to execute the second command
    pid_t pid2 = fork();  

    if (pid2 == 0) {  // Child process 2
        // Redirect the standard input to the read end of the pipe.
        dup2(pipefd[0], STDIN_FILENO);  // Redirect stdin to pipe read end
        close(pipefd[1]);  // Close unused write end of the pipe
        close(pipefd[0]);  // Close read end of the pipe
        execvp(args2[0], args2);  // Execute second command
        perror("Invalid Command");
        exit(EXIT_FAILURE);  // Exit if execution fails
    } else if (pid2 < 0) {  // Fork failed
        perror("fork failed");
        return;
    }

    // Close the pipe ends in the parent process, since they are no longer needed.
    close(pipefd[0]);  // Close pipe ends in parent
    close(pipefd[1]);  // Close pipe ends in parent

    // Wait for both child processes to complete.
    waitpid(pid1, NULL, 0);  // Wait for first child process
    waitpid(pid2, NULL, 0);  // Wait for second child process
}

// Main function
// The main function serves as the command-line interpreter loop, where user input is processed and executed.
// It supports handling of external commands, built-in commands (like 'cd' and 'history'), and pipes.
int main() {
    char input[1024];  // Buffer for input command
    char *args[100];  // Array for command arguments
    char history[100][1024];  // Command history
    int history_count = 0;  // Number of commands in history

    // Command loop
    while (1) {
        printf("MTL458 > ");  // Prompt user for input

        // Read input from stdin and store it in the input buffer.
        // If fgets returns NULL, it indicates an end-of-file or error, and the loop breaks.
        if (!fgets(input, sizeof(input), stdin)) {  
            break;  // Exit loop if end-of-file or error
        }

        // Remove newline character from input
        input[strcspn(input, "\n")] = 0;

        // Check for empty input
        // If the user just presses enter without typing anything, skip processing and prompt again.
        if (strlen(input) == 0) {
            continue;  // Skip processing and prompt again
        }

        // Check if the input contains a pipe
        // If a pipe ('|') is detected in the input, handle_pipe() is called to process it.
        if (strchr(input, '|')) {
            handle_pipe(input);  // Handle pipe commands
            continue;
        }

        // Check if the command is 'exit'
        // The 'exit' command breaks the loop and terminates the program.
        if (strcmp(input, "exit") == 0) {
            break;  // Exit the loop
        }

        // Add command to history
        // The command is stored in the history array for later retrieval.
        strcpy(history[history_count++], input);

        int background = 0;
        // Check if the command is to be executed in the background
        // If the command ends with an '&', it is flagged to run in the background.
        if (input[strlen(input) - 1] == '&') {
            background = 1;
            input[strlen(input) - 1] = 0;  // Remove '&' from the command
        }

        // Tokenize the input command
        // The input is split into separate arguments, which are stored in the args array.
        char *token = strtok(input, " ");
        int i = 0;

        while (token != NULL) {
            args[i++] = token;  // Store argument
            token = strtok(NULL, " ");
        }
        args[i] = NULL;  // End of arguments list

        // Execute built-in commands
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL) {
                change_directory(args[1]);  // Change directory
            } else {
                printf("cd: missing argument\n");
            }
        } else if (strcmp(args[0], "history") == 0) {
            // Print command history
            for (int j = 0; j < history_count; j++) {
                printf("%s\n", history[j]);
            }
        } else {
            // For external commands, execute_command() is called.
            execute_command(args, background);  // Execute external command
        }
    }

    return 0;  // Exit the program
}
