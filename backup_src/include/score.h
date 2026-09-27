#ifndef SCORE_H
#define SCORE_H

#include "quiz.h"

/* Score persistence and reporting functions */
int save_quiz_result(const QuizResult *result);
int load_all_scores(QuizResult results[], int max_results);
void display_scores_menu(void);
void display_all_scores(void);
void display_high_scores(void);
void clear_all_scores(void);

#endif /* SCORE_H */
