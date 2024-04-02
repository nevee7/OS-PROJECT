#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

void listFilesRecursively(const char *basePath) {
    char path[1000];
    struct dirent *entry;
    struct stat statbuf;

    DIR *dir = opendir(basePath);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name);

        if (stat(path, &statbuf) == -1) {
            perror("stat");
            exit(EXIT_FAILURE);
        }

        if (S_ISDIR(statbuf.st_mode)) {
            printf("Directory: %s\n", path);
            listFilesRecursively(path);
        } else {
            printf("File: %s\n", path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(STDERR_FILENO, "Usage: <directory_path>\n", strlen("Usage: <directory_path>\n"));
        exit(EXIT_FAILURE);
    }

    listFilesRecursively(argv[1]);

    return EXIT_SUCCESS;
}