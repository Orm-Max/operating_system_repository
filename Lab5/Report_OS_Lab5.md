# Lab 5: The File System with Your Own Hands (Linux)

**MINISTRY OF EDUCATION, CULTURE AND RESEARCH**  
**OF THE REPUBLIC OF MOLDOVA**  

**Technical University of Moldova**  
**Faculty of Computers, Informatics and Microelectronics**  
**Department of Software Engineering and Automation**  


**MAXIM ORMANJI FAF-242**

Checked by: Reițman Petricia

Chișinău, 2026

## Purpose of the work

The purpose of this lab is to check on a real Linux system, with my own hands, the ideas of a file system: the directory tree and the file types, inodes, hard and symbolic links, permissions, and the disks under the file system. No programming is needed: only commands, careful observation and short explanations.

## Environment

The lab was done on Ubuntu running in a VirtualBox virtual machine, as the normal user `vboxuser` (not root). All files of the lab are in `~/os-lab5`.

---

## Before you start

The experiments with permissions must be done without root, because root ignores most permission checks.

```
vboxuser@Ubuntu:~$ whoami
vboxuser
vboxuser@Ubuntu:~$ mkdir -p ~/os-lab5 && cd ~/os-lab5
vboxuser@Ubuntu:~/os-lab5$
```

The command `whoami` did not print `root`, so I can continue.


## Part 1. The tree and the file types

```
vboxuser@Ubuntu:~/os-lab5$ ls /
bin  boot  cdrom  dev  etc  home  lib  lib64  lost+found  media  mnt  opt  proc  root  run  sbin  snap  srv  sys  tmp  usr  var
vboxuser@Ubuntu:~/os-lab5$ ls -ld /etc /etc/passwd /bin /dev/null /tmp
lrwxrwxrwx   1 root root     7 Apr 20 08:46 /bin -> usr/bin
crw-rw-rw-   1 root root  1, 3 Oct 19 11:45 /dev/null
drwxr-xr-x 142 root root 12288 Oct 19 11:45 /etc
-rw-r--r--   1 root root  2823 Oct  2 12:41 /etc/passwd
drwxrwxrwt  18 root root   420 Oct 19 11:46 /tmp
```

<p align="center"><em>Figure 1.1 – The root of the tree and the types of five paths</em></p>


```
vboxuser@Ubuntu:~/os-lab5$ ls -l /dev | head
total 0
crw-r--r--   1 root     root     10, 235 Oct 19 11:45 autofs
drwxr-xr-x   2 root     root        500 Oct 19 11:45 block
drwxr-xr-x   2 root     root         80 Oct 19 11:45 bsg
crw-------   1 root     root     10, 234 Oct 19 11:45 btrfs-control
drwxr-xr-x   3 root     root         60 Oct 19 11:45 bus
lrwxrwxrwx+  1 root     root          3 Oct 19 11:45 cdrom -> sr0
drwxr-xr-x   2 root     root       3740 Oct 19 11:45 char
crw-------   1 root     tty       5,   1 Oct 19 11:45 console
lrwxrwxrwx   1 root     root         11 Oct 19 11:45 core -> /proc/kcore
```

<p align="center"><em>Figure 1.2 – The beginning of /dev</em></p>


To find one character device and one block device I filtered the same listing by the first letter:

```
vboxuser@Ubuntu:~/os-lab5$ ls -l /dev | grep -m1 '^c'
crw-r--r--   1 root     root     10, 235 Oct 19 11:45 autofs
vboxuser@Ubuntu:~/os-lab5$ ls -l /dev | grep -m1 '^b'
brw-rw----   1 root     disk      7,   0 Oct 19 11:45 loop0
```

<p align="center"><em>Figure 1.3 – A character device and a block device</em></p>


```
vboxuser@Ubuntu:~/os-lab5$ file /usr/bin/ls /etc/passwd /bin
/usr/bin/ls: symbolic link to ../lib/cargo/bin/coreutils/ls
/etc/passwd: ASCII text
/bin:        symbolic link to usr/bin
vboxuser@Ubuntu:~/os-lab5$ du -sh /var/log 2>/dev/null
112M	/var/log
vboxuser@Ubuntu:~/os-lab5$ du -sh /usr 2>/dev/null
5.3G	/usr
```

<p align="center"><em>Figure 1.4 – The output of file and du</em></p>


### Observe

**1. For each path in the second line, write its type from the first letter of `ls -l`.**

| Path | First letter | Type |
|---|---|---|
| `/etc` | `d` | directory |
| `/etc/passwd` | `-` | regular file |
| `/bin` | `l` | symbolic link |
| `/dev/null` | `c` | character device |
| `/tmp` | `d` | directory (the last letter `t` is the sticky bit, see Part 3) |

**2. What is `/bin` on my system: a directory or a link, and to where?**
It is a symbolic link (`lrwxrwxrwx`) to `usr/bin`. The command `file /bin` says the same: `symbolic link to usr/bin`. In this Ubuntu the old directories `/bin`, `/sbin` and `/lib` are only links into `/usr`. Another interesting thing: `/usr/bin/ls` is also a link, to the `coreutils` program in `/usr/lib/cargo/bin`, so this system uses the Rust version of the basic commands.

**3. One character device and one block device in `/dev`.**
Character device: `/dev/autofs` (it starts with `c`; also `/dev/null` is one, with the numbers 1, 3). Block device: `/dev/loop0` (it starts with `b`, major number 7). A character device is read and written as a stream of bytes, a block device is accessed in blocks and can hold a file system.

**4. Which is bigger, `/var/log` or `/usr`, and by roughly how much?**
`/usr` is bigger: 5.3G against 112M for `/var/log`. 5.3 G is about 5430 M, so `/usr` is about 48 times bigger. This is logical: `/usr` holds all the programs and libraries, and `/var/log` holds only the logs.


## Part 2. Inodes and links

### 1. An inode, and a name that is not part of it

```
vboxuser@Ubuntu:~/os-lab5$ echo hello > notes.txt
vboxuser@Ubuntu:~/os-lab5$ stat notes.txt
  File: notes.txt
  Size: 6         	Blocks: 8          IO Block: 4096   regular file
Device: 8,2	Inode: 426610      Links: 1
Access: (0664/-rw-rw-r--)  Uid: ( 1000/vboxuser)   Gid: ( 1000/vboxuser)
Access: 2026-10-19 11:46:52.109831299 +0000
Modify: 2026-10-19 11:46:52.109831299 +0000
Change: 2026-10-19 11:46:52.109831299 +0000
 Birth: 2026-10-19 11:46:52.109831299 +0000
vboxuser@Ubuntu:~/os-lab5$ ls -li notes.txt
426610 -rw-rw-r-- 1 vboxuser vboxuser 6 Oct 19 11:46 notes.txt
```

<p align="center"><em>Figure 2.1 – stat and ls -li of a new file</em></p>


### 2. Hard link, then remove the "original"

```
vboxuser@Ubuntu:~/os-lab5$ ln notes.txt hard.txt
vboxuser@Ubuntu:~/os-lab5$ ls -li notes.txt hard.txt
426610 -rw-rw-r-- 2 vboxuser vboxuser 6 Oct 19 11:46 hard.txt
426610 -rw-rw-r-- 2 vboxuser vboxuser 6 Oct 19 11:46 notes.txt
vboxuser@Ubuntu:~/os-lab5$ rm notes.txt
vboxuser@Ubuntu:~/os-lab5$ cat hard.txt
hello
vboxuser@Ubuntu:~/os-lab5$ ls -li hard.txt
426610 -rw-rw-r-- 1 vboxuser vboxuser 6 Oct 19 11:46 hard.txt
```

<p align="center"><em>Figure 2.2 – A hard link before and after removing the first name</em></p>


### 3. Symbolic link, then remove its target

```
vboxuser@Ubuntu:~/os-lab5$ ln -s hard.txt soft.txt
vboxuser@Ubuntu:~/os-lab5$ ls -li hard.txt soft.txt
426610 -rw-rw-r-- 1 vboxuser vboxuser 6 Oct 19 11:46 hard.txt
426612 lrwxrwxrwx 1 vboxuser vboxuser 8 Oct 19 11:46 soft.txt -> hard.txt
vboxuser@Ubuntu:~/os-lab5$ cat soft.txt
hello
vboxuser@Ubuntu:~/os-lab5$ rm hard.txt
vboxuser@Ubuntu:~/os-lab5$ cat soft.txt
cat: soft.txt: No such file or directory
vboxuser@Ubuntu:~/os-lab5$ ls -l soft.txt
lrwxrwxrwx 1 vboxuser vboxuser 8 Oct 19 11:46 soft.txt -> hard.txt
```

<p align="center"><em>Figure 2.3 – A symbolic link and a broken (dangling) link</em></p>


### 4. Directories have links too

My prediction: a new empty directory has 2 links, and after two subdirectories it has 4.

```
vboxuser@Ubuntu:~/os-lab5$ mkdir d
vboxuser@Ubuntu:~/os-lab5$ ls -ld d
drwxrwxr-x 2 vboxuser vboxuser 4096 Oct 19 11:46 d
vboxuser@Ubuntu:~/os-lab5$ mkdir d/x d/y
vboxuser@Ubuntu:~/os-lab5$ ls -ld d
drwxrwxr-x 4 vboxuser vboxuser 4096 Oct 19 11:46 d
```

<p align="center"><em>Figure 2.4 – The link count of a directory</em></p>


### 5. Size is not space

```
vboxuser@Ubuntu:~/os-lab5$ echo x > one.txt
vboxuser@Ubuntu:~/os-lab5$ ls -l one.txt
-rw-rw-r-- 1 vboxuser vboxuser 2 Oct 19 11:46 one.txt
vboxuser@Ubuntu:~/os-lab5$ du -h one.txt
4.0K	one.txt
vboxuser@Ubuntu:~/os-lab5$ truncate -s 1G big.img
vboxuser@Ubuntu:~/os-lab5$ ls -lh big.img
-rw-rw-r-- 1 vboxuser vboxuser 1.0G Oct 19 11:46 big.img
vboxuser@Ubuntu:~/os-lab5$ du -h big.img
0	big.img
vboxuser@Ubuntu:~/os-lab5$ rm big.img
```

<p align="center"><em>Figure 2.5 – The size of a file and the space it uses</em></p>


### Observe

**1. Which number in `stat` is the inode, and which tells how many names the file has?**
The inode number is `Inode: 426610`. The number of names (hard links) is `Links: 1`. In `ls -li` the inode is the first column and the link count is the third column (the number after the permissions).

**2. After `rm notes.txt`, why could I still read `hard.txt`? What does `rm` really remove?**
A file is not its name. The data and the information about the file (size, owner, permissions, where the blocks are) live in the inode 426610, and the names are only entries in a directory that point to this inode. After `ln` both `notes.txt` and `hard.txt` pointed to the same inode, and the link count was 2. `rm notes.txt` removed only one name (the directory entry) and decreased the link count from 2 to 1. The inode and the data stayed, so `cat hard.txt` still printed `hello`. The data is really freed only when the link count becomes 0 (and no process has the file open).

**3. Why did `cat soft.txt` fail at the end, although `soft.txt` still exists? How big is `soft.txt` and why exactly that size?**
A symbolic link is a separate small file with its own inode (426612, different from 426610). Inside it there is only the text of a path: `hard.txt`. When I do `cat soft.txt`, the system reads this path and looks for the file `hard.txt`. After `rm hard.txt` there is no such name, so the link is broken (dangling) and `cat` says `No such file or directory`, while the link itself is still there. Its size is 8 bytes, because it is exactly the length of the stored text `hard.txt` (8 characters).

**4. A new directory has 2 links, and 4 after two subdirectories. Where does each link come from?**
For the empty directory `d` the two links are: (1) the name `d` in its parent directory, and (2) the entry `.` inside `d` itself, which points to `d`. After `mkdir d/x d/y` each new subdirectory has an entry `..` which points to its parent `d`. That adds two more links: 2 + 2 = 4. So a directory has 2 + (number of subdirectories) links. Ordinary files inside do not change it.

**5. Why does a 2-byte file use 4.0K, and a 1 GB file use 0?**
The file system gives space in blocks, not in bytes, and here the block is 4096 bytes (`IO Block: 4096`; in `stat` of the first file `Blocks: 8` means 8 sectors of 512 bytes = 4096). So even the file `one.txt` (the text `x` and a newline, 2 bytes) takes one whole block, and `du` shows 4.0K. The file `big.img` created with `truncate -s 1G` is a sparse file: `truncate` only sets the size 1 GB in the inode, but it does not write any data and does not allocate any block, so `du` shows 0 (the file is one big hole). `ls -l` shows the size of the file, `du` shows the space it really uses on the disk.


## Part 3. Permissions

### 6. Files

```
vboxuser@Ubuntu:~/os-lab5$ echo secret > s.txt
vboxuser@Ubuntu:~/os-lab5$ chmod 600 s.txt ; ls -l s.txt
-rw------- 1 vboxuser vboxuser 7 Oct 19 11:47 s.txt
vboxuser@Ubuntu:~/os-lab5$ chmod 000 s.txt ; cat s.txt
cat: s.txt: Permission denied
vboxuser@Ubuntu:~/os-lab5$ chmod u+r s.txt ; cat s.txt
secret
vboxuser@Ubuntu:~/os-lab5$ umask ; touch new.txt ; mkdir newdir ; ls -ld new.txt newdir
0002
-rw-rw-r-- 1 vboxuser vboxuser    0 Oct 19 11:47 new.txt
drwxrwxr-x 2 vboxuser vboxuser 4096 Oct 19 11:47 newdir
```

<p align="center"><em>Figure 3.1 – Permissions of files and the umask</em></p>


### 7. Directories

My predictions: with `u=r` the command `ls box` works and `cat box/f` fails; with `u=x` the command `ls box` fails and `cat box/f` works.

```
vboxuser@Ubuntu:~/os-lab5$ mkdir box ; echo hi > box/f
vboxuser@Ubuntu:~/os-lab5$ chmod u=r,go= box ; ls box ; cat box/f
f
cat: box/f: Permission denied
vboxuser@Ubuntu:~/os-lab5$ chmod u=x,go= box ; ls box ; cat box/f
ls: cannot open directory 'box': Permission denied
hi
vboxuser@Ubuntu:~/os-lab5$ chmod 755 box
```

<p align="center"><em>Figure 3.2 – Read only and execute only on a directory</em></p>


### 8. Special bits and other people's files

```
vboxuser@Ubuntu:~/os-lab5$ ls -l /usr/bin/passwd ; ls -ld /tmp
-rwsr-xr-x 1 root root 93640 Feb  2  2026 /usr/bin/passwd
drwxrwxrwt 15 root root 360 Oct 19 11:46 /tmp
vboxuser@Ubuntu:~/os-lab5$ cat /etc/shadow
cat: /etc/shadow: Permission denied
vboxuser@Ubuntu:~/os-lab5$ ls -l /etc/shadow
-rw-r----- 1 root shadow 1336 Oct  2 12:41 /etc/shadow
```

<p align="center"><em>Figure 3.3 – setuid, the sticky bit and /etc/shadow</em></p>


### Observe

**1. Write 600, 644, 755 and 700 as `rwx` strings, and give one real use for each.**

| Mode | String | Real use |
|---|---|---|
| 600 | `rw-------` | a private file that only the owner can read and write, e.g. a private SSH key (`~/.ssh/id_rsa`); my `s.txt` |
| 644 | `rw-r--r--` | a normal file that everybody can read but only the owner can change, e.g. `/etc/passwd` |
| 755 | `rwxr-xr-x` | a program or a public directory: everybody can run or enter it, only the owner can change it, e.g. `/usr/bin/ls` |
| 700 | `rwx------` | a private directory, e.g. `~/.ssh` |

**2. With `chmod 000` I own the file but cannot read it. Why does ownership not help?**
The system checks the permission bits of the class that matches me. I am the owner, so the "user" class is used, and for `000` this class has no `r`, so `cat` is denied. Ownership does not give access by itself; what it gives is the right to change the mode. That is why `chmod u+r s.txt` worked, and after that I could read the file again.

**3. What is my umask, and how does it explain the permissions of `new.txt` and `newdir`?**
My umask is `0002`. The umask says which permission bits are taken away from the default ones. The default for a new file is 666, and 666 minus 002 gives 664 = `rw-rw-r--` (this is what `new.txt` has). The default for a new directory is 777, and 777 minus 002 gives 775 = `rwxrwxr-x` (this is what `newdir` has). So only the "write for others" bit is removed.

**4. Explain the `box` experiment: what does `r` allow on a directory, and what does `x` allow?**
On a directory, `r` allows to read the list of names inside, and `x` allows to go through the directory, that is to access an entry by its name (to open `box/f`, to `cd` into it).
- With `u=r`: `ls box` worked and printed `f` (the list can be read), but `cat box/f` failed, because without `x` I cannot pass through the directory to the file.
- With `u=x`: `ls box` failed (`cannot open directory`), because I cannot read the list, but `cat box/f` worked and printed `hi`, because I know the name and I have `x`.

**5. What do the `s` in `/usr/bin/passwd` and the `t` in `/tmp` mean, and why does `passwd` need the `s`?**
The `s` in `-rwsr-xr-x` is the setuid bit: when any user runs this program, it runs with the rights of the file owner (here `root`) and not with the rights of the user. `passwd` needs it because it must write the new password hash into `/etc/shadow`, and this file is `-rw-r-----` owned by `root:shadow`, so a normal user cannot even read it (`cat /etc/shadow` gave `Permission denied`). The `t` in `drwxrwxrwt` is the sticky bit on a directory: everybody can create files in `/tmp`, but a user can delete or rename only their own files (or the owner of the directory, or root), not the files of other users.


## Part 4. Disks and the kernel as files

```
vboxuser@Ubuntu:~/os-lab5$ lsblk
NAME   MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS
loop0    7:0    0     4K  1 loop /snap/bare/5
loop1    7:1    0  66.8M  1 loop /snap/core24/2124
...
loop17   7:17   0   828K  1 loop /snap/snapd-desktop-integration/391
sda      8:0    0    30G  0 disk 
├─sda1   8:1    0     1M  0 part 
└─sda2   8:2    0    30G  0 part /
sr0     11:0    1  50.7M  1 rom  /run/media/vboxuser/VBox_GAs_7.2.20
```

<p align="center"><em>Figure 4.1 – The block devices (the 18 loop devices of the snap packages are shortened with "...")</em></p>


```
vboxuser@Ubuntu:~/os-lab5$ df -h
Filesystem      Size  Used Avail Use% Mounted on
tmpfs           680M  2.1M  678M   1% /run
/dev/sda2        30G  8.1G   20G  29% /
tmpfs           1.7G     0  1.7G   0% /dev/shm
tmpfs           1.7G  8.0K  1.7G   1% /tmp
none            1.0M     0  1.0M   0% /run/credentials/systemd-journald.service
none            1.0M     0  1.0M   0% /run/credentials/systemd-resolved.service
tmpfs           340M   80K  340M   1% /run/user/1000
/dev/sr0         51M   51M     0 100% /run/media/vboxuser/VBox_GAs_7.2.20
vboxuser@Ubuntu:~/os-lab5$ mount | grep " / "
/dev/sda2 on / type ext4 (rw,relatime)
```

<p align="center"><em>Figure 4.2 – Free space and the root file system</em></p>


```
vboxuser@Ubuntu:~/os-lab5$ grep -c processor /proc/cpuinfo
2
vboxuser@Ubuntu:~/os-lab5$ head -3 /proc/meminfo
MemTotal:        3480328 kB
MemFree:          585244 kB
MemAvailable:    2084016 kB
vboxuser@Ubuntu:~/os-lab5$ head -5 /proc/self/status
Name:	head
Umask:	0002
State:	R (running)
Tgid:	4046
Ngid:	0
vboxuser@Ubuntu:~/os-lab5$ ls -l /proc/self/fd
total 0
lrwx------ 1 vboxuser vboxuser 64 Oct 19 11:47 0 -> /dev/pts/0
l-wx------ 1 vboxuser vboxuser 64 Oct 19 11:47 1 -> pipe:[27963]
l-wx------ 1 vboxuser vboxuser 64 Oct 19 11:47 2 -> pipe:[27963]
lr-x------ 1 vboxuser vboxuser 64 Oct 19 11:47 3 -> /proc/4037/fd
```

<p align="center"><em>Figure 4.3 – The kernel information in /proc</em></p>


In the last command the descriptors 1 and 2 point to a pipe and not to the terminal, because I saved the output of the whole session with `tee`. In a plain terminal they would point to `/dev/pts/0`, like the descriptor 0.

### Observe

**1. Draw my disk: disk name, partitions, which one is mounted at `/` and with what file system type.**

```
sda  (30G disk)
├── sda1  (1M, not mounted)
└── sda2  (30G, mounted at /, file system type ext4)
```

The only real disk is `sda` (a virtual disk of the VirtualBox machine). It has two partitions: a tiny `sda1` of 1M, which is not mounted, and `sda2` of almost the whole disk, mounted at `/`. The command `mount | grep " / "` shows that the type is `ext4`. The other devices in `lsblk` are not real disks: the `loop` devices are the images of the snap packages mounted in `/snap/...`, and `sr0` is the virtual CD with the VirtualBox Guest Additions.

**2. How full is my root file system?**
`/dev/sda2` has 30G, 8.1G is used, 20G is free, so it is 29% full.

**3. How many CPU cores and how much RAM does my machine have, according to `/proc`?**
`grep -c processor /proc/cpuinfo` printed 2, so there are 2 cores (virtual ones). `MemTotal` is 3480328 kB, which is 3480328 / 1024 / 1024 ≈ 3.3 GiB of RAM.

**4. `/proc/self/status` showed the name of a program. Which one, and why that one?**
The name is `head`. `/proc/self` is a special link that always points to the directory of the process which is reading it. The process which opened `/proc/self/status` here was `head` itself, so I saw the information about `head`. `/proc` is not a real directory on the disk: the kernel creates these "files" at the moment when a program reads them.

---

## Conclusion

In Lab 5 I checked on Linux the things that were explained in the lecture. I saw the types of files in the first letter of `ls -l` (`d`, `-`, `l`, `c`, `b`), and that `/bin` is only a link to `/usr/bin`. With `stat` and `ls -li` I saw that the name of a file is not a part of it: a file is an inode, and the names (hard links) are entries in directories, so `rm` removes a name and the data is freed only when the last name is gone. A symbolic link is a different thing: a small file that keeps a path, which becomes broken when the target is removed. I measured that the size of a file is not the space it uses (a 2-byte file takes a 4K block, a 1 GB sparse file takes nothing). I tested the permissions: the owner cannot read a file with mode 000 but can change the mode, `r` and `x` on a directory are different things (the list of names and the passing through), the umask 0002 gives 664 and 775, and the bits `s` and `t` explain how `passwd` can change `/etc/shadow` and why users cannot delete each other's files in `/tmp`. In the end I looked at the disk (`sda2`, ext4, 29% full) and at `/proc`, where the kernel shows its data as files.
