#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){
  char ch;
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror("Error while opening the file.\n");
    exit(EXIT_FAILURE);
  }
  while ((ch = fgetc(fp)) != EOF)
    printf("%c", ch);
}
