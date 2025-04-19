#include <stdio.h>
#include <string.h>

int main() {
    char str[] = "apple|banana|cherry|date";
    char *token = strtok(str, "|");

    while (token != NULL) {
        printf("Token: %s\n", token);
        token = strtok(NULL, "|");
    }

    return 0;
}
