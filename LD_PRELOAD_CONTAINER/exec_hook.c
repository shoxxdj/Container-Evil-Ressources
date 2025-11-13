// exec_hook.c
#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

// Variable pour éviter la récursion
static __thread int in_hook = 0;

void send_exec_info(const char *filename, char *const argv[]) {
    // Protection contre la récursion
    if (in_hook) return;
    in_hook = 1;
    
    // Ne pas logger certaines commandes système critiques
    if (strstr(filename, "ldconfig") || 
        strstr(filename, "ld-linux") ||
        strstr(filename, "ld.so")) {
        in_hook = 0;
        return;
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock >= 0) {
        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(80);
        
        inet_pton(AF_INET, "SERVER", &server.sin_addr);
        
        // Timeout court
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == 0) {
            char args[2048] = "";
            int i = 0;
            while (argv && argv[i] != NULL && strlen(args) < 1800) {
                if (i > 0) strcat(args, " ");
                strcat(args, argv[i]);
                i++;
            }
            
            char body[3096];
            char hostname[256];
            gethostname(hostname, sizeof(hostname));
            
            snprintf(body, sizeof(body), 
                "{\"filename\":\"%s\",\"args\":\"%s\",\"hostname\":\"%s\",\"pid\":%d}",
                filename, args, hostname, getpid());
            
            char http_request[4096];
            snprintf(http_request, sizeof(http_request),
                "POST /api/exec HTTP/1.1\r\n"
                "Host: monitor\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %ld\r\n"
                "Connection: close\r\n"
                "\r\n"
                "%s",
                strlen(body), body);
            
            send(sock, http_request, strlen(http_request), MSG_NOSIGNAL);
        }
        close(sock);
    }
    
    in_hook = 0;
}

int execve(const char *filename, char *const argv[], char *const envp[]) {
    send_exec_info(filename, argv);
    
    // Obtenir le vrai execve
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
    send_exec_info(filename, argv);
    
    static int (*real_execv)(const char*, char *const[]) = NULL;
    if (!real_execv) {
        real_execv = dlsym(RTLD_NEXT, "execv");
    }
    return real_execv ? real_execv(filename, argv) : -1;
}

int execvp(const char *filename, char *const argv[]) {
    send_exec_info(filename, argv);
    
    static int (*real_execvp)(const char*, char *const[]) = NULL;
    if (!real_execvp) {
        real_execvp = dlsym(RTLD_NEXT, "execvp");
    }
    return real_execvp ? real_execvp(filename, argv) : -1;
}

int execvpe(const char *filename, char *const argv[], char *const envp[]) {
    send_exec_info(filename, argv);
    
    static int (*real_execvpe)(const char*, char *const[], char *const[]) = NULL;
    if (!real_execvpe) {
        real_execvpe = dlsym(RTLD_NEXT, "execvpe");
    }
    return real_execvpe ? real_execvpe(filename, argv, envp) : -1;
}
