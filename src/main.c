#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

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

string *string_dupn(const char *buf, size_t n);
void string_free(string **str);
strlist *strlist_new();
void strlist_free(strlist **list);

int main(int argc, char *argv[])
{
    char *raw_images = read_file(IMAGES_F);
    Images *images = parse_images(raw_images);

    for (strlist *im = images->list; im; im = im->next)
        puts(im->str->buf);

    puts("Current image:");
    puts(images->active->str->buf);

    free_images(&images);
    free(raw_images); // TODO: free earlier?
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
            /* Image *image = image_from(buf + start, end - start); */
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
