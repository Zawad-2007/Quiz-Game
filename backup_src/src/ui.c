#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

void clear_screen(void) {
    /* Standard ANSI escape sequence to clear terminal screen and move cursor to home */
    printf("\033[H\033[J");
    fflush(stdout);
}

void print_divider(char ch, int length) {
    for (int i = 0; i < length; i++) {
        putchar(ch);
    }
    putchar('\n');
}

void print_banner(void) {
    printf(COLOR_CYAN COLOR_BOLD);
    printf("======================================================================\n");
    printf("     ___  _   _ ___ _____     ____    _    __  __ _____ \n");
    printf("    / _ \\| | | |_ _|__  /    / ___|  / \\  |  \\/  | ____|\n");
    printf("   | | | | | | || |  / /    | |  _  / _ \\ | |\\/| |  _|  \n");
    printf("   | |_| | |_| || | / /_    | |_| |/ ___ \\| |  | | |___ \n");
    printf("    \\__\\_\\\\___/|___/____|    \\____/_/   \\_\\_|  |_|_____|\n");
    printf("======================================================================\n");
    printf(COLOR_YELLOW "               Test Your Skills & Challenge Your Mind!        \n" COLOR_RESET);
    printf("\n");
}

void print_header(const char *title) {
    printf("\n" COLOR_CYAN COLOR_BOLD);
    print_divider('=', 70);
    printf("  %s\n", title);
    print_divider('-', 70);
    printf(COLOR_RESET);
}

void pause_for_user(void) {
    printf(COLOR_DIM "\nPress Enter to continue..." COLOR_RESET);
    int c;
    /* Consume any pending input up to newline */
    while ((c = getchar()) != '\n' && c != EOF) {
        /* empty */
    }
}

int get_safe_int(const char *prompt, int min_val, int max_val) {
    char line[128];
    int choice = 0;
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            continue;
        }

        /* Strip trailing newline */
        line[strcspn(line, "\r\n")] = '\0';

        /* Skip leading whitespace */
        char *ptr = line;
        while (isspace((unsigned char)*ptr)) ptr++;

        if (*ptr == '\0') {
            printf(COLOR_RED "Input cannot be empty. Please try again.\n" COLOR_RESET);
            continue;
        }

        char *endptr;
        choice = (int)strtol(ptr, &endptr, 10);
        while (isspace((unsigned char)*endptr)) endptr++;

        if (*endptr != '\0') {
            printf(COLOR_RED "Invalid number. Please enter a valid choice (%d - %d).\n" COLOR_RESET, min_val, max_val);
            continue;
        }

        if (choice < min_val || choice > max_val) {
            printf(COLOR_RED "Choice out of range. Please enter a number between %d and %d.\n" COLOR_RESET, min_val, max_val);
            continue;
        }

        return choice;
    }
}

void get_safe_string(const char *prompt, char *buffer, size_t max_size) {
    char line[512];
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            continue;
        }

        line[strcspn(line, "\r\n")] = '\0';

        /* Trim leading whitespace */
        char *start = line;
        while (isspace((unsigned char)*start)) start++;

        /* Trim trailing whitespace */
        if (*start != '\0') {
            char *end = start + strlen(start) - 1;
            while (end > start && isspace((unsigned char)*end)) {
                *end = '\0';
                end--;
            }
        }

        if (strlen(start) == 0) {
            printf(COLOR_RED "Name cannot be blank. Please enter at least 1 character.\n" COLOR_RESET);
            continue;
        }

        strncpy(buffer, start, max_size - 1);
        buffer[max_size - 1] = '\0';
        return;
    }
}

char get_option_choice(const char *prompt) {
    char line[128];
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            continue;
        }

        line[strcspn(line, "\r\n")] = '\0';

        char *ptr = line;
        while (isspace((unsigned char)*ptr)) ptr++;

        if (*ptr == '\0') {
            printf(COLOR_RED "Please enter an option (A, B, C, or D).\n" COLOR_RESET);
            continue;
        }

        char ch = (char)toupper((unsigned char)*ptr);

        /* Also allow 1 -> A, 2 -> B, 3 -> C, 4 -> D */
        if (ch == '1') ch = 'A';
        else if (ch == '2') ch = 'B';
        else if (ch == '3') ch = 'C';
        else if (ch == '4') ch = 'D';

        if (ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D') {
            return ch;
        }

        printf(COLOR_RED "Invalid selection '%c'. Please choose A, B, C, or D.\n" COLOR_RESET, *ptr);
    }
}

bool get_safe_yes_no(const char *prompt) {
    char line[128];
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            continue;
        }

        line[strcspn(line, "\r\n")] = '\0';

        char *ptr = line;
        while (isspace((unsigned char)*ptr)) ptr++;

        if (*ptr == '\0') continue;

        char ch = (char)toupper((unsigned char)*ptr);
        if (ch == 'Y') return true;
        if (ch == 'N') return false;

        printf(COLOR_RED "Invalid input. Please enter 'Y' for Yes or 'N' for No.\n" COLOR_RESET);
    }
}
