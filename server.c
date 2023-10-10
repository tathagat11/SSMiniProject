#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>

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

    recv(new_socket, username, sizeof(username), 0); // Receive username
    recv(new_socket, password, sizeof(password), 0); // Receive password
    recv(new_socket, role, sizeof(role), 0); // Receive role

    // Authenticate the user
    if (authenticate(users, num_users, username, password, role)) {
        send(new_socket, "Authenticated", sizeof("Authenticated"), 0);
    } else {
        send(new_socket, "Authentication failed", sizeof("Authentication failed"), 0);
    }

    close(new_socket);
    close(server_socket);
    return 0;
}

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdbool.h>
// #include <unistd.h>
// #include <arpa/inet.h>

// #define PORT 8080
// #define MAX_BUFFER 1024

// typedef struct {
//     char username[MAX_BUFFER];
//     char password[MAX_BUFFER];
//     char role[MAX_BUFFER];
// } User;

// User users[] = {
//     {"admin1", "pass1", "admin"},
//     {"student1", "pass2", "student"},
//     {"faculty1", "pass3", "faculty"}
// };

// bool authenticate(char *username, char *password, char *role){
//     for(int i = 0; i < sizeof(users)/sizeof(users[0]); i++){
//         if(strcmp(users[i].username, username) == 0 && 
//         strcmp(users[i].password, password) == 0 &&
//         strcmp(users[i].role, role) == 0) return true; // Authenticated.
//     }
//     return false; //Failed authentication.
// }

// // void login(){
// //     printf("Are you a: \n1. Student.\n2. Faculty\n 3. Administrator.");
// // }

// int main(){
//     int server_socket, new_socket;
//     struct sockaddr_in server_addr, new_addr;
//     socklen_t addr_size;
//     char buffer[MAX_BUFFER];

//     //creating socket

//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if(server_socket < 0){
//         perror("Socket error (server)!");
//         exit(1);
//     }
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(PORT);
//     server_addr.sin_addr.s_addr = INADDR_ANY;

//     //Binding the socket
//     if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
//         perror("Server Binding Error!");
//         exit(1);
//     }

//     //Start listening for connections

//     if(listen(server_socket, 10) == 0){
//         printf("Listening...\n");
//     } else {
//         perror("Listening Error! (Server)");
//         exit(1);
//     }

//     addr_size = sizeof(new_addr);
//     new_socket = accept(server_socket, (struct sockaddr*)&new_addr, &addr_size); //Accept connection request.
//     if(new_socket < 0) {
//         perror("Connection to client failed.\n");
//         exit(1);
//     }
//     FILE* user_file = fopen("users.txt", "r");
//     if(user_file == NULL){
//         perror("User data not found");
//         exit(1);
//     }
//     int num_users = 0;
//     User users[MAX_BUFFER];
//     char line[MAX_BUFFER];
//     while(fgets(line, sizeof(line), user_file) != NULL){
        
//     }
// }