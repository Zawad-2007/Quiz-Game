#include "quiz.h"
#include "questions.h"
#include "score.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void display_help_guide(void) {
    clear_screen();
    print_banner();
    print_header("HOW TO PLAY & CUSTOMIZE QUIZZES");

    printf(COLOR_BOLD "1. How to Play:\n" COLOR_RESET);
    printf("   - Select 'Start a New Quiz' from the main menu.\n");
    printf("   - Enter your player name.\n");
    printf("   - Pick one of the available subjects.\n");
    printf("   - For each question, input your chosen option (A, B, C, or D).\n");
    printf("   - You'll receive immediate feedback. If you make a mistake, the\n");
    printf("     correct answer and an explanation will be revealed!\n");
    printf("   - Each correct answer awards 10 marks.\n\n");

    printf(COLOR_BOLD "2. Scores & Leaderboards:\n" COLOR_RESET);
    printf("   - Results are automatically saved to '%s'.\n", SCORES_FILE);
    printf("   - View your past scores chronologically or inspect the Hall of Fame\n");
    printf("     to see the top performers ranked by total marks and accuracy!\n\n");

    printf(COLOR_BOLD "3. Adding New Subjects & Questions:\n" COLOR_RESET);
    printf("   - You can add your own quiz questions anytime without recompiling!\n");
    printf("   - Simply create or edit a '.txt' file inside '%s/'.\n", SUBJECTS_DIR);
    printf("   - File Format Example:\n");
    printf(COLOR_DIM);
    printf("       [Subject: My Custom Subject]\n");
    printf("       Q: What is the output of printf(\"%%d\", 2 + 2)?\n");
    printf("       A: 22\n");
    printf("       B: 4\n");
    printf("       C: Error\n");
    printf("       D: Undefined\n");
    printf("       Ans: B\n");
    printf("       Exp: 2 + 2 evaluates to arithmetic 4.\n");
    printf("       ---\n");
    printf(COLOR_RESET);

    pause_for_user();
}

int main(void) {
    /* Seed pseudo-random number generator */
    srand((unsigned int)time(NULL));

    /* Ensure initial subjects and data directories exist */
    ensure_default_subject_files();

    int choice;
    do {
        clear_screen();
        print_banner();
        print_header("MAIN MENU");

        printf("  1. Start a New Quiz\n");
        printf("  2. View Scores & Leaderboard\n");
        printf("  3. How to Play & Custom Subjects\n");
        printf("  4. Exit\n\n");

        choice = get_safe_int("Enter your choice (1-4): ", 1, 4);

        switch (choice) {
            case 1:
                start_quiz_session();
                break;
            case 2:
                display_scores_menu();
                break;
            case 3:
                display_help_guide();
                break;
            case 4:
                clear_screen();
                print_banner();
                printf(COLOR_GREEN "\nThank you for playing CLI Quiz Game! Have a great day!\n\n" COLOR_RESET);
                break;
            default:
                break;
        }
    } while (choice != 4);

    return 0;
}
