////////////////////////////////////////////////////////////////////////////
// hangc - Terminal based hangman game, written in C                      //
// Copyright (C) 2025 Alexander Reyes <raxleys@gmail.com>                 //
//                                                                        //
// This program is free software: you can redistribute it and/or modify   //
// it under the terms of the GNU General Public License as published by   //
// the Free Software Foundation, either version 3 of the License, or      //
// (at your option) any later version.                                    //
//                                                                        //
// This program is distributed in the hope that it will be useful,        //
// but WITHOUT ANY WARRANTY; without even the implied warranty of         //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
// GNU General Public License for more details.                           //
//                                                                        //
// You should have received a copy of the GNU General Public License      //
// along with this program.  If not, see <https://www.gnu.org/licenses/>. //
////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include "hangc.h"

int main()
{
    seed_rand();

    // ASCII images
    char *raw_images = read_file(IMAGES_F);
    Images *images = parse_images(raw_images);

    // Wordlist
    char *raw_wordlist = read_file(WORD_BANK_F);
    string **words;
    size_t nwords = parse_wordlist(&words, raw_wordlist);

    // Word Queue
    // Select the word order in advance. This way, we are guaranteed
    // not to have duplicate words until the entire list has been
    // exhausted
    size_t *word_queue = malloc(nwords * sizeof(*word_queue));
    assert(word_queue != NULL && "Malloc failed");
    for (size_t i = 0; i < nwords; i++)
        word_queue[i] = i;

    string *word = NULL;
    string *gameword = NULL;
    size_t next_word = nwords;
    uint32_t guessed_letters = 0;

    // Game Loop
    do {
        // Reshuffle word list after all words exhausted
        if (next_word >= nwords) {
            shuffle(word_queue, nwords);
            next_word = 0;
        }

        word = words[word_queue[next_word++]];
        printf("Next word: %s\n", word->buf);

        // Current word status
        if (gameword != NULL)
            string_free(&gameword);
        gameword = string_copy(word);
        memset(gameword->buf, '_', gameword->size);

        // User-guessed letters
        guessed_letters = 0;

        // Reset image
        images->active = images->head;

        // Display game to user
        term_clear();
        render_state(images, gameword, guessed_letters);

        // Main loop
        while (true) {
            printf("\nEnter a guess: ");
            int c = read_char();
            if (c < 0)
                goto cleanup;

            if (!isalnum(c)) {
                term_clear();
                render_state(images, gameword, guessed_letters);
                puts("\nInvalid character!");
                continue;
            }

            char ch = (char)toupper(c);

            // Check if already guessed
            if (was_guessed(ch - 'A', guessed_letters)) {
                printf("'%c' was already guessed!\n\n", ch);
                continue;
            }

            set_guessed(ch - 'A', &guessed_letters);

            // Check if guess was correct
            bool was_miss = true;
            for (size_t i = 0; i < word->size; i++) {
                if (word->buf[i] == ch) {
                    gameword->buf[i] = ch;
                    was_miss = false;
                }
            }

            if (was_miss)
                images->active = images->active->next;

            term_clear();
            render_state(images, gameword, guessed_letters);
            if (was_miss) {
                puts("\nGuess was incorrect!");
                if (images->active == images->tail) {
                    puts("You lost!");
                    break;
                }
            } else {
                puts("\nGuess was correct!");
                if(!was_miss && string_eq(word, gameword)) {
                    puts("You won!");
                    break;
                }
            }
        }

        printf("The word was: %s\n", word->buf);

        printf("\nPlay again? (Y/n): ");
    } while (tolower(read_char()) != 'n');

 cleanup:
    // Free memory
    free(word_queue);
    string_free(&gameword);
    free_words(&words, nwords);
    free_images(&images);
    free(raw_images);
    free(raw_wordlist);
    return 0;
}

// UTILS
char *read_file(const char *fname)
{
    FILE *file = fopen(fname, "rb");
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

void seed_rand()
{
    srand(time(NULL));
}

// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
void shuffle(size_t *arr, size_t n)
{
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int read_char()
{
    int c = getchar();
    if (c == EOF)
        return -1;
    else if (c == '\n')
        return c;

    // Consume newline
    while (getchar() != '\n');

    // Handle DOS
    if (c == '\r')
        return '\n';

    return c;
}


// OTHER
Images *parse_images(const char *buf)
{
    Images *images = calloc(1, sizeof(*images));
    assert(images != NULL && "Malloc failed");

    const char *p = buf;
    size_t start = 0;
    size_t end = 0;
    while (true) {
        for (; *p && *p != ','; p++)
            end++;

        if (*p) {
            string *image = string_dupn(buf + start, end - start);
            assert(image != NULL && "Malloc failed");

            strlist *node = strlist_new();
            assert(node != NULL && "Malloc failed");

            node->str = image;

            if (!images->head)
                images->head = node;

            if (images->tail)
                images->tail->next = node;

            images->tail = node;
            images->size++;

            // Consume , and newline character(s)
            for (; *p != '\0' && (*p == ',' || *p == '\n' || *p == '\r'); p++, end++);
            start = end;
            continue;
        }

        break;
    }

    images->active = images->head;
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

string *string_dup(const char *buf)
{
    size_t len = strlen(buf);
    return string_dupn(buf, len);
}

string *string_copy(const string *word)
{
    return string_dup(word->buf);
}

string *string_to_upper(string *word)
{
    for (char *c = word->buf; *c != '\0'; c++)
        *c = toupper(*c);
    return word;
}

bool string_eq(string *s1, string *s2)
{
    return strcmp(s1->buf, s2->buf) == 0;
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
    strlist_free(&(*images)->head);
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
        for (; *p != '\0' && *p != '\n' && *p != '\r'; p++)
            end++;

        if (*p == '\0')
            break;

        string *w = string_dupn(raw_wordlist + start, end - start);
        assert(w != NULL && "Malloc failed");
        words[words_len++] = string_to_upper(w);

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

// let refers to a letter in the alphabet from 0-26, not the ASCII
// value.
bool was_guessed(char let, uint32_t guessed_letters)
{
    return (guessed_letters & (1u << let)) != 0;
}

// let refers to a letter in the alphabet from 0-26, not the ASCII
// value.
void set_guessed(char let, uint32_t *guessed_letters)
{
    *guessed_letters |= 1u << let;
}

void display_alphabet(uint32_t guessed_letters)
{
    for (char i = 0; i < NLETTERS; i++) {
        if (was_guessed(i, guessed_letters)) {
            printf("%s", ASCII_GRAY);
        }

        printf("%c%s", 'A' + i, ASCII_RESET);

        if ((i + 1) % 9 == 0) {
            putchar('\n');
        } else {
            putchar(' ');
        }
    }
    putchar('\n');
}

void display_gameword(const string *gameword)
{
    for (size_t i = 0; i < gameword->size; i++) {
        putchar(gameword->buf[i]);
        if (i < gameword->size - 1)
            putchar(' ');
    }
    putchar('\n');
}

void render_state(const Images *images, const string *gameword,
                  uint32_t guessed_letters)
{
    puts(images->active->str->buf);

    printf("\nWord: ");
    display_gameword(gameword);
    putchar('\n');

    // Display guessed letters
    display_alphabet(guessed_letters);
}

void term_clear()
{
    if (strcmp(getenv("TERM"), "dumb") != 0)
        printf("%s", TERM_CLEAR);
}
