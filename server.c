#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PORT 8080
#define MAX_BUFFER 1024

// User structure to store user data
typedef struct {
    char username[MAX_BUFFER];
    char password[MAX_BUFFER];
    char role[MAX_BUFFER];
} User;

bool authenticate(User* users, int num_users, char* username, char* password, char* role) {
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0 &&
            strcmp(users[i].role, role) == 0) {
            return true; // Authentication successful
        }
    }
    return false; // Authentication failed
}

int main() {
    int server_socket, new_socket;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[MAX_BUFFER];

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding error");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) == 0) {
        printf("Listening...\n");
    } else {
        perror("Listening error");
        exit(1);
    }

    addr_size = sizeof(new_addr);
    new_socket = accept(server_socket, (struct sockaddr*)&new_addr, &addr_size); // Accept connection
    if (new_socket < 0) {
        perror("Acceptance error");
        exit(1);
    }

    // Read user data from a file
    FILE* user_file = fopen("users.txt", "r");
    if (user_file == NULL) {
        perror("User data file not found");
        exit(1);
    }

    int num_users = 0;
    User users[MAX_BUFFER];
    char line[MAX_BUFFER];

    while (fgets(line, sizeof(line), user_file) != NULL) {
        sscanf(line, "%s %s %s", users[num_users].username, users[num_users].password, users[num_users].role);
        num_users++;
    }

    fclose(user_file);

    // Handle authentication
    char username[MAX_BUFFER];
    char password[MAX_BUFFER];
    char role[MAX_BUFFER];
    bool loggedin = false;

    recv(new_socket, username, sizeof(username), 0); // Receive username
    recv(new_socket, password, sizeof(password), 0); // Receive password
    recv(new_socket, role, sizeof(role), 0); // Receive role

    // Authenticate the user
    if (authenticate(users, num_users, username, password, role)) {
        send(new_socket, "Authenticated", sizeof("Authenticated"), 0);
        loggedin = true;
    } else {
        send(new_socket, "Authentication failed", sizeof("Authentication failed"), 0);
    }

    if(loggedin == true){
        char choice[10];
        recv(new_socket, &choice, sizeof(choice), 0);
        int choice_num = atoi(choice);
        //admin menu handler
        if(strcmp(role, "admin") == 0){
            switch(choice_num){
                case 1: { //add new student
                    char newUsername[512];
                    recv(new_socket, newUsername, sizeof(newUsername), 0);
                    int fd1 = open("users.txt", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
                    if(fd1 < 0){
                        perror("FD not opened at line 120!");
                        exit(1);
                    } else {
                        char newStudent[MAX_BUFFER];
                        sprintf(newStudent, "\n%s %s %s", newUsername, "changeme", "student");
                        ssize_t ns_written = write(fd1, newStudent, strlen(newStudent));
                        if(ns_written < 0){
                            perror("Failed to add new student.");
                            exit(1);
                        } else {
                            close(fd1);
                            send(new_socket, "Student added successfully!\n", sizeof("Student added successfully!\n"), 0);
                        }
                    }
                } break;
                case 2: {
                    char newUsername[512];
                    recv(new_socket, newUsername, sizeof(newUsername), 0);
                    int fd1 = open("users.txt", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
                    if(fd1 < 0){
                        perror("FD not opened at line 138!");
                        exit(1);
                    } else {
                        char newFaculty[MAX_BUFFER];
                        sprintf(newFaculty, "\n%s %s %s", newUsername, "changeme", "faculty");
                        ssize_t nf_written = write(fd1, newFaculty, strlen(newFaculty));
                        if(nf_written < 0){
                            perror("Failed to add new faculty.");
                            exit(1);
                        } else {
                            close(fd1);
                            send(new_socket, "Faculty added successfully!\n", sizeof("Faculty added successfully!\n"), 0);
                        }
                    }
                } break;
                default: break;
            }
        }
    }
    
    close(new_socket);
    close(server_socket);
    return 0;
}

