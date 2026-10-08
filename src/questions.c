#include "../include/questions.h"
#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define CREATE_DIR(dir) _mkdir(dir)
#else
#define CREATE_DIR(dir) mkdir(dir, 0755)
#endif

/* Helper to strip leading and trailing whitespace */
static void trim(char *str) {
    if (!str) return;
    char *start = str;
    while (isspace((unsigned char)*start)) start++;

    char *end = start + strlen(start) - 1;
    while (end >= start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

/* Safely copy string and guarantee null-termination */
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

/* Parse a single subject file to determine its display name and question count */
static int inspect_subject_file(const char *filepath, char *out_name, size_t max_name_len) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) return 0;

    char line[512];
    int question_count = 0;
    out_name[0] = '\0';

    while (fgets(line, sizeof(line), fp)) {
        trim(line);
        if (line[0] == '\0' || line[0] == '#') continue;

        if (strncmp(line, "[Subject:", 9) == 0) {
            char *closing = strchr(line, ']');
            if (closing) *closing = '\0';
            char *sname = line + 9;
            trim(sname);
            safe_copy(out_name, max_name_len, sname);
        } else if (strncmp(line, "Q:", 2) == 0) {
            question_count++;
        }
    }
    fclose(fp);

    /* Fallback subject name from filename if tag was not found */
    if (out_name[0] == '\0') {
        const char *base = strrchr(filepath, '/');
#ifdef _WIN32
        const char *base_win = strrchr(filepath, '\\');
        if (base_win && (!base || base_win > base)) base = base_win;
#endif
        base = base ? base + 1 : filepath;
        safe_copy(out_name, max_name_len, base);
        char *dot = strrchr(out_name, '.');
        if (dot) *dot = '\0';
    }

    return question_count;
}

int load_questions_from_file(const char *filepath, Question questions[], int max_questions) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        perror("Error opening question file");
        return 0;
    }

    char line[512];
    int count = 0;
    Question current_q;
    memset(&current_q, 0, sizeof(Question));
    current_q.correct_option = -1;
    bool in_question = false;

    while (fgets(line, sizeof(line), fp) && count < max_questions) {
        trim(line);
        if (line[0] == '\0' || line[0] == '#') continue;

        if (strncmp(line, "Q:", 2) == 0) {
            in_question = true;
            char *text = line + 2;
            trim(text);
            safe_copy(current_q.question_text, sizeof(current_q.question_text), text);
        } else if (strncmp(line, "A:", 2) == 0) {
            char *text = line + 2;
            trim(text);
            safe_copy(current_q.options[0], sizeof(current_q.options[0]), text);
        } else if (strncmp(line, "B:", 2) == 0) {
            char *text = line + 2;
            trim(text);
            safe_copy(current_q.options[1], sizeof(current_q.options[1]), text);
        } else if (strncmp(line, "C:", 2) == 0) {
            char *text = line + 2;
            trim(text);
            safe_copy(current_q.options[2], sizeof(current_q.options[2]), text);
        } else if (strncmp(line, "D:", 2) == 0) {
            char *text = line + 2;
            trim(text);
            safe_copy(current_q.options[3], sizeof(current_q.options[3]), text);
        } else if (strncmp(line, "Ans:", 4) == 0 || strncmp(line, "Correct:", 8) == 0) {
            char *text = strchr(line, ':') + 1;
            trim(text);
            char ch = (char)toupper((unsigned char)text[0]);
            if (ch >= 'A' && ch <= 'D') {
                current_q.correct_option = ch - 'A';
            } else if (ch >= '1' && ch <= '4') {
                current_q.correct_option = ch - '1';
            }
        } else if (strncmp(line, "Exp:", 4) == 0 || strncmp(line, "Explanation:", 12) == 0) {
            char *text = strchr(line, ':') + 1;
            trim(text);
            safe_copy(current_q.explanation, sizeof(current_q.explanation), text);
        } else if (strncmp(line, "---", 3) == 0) {
            if (in_question && current_q.correct_option >= 0 && current_q.correct_option < 4) {
                questions[count++] = current_q;
            }
            memset(&current_q, 0, sizeof(Question));
            current_q.correct_option = -1;
            in_question = false;
        }
    }

    /* Save trailing question if file didn't end with delimiter */
    if (in_question && current_q.correct_option >= 0 && current_q.correct_option < 4 && count < max_questions) {
        questions[count++] = current_q;
    }

    fclose(fp);
    return count;
}

static int compare_subjects(const void *a, const void *b) {
    const Subject *s1 = (const Subject *)a;
    const Subject *s2 = (const Subject *)b;
    return strcmp(s1->name, s2->name);
}

int discover_subjects(Subject subjects[], int max_subjects) {
    ensure_default_subject_files();

    DIR *dir = opendir(SUBJECTS_DIR);
    if (!dir) {
        return 0;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL && count < max_subjects) {
        /* Filter for .txt files */
        char *ext = strrchr(entry->d_name, '.');
        if (!ext || strcmp(ext, ".txt") != 0) {
            continue;
        }

        char path[MAX_PATH_LEN * 2];
        snprintf(path, sizeof(path), "%s/%s", SUBJECTS_DIR, entry->d_name);

        char display_name[MAX_NAME_LEN];
        int q_count = inspect_subject_file(path, display_name, sizeof(display_name));

        if (q_count > 0) {
            safe_copy(subjects[count].name, sizeof(subjects[count].name), display_name);
            safe_copy(subjects[count].filename, sizeof(subjects[count].filename), path);
            subjects[count].question_count = q_count;
            count++;
        }
    }

    closedir(dir);

    /* Sort subjects alphabetically */
    if (count > 1) {
        qsort(subjects, count, sizeof(Subject), compare_subjects);
    }

    return count;
}

static void create_file_if_missing(const char *path, const char *content) {
    FILE *fp = fopen(path, "r");
    if (fp) {
        fclose(fp);
        return; /* File already exists */
    }

    fp = fopen(path, "w");
    if (fp) {
        fputs(content, fp);
        fclose(fp);
    }
}

void ensure_default_subject_files(void) {
    CREATE_DIR("data");
    CREATE_DIR(SUBJECTS_DIR);

    /* 1. C Programming Question Bank */
    const char *c_prog =
        "[Subject: C Programming]\n"
        "# Multiple-Choice Questions for C Programming\n"
        "\n"
        "Q: Which format specifier is used to print an integer in C?\n"
        "A: %f\n"
        "B: %d\n"
        "C: %c\n"
        "D: %s\n"
        "Ans: B\n"
        "Exp: %d (or %i) is used for signed decimal integers.\n"
        "---\n"
        "Q: What is the size of 'char' data type in C standard?\n"
        "A: 1 byte\n"
        "B: 2 bytes\n"
        "C: 4 bytes\n"
        "D: Architecture dependent\n"
        "Ans: A\n"
        "Exp: In C, sizeof(char) is guaranteed to be 1 byte.\n"
        "---\n"
        "Q: Which header file is required to use malloc() and free()?\n"
        "A: <stdio.h>\n"
        "B: <string.h>\n"
        "C: <stdlib.h>\n"
        "D: <math.h>\n"
        "Ans: C\n"
        "Exp: Dynamic memory management functions are declared in <stdlib.h>.\n"
        "---\n"
        "Q: What does the '&' operator represent when placed before a variable name?\n"
        "A: Bitwise NOT\n"
        "B: Logical AND\n"
        "C: Address-of operator\n"
        "D: Pointer dereference\n"
        "Ans: C\n"
        "Exp: The '&' operator retrieves the memory address of its operand.\n"
        "---\n"
        "Q: Which symbol is used to terminate a statement in C?\n"
        "A: Period (.)\n"
        "B: Colon (:)\n"
        "C: Semicolon (;)\n"
        "D: Comma (,)\n"
        "Ans: C\n"
        "Exp: In C syntax, all expressions and statements must end with a semicolon (;).\n"
        "---\n"
        "Q: What value is represented by NULL pointer in C?\n"
        "A: Undefined\n"
        "B: Zero (0)\n"
        "C: Negative One (-1)\n"
        "D: 255\n"
        "Ans: B\n"
        "Exp: A NULL pointer is conceptually an address constant equal to 0.\n"
        "---\n"
        "Q: Which keyword is used to prevent modification of a variable's value?\n"
        "A: static\n"
        "B: volatile\n"
        "C: register\n"
        "D: const\n"
        "Ans: D\n"
        "Exp: The const keyword marks a variable as read-only.\n"
        "---\n"
        "Q: What is the return type of fopen() when it fails to open a file?\n"
        "A: -1\n"
        "B: EOF\n"
        "C: NULL\n"
        "D: False\n"
        "Ans: C\n"
        "Exp: fopen returns a valid FILE pointer on success, or NULL on error.\n"
        "---\n"
        "Q: Which loop structure guarantees at least one execution of its body?\n"
        "A: for loop\n"
        "B: while loop\n"
        "C: do-while loop\n"
        "D: nested for loop\n"
        "Ans: C\n"
        "Exp: The do-while loop tests its condition at the end of each iteration.\n"
        "---\n"
        "Q: In C, string literals are terminated by which special character?\n"
        "A: '\\n'\n"
        "B: '\\t'\n"
        "C: '\\0'\n"
        "D: '$'\n"
        "Ans: C\n"
        "Exp: Strings in C are null-terminated character arrays ending with '\\0'.\n"
        "---\n";

    /* 2. Computer Science Fundamentals */
    const char *cs_fund =
        "[Subject: Computer Science Fundamentals]\n"
        "# Multiple-Choice Questions for Computer Science\n"
        "\n"
        "Q: Which data structure operates on a First-In-First-Out (FIFO) basis?\n"
        "A: Stack\n"
        "B: Queue\n"
        "C: Binary Tree\n"
        "D: Hash Map\n"
        "Ans: B\n"
        "Exp: A Queue processes elements in FIFO order, while a Stack uses LIFO.\n"
        "---\n"
        "Q: What is the average time complexity of searching in a balanced Binary Search Tree?\n"
        "A: O(1)\n"
        "B: O(n)\n"
        "C: O(log n)\n"
        "D: O(n log n)\n"
        "Ans: C\n"
        "Exp: Balanced BSTs halve the search space at each step, yielding O(log n).\n"
        "---\n"
        "Q: Which layer of the OSI model is responsible for end-to-end communication and port addressing?\n"
        "A: Physical Layer\n"
        "B: Network Layer\n"
        "C: Transport Layer\n"
        "D: Session Layer\n"
        "Ans: C\n"
        "Exp: Transport Layer (Layer 4) handles TCP/UDP and end-to-end reliability.\n"
        "---\n"
        "Q: What is the primary purpose of Virtual Memory in modern operating systems?\n"
        "A: To speed up CPU clock rate\n"
        "B: To allow processes to use more memory than physically available RAM\n"
        "C: To replace SSDs completely\n"
        "D: To encrypt files on disk\n"
        "Ans: B\n"
        "Exp: Virtual memory provides an illusion of contiguous large memory via paging.\n"
        "---\n"
        "Q: Which scheduling algorithm may suffer from the 'convoy effect'?\n"
        "A: Round Robin (RR)\n"
        "B: Shortest Job First (SJF)\n"
        "C: First-Come, First-Served (FCFS)\n"
        "D: Priority Scheduling\n"
        "Ans: C\n"
        "Exp: In FCFS, short processes get stuck waiting behind long CPU-bound processes.\n"
        "---\n"
        "Q: What is the binary representation of the decimal number 13?\n"
        "A: 1100\n"
        "B: 1101\n"
        "C: 1011\n"
        "D: 1110\n"
        "Ans: B\n"
        "Exp: 8 + 4 + 1 = 13, which is 1101 in binary.\n"
        "---\n"
        "Q: In relational databases, what does SQL stand for?\n"
        "A: Standard Query Logic\n"
        "B: Structured Query Language\n"
        "C: Simple Question Language\n"
        "D: Sequential Query List\n"
        "Ans: B\n"
        "Exp: SQL stands for Structured Query Language.\n"
        "---\n"
        "Q: Which algorithm design paradigm does Merge Sort use?\n"
        "A: Greedy Approach\n"
        "B: Dynamic Programming\n"
        "C: Divide and Conquer\n"
        "D: Backtracking\n"
        "Ans: C\n"
        "Exp: Merge Sort divides the array into halves, sorts recursively, and merges.\n"
        "---\n";

    /* 3. General Knowledge */
    const char *gk =
        "[Subject: General Knowledge]\n"
        "# Multiple-Choice Questions for General Knowledge\n"
        "\n"
        "Q: What is the capital city of Australia?\n"
        "A: Sydney\n"
        "B: Melbourne\n"
        "C: Canberra\n"
        "D: Brisbane\n"
        "Ans: C\n"
        "Exp: Canberra was chosen as the compromise capital between Sydney and Melbourne in 1908.\n"
        "---\n"
        "Q: Who wrote the classic play 'Romeo and Juliet'?\n"
        "A: Charles Dickens\n"
        "B: William Shakespeare\n"
        "C: Mark Twain\n"
        "D: Jane Austen\n"
        "Ans: B\n"
        "Exp: William Shakespeare wrote Romeo and Juliet early in his career.\n"
        "---\n"
        "Q: What is the largest ocean on Earth?\n"
        "A: Atlantic Ocean\n"
        "B: Indian Ocean\n"
        "C: Arctic Ocean\n"
        "D: Pacific Ocean\n"
        "Ans: D\n"
        "Exp: The Pacific Ocean is the largest and deepest ocean on Earth.\n"
        "---\n"
        "Q: In which year did the Apollo 11 mission successfully land humans on the Moon?\n"
        "A: 1965\n"
        "B: 1969\n"
        "C: 1972\n"
        "D: 1975\n"
        "Ans: B\n"
        "Exp: Neil Armstrong and Buzz Aldrin landed on the Moon on July 20, 1969.\n"
        "---\n"
        "Q: Which country is known as the Land of the Rising Sun?\n"
        "A: China\n"
        "B: Japan\n"
        "C: South Korea\n"
        "D: Thailand\n"
        "Ans: B\n"
        "Exp: Japan is historically called the Land of the Rising Sun (Nihon / Nippon).\n"
        "---\n"
        "Q: How many continents are there on Earth?\n"
        "A: 5\n"
        "B: 6\n"
        "C: 7\n"
        "D: 8\n"
        "Ans: C\n"
        "Exp: The seven continents are Asia, Africa, North America, South America, Antarctica, Europe, and Australia.\n"
        "---\n";

    /* 4. Science */
    const char *science =
        "[Subject: Science]\n"
        "# Multiple-Choice Questions for Science\n"
        "\n"
        "Q: What is the chemical symbol for Gold?\n"
        "A: Ag\n"
        "B: Au\n"
        "C: Fe\n"
        "D: Pb\n"
        "Ans: B\n"
        "Exp: Au comes from the Latin word 'Aurum', meaning shining dawn.\n"
        "---\n"
        "Q: What organelle is known as the powerhouse of the cell?\n"
        "A: Nucleus\n"
        "B: Ribosome\n"
        "C: Mitochondria\n"
        "D: Endoplasmic Reticulum\n"
        "Ans: C\n"
        "Exp: Mitochondria generate most of the chemical energy (ATP) needed by the cell.\n"
        "---\n"
        "Q: What is the approximate speed of light in a vacuum?\n"
        "A: 300,000 km/s\n"
        "B: 150,000 km/s\n"
        "C: 3,000 km/s\n"
        "D: 30,000,000 km/s\n"
        "Ans: A\n"
        "Exp: The speed of light c is approximately 299,792 km/s (~300,000 km/s).\n"
        "---\n"
        "Q: What gas do green plants primarily absorb during photosynthesis?\n"
        "A: Oxygen\n"
        "B: Nitrogen\n"
        "C: Carbon Dioxide\n"
        "D: Hydrogen\n"
        "Ans: C\n"
        "Exp: Plants use carbon dioxide and water to produce glucose and release oxygen.\n"
        "---\n"
        "Q: Which planet in our solar system is nicknamed the Red Planet?\n"
        "A: Venus\n"
        "B: Jupiter\n"
        "C: Saturn\n"
        "D: Mars\n"
        "Ans: D\n"
        "Exp: Mars has a reddish appearance due to iron oxide (rust) on its surface.\n"
        "---\n";

    create_file_if_missing("data/subjects/c_programming.txt", c_prog);
    create_file_if_missing("data/subjects/computer_science.txt", cs_fund);
    create_file_if_missing("data/subjects/general_knowledge.txt", gk);
    create_file_if_missing("data/subjects/science.txt", science);
}
