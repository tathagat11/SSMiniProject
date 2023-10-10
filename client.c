#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER];
    // Create socket 
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection error");
        exit(1);
    }
    int role_n;
    char username[MAX_BUFFER];
    char password[MAX_BUFFER];
    char role[MAX_BUFFER];

    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);
    printf("Enter number corresponding to your role:\n1. Student\n2. Faculty\n3. Administrator\n");
    scanf("%d", &role_n);
    switch(role_n){
        case 1: strcpy(role, "student");
                break;
        case 2: strcpy(role, "faculty");
                break;
        case 3: strcpy(role, "admin");
                break;
    }

    // Send username and password to the server
    send(client_socket, username, sizeof(username), 0);
    send(client_socket, password, sizeof(password), 0);
    send(client_socket, role, sizeof(role), 0);

    // Receive authentication result from the server
    recv(client_socket, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);

    close(client_socket);
    return 0;
}