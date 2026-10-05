# Lab 1: The OS as a Resource Manager
 
**MINISTRY OF EDUCATION, CULTURE AND RESEARCH**
**OF THE REPUBLIC OF MOLDOVA**
 
**Technical University of Moldova**
**Faculty of Computers, Informatics and Microelectronics**
**Department of Software Engineering and Automation**
 
 
**MAXIM ORMANJI FAF-242**
 
 
# Report
 
**Laboratory work**
**of Operating System**
 

## Before you start

I created the working folder `~/os-lab1` and checked who I am for the OS, which kernel is running and how long the system has been up.

```
vboxuser@Ubuntu:~$ cd os-lab1
vboxuser@Ubuntu:~/os-lab1$ whoami
vboxuser
vboxuser@Ubuntu:~/os-lab1$ uname -a
Linux Ubuntu 7.0.0-34-generic #34-Ubuntu SMP PREEMPT_DYNAMIC Wed Sep  2 14:29:37 UTC 2026 x86_64 GNU/Linux
vboxuser@Ubuntu:~/os-lab1$ uptime
 07:30:43 up 26 min,  1 user,  load average: 0.14, 0.08, 0.12
```

The OS sees me as the user `vboxuser`, and the kernel is Linux 7.0.0-34-generic.


## Part 1. Files and directories (file management)

### Navigating the tree

```
boxuser@Ubuntu:~/os-lab1$ pwd
/home/vboxuser/os-lab1
vboxuser@Ubuntu:~/os-lab1$ ls -la /
...
drwxr-xr-x 141 root root 12288 Sep 29 16:36 etc
drwxr-xr-x   3 root root  4096 Sep 29 16:22 home
...
vboxuser@Ubuntu:~/os-lab1$ ls -la ~
...
drwxrwxr-x  2 vboxuser vboxuser 4096 Sep 30 07:42 os-lab1
...
vboxuser@Ubuntu:~/os-lab1$ cd /etc && ls | head
ModemManager
NetworkManager
PackageKit
...
```

### Creating, copying, renaming and removing

```
vboxuser@Ubuntu:~/os-lab1$ cd ~/os-lab1
vboxuser@Ubuntu:~/os-lab1$ mkdir demo && cd demo
vboxuser@Ubuntu:~/os-lab1/demo$ echo "hello operating systems" > note.txt
vboxuser@Ubuntu:~/os-lab1/demo$ cp note.txt copy.txt
vboxuser@Ubuntu:~/os-lab1/demo$ mv copy.txt renamed.txt
vboxuser@Ubuntu:~/os-lab1/demo$ ls -l
total 8
-rw-rw-r-- 1 vboxuser vboxuser 24 Oct  1 11:29 note.txt
-rw-rw-r-- 1 vboxuser vboxuser 24 Oct  1 11:30 renamed.txt
vboxuser@Ubuntu:~/os-lab1/demo$ rm renamed.txt
vboxuser@Ubuntu:~/os-lab1/demo$ ls
note.txt
```

### Ownership and permissions

```
vboxuser@Ubuntu:~/os-lab1/demo$ ls -l note.txt
-rw-rw-r-- 1 vboxuser vboxuser 24 Oct  1 11:29 note.txt
vboxuser@Ubuntu:~/os-lab1/demo$ chmod 600 note.txt
vboxuser@Ubuntu:~/os-lab1/demo$ ls -l note.txt
-rw------- 1 vboxuser vboxuser 24 Oct  1 11:29 note.txt
vboxuser@Ubuntu:~/os-lab1/demo$ chmod 644 note.txt
```

### Observe

1. **Who owns the files and which group?** The files I created (`note.txt` and `renamed.txt`) are owned by the user `vboxuser`, and the group is also `vboxuser`. The names are the same because on Ubuntu every user gets a personal group with the same name.

2. **What do the ten characters mean after `chmod 600`?** The result was `-rw-------`. The first character `-` means it is a normal file (a directory would have `d`). After it there are three groups of three characters: the owner has `rw-` (can read and write, cannot execute), the group has `---` (nothing allowed) and the others have `---` (nothing allowed). So only I can use this file. The number 600 says the same: 6 = 4 (read) + 2 (write), and the two zeros mean no rights.

3. **Two directories under `/`:**
   - `/etc` holds the configuration files of the system and the programs.
   - `/home` holds the personal folders of the users (mine is `/home/vboxuser`).


## Part 2. Processes (process management)

### Listing the processes

```
vboxuser@Ubuntu:~/os-lab1$ ps aux | head
USER         PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
root           1  0.3  0.4  25284 16688 ?        Ss   13:02   0:01 /usr/lib/systemd/systemd --switched-root --system --deserialize=51
...
vboxuser@Ubuntu:~/os-lab1$ ps aux | wc -l
232
vboxuser@Ubuntu:~/os-lab1$ top
top - 13:12:36 up 10 min,  1 user,  load average: 0.14, 0.33, 0.25
Tasks: 229 total,   1 running, 228 sleeping,   0 stopped,   0 zombie
%Cpu(s):  5.7 us,  8.8 sy,  0.0 ni, 84.9 id,  0.0 wa,  0.0 hi,  0.6 si,  0.0 st 
MiB Mem :   3398.8 total,    354.2 free,   2068.1 used,   1144.0 buff/cache     
MiB Swap:      0.0 total,      0.0 free,      0.0 used.   1330.7 avail Mem 

    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND                                    
   2821 vboxuser  20   0 4035984 327048  97400 S  21.0   9.4   0:31.79 gnome-shell                                
   5018 vboxuser  20   0 1807516 373112 131124 S  11.3  10.7   0:08.64 ptyxis                                     
   4788 vboxuser  20   0   15.1g 428576 119272 S   6.3  12.3   0:26.20 Isolated Web Co                            
   3857 vboxuser  20   0   11.6g 538168 188180 S   5.7  15.5   0:26.16 firefox                                    
...
```

### A background process

```
vboxuser@Ubuntu:~/os-lab1$ sleep 300 &
[1] 5981
vboxuser@Ubuntu:~/os-lab1$ jobs
[1]+  Running                    sleep 300 &
vboxuser@Ubuntu:~/os-lab1$ ps -ef | grep sleep
vboxuser    5981    5057  0 13:16 pts/0    00:00:00 sleep 300
vboxuser    6006    5057  0 13:17 pts/0    00:00:00 grep --color=auto sleep
vboxuser@Ubuntu:~/os-lab1$ kill 5981
[1]+  Terminated                 sleep 300
```

### The /proc pseudo-filesystem

```
vboxuser@Ubuntu:~/os-lab1$ sleep 300 &
[1] 6453
vboxuser@Ubuntu:~/os-lab1$ PID=$!
vboxuser@Ubuntu:~/os-lab1$ ls /proc/$PID/
...
vboxuser@Ubuntu:~/os-lab1$ cat /proc/$PID/status | head -20
Name:	sleep
...
State:	S (sleeping)
...
VmSize:	   16112 kB
...
vboxuser@Ubuntu:~/os-lab1$ kill $PID
[1]+  Terminated                 sleep 300
```

### Observe

1. **PID of process number 1:** the PID is 1 and the process is `systemd` (the first line of `ps aux` shows `/usr/lib/systemd/systemd`). I looked it up: it is the first process that the kernel starts when the computer boots, and it starts the other services and programs.

2. **How many processes on the idle VM?** `ps aux | wc -l` printed 232, and one line of that is the header, so about 231 processes. `top` showed 229 tasks (1 running, 228 sleeping). So roughly 230 processes. The numbers are a little different because processes start and stop all the time. Many of them have names in square brackets, like `[kworker/...]`, and these are kernel threads, not normal programs.

3. **The `State:` line of the sleeping process:** it says `S (sleeping)`. The `sleep 300` process is just waiting for its timer and does not use the CPU, the OS wakes it up when the time is over. On my idle VM almost all processes are sleeping like this (228 of 229 in `top`).


## Part 3. Memory (memory management)

```
vboxuser@Ubuntu:~/os-lab1$ free -h
               total        used        free      shared  buff/cache   available
Mem:           3.3Gi       2.3Gi       175Mi        62Mi       1.0Gi       1.0Gi
Swap:             0B          0B          0B
vboxuser@Ubuntu:~/os-lab1$ cat /proc/meminfo | head -6
MemTotal:        3480360 kB
MemFree:          179168 kB
MemAvailable:    1059544 kB
...
vboxuser@Ubuntu:~/os-lab1$ sleep 300 & PID=$!
[1] 6829
vboxuser@Ubuntu:~/os-lab1$ grep VmRSS /proc/$PID/status
VmRSS:	    7684 kB
```

### Observe

1. **Total RAM and free RAM:** `free -h` shows 3.3 GiB total (`MemTotal` is 3480360 kB) and only 175 MiB free. It looks very low, but Linux uses spare RAM as cache (`buff/cache` is about 1.0 GiB) and gives it back to programs when they need it. So the column `available` (1.0 GiB) is the more honest number of memory that programs can still get.

2. **What is swap and how much is configured?** Swap is a space on the disk that the OS can use like extra memory: when RAM is full, it moves the pages that are not used much to the disk. On my VM swap is `0B`, so no swap is configured.

3. **VmRSS of a bare `sleep`:** `VmRSS` is 7684 kB, about 7.5 MB. It surprised me a bit, because `sleep` does nothing, but I think it still needs its code and the shared libraries (like libc) loaded in RAM. Also, in the `/proc` block of Part 2 the `VmSize` of a similar `sleep` process was 16112 kB, so the virtual size is bigger than the RAM it really uses.


## Part 4. Devices and storage (I/O management)

```
vboxuser@Ubuntu:~/os-lab1$ df -h
Filesystem      Size  Used Avail Use% Mounted on
...
/dev/sda2        30G  6.8G   22G  25% /
...
/dev/sr0         51M   51M     0 100% /run/media/vboxuser/VBox_GAs_7.2.20
vboxuser@Ubuntu:~/os-lab1$ lsblk 
NAME   MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS
loop0    7:0    0     4K  1 loop /snap/bare/5
...
loop4    7:4    0 260.3M  1 loop /snap/firefox/8763
...
sda      8:0    0    30G  0 disk 
├─sda1   8:1    0     1M  0 part 
└─sda2   8:2    0    30G  0 part /
sr0     11:0    1  50.7M  1 rom  /run/media/vboxuser/VBox_GAs_7.2.20
vboxuser@Ubuntu:~/os-lab1$ du -sh ~/os-lab1
28K	/home/vboxuser/os-lab1
vboxuser@Ubuntu:~/os-lab1$ ls -l /dev | head
...
lrwxrwxrwx+ 1 root     root          3 Oct  2 13:00 cdrom -> sr0
...
vboxuser@Ubuntu:~/os-lab1$ mount | head
...
/dev/sda2 on / type ext4 (rw,relatime)
...
```

### Observe

1. **Device of the root filesystem `/`:** it is mounted on `/dev/sda2` (the line `/dev/sda2 30G 6.8G 22G 25% /` in `df -h`). `mount` shows that its type is `ext4`. In `lsblk` it is the partition `sda2` of the 30 GB disk `sda`.

2. **One entry from `/dev`:** `/dev/cdrom`, which is a link to `sr0`. It stands for the CD/DVD drive. In my VM it is a virtual drive with the VirtualBox Guest Additions disc (it is also visible in `df -h` as `/dev/sr0`).

3. **"Everything is a file":** the OS shows disks, terminals and even information about processes in the same way as normal files, with paths and read/write operations. For example, I read `/proc/6453/status` like a text file, and my disk appears as `/dev/sda2`.


## Conclusion

The OS manages four resources: files, processes, memory and devices. I saw the files with `ls -l` (owner, group and permissions of `note.txt`), the processes with `ps aux` (about 230 processes, and `systemd` is PID 1) and the memory with `free -h` (3.3 GiB of RAM and no swap). I saw the devices with `lsblk`, which showed the disk `sda` that my root filesystem `/` is mounted on.
