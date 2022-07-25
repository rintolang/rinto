#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char varKeywords[][] = {"short int",
                        "unsigned short int",
                        "unsigned int",
                        "int",
                        "long int",
                        "unsigned long int",
                        "long long int",
                        "unsigned long long int",
                        "char",
                        "float",
                        "double",
                        "long double",
                        "string"};
char methodKeyword[][] = {"for",
                          "while",
                          "if",
                          "else"};
int main(int argc, char **argv){
  char ch;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

// exit(EXIT_SUCCESS);
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror("Error while opening the file.\n");
    exit(EXIT_FAILURE);
  }
  while ((read = getline(&line, &len, fp)) != -1) {
      printf("Retrieved line of length %zu:\n", read);
      printf("%s", line);
  }

  fclose(fp);
  if (line)
      free(line);
}
