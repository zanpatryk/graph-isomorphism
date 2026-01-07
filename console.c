 
#include "console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

void console_init(void) {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode)) {
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

void set_color_green(void) { printf("\033[32m"); }
void set_color_bold(void) { printf("\033[1m"); }
void set_color_reset(void) { printf("\033[0m"); }
void set_color_red(void) { printf("\033[31m"); }

bool prompt_continue(const char *msg) {
    printf("\n%s [Y/n]: ", msg);
    fflush(stdout);
    char buf[16];
    if (!fgets(buf, sizeof(buf), stdin)) return false;
    return (buf[0] != 'N' && buf[0] != 'n');
}

void print_matrix_highlighted(int n, const int *current, const int *previous) {
    printf("    ");
    for (int c = 0; c < n; c++) printf("%4d", c + 1);
    printf("\n");
    for (int r = 0; r < n; r++) {
        printf("%4d", r + 1);
        for (int c = 0; c < n; c++) {
            int curr = current[r * n + c];
            int prev = previous ? previous[r * n + c] : curr;
            if (curr != prev) {
                set_color_bold();
                set_color_green();
            }
            printf("%4d", curr);
            if (curr != prev) set_color_reset();
        }
        printf("\n");
    }
}

void print_matrix_with_mapping(int n_h, const int *adj_h, const int *previous,
                               int n_g, const int *adj_g, const int *mapping) {
    // Build mask of which H cells are part of mapping
    bool *is_mapped = (bool *) calloc((size_t) n_h * n_h, sizeof(bool));
    if (mapping && adj_g) {
        for (int i = 0; i < n_g; i++) {
            for (int j = 0; j < n_g; j++) {
                if (adj_g[i * n_g + j] > 0) {
                    int hi = mapping[i];
                    int hj = mapping[j];
                    is_mapped[hi * n_h + hj] = true;
                }
            }
        }
    }

    printf("    ");
    for (int c = 0; c < n_h; c++) printf("%4d", c + 1);
    printf("\n");

    for (int r = 0; r < n_h; r++) {
        printf("%4d", r + 1);
        for (int c = 0; c < n_h; c++) {
            int idx = r * n_h + c;
            int curr = adj_h[idx];
            int prev = previous ? previous[idx] : curr;
            bool is_new = (curr != prev);
            bool mapped = is_mapped[idx];

            if (is_new) {
                set_color_bold();
                set_color_green();
            } else if (mapped) {
                set_color_bold();
                set_color_red();
            }
            printf("%4d", curr);
            if (is_new || mapped) set_color_reset();
        }
        printf("\n");
    }
    free(is_mapped);
}
