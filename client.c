#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER 1024
#define MAX_STUDENTS 1024



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
    if(strcmp(buffer, "Authenticated") == 0){
        int choice_num;
        printf("Welcome %s!\n what would you like to do: \n", username);
        //if admin
        if(strcmp(role, "admin") == 0){
            printf("1. Add new student.\n2. Add new faculty.\n3. Delete student/faculty.\n4. Update Student/Faculty details.\n5. Activate/deactivate student/faculty.\n5. Exit.\n");
            scanf("%d",&choice_num);
            char choice_arr[10];
            sprintf(choice_arr,"%d", choice_num);
            send(client_socket, choice_arr, sizeof(choice_arr), 0);
            switch (choice_num){
                case 1: {
                    char newUsername[512];
                    printf("Enter student username: ");
                    scanf("%s", newUsername);
                    send(client_socket, newUsername, sizeof(newUsername), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    printf("%s", buffer);
                } break;
                case 2:{
                    char newUsername[512];
                    printf("Enter faculty username: ");
                    scanf("%s", newUsername);
                    send(client_socket, newUsername, sizeof(newUsername), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    printf("%s", buffer);
                } break;
                case 3:{
                    char delUsername[512];
                    printf("Enter username whose data you wish to delete: ");
                    scanf("%s", delUsername);
                    send(client_socket, delUsername, sizeof(delUsername), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    printf("%s\n", buffer);
                } break;
                case 4: break;
                default: exit(0);
            }
        }
        //if student

        else if(strcmp(role, "student") == 0){
            printf("1. Enroll to new course.\n2. Unenroll from a course.\n3. View enrolled courses.\n4. Change password.\n5. Exit.\n");
            scanf("%d",&choice_num);
            switch (choice_num){
                case 1: break;
                case 2: break;
                case 3: break;
                case 4: break;
                default: exit(0);
            }
        }

        else if(strcmp(role, "faculty") == 0){
            printf("1. Add new course.\n2. Remove a course.\n3. View enrollments in courses.\n4. Change password.\n5. Exit.\n");
            scanf("%d",&choice_num);
            
            switch (choice_num){
                case 1: break;
                case 2: break;
                case 3: break;
                case 4: break;
                default: exit(0);
            }
        }
    }

    close(client_socket);
    return 0;
}