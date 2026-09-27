#ifndef UI_H
#define UI_H

#include <stddef.h>
#include <stdbool.h>

/* ANSI Terminal Styling Codes */
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_DIM     "\033[2m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

/* Background Colors */
#define BG_BLUE       "\033[44m"
#define BG_GREEN      "\033[42m"
#define BG_RED        "\033[41m"
#define BG_CYAN       "\033[46m"

/* UI & Formatting Helpers */
void clear_screen(void);
void print_banner(void);
void print_header(const char *title);
void print_divider(char ch, int length);
void pause_for_user(void);
int get_safe_int(const char *prompt, int min_val, int max_val);
void get_safe_string(const char *prompt, char *buffer, size_t max_size);
char get_option_choice(const char *prompt);
bool get_safe_yes_no(const char *prompt);

#endif /* UI_H */
