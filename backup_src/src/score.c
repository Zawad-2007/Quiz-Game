#include "score.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RECORDS 200

/* Comparison function for qsort to rank results by marks (desc), then accuracy (desc) */
static int compare_results(const void *a, const void *b) {
    const QuizResult *r1 = (const QuizResult *)a;
    const QuizResult *r2 = (const QuizResult *)b;

    if (r2->total_marks != r1->total_marks) {
        return r2->total_marks - r1->total_marks;
    }
    if (r2->accuracy > r1->accuracy) return 1;
    if (r2->accuracy < r1->accuracy) return -1;
    return 0;
}

static void safe_copy(char *dest, size_t dest_size, const char *src) {
    if (!dest || dest_size == 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    size_t i = 0;
    while (i + 1 < dest_size && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static char *next_field(char **cursor) {
    if (!cursor || !*cursor) return NULL;
    char *start = *cursor;
    char *delim = strchr(start, '|');
    if (delim) {
        *delim = '\0';
        *cursor = delim + 1;
    } else {
        *cursor = NULL;
    }
    return start;
}

int save_quiz_result(const QuizResult *result) {
    FILE *fp = fopen(SCORES_FILE, "a");
    if (!fp) {
        /* Attempt to create data directory if missing */
#ifdef _WIN32
        system("mkdir data >nul 2>&1");
#else
        system("mkdir -p data >/dev/null 2>&1");
#endif
        fp = fopen(SCORES_FILE, "a");
        if (!fp) {
            printf(COLOR_RED "Error: Unable to open score file for writing.\n" COLOR_RESET);
            return -1;
        }
    }

    fprintf(fp, "%s|%s|%d|%d|%d|%d|%.2f|%s\n",
            result->player_name,
            result->subject_name,
            result->total_questions,
            result->correct_answers,
            result->incorrect_answers,
            result->total_marks,
            result->accuracy,
            result->timestamp);

    fclose(fp);
    return 0;
}

int load_all_scores(QuizResult results[], int max_results) {
    FILE *fp = fopen(SCORES_FILE, "r");
    if (!fp) {
        return 0;
    }

    char line[512];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < max_results) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;

        char *token;
        char *rest = line;

        /* 1. Player Name */
        token = next_field(&rest);
        if (!token) continue;
        safe_copy(results[count].player_name, sizeof(results[count].player_name), token);

        /* 2. Subject Name */
        token = next_field(&rest);
        if (!token) continue;
        safe_copy(results[count].subject_name, sizeof(results[count].subject_name), token);

        /* 3. Total Questions */
        token = next_field(&rest);
        if (!token) continue;
        results[count].total_questions = atoi(token);

        /* 4. Correct Answers */
        token = next_field(&rest);
        if (!token) continue;
        results[count].correct_answers = atoi(token);

        /* 5. Incorrect Answers */
        token = next_field(&rest);
        if (!token) continue;
        results[count].incorrect_answers = atoi(token);

        /* 6. Total Marks */
        token = next_field(&rest);
        if (!token) continue;
        results[count].total_marks = atoi(token);

        /* 7. Accuracy */
        token = next_field(&rest);
        if (!token) continue;
        results[count].accuracy = atof(token);

        /* 8. Timestamp */
        token = next_field(&rest);
        if (token) {
            safe_copy(results[count].timestamp, sizeof(results[count].timestamp), token);
        } else {
            safe_copy(results[count].timestamp, sizeof(results[count].timestamp), "N/A");
        }

        count++;
    }

    fclose(fp);
    return count;
}

static void print_table_header(bool show_rank) {
    printf(COLOR_CYAN);
    if (show_rank) {
        printf("+------+--------------------+------------------------+-------+-----------+----------+-------------------+\n");
        printf("| Rank | Player Name        | Subject                | Score | Correct   | Accuracy | Date / Time       |\n");
        printf("+------+--------------------+------------------------+-------+-----------+----------+-------------------+\n");
    } else {
        printf("+----+--------------------+------------------------+-------+-----------+----------+-------------------+\n");
        printf("| #  | Player Name        | Subject                | Score | Correct   | Accuracy | Date / Time       |\n");
        printf("+----+--------------------+------------------------+-------+-----------+----------+-------------------+\n");
    }
    printf(COLOR_RESET);
}

static void print_table_footer(bool show_rank) {
    printf(COLOR_CYAN);
    if (show_rank) {
        printf("+------+--------------------+------------------------+-------+-----------+----------+-------------------+\n");
    } else {
        printf("+----+--------------------+------------------------+-------+-----------+----------+-------------------+\n");
    }
    printf(COLOR_RESET);
}

void display_all_scores(void) {
    clear_screen();
    print_banner();
    print_header("PAST QUIZ RESULTS (CHRONOLOGICAL)");

    QuizResult records[MAX_RECORDS];
    int count = load_all_scores(records, MAX_RECORDS);

    if (count == 0) {
        printf(COLOR_YELLOW "\nNo previous quiz records found. Play a quiz first!\n" COLOR_RESET);
        pause_for_user();
        return;
    }

    print_table_header(false);
    for (int i = 0; i < count; i++) {
        char correct_str[32];
        snprintf(correct_str, sizeof(correct_str), "%d/%d", records[i].correct_answers, records[i].total_questions);

        printf("| %-2d | %-18.18s | %-22.22s | %-5d | %-9s | %6.1f%%  | %-17.17s |\n",
               i + 1,
               records[i].player_name,
               records[i].subject_name,
               records[i].total_marks,
               correct_str,
               records[i].accuracy,
               records[i].timestamp);
    }
    print_table_footer(false);

    printf(COLOR_GREEN "\nTotal records displayed: %d\n" COLOR_RESET, count);
    pause_for_user();
}

void display_high_scores(void) {
    clear_screen();
    print_banner();
    print_header("HALL OF FAME - HIGH SCORES & LEADERBOARD");

    QuizResult records[MAX_RECORDS];
    int count = load_all_scores(records, MAX_RECORDS);

    if (count == 0) {
        printf(COLOR_YELLOW "\nNo quiz records found to rank. Play a quiz first!\n" COLOR_RESET);
        pause_for_user();
        return;
    }

    /* Sort by marks descending, then accuracy descending */
    qsort(records, count, sizeof(QuizResult), compare_results);

    print_table_header(true);
    for (int i = 0; i < count && i < 15; i++) {
        char correct_str[32];
        snprintf(correct_str, sizeof(correct_str), "%d/%d", records[i].correct_answers, records[i].total_questions);

        const char *rank_color = COLOR_WHITE;
        if (i == 0) rank_color = COLOR_YELLOW COLOR_BOLD; /* Gold */
        else if (i == 1) rank_color = COLOR_WHITE COLOR_BOLD; /* Silver */
        else if (i == 2) rank_color = COLOR_CYAN COLOR_BOLD; /* Bronze */

        printf("%s| %-4d | %-18.18s | %-22.22s | %-5d | %-9s | %6.1f%%  | %-17.17s |" COLOR_RESET "\n",
               rank_color,
               i + 1,
               records[i].player_name,
               records[i].subject_name,
               records[i].total_marks,
               correct_str,
               records[i].accuracy,
               records[i].timestamp);
    }
    print_table_footer(true);

    if (count > 15) {
        printf(COLOR_DIM "(Showing top 15 out of %d records)\n" COLOR_RESET, count);
    }

    pause_for_user();
}

void clear_all_scores(void) {
    clear_screen();
    print_banner();
    print_header("CLEAR SCORE HISTORY");

    printf(COLOR_RED COLOR_BOLD "WARNING: This will permanently delete all saved quiz results!\n" COLOR_RESET);
    bool confirmed = get_safe_yes_no("Are you sure you want to delete all scores? (Y/N): ");

    if (confirmed) {
        FILE *fp = fopen(SCORES_FILE, "w");
        if (fp) {
            fclose(fp);
            printf(COLOR_GREEN "\n[✓] Score history successfully cleared.\n" COLOR_RESET);
        } else {
            printf(COLOR_RED "\n[✗] Failed to clear score history.\n" COLOR_RESET);
        }
    } else {
        printf(COLOR_YELLOW "\n[!] Operation cancelled.\n" COLOR_RESET);
    }

    pause_for_user();
}

void display_scores_menu(void) {
    while (1) {
        clear_screen();
        print_banner();
        print_header("SCORE HISTORY & LEADERBOARDS");

        printf("  1. View All Past Scores (Chronological)\n");
        printf("  2. View High Scores (Leaderboard)\n");
        printf("  3. Clear Score History\n");
        printf("  4. Return to Main Menu\n\n");

        int choice = get_safe_int("Enter your choice (1-4): ", 1, 4);

        switch (choice) {
            case 1:
                display_all_scores();
                break;
            case 2:
                display_high_scores();
                break;
            case 3:
                clear_all_scores();
                break;
            case 4:
                return;
            default:
                break;
        }
    }
}
