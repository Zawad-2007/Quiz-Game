#ifndef QUIZ_H
#define QUIZ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_QUESTIONS 100
#define MAX_SUBJECTS 20
#define MAX_NAME_LEN 64
#define MAX_TEXT_LEN 512
#define MAX_OPTION_LEN 256
#define MAX_EXPLANATION_LEN 512
#define MAX_PATH_LEN 256

#define SCORES_FILE "data/scores.txt"
#define SUBJECTS_DIR "data/subjects"
#define MARKS_PER_QUESTION 10

typedef struct {
    char question_text[MAX_TEXT_LEN];
    char options[4][MAX_OPTION_LEN];
    int correct_option; // 0 for A, 1 for B, 2 for C, 3 for D
    char explanation[MAX_EXPLANATION_LEN];
} Question;

typedef struct {
    char name[MAX_NAME_LEN];
    char filename[MAX_PATH_LEN];
    int question_count;
} Subject;

typedef struct {
    char player_name[MAX_NAME_LEN];
    char subject_name[MAX_NAME_LEN];
    int total_questions;
    int correct_answers;
    int incorrect_answers;
    int total_marks;
    double accuracy;
    char timestamp[32];
} QuizResult;

/* Core quiz execution functions */
void start_quiz_session(void);
void run_quiz(const char *player_name, const Subject *subject);

#endif /* QUIZ_H */
