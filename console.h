#ifndef AAC_CONSOLE_H
#define AAC_CONSOLE_H

#include <stdbool.h>

void console_init(void);

void set_color_green(void);

void set_color_red(void);

void set_color_bold(void);

void set_color_reset(void);

bool prompt_continue(const char *msg);

void print_matrix_highlighted(int n, const int *current, const int *previous);

void print_matrix_with_mapping(int n_h, const int *adj_h, const int *previous,
                               int n_g, const int *adj_g, const int *mapping);

#endif //AAC_CONSOLE_H
