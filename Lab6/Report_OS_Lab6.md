# Lab 6: Inside the xv6 File System

**MINISTRY OF EDUCATION, CULTURE AND RESEARCH**  
**OF THE REPUBLIC OF MOLDOVA**  

**Technical University of Moldova**  
**Faculty of Computers, Informatics and Microelectronics**  
**Department of Software Engineering and Automation**  


**MAXIM ORMANJI FAF-242**

Checked by: Reițman Petricia

Chișinău, 2026

## Purpose of the work

The purpose of this lab is to look at the ideas of a file system inside a very small teaching system, xv6. I read the xv6 file system code, write my own `stat` program (`mystat`), measure the limits written in `kernel/fs.h`, and write `find`, which walks the directory tree recursively.

## Environment

The lab was done on Ubuntu (a VirtualBox virtual machine, user `vboxuser`), in the folder `~/xv6-riscv`, with the RISC-V toolchain and QEMU, started with `make qemu`. To leave QEMU I used Ctrl-A and then X.

---

## Part 1. Read what xv6 shows you

First I looked at the type constants, then I started xv6 and ran `ls`.

```
vboxuser@Ubuntu:~/xv6-riscv$ grep -n "T_DIR\|T_FILE\|T_DEVICE" kernel/stat.h
1:#define T_DIR    1 // Directory
2:#define T_FILE   2 // File
3:#define T_DEVICE 3 // Device
vboxuser@Ubuntu:~/xv6-riscv$ make qemu
...
xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2441
args           2 3 35272
cat            2 4 36728
echo           2 5 35592
...
sync           2 26 34944
console        3 27 0
$ mkdir a
$ ls a
.              1 28 32
..             1 1 1024
```

<p align="center"><em>Figure 6.1 – ls in xv6 (the middle of the list is shortened with "...")</em></p>


In every line the columns are: name, type, inode number, size in bytes.

### Observe

**1. What do types 1, 2 and 3 mean?**
From `kernel/stat.h`: 1 is `T_DIR` (a directory), 2 is `T_FILE` (a regular file), 3 is `T_DEVICE` (a device). In the listing `.`, `..` have type 1, `README` and the programs have type 2, and `console` has type 3.

**2. Which inode number does the root directory have, and why are `.` and `..` the same there?**
The root directory has the inode number 1. In the root both `.` and `..` show `1 1 1024`, because the root has no parent directory, so its `..` points to itself. In the directory `a` it is different: `.` is the inode 28 (the directory itself) and `..` is the inode 1 (the root).

**3. An empty directory `a` has size 32. How big is one entry, and which two entries does `a` contain?**
A directory is a file with a table of fixed-size entries `struct dirent`: a 2-byte inode number (`ushort inum`) and a 14-byte name (`char name[DIRSIZ]`), so one entry is 2 + 14 = 16 bytes. The size 32 is 2 entries × 16 bytes. The two entries are `.` (inode 28, the directory itself) and `..` (inode 1, the parent).


## Part 2. Write mystat

The program `user/mystat.c` calls `stat()` for every argument and prints the name, the type, the inode, the number of links and the size. If `stat` fails, it writes an error to the descriptor 2 and goes to the next argument.

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char *
typename(int t)
{
  switch(t){
  case T_DIR: return "directory";
  case T_FILE: return "file";
  case T_DEVICE: return "device";
  }
  return "?";
}

int
main(int argc, char *argv[])
{
  struct stat st;
  for(int i = 1; i < argc; i++){
    if(stat(argv[i], &st) < 0){
      fprintf(2, "mystat: cannot stat %s\n", argv[i]);
      continue;
    }
    printf("%s: %s, inode %d, links %d, size %d bytes\n",
           argv[i], typename(st.type), st.ino, st.nlink, (int)st.size);
  }
  exit(0);
}
```

<p align="center"><em>Figure 6.2 – user/mystat.c</em></p>


I added the program to `UPROGS` in the `Makefile` (a TAB at the beginning, nothing after the backslash):

```
vboxuser@Ubuntu:~/xv6-riscv$ grep -n "mystat\|_find" Makefile
151:	$U/_mystat\
152:	$U/_find\
```

<p align="center"><em>Figure 6.3 – The new lines in UPROGS</em></p>


Hard links at work:

```
$ mystat README . console
README: file, inode 2, links 1, size 2441 bytes
.: directory, inode 1, links 2, size 1024 bytes
console: device, inode 27, links 1, size 0 bytes
$ ln README r2
$ mystat README r2
README: file, inode 2, links 2, size 2441 bytes
r2: file, inode 2, links 2, size 2441 bytes
$ rm README
$ mystat r2
r2: file, inode 2, links 1, size 2441 bytes
$ cat r2 | wc
48 336 2441
```

<p align="center"><em>Figure 6.4 – mystat and hard links in xv6</em></p>


Two small differences from the example in the task. The `console` has the inode 27 and not 25, because I have two more programs (`mystat` and `find`) in the image. The directory `.` has 2 links and not 1, because I had created the directory `a` in Part 1 and the disk image was not rebuilt (its `..` is one more link to the root).

Then I looked at the kernel code on the host:

```
vboxuser@Ubuntu:~/xv6-riscv$ grep -n "nlink--" kernel/sysfile.c
178:  ip->nlink--;
239:    dp->nlink--;
244:  ip->nlink--;
vboxuser@Ubuntu:~/xv6-riscv$ sed -n '/struct dinode/,/};/p' kernel/fs.h
struct dinode {
  short type;              // File type
  short major;             // Major device number (T_DEVICE only)
  short minor;             // Minor device number (T_DEVICE only)
  short nlink;             // Number of links to inode in file system
  uint size;               // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};
```

<p align="center"><em>Figure 6.5 – The decrements of nlink and struct dinode</em></p>


### Observe

**1. Paste the outputs after `ln` and after `rm`. What changed and what did not?**
The outputs are in Figure 6.4. After `ln README r2` both names show the same inode 2, the same size 2441 and `links 2`: it is one file with two names. After `rm README` only `r2` is left, and its `links` went from 2 to 1. What did not change: the inode number (2), the size (2441) and the content: `cat r2 | wc` printed `48 336 2441`, so all the 2441 bytes are still there.

**2. Find the line in `sys_unlink` that decrements the link count. When are the data blocks freed?**
In `kernel/sysfile.c` the line is 244, `ip->nlink--;`, in the function `sys_unlink` (the line 239, `dp->nlink--;`, is in the same function and decreases the link count of the parent when a subdirectory is removed; the line 178 is in `sys_link` and only undoes the increment if `link` fails). The call to `unlink` removes the directory entry and decreases `nlink`, but it does not free the data. The blocks are freed when the link count is 0 and nobody keeps the file open: then `iput()` in `kernel/fs.c` calls `itrunc()`, which frees the data blocks, and marks the inode as free. This is the same rule that ext4 uses: a file is freed only when its last name is removed and no process keeps it open.

**3. xv6 has no symbolic links and no permissions. Which fields does an xv6 inode have?**
`struct dinode` (the inode on the disk) has: `type` (file, directory or device), `major` and `minor` (the device numbers, only for devices), `nlink` (the number of names), `size` (in bytes) and `addrs[NDIRECT+1]` (the addresses of the data blocks: 12 direct and 1 indirect). There is no owner, no permission bits and no times, and there is no type for a symbolic link. This is much smaller than an inode of ext4.


## Part 3. Measure the limits

```
vboxuser@Ubuntu:~/xv6-riscv$ grep -n "define BSIZE\|define NDIRECT\|define NINDIRECT\|define DIRSIZ" kernel/fs.h
5:#define BSIZE   1024 // block size
26:#define NDIRECT   12
27:#define NINDIRECT (BSIZE / sizeof(uint))
54:#define DIRSIZ 14
vboxuser@Ubuntu:~/xv6-riscv$ make clean ; make fs.img 2>&1 | grep nmeta
nmeta 47 (boot, super, log blocks 31, inode blocks 13, bitmap blocks 1) blocks 1953 total 2000
```

<p align="center"><em>Figure 6.6 – The constants from kernel/fs.h and the disk layout</em></p>


```
vboxuser@Ubuntu:~/xv6-riscv$ make qemu
...
$ mkdir averyveryverylongname
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2441
...
console        3 27 0
averyveryveryl 1 28 32
```

<p align="center"><em>Figure 6.7 – A directory with a too long name</em></p>


### Observe

**1. What does `ls` show for `averyveryverylongname`, and which constant explains it?**
`ls` shows `averyveryveryl`, the first 14 characters of the name. The constant is `DIRSIZ 14`: the name field in a directory entry (`struct dirent`) has only 14 characters, so a longer name is cut when it is written into the directory.

**2. The largest file xv6 can hold.**
`NINDIRECT = BSIZE / sizeof(uint) = 1024 / 4 = 256` block numbers fit in one indirect block. An inode has `NDIRECT = 12` direct blocks and one indirect block, so the maximum is 12 + 256 = 268 blocks.

```
268 blocks × 1024 bytes = 274432 bytes = 268 KiB
```

**3. From the `nmeta` line: how many blocks does the whole disk have, and how many are for the log, the inodes, and the data?**
The disk has 2000 blocks in total. Of them:
- 1 boot block and 1 superblock;
- 31 blocks for the log;
- 13 blocks for the inodes;
- 1 block for the bitmap of free blocks;
- the first four groups together are `nmeta = 47` (1 + 1 + 31 + 13 + 1) blocks;
- 1953 blocks are for the data (2000 − 47).


## Part 4. Write find

The program `user/find.c` is made from `user/ls.c`. For the directory `path` it opens it, reads the `struct dirent` entries one by one, builds the full path in a buffer, prints the path if the last component is equal to `name`, and calls itself for every subdirectory. It skips the free slots (`inum == 0`) and the entries `.` and `..`, and it closes every directory it opened (a process can have only 16 open files).

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
find(char *path, char *name)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if(fstat(fd, &st) < 0 || st.type != T_DIR){
    close(fd);
    return;
  }
  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
    fprintf(2, "find: path too long\n");
    close(fd);
    return;
  }
  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0)
      continue;
    if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
      continue;
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if(strcmp(p, name) == 0)
      printf("%s\n", buf);
    if(stat(buf, &st) < 0)
      continue;
    if(st.type == T_DIR)
      find(buf, name);
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "usage: find <dir> <name>\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}
```

<p align="center"><em>Figure 6.8 – user/find.c</em></p>


The test on a fresh disk (`make clean ; make qemu`):

```
$ mkdir a
$ mkdir a/b
$ echo x > a/b/r2
$ echo y > a/r2
$ ln cat r2
$ find . r2
./a/b/r2
./a/r2
./r2
$ find a b
a/b
```

<p align="center"><em>Figure 6.9 – Testing find</em></p>


`find . r2` found all three files named `r2`, in the order of the directory entries, and `find a b` found the directory `a/b`. Both results are the same as in the task.

One remark: the shell of xv6 reads at most 100 characters in one line. When I first typed the whole test as one long line, it was cut in the middle, so `find` started without arguments and printed `usage: find <dir> <name>`. The program was correct; I started xv6 again (the disk is kept between the runs) and ran `find a b` as a separate short line.

---

## Conclusion

In Lab 6 I found the same ideas in the small file system of xv6. The inode has only the type, the device numbers, `nlink`, the size and the block addresses; a directory is a table of 16-byte entries, which is why an empty directory is 32 bytes and a name can have only 14 characters. I wrote `mystat` and with it I saw that after `ln` two names have the same inode and `links 2`, and after `rm` the file stays with `links 1` and all its data. From `kernel/fs.h` I calculated that the largest file is (12 + 256) × 1024 = 274432 bytes, and from the line `nmeta` I saw how the 2000 blocks of the disk are divided (47 blocks of metadata and 1953 blocks of data). At the end I wrote `find`, which walks the directory tree recursively with `open`, `read` of `struct dirent`, `stat` and `close`, and it found all three files `r2`. Inodes, directories as tables of names, hard links and a log are the same ideas that real file systems like ext4 use, and xv6 shows them in a few hundred readable lines.
