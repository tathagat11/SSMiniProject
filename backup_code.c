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

u3 p3 faculty -1 1
u2 p2 student 1 1
tathagat10 changeme student 2 0
u5 changeme student 3 1
u6 changeme student 4 1