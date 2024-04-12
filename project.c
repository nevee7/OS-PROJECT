#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>

#define MAX_PATH_LENGTH 1000

void listFilesRecursively(const char *basePath, int snapshotFile) {
    char path[MAX_PATH_LENGTH];
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
            char buffer[1024];
            int len = snprintf(buffer, sizeof(buffer), "Directory: %s\n", path);
            if (write(snapshotFile, buffer, len) != len) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            listFilesRecursively(path, snapshotFile);
        } else {
            char buffer[1024];
            int len = snprintf(buffer, sizeof(buffer), "File: %s\n", path);
            if (write(snapshotFile, buffer, len) != len) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
    }

    closedir(dir);
}

void createSnapshot(const char *basePath, const char *outputDir) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char filename[MAX_PATH_LENGTH];
    sprintf(filename, "%s/snapshot_%d-%02d-%02d_%02d-%02d-%02d.txt", outputDir, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    int snapshotFile = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (snapshotFile == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    listFilesRecursively(basePath, snapshotFile);

    close(snapshotFile);
    printf("Snapshot created: %s\n", filename);
}

void updateSnapshot(const char *basePath, const char *outputDir) {
    char newSnapshot[MAX_PATH_LENGTH];
    createSnapshot(basePath, outputDir);
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(newSnapshot, sizeof(newSnapshot), "%s/snapshot_%d-%02d-%02d_%02d-%02d-%02d.txt", outputDir, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    char oldSnapshot[MAX_PATH_LENGTH];
    snprintf(oldSnapshot, sizeof(oldSnapshot), "%s/new_snapshot.txt", outputDir);

    if (rename(newSnapshot, oldSnapshot) != 0) {
        perror("rename");
        exit(EXIT_FAILURE);
    }
    printf("Snapshot updated.\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        write(STDERR_FILENO, "Usage: ./program_exe -o <output_directory> <directory_path1> <directory_path2> ...\n", strlen("Usage: ./program_exe -o <output_directory> <directory_path1> <directory_path2> ...\n"));
        exit(EXIT_FAILURE);
    }

    int opt;
    char *outputDir = NULL;

    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o':
                outputDir = optarg;
                break;
            default:
                write(STDERR_FILENO, "Usage: ./program_exe -o <output_directory> <directory_path1> <directory_path2> ...\n", strlen("Usage: ./program_exe -o <output_directory> <directory_path1> <directory_path2> ...\n"));
                exit(EXIT_FAILURE);
        }
    }

    if (outputDir == NULL) {
        write(STDERR_FILENO, "Output directory not specified.\n", strlen("Output directory not specified.\n"));
        exit(EXIT_FAILURE);
    }

    for (int i = optind; i < argc; i++) {
        char *path = argv[i];
        updateSnapshot(path, outputDir);
    }

    return EXIT_SUCCESS;
}
