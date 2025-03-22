#ifndef HANGC_H
#define HANGC_H

// Constants
// Path from project root
#define DATA_PATH "./data/"
#define WORD_BANK_F DATA_PATH "words.txt"
#define IMAGES_F DATA_PATH "images.txt"
/* #define WORD_BANK_F DATA_PATH "words-dos.txt" */
/* #define IMAGES_F DATA_PATH "images-dos.txt" */

// ANSI escape codes for text colors
#define ASCII_RESET "\033[0m"
#define ASCII_GRAY "\033[90m"
#define TERM_CLEAR "\033[H\033[J"

#define NLETTERS 26

// Types
typedef struct string {
    size_t size;
    char *buf;
} string;

typedef struct strlist {
    string *str;
    struct strlist *next;
} strlist;

typedef struct Images {
    size_t size;
    strlist *head;
    strlist *tail;
    strlist *active; // current image displayed
} Images;

// String functions
string *string_dup(const char *buf);
string *string_dupn(const char *buf, size_t n);
string *string_copy(const string *word);
string *string_to_upper(string *word);
void string_free(string **str);
bool string_eq(string *s1, string *s2);

// String linked-list functions
strlist *strlist_new();
void strlist_free(strlist **list);

// Images functions
Images *parse_images(const char *buf);
void free_images(Images **images);

// Game word list functions
// Returns number of words (not buffersize allocated)
size_t parse_wordlist(string ***wordlist, char *raw_wordlist);
void free_words(string ***wordlist, size_t nwords);

// Game state
bool was_guessed(char let, uint32_t guessed_letters);
void set_guessed(char let, uint32_t *guessed_letters);

// Display functions
void display_alphabet(uint32_t guessed_letters);
void display_gameword(const string *gameword);
void render_state(const Images *images, const string *gameword,
                  uint32_t guessed_letters);
void term_clear();

// Utils
#define arg_shift(xs, xs_sz) (assert((xs_sz) > 0), (xs_sz)--, *(xs)++)
char *read_file(const char *fname);
void seed_rand();
void shuffle(size_t *arr, size_t n);

#endif // HANGC_H
