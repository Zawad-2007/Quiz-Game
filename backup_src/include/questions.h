#ifndef QUESTIONS_H
#define QUESTIONS_H

#include "quiz.h"

/* Subject discovery and question loading */
int discover_subjects(Subject subjects[], int max_subjects);
int load_questions_from_file(const char *filepath, Question questions[], int max_questions);
void ensure_default_subject_files(void);

#endif /* QUESTIONS_H */
