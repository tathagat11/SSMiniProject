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
#define MAX_COURSES 1024

// User structure to store user data
typedef struct {
    char username[MAX_BUFFER];
    char password[MAX_BUFFER];
    char role[MAX_BUFFER];
    char rollno[10];
    bool activated;
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
} ThreadData;

bool authenticate(User* users, int num_users, char* username, char* password, char* role) {
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0 &&
            strcmp(users[i].role, role) == 0) {
                if(strcmp(role, "student") == 0 && users[i].activated){
                    return true; //authentication successful
                }
                else if(strcmp(role,"faculty") == 0 || strcmp(role, "admin") == 0){
                    return true; //authentication successful
                }
        }
    }
    return false; // Authentication failed
}

//function to load courses data.
// int loadCourseData(Course courses[], int max_courses) {
//     int fd = open("courses.txt", O_RDONLY);
//     if (fd == -1) {
//         perror("Course data file not found");
//         return -1;
//     }

//     char buffer[MAX_BUFFER];
//     int numCourses = 0;

//     while (numCourses < max_courses) {
//         ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
//         if (bytes_read <= 0) {
//             break; // Reached end of file
//         }

//         buffer[bytes_read] = '\0'; // Null-terminate the buffer
//         char *token = strtok(buffer, "\n");

//         if (token != NULL) {
//             // Parse the line
//             sscanf(token, "%[^:]:%[^:]:%[^:]:%[^\n]", courses[numCourses].courseID, courses[numCourses].courseName, courses[numCourses].facultyName, buffer);
//             courses[numCourses].numStudents = 0;

//             char *student = strtok(buffer, ",");
//             int studentIndex = 0;
//             while (student != NULL && studentIndex < MAX_STUDENTS) {
//                 strncpy(courses[numCourses].students[studentIndex], student, sizeof(courses[numCourses].students[studentIndex]));
//                 studentIndex++;
//                 courses[numCourses].numStudents++;
//                 student = strtok(NULL, ",");
//             }

//             numCourses++;
//         }
//     }

//     close(fd);
//     return numCourses;
// }

// Lock the file for reading
int lockFileForReading(int fd) {
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_RDLCK; 
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    return fcntl(fd, F_SETLK, &lock);
}

// Lock the file for writing
int lockFileForWriting(int fd) {
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK; 
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    return fcntl(fd, F_SETLK, &lock);
}

// Unlock the file
int unlockFile(int fd) {
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_UNLCK;

    return fcntl(fd, F_SETLK, &lock);
}

//function to read users data.
int readUserFile(User users[], int max_users) {
    int fd = open("users.txt", O_RDONLY);
    if (fd == -1) {
        perror("Error opening user data file");
        exit(1);
    }

    if (lockFileForReading(fd) == -1) {
        perror("Error acquiring read lock on file");
        close(fd);
        return -1;
    }

    int num_users = 0;
    char buffer[MAX_BUFFER];
    char c;
    int buffer_index = 0;

    while (num_users < max_users && read(fd, &c, 1) > 0) {
        if (c != '\n') {
            buffer[buffer_index++] = c;
        } else {

            buffer[buffer_index] = '\0';

            char activated[1];
            sscanf(buffer, "%s %s %s %s %s",
                   users[num_users].username, users[num_users].password,
                   users[num_users].role, users[num_users].rollno, activated);
            if (strcmp(activated, "1") == 0) users[num_users].activated = 1;
            else users[num_users].activated = 0;

            buffer_index = 0;
            num_users++;
        }
    }

    unlockFile(fd);
    close(fd);
    return num_users;
}

// each separate client request is handled by this.
void* handleClient(void* data){

    ThreadData* thread_data = (ThreadData*)data;
    int new_socket = thread_data->new_socket;

    User users[MAX_BUFFER];
    int num_users = readUserFile(users, MAX_BUFFER);
    if (num_users <= 0) {
        exit(1);
    }
    // Course courses[MAX_BUFFER];
    // int num_courses = loadCourseData(courses, MAX_BUFFER);

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
                    char newRollno[10];
                    recv(new_socket, newUsername, sizeof(newUsername), 0);
                    recv(new_socket, newRollno, sizeof(newRollno), 0);
                    
                    //Update Users array.
                    strcpy(users[num_users].username, newUsername);
                    strcpy(users[num_users].password, "changeme");
                    strcpy(users[num_users].role, "student");
                    strcpy(users[num_users].rollno,newRollno);
                    users[num_users].activated = 1;
                    num_users++;

                    int fd1 = open("users.txt", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
                    if(fd1 < 0){
                        perror("FD not opened at line 120!");
                        exit(1);
                    } else {
                        if (lockFileForWriting(fd1) == -1) {
                            perror("Error acquiring write lock on file");
                            close(fd1);
                            exit(1);
                        }
                        char newStudent[MAX_BUFFER];
                        sprintf(newStudent, "%s %s %s %s %s\n", newUsername, "changeme", "student", newRollno, "1");
                        ssize_t ns_written = write(fd1, newStudent, strlen(newStudent));
                        
                        unlockFile(fd1);

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
                    
                    strcpy(users[num_users].username, newUsername);
                    strcpy(users[num_users].password, "changeme");
                    strcpy(users[num_users].role, "faculty");
                    strcpy(users[num_users].rollno, "-1");
                    users[num_users].activated = 1;
                    num_users++;

                    int fd1 = open("users.txt", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
                    if(fd1 < 0){
                        perror("FD not opened at line 138!");
                        exit(1);
                    } else {
                        
                        if (lockFileForWriting(fd1) == -1) {
                            perror("Error acquiring write lock on file");
                            close(fd1);
                            exit(1);
                        }

                        char newFaculty[MAX_BUFFER];
                        sprintf(newFaculty, "%s %s %s %s %s\n", newUsername, "changeme", "faculty", "-1", "1");
                        ssize_t nf_written = write(fd1, newFaculty, strlen(newFaculty));
                        
                        unlockFile(fd1);

                        if(nf_written < 0){
                            perror("Failed to add new faculty.");
                            exit(1);
                        } else {
                            close(fd1);
                            send(new_socket, "Faculty added successfully!\n", sizeof("Faculty added successfully!\n"), 0);
                        }
                    }
                } break;
                case 3: {
                    char delUsername[512];
                    recv(new_socket, delUsername, sizeof(delUsername), 0);
                    bool removed = false;
                    int removeIndex;

                    for (int i = 0; i < num_users; i++) {
                        if (strcmp(users[i].username, delUsername) == 0) {
                            removed = true;
                            removeIndex = i;
                            break;
                        }
                    }

                    if (removed) {
                        int fd = open("users.txt", O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
                        if (fd < 0) {
                            perror("Error opening users.txt for writing");
                            exit(1);
                        }

                        if (lockFileForWriting(fd) == -1) {
                            perror("Error acquiring write lock on file");
                            close(fd);
                            exit(1);
                        }

                        for (int i = 0; i < num_users; i++) {
                            if (i != removeIndex) {
                                char activated = users[i].activated ? '1' : '0';
                                dprintf(fd, "%s %s %s %s %c\n", users[i].username, users[i].password, users[i].role, users[i].rollno, activated);
                            }
                        }

                        unlockFile(fd);

                        close(fd);

                        send(new_socket, "User removed successfully!", sizeof("User removed successfully!"), 0);
                    } else {
                        send(new_socket, "User not found!", sizeof("User not found!"), 0);
                    }
                } break;

                case 4: { // Modify username
                    char oldUsername[512];
                    char newUsername[512];
                    recv(new_socket, oldUsername, sizeof(oldUsername), 0);
                    recv(new_socket, newUsername, sizeof(newUsername), 0);

                    int fd = open("users.txt", O_RDWR);
                    if (fd < 0) {
                        perror("Error opening users.txt for reading and writing");
                        exit(1);
                    }

                    if (lockFileForWriting(fd) == -1) {
                        perror("Error acquiring write lock on file");
                        close(fd);
                        exit(1);
                    }

                    int found = 0;
                    for (int i = 0; i < num_users; i++) {
                        if (strcmp(users[i].username, oldUsername) == 0) {
                            found = 1;
                            strcpy(users[i].username, newUsername);
                            break;
                        }
                    }

                    if (found) {
                        if (ftruncate(fd, 0) == -1) {
                            perror("Error clearing the users.txt file");
                            unlockFile(fd);
                            close(fd);
                            exit(1);
                        }
                        lseek(fd, 0, SEEK_SET);
                        
                        for (int j = 0; j < num_users; j++) {
                            char activated = users[j].activated ? '1' : '0';
                            dprintf(fd, "%s %s %s %s %c\n", users[j].username, users[j].password, users[j].role, users[j].rollno, activated);
                        }

                        unlockFile(fd);

                        send(new_socket, "Username modified successfully!", sizeof("Username modified successfully!"), 0);
                        close(fd);
                    } else {
                        unlockFile(fd);
                        close(fd);
                        send(new_socket, "User not found!", sizeof("User not found!"), 0);
                    }
                } break;

                case 5: {
                    char username[512];
                    recv(new_socket, username, sizeof(username), 0);

                    int found = 0;
                    int fd = open("users.txt", O_RDWR);
                    if (fd < 0) {
                        perror("Error opening users.txt for writing");
                        exit(1);
                    }

                    if (lockFileForWriting(fd) == -1) {
                        perror("Error locking the file");
                        close(fd);
                        exit(1);
                    }

                    for (int i = 0; i < num_users; i++) {
                        if (strcmp(users[i].username, username) == 0) {
                            found = 1;
                            users[i].activated = !users[i].activated;
                            lseek(fd, 0, SEEK_SET); // Move the file pointer to the beginning

                            for (int j = 0; j < num_users; j++) {
                                char activated = users[j].activated ? '1' : '0';
                                dprintf(fd, "%s %s %s %s %c\n", users[j].username, users[j].password, users[j].role, users[j].rollno, activated);
                            }

                            char response[MAX_BUFFER];
                            if (users[i].activated) {
                                sprintf(response, "User '%s' activated successfully!", username);
                            } else {
                                sprintf(response, "User '%s' deactivated successfully!", username);
                            }
                            send(new_socket, response, sizeof(response), 0);
                            break;
                        }
                    }

                    if (!found) {
                        send(new_socket, "User not found!", sizeof("User not found!"), 0);
                    }

                    unlockFile(fd); // Unlock the file
                    close(fd);
                } break;

                default: break;
            }
        } 
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
    
    while(1){
        new_socket = accept(server_socket, (struct sockaddr*)&new_addr, &addr_size);
        if(new_socket < 0){
            perror("Acceptance error!");
            exit(1);
        }
        pthread_t client_thread;
        ThreadData thread_data;
        thread_data.new_socket = new_socket;


        if(pthread_create(&client_thread, NULL, handleClient, &thread_data) != 0){
            perror("Failed to create a new thread");
            close(new_socket);
        }
    }
    close(server_socket);
    return 0;
}