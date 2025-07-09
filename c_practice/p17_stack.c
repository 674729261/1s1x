#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define MAX_SIZE 1024

struct Stack
{
    int stack[MAX_SIZE];
    int count;
};

struct Stack stack;


void die(const char *message) {
  if (errno) {
    perror(message);
  } else {
    printf("ERROR: %s\n", message);
  }

  exit(1);
}

void load_stack(const char* path)
{
    FILE *file = fopen(path, "rb+");
    if(!file)
        die("Failed to access file.");
    int rc = fread(&stack, sizeof(stack), 1, file);
    if(rc != 1)
        die("Failed to load from file.");
    fclose(file);
}

void save_stack(const char* path)
{
    FILE *file = fopen(path, "wb");
    if(!file)
        die("Failed to access file.");
    int rc = fwrite(&stack, sizeof(stack), 1, file);
    if(rc != 1)
        die("Failed to write to file.");
    fclose(file);
}

void show()
{
    for(int i = 0; i < stack.count; i++)
    {
        printf("%d ", stack.stack[i]);
    }
    printf("\n%d element(s)\n", stack.count);
}

int main(int argc, char *argv[])
{
    if(argc < 3 || argc > 4)
        die("Useage : <file name> <c/l/p/q> <param>\nc : create new stack\np <integer>: push an integer\nq : pop an integer\nl : show the whole stack");       
    
    switch(argv[2][0])
    {
        case 'c':
            break;
        case 'l':
            load_stack(argv[1]);
            show();
            break;
        case 'p':
            load_stack(argv[1]);
            int val = atoi(argv[3]);
            if(stack.count == MAX_SIZE)
                die("Too many elements.");
            stack.stack[stack.count++] = val;
            break;
        case 'q':
            load_stack(argv[1]);
            if(stack.count == 0)
                die("No element to pop.");
            stack.count--;
            break;
        default:
            die("Unknown command.");
    }
    
    save_stack(argv[1]);
        
}
