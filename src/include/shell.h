#ifndef SHELL_H
#define SHELL_H

void shell_init();
void shell_handle_key(char c); // Nova
void shell_execute(char* command);
void shell_draw(); // Nova

#endif