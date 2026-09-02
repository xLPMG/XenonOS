#ifndef COMMANDS_H
#define COMMANDS_H

void execute_command(char *input, int input_length);

void help(void);
void clear(void);
void info(void);
void echo(char *input);

void shutdown(void);

// debug commands
void memtest(void);

#endif