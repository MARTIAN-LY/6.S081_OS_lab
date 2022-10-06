#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

// find file_name
// find path file_name

void find(char *path, char *filename)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  // Check if the path exists.
  if ((fd = open(path, 0)) < 0)
  {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  // Get information of the path.
  if (fstat(fd, &st) < 0)
  {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type)
  {
  case T_FILE:
    printf("find : wrong path\n");
    break;

  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
    {
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    /*
      ----path---/-----------
      ^           ^
     buf          p
    */

    // Directory is a file containing a sequence of dirent structures.
    // Read everything in the directory.
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
      // inum == 0 --> directory doesn't exist.
      if (de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      /*
        ----path---/---name---0
        ^           ^
       buf          p
      */
      if (stat(buf, &st) < 0)
      {
        printf("find: cannot stat %s\n", buf);
        continue;
      }

      // Find a file with the same name.
      if (st.type == T_FILE && (0 == strcmp(de.name, filename)))
      {
        printf("%s\n", buf);
        continue;
      }
      // Don't recurse into "." and "..".
      if (st.type == T_DIR && (0 != strcmp(de.name, ".")) && (0 != strcmp(de.name, "..")))
      {
        find(buf, filename);
      }
    }
    break;
  }
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc == 2)
  {
    find(".", argv[1]);
    exit(0);
  }
  if (argc == 3)
  {
    find(argv[1], argv[2]);
    exit(0);
  }
  printf("find: wrong number of arguments\n");
  printf("Usage: find [path] [filename]  or  find [filename]\n");
  exit(1);
}
