#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>

// Path from project root
#define DATA_PATH "./data/"
#define WORD_BANK_F DATA_PATH "words.txt"
#define IMAGES_F DATA_PATH "images.txt"

const char *WORDS[] = {"ant", "bear", "wolf"};

char *read_file(const char *fname);

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
    strlist *list;
    strlist *active; // current image displayed
} Images;

Images *parse_images(const char *buf);
void free_images(Images **images);

// Returns number of words (not buffersize allocated)
size_t parse_wordlist(string ***wordlist, char *raw_wordlist);
void free_words(string ***wordlist, size_t nwords);

string *string_dupn(const char *buf, size_t n);
void string_free(string **str);
strlist *strlist_new();
void strlist_free(strlist **list);

int main(int argc, char *argv[])
{
    // Hush gcc!
    (void)argc;
    (void)argv;

    // ASCII images
    char *raw_images = read_file(IMAGES_F);
    Images *images = parse_images(raw_images);

    // Print images
    /* for (strlist *im = images->list; im; im = im->next) */
        /* puts(im->str->buf); */

    puts("Current image:");
    puts(images->active->str->buf);

    // Wordlist
    char *raw_wordlist = read_file(WORD_BANK_F);
    string **words;
    size_t nwords = parse_wordlist(&words, raw_wordlist);

    // Print words
    /* for (size_t i = 0; i < nwords; i++) */
        /* printf("Word #%zu: %s\n", i + 1, words[i]->buf); */

    // Free memory
    free_words(&words, nwords);
    free_images(&images);
    free(raw_images); // TODO: free earlier?
    free(raw_wordlist);
    return 0;
}

char *read_file(const char *fname)
{
    FILE *file = fopen(fname, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buf = malloc(fsize + 1);
    if (buf == NULL) {
        perror("OOM");
        fclose(file);
        return NULL;
    }

    if (fread(buf, 1, fsize, file) != (size_t)fsize) {
        perror("Failed to read file");
        free(buf);
        fclose(file);
        return NULL;
    }

    fclose(file);
    buf[fsize] = '\0';
    return buf;
}

Images *parse_images(const char *buf)
{
    Images *images = calloc(1, sizeof(*images));
    assert(images != NULL && "Malloc failed");
    
    const char *p = buf;
    size_t start = 0;
    size_t end = 0;

    strlist *tail = NULL;
    while (true) {
        for (; *p && *p != ','; p++)
            end++;

        if (*p) {
            string *image = string_dupn(buf + start, end - start);
            assert(image != NULL && "Malloc failed");

            strlist *node = strlist_new();
            assert(node != NULL && "Malloc failed");

            node->str = image;

            if (!images->list)
                images->list = node;

            if (tail)
                tail->next = node;
        
            tail = node;
            images->size++;

            // Consume , and \n
            end += 2;
            start = end;
            p += 2;
            continue;
        }

        break;
    }

    images->active = images->list;
    return images;
}

string *string_dupn(const char *buf, size_t n)
{
    string *str = malloc(sizeof(*str));
    assert(str != NULL && "Malloc failed");
    
    str->buf = strndup(buf, n);
    assert(str->buf != NULL && "Malloc failed");
    
    str->size = n;
    return str;
}

void string_free(string **str)
{
    free((*str)->buf);
    free(*str);
    *str = NULL;
}

strlist *strlist_new()
{
    strlist *list = calloc(1, sizeof(*list));
    assert(list != NULL && "Malloc failed");
    return list;
}

void strlist_free(strlist **list)
{
    for (strlist *p = *list; p;) {
        strlist *tmp = p->next;
        string_free(&p->str);
        free(p);
        p = tmp;
    }
    
    *list = NULL;
}

void free_images(Images **images)
{
    strlist_free(&(*images)->list);
    free(*images);
    *images = NULL;
}

size_t parse_wordlist(string ***wordlist, char *raw_wordlist)
{
    // Count # of newlines.
    // A buffer that can fit that can be guaranteed to fit enough words
    size_t nwords = 0;
    for (char *p = raw_wordlist; *p != '\0'; p++)
        if (*p == '\n') nwords++;
    
    size_t words_len = 0;
    string **words = malloc(nwords * sizeof(*words));
    assert(words != NULL && "Malloc failed");

    char *p = raw_wordlist;
    size_t start = 0;
    size_t end = 0;

    while (true) {
        for (; *p != '\0' && *p != '\n'; p++)
            end++;

        if (*p == '\0')
            break;

        string *w = string_dupn(raw_wordlist + start, end - start);
        printf("DUPED: %s\n", w->buf);
        assert(w != NULL && "Malloc failed");

        words[words_len++] = w;

        // Skip empty strings
        for (; *p != '\0' && isspace(*p); p++)
            end++;
        
        if (*p == '\0')
            break;

        start = end;
    }

    *wordlist = words;
    return words_len;
}

void free_words(string ***wordlist, size_t nwords)
{
    string **words = *wordlist;
    for (size_t i = 0; i < nwords; i++)
        string_free(&words[i]);

    free(words);
    *wordlist = NULL;
}
