#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char *
typename(int t)
{
  // TODO: return "directory", "file" or "device" for T_DIR, T_FILE, T_DEVICE
  if(t == T_DIR) return "directory";
  if(t == T_FILE) return "file";
  if(t == T_DEVICE) return "device";
  return "?";
}

int
main(int argc, char *argv[])
{
  struct stat st;
  for(int i = 1; i < argc; i++){
    // TODO: call stat(argv[i], &st); on error print to fd 2 and continue
    // TODO: print  NAME: TYPE, inode N, links N, size N bytes
    if(stat(argv[i], &st) < 0){fprintf(2, "mystat: cannot stat %s\n", argv[i]); continue; }
    printf("%s: %s, inode %d, links %d, size %d bytes\n", argv[i], typename(st.type), st.ino, st.nlink, (int)st.size);
  }
  exit(0);
}
