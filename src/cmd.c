// SPDX-License-Identifier: BSD-3-Clause

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "cmd.h"
#include "utils.h"

#define READ		0
#define WRITE		1

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	/* TODO: Execute cd. */
	char *path = get_word(dir);
	if (chdir(path) != 0) { return false; }
	return true;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{
	/* TODO: Execute exit/quit. */
	exit(0);

	return 0; /* TODO: Replace with actual exit code. */
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	/* TODO: Sanity checks. */

	if (!s || !s->verb) {
		return 0;
	}

	char *verb = get_word(s->verb);
	if (!verb) { // if the verb provided is invalid
		return 0;
	}

	int output;

	/* TODO: If builtin command, execute the command. */
	if (strcmp(verb, "cd") == 0) { // If the provided verb is a cd command
		int original_output = dup(STDOUT_FILENO); // Save the default output of the shell
		// Redirect command output as needed
		if (s->out) {
			char *output = get_word(s->out);
			int flags = O_WRONLY | O_CREAT;
			
			if (s->io_flags & IO_OUT_APPEND || s->err) {
				flags |= O_APPEND;
			} else {
				flags |= O_TRUNC;
			}
			int out = open(output, flags, 0644);
			if (out < 0) {
				exit(1);
			}
			dup2(out, STDOUT_FILENO);
			close(out);
		}
		// Input redirection
		if (s->in) {
			char *input = get_word(s->in);
			int in = open(input, O_RDONLY);
			if (in < 0) {
				exit(1);
			}
			dup2(in, STDIN_FILENO);
			close(in);
		}
		// Error redirection
		if (s->err) {
			char *file = get_word(s->err);
			int flags = O_WRONLY | O_CREAT;
			if (s->io_flags & IO_ERR_APPEND) {
				flags |= O_APPEND;
			} else {
				flags |= O_TRUNC;
			}
			int des = open(file, flags, 0644);
			if (des < 0) {
				exit(1);
			}
			dup2(des, STDERR_FILENO);
			close(des);
		}
		bool cd_status = shell_cd(s->params); // Perform the cd command 
		dup2(original_output, STDOUT_FILENO); // Reset the default output 
		close(original_output);
		return cd_status ? 0 : 1; // If cd is successful, return 0
	} else if (strcmp(verb, "exit") == 0 || strcmp(verb, "quit") == 0) { // If provided verb is exit or quit, then exit the shell
		return shell_exit();
	}

	/* TODO: If variable assignment, execute the assignment and return
	 * the exit status.
	 */

	char *equals = strchr(verb, '=');
	if (equals != NULL) { // If provided verb is equals, perform variable assignment
		*equals = '\0'; // Seperate the variable name from the value using the equals sign 
		char *name = verb; // Left side of equals
		char *value = equals + 1; // Right side of equals

		// Perform the variable assignment using setenv
		int rc = setenv(name, value, 1);
		return rc;
	}

	/* TODO: If external command:
	 *   1. Fork new process
	 *     2c. Perform redirections in child
	 *     3c. Load executable in child
	 *   2. Wait for child
	 *   3. Return exit status
	 */
	pid_t pid = fork(); // Create child process
	if (pid < 0) {
		perror("fork");
		return 1;
	} else if (pid == 0) {
		// Redirect inputs and outputs as needed (my helper function)
		if (s->out) {
			char *output = get_word(s->out);
			int flags = O_WRONLY | O_CREAT;
			
			if (s->io_flags & IO_OUT_APPEND || s->err) {
				flags |= O_APPEND;
			} else {
				flags |= O_TRUNC;
			}
			int out = open(output, flags, 0644);
			if (out < 0) {
				exit(1);
			}
			dup2(out, STDOUT_FILENO);
			close(out);
		}
		// Input redirection
		if (s->in) {
			char *input = get_word(s->in);
			int in = open(input, O_RDONLY);
			if (in < 0) {
				exit(1);
			}
			dup2(in, STDIN_FILENO);
			close(in);
		}
		// Error redirection
		if (s->err) {
			char *file = get_word(s->err);
			int flags = O_WRONLY | O_CREAT;
			if (s->io_flags & IO_ERR_APPEND) {
				flags |= O_APPEND;
			} else {
				flags |= O_TRUNC;
			}
			int des = open(file, flags, 0644);
			if (des < 0) {
				exit(1);
			}
			dup2(des, STDERR_FILENO);
			close(des);
		}

		int params = 0; // number of parameters
		word_t *word = s->params;
		while (word) { // Count the number of parameters for memory allocation
			params++;
			word = word->next_word;
		}
		char **arguments = calloc(params + 2, sizeof(char *)); // Allocate the space of the parameters to a char array
		if (!arguments) { // Error catching
			exit(1);
		}
		
		arguments[0] = verb;
		int i = 1;
		word = s->params;
		while (word) {
			arguments[i++] = get_word(word); // Assign the parameters to array
			word = word->next_word;
		}
		arguments[i] = NULL;

		execvp(arguments[0], arguments); // Execute the command with the argument array

		fprintf(stderr, "Execution failed for '%s'\n", verb); // Error message
		free(arguments);
		exit(1);
	} else {
		int status;
		waitpid(pid, &status, 0); // Have parent wait for child

		// Return exit code of child
		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		return status;
	}

	return output; /* TODO: Replace with actual exit status. */
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool run_in_parallel(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* TODO: Execute cmd1 and cmd2 simultaneously. */
	pid_t process1 = fork(); // Fork for the first process
    if (process1 < 0) {
        perror("fork");
        return false;
    }

    if (process1 == 0) { // if first child, run the first command
        int stat = parse_command(cmd1, level + 1, father);
        exit(stat); 
    }

    pid_t process2 = fork(); // Forkk for the second process
    if (process2 < 0) {
        perror("fork");
        return false;
    }

    if (process2 == 0) { // if second child, run the second command
        int out = parse_command(cmd2, level + 1, father);
        exit(out);
    }

    int status1, status2;
    waitpid(process1, &status1, 0); // Wait until both processes are completed
    waitpid(process2, &status2, 0);
	// Check if both processes executed successfully, if so return true
    bool success1 = (WIFEXITED(status1) && (WEXITSTATUS(status1) == 0));
    bool success2 = (WIFEXITED(status2) && (WEXITSTATUS(status2) == 0));

    return (success1 && success2);
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2).
 */
static bool run_on_pipe(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* TODO: Redirect the output of cmd1 to the input of cmd2. */
	int des[4];
    if (pipe(des) < 0) {
        return false;
    }

    pid_t process1 = fork(); // Create child process for first command
    if (process1 < 0) {
        return false;
    }
	
    if (process1 == 0) {
        dup2(des[1], STDOUT_FILENO); // Redirect output to pipe

        close(des[0]);
        close(des[1]);

        int out = parse_command(cmd1, level + 1, father); // Execute the command
        exit(out); // Exit process
    }

    pid_t process2 = fork(); // Create second child process for second command
    if (process2 < 0) {
        return false;
    }

    if (process2 == 0) {
        dup2(des[0], STDIN_FILENO); //Redirect outpout to pipe
        close(des[1]);
        close(des[0]);

        int out = parse_command(cmd2, level + 1, father); // Execute the command
        exit(out);
    }

    int status1, status2;
    close(des[0]);
    close(des[1]);

    waitpid(process1, &status1, 0); // Wait until both children are finished
    waitpid(process2, &status2, 0);

	// Return true of processes are successful

    bool success = (WIFEXITED(status2) && (WEXITSTATUS(status2) == 0));
    return success;
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	/* TODO: sanity checks */

	if (!c) {
        return 0;
    }

    if (c->op == OP_NONE) { // If no operator then it is a simple command
        return parse_simple(c->scmd, level, c);
    }

    switch (c->op) {
    case OP_SEQUENTIAL: {
		// If ;, it is sequential
        parse_command(c->cmd1, level + 1, c);
        int out = parse_command(c->cmd2, level + 1, c);
        return out; 
    }
    case OP_PARALLEL: {
        // If &, it is parallel
        bool success = run_in_parallel(c->cmd1, c->cmd2, level + 1, c);
        return success ? 0 : 1;
    }
    case OP_CONDITIONAL_NZERO: {
        // If ||, it is conditional nzero
        int out = parse_command(c->cmd1, level + 1, c);
        if (out != 0) {return parse_command(c->cmd2, level + 1, c);}
        return out; 
    }
    case OP_CONDITIONAL_ZERO: {
		// If &&, its condiational zero
        int out = parse_command(c->cmd1, level + 1, c);
        if (out == 0) {
            return parse_command(c->cmd2, level + 1, c);
        }
        return out;
    }
    case OP_PIPE: {
		// If |, its pipe
        bool success = run_on_pipe(c->cmd1, c->cmd2, level + 1, c);
        return success ? 0 : 1;
    }
    default:
        return SHELL_EXIT;
    }
}
