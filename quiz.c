/* ============================================================
   ADAPTIVE QUIZ SYSTEM  -  single-file version (quiz.c)
   All 5 modules + main() are merged into ONE program.

   Sections:
     1. Constants and shared data
     2. Function prototypes
     3. Person 1 - Login + Registration
     4. Person 2 - Main Menu + Topic/Difficulty Selector
     5. Person 3 - Question Bank + Admin Add Question
     6. Person 4 - Quiz Engine + Score/Negative Marking
     7. Person 5 - Result History + Leaderboard
     8. main()

   Data files needed in the SAME folder as the program:
     users.txt, c_easy.txt, c_medium.txt, c_hard.txt,
     maths_*.txt, gk_*.txt   (results.txt is created automatically)
   ============================================================ */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* ---------- 1. Constants and shared data ---------- */
#define MAX_Q 30      /* maximum questions kept in memory   */
#define NUM_Q 5       /* number of questions asked per quiz */

char questions[MAX_Q][100];   /* question text             */
char options[MAX_Q][4][50];   /* 4 options per question    */
char answers[MAX_Q];          /* correct answer: A,B,C,D   */
int  qlevel[MAX_Q];           /* 1=Easy 2=Medium 3=Hard    */
int  count = 0;               /* how many questions loaded */
int  adaptive = 0;            /* 1 = adaptive mode on      */

/* ---------- 2. Function prototypes ---------- */
/* Person 1 */
int   login(char name[]);
void  registerUser();
/* Person 2 */
void  mainMenu();
int   selectTopic();
int   selectDifficulty();
const char *topicName(int t);
const char *levelName(int l);
/* Person 3 */
int   loadQuestions(int topic, int level);
void  addQuestion();
/* Person 4 */
void  runQuiz(int *correct, int *wrong, int *skipped);
float calculateScore(int correct, int wrong, int skipped);
char  getGrade(float percent);
/* Person 5 */
void  saveResult(char name[], int topic, float score);
void  showHistory(char name[]);
void  showLeaderboard();

/* ================= 3. PERSON 1: Login + Registration =================
   File used: users.txt  (format: username password) */
/* Module 1: LOGIN
   Asks username and password, searches users.txt.
   Returns 1 if found, 0 if not. Copies the username into name[]. */
int login(char name[]) {
    char user[30], pass[30];     /* what the student types */
    char fileUser[30], filePass[30];   /* what is read from the file */

    printf("Username: ");
    scanf("%s", user);
    printf("Password: ");
    scanf("%s", pass);

    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL) {
        printf("No users found. Please register first.\n");
        return 0;
    }

    /* read every line of the file: one username and one password */
    while (fscanf(fp, "%s %s", fileUser, filePass) == 2) {
        if (strcmp(user, fileUser) == 0 && strcmp(pass, filePass) == 0) {
            strcpy(name, user);      /* remember who logged in */
            fclose(fp);
            return 1;                /* success */
        }
    }

    fclose(fp);
    return 0;                        /* not found */
}

/* Module 2: REGISTRATION
   New student enters a username and password; saved at end of users.txt */
void registerUser() {
    char user[30], pass[30];
    char fileUser[30], filePass[30];

    printf("Choose a username: ");
    scanf("%s", user);

    /* first check that the username is not already taken */
    FILE *fp = fopen("users.txt", "r");
    if (fp != NULL) {
        while (fscanf(fp, "%s %s", fileUser, filePass) == 2) {
            if (strcmp(user, fileUser) == 0) {
                printf("Username already exists!\n");
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }

    printf("Choose a password: ");
    scanf("%s", pass);

    fp = fopen("users.txt", "a");    /* "a" = add at the end */
    fprintf(fp, "%s %s\n", user, pass);
    fclose(fp);

    printf("Registration successful! You can login now.\n");
}

/* ================= 4. PERSON 2: Main Menu + Selectors ================= */
/* Converts topic number to a name (used for file names) */
const char *topicName(int t) {
    if (t == 1) return "c";
    if (t == 2) return "maths";
    return "gk";
}

/* Converts level number to a name */
const char *levelName(int l) {
    if (l == 1) return "easy";
    if (l == 2) return "medium";
    return "hard";
}

/* Module 4a: TOPIC SELECTOR - keeps asking until the choice is valid */
int selectTopic() {
    int t;
    do {
        printf("\nChoose topic:\n 1. C Programming\n 2. Maths\n 3. General Knowledge\nEnter: ");
        scanf("%d", &t);
    } while (t < 1 || t > 3);
    return t;
}

/* Module 4b: DIFFICULTY SELECTOR
   4 = Adaptive mode (difficulty changes with the student's answers) */
int selectDifficulty() {
    int l;
    do {
        printf("\nChoose difficulty:\n 1. Easy\n 2. Medium\n 3. Hard\n 4. Adaptive\nEnter: ");
        scanf("%d", &l);
    } while (l < 1 || l > 4);
    return l;
}

/* Module 3: MAIN MENU - connects every module together */
void mainMenu() {
    char name[30];
    int choice, loggedIn = 0;

    /* ---- Part 1: login / register loop ---- */
    while (!loggedIn) {
        printf("\n===== ADAPTIVE QUIZ SYSTEM =====\n");
        printf("1. Login\n2. Register\n3. Exit\nEnter: ");
        scanf("%d", &choice);

        if (choice == 1) {
            if (login(name)) loggedIn = 1;
            else printf("Wrong username or password!\n");
        }
        else if (choice == 2) registerUser();
        else if (choice == 3) return;
    }

    printf("\nWelcome, %s!\n", name);

    /* ---- Part 2: student menu loop ---- */
    do {
        printf("\n----- MAIN MENU -----\n");
        printf("1. Take Quiz\n2. My Previous Results\n3. Leaderboard\n");
        printf("4. Add Question (Admin)\n5. Logout\nEnter: ");
        scanf("%d", &choice);

        if (choice == 1) {
            int topic = selectTopic();
            int level = selectDifficulty();

            if (loadQuestions(topic, level) == 0) {
                printf("No questions found for this selection!\n");
                continue;
            }

            int correct, wrong, skipped;
            runQuiz(&correct, &wrong, &skipped);

            float marks   = calculateScore(correct, wrong, skipped);
            int   total   = correct + wrong + skipped;
            float percent = marks / total * 100;
            if (percent < 0) percent = 0;      /* no negative percentage */

            printf("\n===== RESULT =====\n");
            printf("Correct: %d  Wrong: %d  Skipped: %d\n", correct, wrong, skipped);
            printf("Marks: %.2f / %d\n", marks, total);
            printf("Percentage: %.1f%%   Grade: %c\n", percent, getGrade(percent));

            saveResult(name, topic, percent);
        }
        else if (choice == 2) showHistory(name);
        else if (choice == 3) showLeaderboard();
        else if (choice == 4) addQuestion();
    } while (choice != 5);

    printf("Logged out. Goodbye!\n");
}

/* ================= 5. PERSON 3: Question Bank + Admin ================
   Question file name: topic_level.txt (example: c_easy.txt)
   Each question uses 6 lines: question, options A-D, correct letter */
/* Removes the '\n' that fgets leaves at the end of a line */
static void removeNewline(char s[]) {
    int n = strlen(s);
    if (n > 0 && s[n - 1] == '\n') s[n - 1] = '\0';
    if (n > 1 && s[n - 2] == '\r') s[n - 2] = '\0';
}

/* Reads ONE file (one topic + one level) into the arrays */
static void readFile(int topic, int level) {
    char fileName[50];
    sprintf(fileName, "%s_%s.txt", topicName(topic), levelName(level));

    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) return;                  /* file missing: nothing loaded */

    char line[20];
    while (count < MAX_Q && fgets(questions[count], 100, fp) != NULL) {
        removeNewline(questions[count]);
        if (strlen(questions[count]) == 0) continue;   /* skip blank lines */

        for (int j = 0; j < 4; j++) {        /* read 4 options */
            fgets(options[count][j], 50, fp);
            removeNewline(options[count][j]);
        }
        fgets(line, 20, fp);                 /* read correct answer */
        answers[count] = toupper(line[0]);
        qlevel[count] = level;
        count++;
    }
    fclose(fp);
}

/* Module 5: QUESTION BANK
   Loads questions into the arrays. Returns how many were loaded.
   level 4 (adaptive) loads Easy + Medium + Hard together. */
int loadQuestions(int topic, int level) {
    count = 0;
    adaptive = (level == 4);

    if (adaptive) {
        for (int l = 1; l <= 3; l++) readFile(topic, l);
    } else {
        readFile(topic, level);
    }
    return count;
}

/* Module 10: ADMIN - ADD QUESTION
   Teacher adds a new question at the end of a topic file. */
void addQuestion() {
    char pass[30];
    printf("Admin password: ");
    scanf("%s", pass);
    if (strcmp(pass, "admin123") != 0) {
        printf("Wrong password!\n");
        return;
    }

    int topic = selectTopic();
    int level;
    do {
        printf("Level (1=Easy 2=Medium 3=Hard): ");
        scanf("%d", &level);
    } while (level < 1 || level > 3);

    char q[100], opt[4][50], ans;

    printf("Enter question: ");
    scanf(" %[^\n]", q);                     /* reads a full line with spaces */
    for (int i = 0; i < 4; i++) {
        printf("Option %c: ", 'A' + i);
        scanf(" %[^\n]", opt[i]);
    }
    printf("Correct answer (A-D): ");
    scanf(" %c", &ans);

    char fileName[50];
    sprintf(fileName, "%s_%s.txt", topicName(topic), levelName(level));

    FILE *fp = fopen(fileName, "a");
    fprintf(fp, "%s\n", q);
    for (int i = 0; i < 4; i++) fprintf(fp, "%s\n", opt[i]);
    fprintf(fp, "%c\n", toupper(ans));
    fclose(fp);

    printf("Question added to %s\n", fileName);
}

/* ================= 6. PERSON 4: Quiz Engine + Scoring ================= */
/* Finds the next question that is not asked yet.
   First tries the wanted level; if none left, takes any unused question.
   Returns the question number, or -1 if all are used. */
static int pickQuestion(int wantedLevel, int used[]) {
    for (int i = 0; i < count; i++)
        if (!used[i] && qlevel[i] == wantedLevel) return i;
    for (int i = 0; i < count; i++)
        if (!used[i]) return i;
    return -1;
}

/* Module 6: QUIZ ENGINE
   Asks the questions one by one and counts correct / wrong / skipped.
   Adaptive mode: correct answer -> harder level, wrong answer -> easier level. */
void runQuiz(int *correct, int *wrong, int *skipped) {
    int used[MAX_Q] = {0};                       /* 1 = question already asked */
    int total = (count < NUM_Q) ? count : NUM_Q; /* how many to ask */
    int level = adaptive ? 2 : qlevel[0];        /* adaptive starts at Medium */
    char ans;

    *correct = 0; *wrong = 0; *skipped = 0;

    for (int n = 0; n < total; n++) {
        int i = pickQuestion(level, used);
        if (i == -1) break;
        used[i] = 1;

        /* show the question and its 4 options */
        printf("\nQ%d. %s", n + 1, questions[i]);
        if (adaptive) printf("   [Level: %s]", levelName(qlevel[i]));
        printf("\n");
        for (int j = 0; j < 4; j++)
            printf("   %c) %s\n", 'A' + j, options[i][j]);

        printf("Your answer (A-D, or S to skip): ");
        scanf(" %c", &ans);
        ans = toupper(ans);

        if (ans == 'S') {                        /* skipped */
            (*skipped)++;
        }
        else if (ans == answers[i]) {            /* correct */
            (*correct)++;
            printf("Correct!\n");
            if (adaptive && level < 3) level++;  /* go harder */
        }
        else {                                   /* wrong */
            (*wrong)++;
            printf("Wrong! Correct answer was %c\n", answers[i]);
            if (adaptive && level > 1) level--;  /* go easier */
        }
    }
}

/* Module 7a: SCORE with NEGATIVE MARKING
   +1 for correct, -0.25 for wrong, 0 for skipped */
float calculateScore(int correct, int wrong, int skipped) {
    float marks = correct * 1.0 - wrong * 0.25 + skipped * 0;
    /* skipped questions give 0 marks */
    return marks;
}

/* Module 7b: GRADE from percentage */
char getGrade(float percent) {
    if (percent >= 80) return 'A';
    if (percent >= 60) return 'B';
    if (percent >= 40) return 'C';
    return 'F';
}

/* ================= 7. PERSON 5: History + Leaderboard =================
   File used: results.txt  (format: username topic percentage) */
/* Module 8a: SAVE RESULT - adds one line at the end of results.txt */
void saveResult(char name[], int topic, float score) {
    FILE *fp = fopen("results.txt", "a");
    if (fp == NULL) {
        printf("Could not save result!\n");
        return;
    }
    fprintf(fp, "%s %d %.1f\n", name, topic, score);
    fclose(fp);
    printf("Result saved.\n");
}

/* Module 8b: SHOW HISTORY - prints only this student's lines */
void showHistory(char name[]) {
    char user[30];
    int topic, found = 0;
    float score;

    FILE *fp = fopen("results.txt", "r");
    if (fp == NULL) {
        printf("No results saved yet.\n");
        return;
    }

    printf("\n--- Previous results of %s ---\n", name);
    while (fscanf(fp, "%s %d %f", user, &topic, &score) == 3) {
        if (strcmp(user, name) == 0) {           /* only this student */
            printf("Topic: %-6s  Score: %.1f%%\n", topicName(topic), score);
            found = 1;
        }
    }
    if (!found) printf("No attempts yet.\n");
    fclose(fp);
}

/* Module 9: LEADERBOARD - reads all results, sorts high to low, shows top 5 */
void showLeaderboard() {
    char names[100][30];
    int  topics[100];
    float scores[100];
    int n = 0;

    FILE *fp = fopen("results.txt", "r");
    if (fp == NULL) {
        printf("No results saved yet.\n");
        return;
    }

    /* step 1: read everything into arrays */
    while (n < 100 && fscanf(fp, "%s %d %f", names[n], &topics[n], &scores[n]) == 3)
        n++;
    fclose(fp);

    /* step 2: bubble sort (highest score first) */
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (scores[j] < scores[j + 1]) {
                /* swap score */
                float ts = scores[j]; scores[j] = scores[j + 1]; scores[j + 1] = ts;
                /* swap topic */
                int tt = topics[j]; topics[j] = topics[j + 1]; topics[j + 1] = tt;
                /* swap name */
                char tn[30];
                strcpy(tn, names[j]);
                strcpy(names[j], names[j + 1]);
                strcpy(names[j + 1], tn);
            }
        }
    }

    /* step 3: print top 5 */
    printf("\n===== LEADERBOARD (Top 5) =====\n");
    for (int i = 0; i < n && i < 5; i++)
        printf("%d. %-10s %-6s %.1f%%\n", i + 1, names[i], topicName(topics[i]), scores[i]);
}

/* ================= 8. MAIN: program starts here ================= */
int main() {
    mainMenu();
    return 0;
}
