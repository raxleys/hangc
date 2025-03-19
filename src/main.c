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

typedef struct Image {
    size_t size;
    char *buf;
    struct Image *next;
} Image;

Image *image_from(const char *buf, size_t n);

typedef struct Images {
    size_t size;
    Image *head;
    Image *tail;
    Image *active; // current image displayed
} Images;

Images *parse_images(const char *buf);

int main(int argc, char *argv[])
{
    char *raw_images = read_file(IMAGES_F);
    printf("Reading: %s\n", IMAGES_F);

    Images *images = parse_images(raw_images);

    for (Image *p = images->head; p; p = p->next)
        printf(">>>>>>>>>>\n%s\n<<<<<<<<<<\n", p->buf);

    for (Image *p = images->head; p;) {
        Image *tmp = p->next;
        free(p->buf);
        free(p);
        p = tmp;
    }

    free(images);
    free(raw_images);
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

    while (true) {
        for (; *p && *p != ','; p++)
            end++;

        if (*p) {
            Image *image = image_from(buf + start, end - start);
            assert(image != NULL && "Malloc failed");

            if (!images->head)
                images->head = image;
            
            if (images->tail)
                images->tail->next = image;
        
            images->tail = image;
            images->size++;

            // Consume , and \n
            end += 2;
            start = end;
            p += 2;
            continue;
        }

        break;
    }

    return images;
}

Image *image_from(const char *buf, size_t n)
{
    Image *img = malloc(sizeof(*img));
    assert(img != NULL && "Malloc failed");
    
    img->buf = strndup(buf, n);
    assert(img->buf != NULL && "Malloc failed");
    
    img->size = n;
    img->next = NULL;
    return img;
}
