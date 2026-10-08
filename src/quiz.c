#include "../include/quiz.h"
#include "../include/questions.h"
#include "../include/score.h"
#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Helper to get formatted current local timestamp */
static void get_current_timestamp(char *buffer, size_t max_size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (tm_info) {
        strftime(buffer, max_size, "%Y-%m-%d %H:%M", tm_info);
    } else {
        strncpy(buffer, "Unknown", max_size - 1);
        buffer[max_size - 1] = '\0';
    }
}

/* Fisher-Yates shuffle for question array */
static void shuffle_questions(Question arr[], int n) {
    if (n <= 1) return;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Question temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

static const char *get_performance_rating(double accuracy) {
    if (accuracy >= 90.0) return COLOR_GREEN COLOR_BOLD "Outstanding! ★★★★★" COLOR_RESET;
    if (accuracy >= 75.0) return COLOR_CYAN COLOR_BOLD "Excellent! ★★★★☆" COLOR_RESET;
    if (accuracy >= 60.0) return COLOR_YELLOW COLOR_BOLD "Good Job! ★★★☆☆" COLOR_RESET;
    if (accuracy >= 40.0) return COLOR_MAGENTA "Average - Keep Practicing! ★★☆☆☆" COLOR_RESET;
    return COLOR_RED "Needs Improvement! ★☆☆☆☆" COLOR_RESET;
}

void run_quiz(const char *player_name, const Subject *subject) {
    Question questions[MAX_QUESTIONS];
    int q_count = load_questions_from_file(subject->filename, questions, MAX_QUESTIONS);

    if (q_count <= 0) {
        printf(COLOR_RED "\nError: No questions found for subject '%s'.\n" COLOR_RESET, subject->name);
        pause_for_user();
        return;
    }

    /* Ask if player wants questions shuffled */
    clear_screen();
    print_banner();
    print_header("QUIZ SETTINGS");
    printf("Subject: " COLOR_CYAN "%s" COLOR_RESET " (%d questions available)\n\n", subject->name, q_count);
    bool shuffle = get_safe_yes_no("Would you like to randomize question order? (Y/N): ");

    if (shuffle) {
        shuffle_questions(questions, q_count);
    }

    int correct = 0;
    int incorrect = 0;
    int total_marks = 0;

    for (int i = 0; i < q_count; i++) {
        clear_screen();
        print_banner();

        /* Status header */
        printf(COLOR_BLUE "======================================================================\n" COLOR_RESET);
        printf(COLOR_BOLD " Question %d of %d" COLOR_RESET " | Player: " COLOR_YELLOW "%s" COLOR_RESET
               " | Subject: " COLOR_CYAN "%s" COLOR_RESET
               " | Score: " COLOR_GREEN "%d" COLOR_RESET "\n",
               i + 1, q_count, player_name, subject->name, total_marks);
        printf(COLOR_BLUE "======================================================================\n\n" COLOR_RESET);

        /* Question text */
        printf(COLOR_WHITE COLOR_BOLD "Q%d: %s\n\n" COLOR_RESET, i + 1, questions[i].question_text);

        /* Options */
        const char opt_letters[] = {'A', 'B', 'C', 'D'};
        for (int opt = 0; opt < 4; opt++) {
            printf("   " COLOR_CYAN "[%c]" COLOR_RESET " %s\n", opt_letters[opt], questions[i].options[opt]);
        }
        printf("\n");

        /* Get answer */
        char answer = get_option_choice("Your answer (A, B, C, or D): ");
        int chosen_idx = answer - 'A';

        printf("\n");
        print_divider('-', 70);

        if (chosen_idx == questions[i].correct_option) {
            correct++;
            total_marks += MARKS_PER_QUESTION;
            printf(COLOR_GREEN COLOR_BOLD " [✓] CORRECT! " COLOR_RESET
                   COLOR_GREEN "(+%d marks)\n" COLOR_RESET, MARKS_PER_QUESTION);
        } else {
            incorrect++;
            char correct_char = opt_letters[questions[i].correct_option];
            printf(COLOR_RED COLOR_BOLD " [✗] INCORRECT!\n" COLOR_RESET);
            printf("     Your answer   : " COLOR_RED "[%c] %s\n" COLOR_RESET,
                   answer, questions[i].options[chosen_idx]);
            printf("     Correct answer: " COLOR_GREEN "[%c] %s\n" COLOR_RESET,
                   correct_char, questions[i].options[questions[i].correct_option]);
        }

        if (strlen(questions[i].explanation) > 0) {
            printf(COLOR_DIM "\n Explanation: %s\n" COLOR_RESET, questions[i].explanation);
        }
        print_divider('-', 70);

        if (i < q_count - 1) {
            pause_for_user();
        } else {
            printf(COLOR_CYAN "\nYou have completed all questions! Press Enter to view your final score..." COLOR_RESET);
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
        }
    }

    /* Calculate results */
    double accuracy = (q_count > 0) ? ((double)correct / (double)q_count) * 100.0 : 0.0;
    int max_possible_marks = q_count * MARKS_PER_QUESTION;

    QuizResult result;
    memset(&result, 0, sizeof(QuizResult));
    snprintf(result.player_name, sizeof(result.player_name), "%s", player_name);
    snprintf(result.subject_name, sizeof(result.subject_name), "%s", subject->name);
    result.total_questions = q_count;
    result.correct_answers = correct;
    result.incorrect_answers = incorrect;
    result.total_marks = total_marks;
    result.accuracy = accuracy;
    get_current_timestamp(result.timestamp, sizeof(result.timestamp));

    /* Save score */
    save_quiz_result(&result);

    /* Display final scorecard */
    clear_screen();
    print_banner();
    print_header("QUIZ PERFORMANCE REPORT");

    char marks_str[64];
    snprintf(marks_str, sizeof(marks_str), "%d / %d", result.total_marks, max_possible_marks);

    char accuracy_str[64];
    snprintf(accuracy_str, sizeof(accuracy_str), "%.2f%%", result.accuracy);

    printf("\n");
    printf(COLOR_CYAN "+--------------------------------------------------------------------+\n" COLOR_RESET);
    printf("| " COLOR_BOLD "%-20s" COLOR_RESET " : %-43s |\n", "Player Name", result.player_name);
    printf("| " COLOR_BOLD "%-20s" COLOR_RESET " : %-43s |\n", "Subject", result.subject_name);
    printf("| " COLOR_BOLD "%-20s" COLOR_RESET " : %-43s |\n", "Date & Time", result.timestamp);
    printf(COLOR_CYAN "+--------------------------------------------------------------------+\n" COLOR_RESET);
    printf("| " COLOR_BOLD "%-20s" COLOR_RESET " : %-43d |\n", "Total Questions", result.total_questions);
    printf("| " COLOR_BOLD "%-20s" COLOR_RESET " : " COLOR_GREEN "%-43d" COLOR_RESET " |\n", "Correct Answers", result.correct_answers);
    printf("| " COLOR_BOLD "%-20s" COLOR_RESET " : " COLOR_RED "%-43d" COLOR_RESET " |\n", "Incorrect Answers", result.incorrect_answers);
    printf("| " COLOR_BOLD "%-20s" COLOR_RESET " : %-43s |\n", "Accuracy", accuracy_str);
    printf("| " COLOR_BOLD "%-20s" COLOR_RESET " : " COLOR_YELLOW COLOR_BOLD "%-43s" COLOR_RESET " |\n", "Total Marks", marks_str);
    printf(COLOR_CYAN "+--------------------------------------------------------------------+\n" COLOR_RESET);
    printf("  Performance Rating: %s\n\n", get_performance_rating(result.accuracy));
    printf(COLOR_GREEN " [✓] Score successfully saved to history.\n" COLOR_RESET);

    pause_for_user();
}

void start_quiz_session(void) {
    clear_screen();
    print_banner();
    print_header("NEW QUIZ - REGISTRATION");

    char player_name[MAX_NAME_LEN];
    get_safe_string("Enter your name: ", player_name, sizeof(player_name));

    Subject subjects[MAX_SUBJECTS];
    int subject_count = discover_subjects(subjects, MAX_SUBJECTS);

    if (subject_count == 0) {
        printf(COLOR_RED "\nNo quiz subjects found in '%s'.\n" COLOR_RESET, SUBJECTS_DIR);
        pause_for_user();
        return;
    }

    clear_screen();
    print_banner();
    print_header("CHOOSE A SUBJECT");

    printf("Welcome, " COLOR_BOLD COLOR_YELLOW "%s" COLOR_RESET "! Select a subject to begin:\n\n", player_name);
    for (int i = 0; i < subject_count; i++) {
        printf("  %2d. %-32s " COLOR_DIM "(%d questions)" COLOR_RESET "\n",
               i + 1, subjects[i].name, subjects[i].question_count);
    }
    printf("\n   0. Return to Main Menu\n\n");

    int choice = get_safe_int("Select subject number (0 to return): ", 0, subject_count);
    if (choice == 0) {
        return;
    }

    run_quiz(player_name, &subjects[choice - 1]);
}
