#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

void listFilesRecursively(const char *basePath, FILE *snapshotFile) {
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
            fprintf(snapshotFile, "Directory: %s\n", path);
            printf("Directory: %s\n", path);
            listFilesRecursively(path, snapshotFile);
        } else {
            fprintf(snapshotFile, "File: %s\n", path);
            printf("File: %s\n", path);
        }
    }

    closedir(dir);
}

void createSnapshot(const char *basePath) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char filename[100];
    sprintf(filename, "snapshot_%d-%02d-%02d_%02d-%02d-%02d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    FILE *snapshotFile = fopen(filename, "w");
    if (!snapshotFile) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    listFilesRecursively(basePath, snapshotFile);

    fclose(snapshotFile);
    printf("Snapshot created: %s\n", filename);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(STDERR_FILENO, "Usage: <directory_path>\n", strlen("Usage: <directory_path>\n"));
        exit(EXIT_FAILURE);
    }

    createSnapshot(argv[1]);

    return EXIT_SUCCESS;
}
