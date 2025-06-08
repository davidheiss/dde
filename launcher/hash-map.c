#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xxhash.h>

struct Entry {
    size_t value;
    size_t len;
    char string[];
};

struct EntryArray {
    size_t len;
    struct Entry entries[];
};

struct EntryHash {
    XXH64_hash_t hash;
    struct Entry *entry;
};

struct EntryMap {
    int capacity;
    struct EntryHash entries[];
};

struct EntryArray *entry_array_open(const char *file)
{
    int fd = open(file, O_CREAT | O_RDWR | O_SYNC, 644);
    if (fd == -1) {
        perror("open");
        return NULL;
    }

    struct stat *sb = malloc(sizeof(struct stat));
    if (!sb) {
        perror("malloc");
        if (close(fd) == -1)
            perror("close");
        return NULL;
    }
    if (fstat(fd, sb) == -1) {
        perror("fstat");
        free(sb);
        if (close(fd) == -1)
            perror("close");
        return NULL;
    }

    if (!sb->st_size) {
        sb->st_size = sizeof(struct EntryArray);
        if (ftruncate(fd, sb->st_size) == -1) {
            free(sb);
            if (close(fd) == -1)
                perror("close");
            return NULL;
        }
    }

    struct EntryArray *array = mmap(
        NULL, sb->st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0
    );
    if (array == MAP_FAILED) {
        free(sb);
        if (close(fd) == -1)
            perror("close");
        return NULL;
    }

    free(sb);
    return NULL;
}

int main() {

    return 0;
}