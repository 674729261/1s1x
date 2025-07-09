#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int max_row = 0;
int max_data = 0;
int db_size = 0;
int address_size = 0;

struct Address {
  int id;
  int valid;
  char *name;
  char *email;
};

unsigned char *buffer;

void die(const char *message) {
  if (errno) {
    perror(message);
  } else {
    printf("ERROR: %s\n", message);
  }
  free(buffer);
  exit(1);
}
void build_db() {
  buffer = malloc(db_size);
  memset(buffer, 0, db_size);
  if (!buffer)
    die("Failed to allocate memory for buffer");
  int *integer_writer = (int *)buffer;
  *integer_writer = max_row;
  integer_writer++;
  *integer_writer = max_data;
  integer_writer++;

  address_size = sizeof(int) * 2 + sizeof(char) * max_data * 2;
  for (int i = 0; i < max_row; ++i) {
    *integer_writer = i;
    *(integer_writer + 1) = 0;
    integer_writer = (int *)(((unsigned char *)integer_writer) + address_size);
  }
}
void write_to_file(const char *file_name) {
  FILE *db_file = fopen(file_name, "w");
  if (!db_file)
    die("Failed to access database file");
  int rc = fwrite(buffer, sizeof(unsigned char) * db_size, 1, db_file);
  if (rc != 1)
    die("Failed to write database.");
  rc = fflush(db_file);
  if (rc == -1)
    die("Cannot flush database.");
  fclose(db_file);
}

void read_from_file(const char *file_name) {
  FILE *db_file = fopen(file_name, "rb+");
  if (!db_file)
    die("Failed to access database file");

  int sz[2];
  int rc = fread(sz, sizeof(int) * 2, 1, db_file);
  if (rc != 1)
    die("Failed to load size info.");
  max_row = sz[0];
  max_data = sz[1];
  address_size = sizeof(int) * 2 + sizeof(char) * max_data * 2;
  db_size = sizeof(int) * 2 + max_row * address_size;

  build_db();
  rewind(db_file);
  rc = fread(buffer, db_size, 1, db_file);
  if (rc != 1)
    die("Failed to load database.");
  fclose(db_file);
}

struct Address get_address(int id) {
  if (id < 0 || id >= max_row)
    die("ID must be less than max_row and positive");
  struct Address ret;
  const unsigned char *read_pointer = buffer;
  read_pointer += sizeof(int) * 2 + id * address_size;
  ret.id = *(int *)read_pointer;
  read_pointer += sizeof(int);
  ret.valid = *(int *)read_pointer;
  read_pointer += sizeof(int);
  ret.name = (char *)read_pointer;
  read_pointer += sizeof(char) * max_data;
  ret.email = (char *)read_pointer;
  return ret;
}

void set_address(int id, const char *name, const char *email) {
  char *writer_pointer = (char *)buffer;
  writer_pointer += sizeof(int) * 2 + address_size * id + sizeof(int);
  *(int *)writer_pointer = 1;
  writer_pointer += sizeof(int);
  char *res = strncpy(writer_pointer, name, max_data - 1);
  if (!res)
    die("Name copy failed");
  writer_pointer += max_data;
  res = strncpy(writer_pointer, email, max_data - 1);
  if (!res)
    die("Email copy failed");
}
void delete_address(int id) {
  char *writer_pointer = (char *)buffer;
  writer_pointer += sizeof(int) * 2 + address_size * id + sizeof(int);
  *(int *)writer_pointer = 0;
}
void print_address(struct Address addr) {
  printf("ID    : %8d\tName  : %s\tEmail : %s\n", addr.id, addr.name,
         addr.email);
}

void list_db() {
  for (int i = 0; i < max_row; ++i) {
    struct Address addr = get_address(i);
    if (addr.valid) {
      print_address(addr);
    }
  }
}

void find_name(const char *name) {
  for (int i = 0; i < max_row; ++i) {
    struct Address addr = get_address(i);
    if (addr.valid && strcmp(addr.name, name) == 0) {
      print_address(addr);
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3)
    die("USAGE: p17 <dbfile> <action> [action params]");

  char *filename = argv[1];
  char action = argv[2][0];
  int id = -1;
  struct Address addr;
  switch (action) {
  case 'c':
    if (argc != 5)
      die("Need max_rows and max_data");
    max_row = atoi(argv[3]);
    max_data = atoi(argv[4]);
    db_size = sizeof(int) * 2 +
              max_row * (sizeof(int) * 2 + sizeof(char) * max_data * 2);
    build_db();
    write_to_file(filename);
    break;

  case 'g':
    if (argc != 4)
      die("Need an id to get");

    read_from_file(filename);
    id = atoi(argv[3]);

    addr = get_address(id);
    if (!addr.valid)
      die("This address is not set");
    print_address(addr);
    break;

  case 's':
    if (argc != 6)
      die("Need id, name, email to set");
    id = atoi(argv[3]);
    read_from_file(filename);
    addr = get_address(id);
    if (addr.valid)
      die("Already set, delete it first");
    set_address(id, argv[4], argv[5]);
    write_to_file(filename);
    break;

  case 'd':
    if (argc != 4)
      die("Need id to delete");

    read_from_file(filename);
    id = atoi(argv[3]);
    delete_address(id);
    write_to_file(filename);
    break;

  case 'l':
    read_from_file(filename);
    list_db();
    break;

  case 'f':
    if (argc != 4)
      die("Need name to find");
    read_from_file(filename);
    find_name(argv[3]);
    break;
  default:
    die("Invalid action, only: c=create, g=get, s=set, d=del, l=list, f=find");
  }
  free(buffer);
  return 0;
}
