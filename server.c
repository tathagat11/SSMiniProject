#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#define PORT 8080
#define MAX_BUFFER 1024
#define MAX_STUDENTS 1024

// User structure to store user data
typedef struct {
    char username[MAX_BUFFER];
    char password[MAX_BUFFER];
    char role[MAX_BUFFER];
} User;

typedef struct {
    char courseID[MAX_BUFFER];
    char courseName[MAX_BUFFER];
    char facultyName[MAX_BUFFER];
    int numStudents;
    char students[MAX_STUDENTS][MAX_BUFFER];
} Course;

typedef struct {
    int new_socket;
    User *users;
    int* num_users;
} ThreadData;

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

void loadCourseData(Course courses[], int* numCourses) {
    FILE* file = fopen("courses.txt", "r");
    if (file == NULL) {
        perror("Unable to open courses.txt");
        exit(1);
    }

    char buffer[MAX_BUFFER];
    while (*numCourses < MAX_BUFFER && fgets(buffer, sizeof(buffer), file) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline character

        int parsedFields = sscanf(buffer, "%[^:]:%[^:]:%[^:]:%d:%[^\n]", courses[*numCourses].courseID,
            courses[*numCourses].courseName, courses[*numCourses].facultyName,
            &courses[*numCourses].numStudents, courses[*numCourses].students);

        if (parsedFields == 5) {
            (*numCourses)++;
        }
    }

    fclose(file);
}

// each separate client request is handled by this.
void* handleClient(void* data){
    ThreadData* thread_data = (ThreadData*)data;
    int new_socket = thread_data->new_socket;
    User *users = thread_data->users;
    int num_users = *thread_data->num_users;
    
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
                case 3:{
                    char delUsername[512];
                    recv(new_socket, delUsername, sizeof(delUsername), 0);
                    bool removed = false;
                    int removeIndex;

                    for(int i = 0; i < num_users; i++){
                        if(strcmp(users[i].username, delUsername) == 0){
                            removed = true;
                            removeIndex = i;
                            break;
                        }
                    }
                    if(removed){
                        for(int i = removeIndex; i < num_users - 1; i++){
                            users[i] = users[i + 1];
                        }
                        num_users--;

                        int fd2 = open("users.txt", O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
                        if(fd2 < 0){
                            perror("Error opening users.txt line 131.");
                            exit(1);
                        } else {
                            for(int i = 0; i < num_users; i++){
                                char userData[MAX_BUFFER];
                                sprintf(userData, "%s %s %s\n", users[i].username, users[i].password, users[i].role);

                                ssize_t bytes_written = write(fd2, userData, strlen(userData));
                                if(bytes_written < 0){
                                    close(fd2);
                                    perror("Error writing after deletion.");
                                    exit(1);
                                }
                            }
                            close(fd2);
                            send(new_socket, "User removed successfully!", sizeof("User removed successfully!"), 0);
                        }
                    } else {
                        send(new_socket, "User not found!", sizeof("User not found!"), 0);
                    }
                } break;
                default: break;
            }
        } 
        // else if(strcmp(role, "student") == 0) {
        //     //If logged in user is a student
        //     Course courses[MAX_BUFFER];
        //     int num_courses = 0;
        //     loadCourseData(courses, &num_courses);
        //     switch (choice_num)
        //     {
        //     case 1: { //enroll in a new course
        //         printf("%d", num_courses);
        //     }
        //         break;
            
        //     default:  exit(0);
        //         break;
        //     }
        // }
    }

    close(new_socket);
    return NULL;
}

int main (){
    int server_socket, new_socket;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[MAX_BUFFER];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0){
        perror("Socket creation error");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("binding error");
        exit(1);
    }

    if(listen(server_socket, 10) == 0){
        printf("Listening...\n");
    } else {
        perror("Listening error!");
        exit(1);
    }

    addr_size = sizeof(new_addr);

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
    
    while(1){
        new_socket = accept(server_socket, (struct sockaddr*)&new_addr, &addr_size);
        if(new_socket < 0){
            perror("Acceptance error!");
            exit(1);
        }
        pthread_t client_thread;
        ThreadData thread_data;
        thread_data.new_socket = new_socket;
        thread_data.users = users;
        thread_data.num_users = &num_users;


        if(pthread_create(&client_thread, NULL, handleClient, &thread_data) != 0){
            perror("Failed to create a new thread");
            close(new_socket);
        }
    }
    close(server_socket);
    return 0;
}