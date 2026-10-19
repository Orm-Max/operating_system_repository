# Lab 3: The Command Line: Shell, Pipes and Text Filters

**MINISTRY OF EDUCATION, CULTURE AND RESEARCH**  
**OF THE REPUBLIC OF MOLDOVA**  

**Technical University of Moldova**  
**Faculty of Computers, Informatics and Microelectronics**  
**Department of Software Engineering and Automation**  


**MAXIM ORMANJI FAF-242**

Checked by: Reițman Petricia

Chișinău, 2026

## Purpose of the work

The purpose of this work is to learn to work fast in bash, to predict what the shell does with a command line before the command runs, to redirect the output where I want, and to answer questions about data with one line of small filters joined by pipes. At the end I put some of the one-liners into my first shell script.


## Before you start

I ran the course setup script once, created the working folder `~/os-lab3` and generated the artificial web access log with the command from the task.

```
vboxuser@Ubuntu:~/os-lab3$ bash ~/Downloads/os2026-setup.sh
vboxuser@Ubuntu:~/os-lab3$ cd ~/os-lab3
vboxuser@Ubuntu:~/os-lab3$ for i in $(seq 1 500); do
> echo "2026-10-05 10:$(printf %02d $((i % 60))) user$((i % 7)) $([ $((i % 5)) -eq 0 ] && echo FAIL || echo OK) /page$((i % 4))"
> done > access.log
vboxuser@Ubuntu:~/os-lab3$ head -3 access.log ; wc -l access.log
2026-10-05 10:01 user1 OK /page1
2026-10-05 10:02 user2 OK /page2
2026-10-05 10:03 user3 OK /page3
500 access.log
vboxuser@Ubuntu:~/os-lab3$ ls
access.log
```

The check is correct: the last command printed `500 access.log`. Every line of the log has the date, the time, the user, the result (OK or FAIL) and the page.


## Part 1. Move fast, find help

### Tab completion and history

I typed `ls /usr/sh` and pressed Tab: bash completed it to `ls /usr/share/`. When I pressed Tab one more time, it asked if I want to see all the possible names. The same happened with `cd /et`: Tab completed it to `cd /etc/`, and one more Tab gave the question about the possibilities.

```
vboxuser@Ubuntu:~/os-lab3$ ls /usr/share/
Display all 253 possibilities? (y or n)
vboxuser@Ubuntu:~/os-lab3$ cd /etc/
Display all 140 possibilities? (y or n)
```

<p align="center"><em>Figure 1.1 – Tab completion</em></p>


Then I ran three different commands (`whoami`, `pwd`, `ls`), pressed Ctrl+R and typed `wh`. Bash showed the old command `whoami` from the history:

```
vboxuser@Ubuntu:~/os-lab3$ whoami
vboxuser
vboxuser@Ubuntu:~/os-lab3$ pwd
/home/vboxuser/os-lab3
vboxuser@Ubuntu:~/os-lab3$ ls
access.log
(reverse-i-search)`wh': whoami
```

### Built-in or external, and the PATH

```
vboxuser@Ubuntu:~/os-lab3$ type cd ; type ls ; type echo ; which ls
cd is a shell builtin
ls is aliased to `ls --color=auto'
echo is a shell builtin
/usr/bin/ls
```

<p align="center"><em>Figure 1.2 – Output of type and which</em></p>


```
vboxuser@Ubuntu:~/os-lab3$ echo $PATH
/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/snap/bin
vboxuser@Ubuntu:~/os-lab3$ echo $PATH | tr ':' '\n'
/usr/local/sbin
/usr/local/bin
/usr/sbin
/usr/bin
/sbin
/bin
/usr/games
/usr/local/games
/snap/bin
/snap/bin
```

<p align="center"><em>Figure 1.3 – The directories in PATH</em></p>


```
vboxuser@Ubuntu:~/os-lab3$ nosuchprog ; echo "exit code: $?"
nosuchprog: command not found
exit code: 127
```

<p align="center"><em>Figure 1.4 – Exit code of a missing command</em></p>


### Help without a browser

```
vboxuser@Ubuntu:~/os-lab3$ man ls
LS(1)                                        General Commands Manual                                        LS(1)

NAME
       ls - List directory contents.  Ignore files and directories starting with a '.' by default

SYNOPSIS
       ls  [--help] 
...
vboxuser@Ubuntu:~/os-lab3$ ls --help | head
List directory contents.
Ignore files and directories starting with a '.' by default

Usage: ls [OPTION]... [FILE]...

Arguments:
  [paths]...  

Options:
      --help
vboxuser@Ubuntu:~/os-lab3$ man -k "list directory"
dir (1)              - List directory contents. Ignore files and directories starting with a '.' by default
gnudir (1)           - list directory contents
gnuls (1)            - list directory contents
gnuvdir (1)          - list directory contents
ls (1)               - List directory contents. Ignore files and directories starting with a '.' by default
ntfsls (8)           - list directory contents on an NTFS filesystem
rust-dir (1)         - List directory contents. Ignore files and directories starting with a '.' by default
rust-ls (1)          - List directory contents. Ignore files and directories starting with a '.' by default
rust-vdir (1)        - List directory contents. Ignore files and directories starting with a '.' by default
vdir (1)             - List directory contents. Ignore files and directories starting with a '.' by default
vboxuser@Ubuntu:~/os-lab3$ help cd
cd: cd [-L|[-P [-e]]] [-@] [dir]
    Change the shell working directory.
    
    Change the current directory to DIR.  The default DIR is the value of the
    HOME shell variable. If DIR is "-", it is converted to $OLDPWD.
    
    ...
```

### Observe

**1. Which of `cd`, `ls`, `echo` are built-ins, and why MUST `cd` be one?**
`cd` and `echo` are shell built-ins (`type` says "is a shell builtin"). `ls` is not a built-in: it is an external program, the real file is `/usr/bin/ls`. In my shell `ls` is also an alias, so bash adds `--color=auto` every time I type `ls`. `cd` must be a built-in because the current directory belongs to the process. An external program is started as a child process: if `cd` was such a program, it would change the directory only of its own process and then exit, and my shell would stay in the old directory. So the shell has to do `cd` itself.

**2. How many directories are in the PATH, and in which one is `ls`?**
`echo $PATH | tr ':' '\n'` printed 10 lines, so the PATH has 10 entries. One of them (`/snap/bin`) is written two times, so there are 9 different directories. `ls` is in `/usr/bin` (`which ls` printed `/usr/bin/ls`).

**3. What exit code does a command that is not found return?**
It returns 127 (the message was `nosuchprog: command not found` and `exit code: 127`). A normal successful command returns 0.

**4. Which option of `ls` sorts by modification time?**
The option `-t`: it sorts the files by modification time, the newest first.

I found it in the manual of `ls` with `grep` (line 142 of the page says `-t     Sort by modification time (the 'mtime' in the inode), newest first.`):

```
vboxuser@Ubuntu:~/os-lab3$ man ls | grep -n -i "modif"
102:              modification time: mtime, modification.  birth time: birth, creation;
105:              in the inode) instead of the modification time. When explicitly sorting by time (--sort=time or -t)
109:              the modification time. When explicitly sorting by time (--sort=time or -t) or when not using a long
142:       -t     Sort by modification time (the 'mtime' in the inode), newest first.
```

Then I tested it with `ls -lt`: the newest file (`report.sh`) is at the top and the oldest (`access.log`) is at the bottom.

```
vboxuser@Ubuntu:~/os-lab3$ ls -lt
total 52
-rwxrwxr-x 1 vboxuser vboxuser   303 Oct 12 14:24 report.sh
-rw-rw-r-- 1 vboxuser vboxuser  1031 Oct 12 14:15 oneliners.sh
-rw-rw-r-- 1 vboxuser vboxuser  2288 Oct 12 13:36 all.txt
drwxrwxr-x 2 vboxuser vboxuser  4096 Oct 12 13:34 tmp
-rw-rw-r-- 1 vboxuser vboxuser    13 Oct 12 13:31 notes.md
-rw-rw-r-- 1 vboxuser vboxuser    53 Oct 12 13:30 err.txt
-rw-rw-r-- 1 vboxuser vboxuser  2288 Oct 12 13:30 out.txt
drwxrwxr-x 2 vboxuser vboxuser  4096 Oct 12 13:02 play
-rw-rw-r-- 1 vboxuser vboxuser 16700 Oct 12 11:12 access.log
```


## Part 2. What the shell does to your line

### Setup

```
vboxuser@Ubuntu:~/os-lab3$ mkdir -p ~/os-lab3/play && cd ~/os-lab3/play
vboxuser@Ubuntu:~/os-lab3/play$ touch a.c b.c file1.txt file2.txt notes.txt "alfa beta.txt"; ls
 a.c  'alfa beta.txt'   b.c   file1.txt   file2.txt   notes.txt
```

<p align="center"><em>Figure 2.1 – Files created in the play directory</em></p>


### Predictions and real results

Before running each line, I wrote down what I think it will print. The table has my prediction, the real output and an explanation if they were different.

| Command | My prediction | Real output | Explanation |
|---|---|---|---|
| `echo *.c` | all .c files | `a.c b.c` | |
| `echo file?.txt` | file + any one character + .txt | `file1.txt file2.txt` | |
| `echo [a-b]*` | all files starting with a or b | `a.c alfa beta.txt b.c` | |
| `echo *.xyz` | no such files | `*.xyz` | No file matches the pattern, so bash leaves it unexpanded and passes it to `echo` as plain text. |
| `ls alfa beta.txt` | No such file (2 nonexistent arguments) | `ls: cannot access 'alfa': No such file or directory`<br>`ls: cannot access 'beta.txt': No such file or directory` | |
| `ls "alfa beta.txt"` | same file | `'alfa beta.txt'` | |
| `echo '$HOME' "$HOME" \$HOME` | main folder | `$HOME /home/vboxuser $HOME` | Only the middle argument is the home folder. Single quotes disable expansion, so `'$HOME'` stays literal. Double quotes allow `$` expansion, so `"$HOME"` becomes `/home/vboxuser`. The backslash escapes `$`, so `\$HOME` stays literal. |
| `echo {1..5} ; echo $((7 * 6))` | 1 2 3 ... and 42 | `1 2 3 4 5` and `42` | |
| `echo "today is $(date +%A), I am $(whoami)"` | message with today's day and user name | `today is Monday, I am vboxuser` | |
| `touch log{1..3}.txt && ls log*` | creates log1.txt, log2.txt, log3.txt and lists them | `log1.txt log2.txt log3.txt` | |

Globbing results (Figure 2.2):

```
vboxuser@Ubuntu:~/os-lab3/play$ echo *.c
a.c b.c
vboxuser@Ubuntu:~/os-lab3/play$ echo file?.txt
file1.txt file2.txt
vboxuser@Ubuntu:~/os-lab3/play$ echo [a-b]*
a.c alfa beta.txt b.c
vboxuser@Ubuntu:~/os-lab3/play$ echo *.xyz
*.xyz
```

<p align="center"><em>Figure 2.2 – Globbing results</em></p>


The name with a space (Figure 2.3):

```
vboxuser@Ubuntu:~/os-lab3/play$ ls alfa beta.txt
ls: cannot access 'alfa': No such file or directory
ls: cannot access 'beta.txt': No such file or directory
vboxuser@Ubuntu:~/os-lab3/play$ ls "alfa beta.txt"
'alfa beta.txt'
```

<p align="center"><em>Figure 2.3 – The ls error on a name with a space, and the fix</em></p>


Quotes and backslash (Figure 2.4):

```
vboxuser@Ubuntu:~/os-lab3/play$ echo '$HOME' "$HOME" \$HOME
$HOME /home/vboxuser $HOME
```

<p align="center"><em>Figure 2.4 – Single quotes, double quotes and backslash</em></p>


The other lines:

```
vboxuser@Ubuntu:~/os-lab3/play$ echo {1..5} ; echo $((7 * 6))
1 2 3 4 5
42
vboxuser@Ubuntu:~/os-lab3/play$ echo "today is $(date +%A), I am $(whoami)"
today is Monday, I am vboxuser
vboxuser@Ubuntu:~/os-lab3/play$ touch log{1..3}.txt && ls log*
log1.txt log2.txt log3.txt
```

### Observe

**1. For each line: prediction, real output and one sentence if they differ.**
See the table above. All my predictions were right in the idea. The only line that needs an explanation is `echo *.xyz`: I expected nothing, but bash printed the pattern itself, because when no file matches, bash leaves the pattern as it is.

**2. `echo` knows nothing about files, yet `echo *.c` printed file names. Who produced them?**
The shell did it. Before running `echo`, bash looks at the line, sees the pattern `*.c`, replaces it with the names of the matching files (`a.c b.c`) and only then starts `echo` with these two words as arguments. So `echo` just printed what it received. Other things that the shell does before the command runs are the same kind: `{1..5}`, `$((7 * 6))`, `$(date +%A)` and `$HOME` are also replaced by the shell.

**3. Why did `ls alfa beta.txt` fail? Two ways to fix it.**
The shell splits the line into words at the spaces, so `ls` got two arguments, `alfa` and `beta.txt`, and neither of these files exists. The file is really called `alfa beta.txt` (one name with a space inside). Two ways to fix it: put the name in quotes, `ls "alfa beta.txt"` (this one I ran and it worked), or put a backslash before the space, `ls alfa\ beta.txt`, so the space is not a separator any more.

**4. What is the difference between single and double quotes?**
Single quotes keep everything as plain text: nothing inside is expanded, so `'$HOME'` stays `$HOME`. Double quotes keep the spaces together, but still allow expansion with `$`, so `"$HOME"` became `/home/vboxuser` and `"$(whoami)"` would give my user name. The backslash is a third way: it takes away the special meaning of only one next character (`\$HOME` stayed `$HOME`).

**Why does `ls` show `'alfa beta.txt'` in quotes?**
Because the name has a space. `ls` adds the quotes to show that it is one name and not two files, and so I can copy it into a command and it will work.


## Part 3. Redirection, chaining and jobs

### stdout and stderr

```
vboxuser@Ubuntu:~/os-lab3$ ls /etc /nope > out.txt 2> err.txt
vboxuser@Ubuntu:~/os-lab3$ wc -l out.txt ; cat err.txt
238 out.txt
ls: cannot access '/nope': No such file or directory
vboxuser@Ubuntu:~/os-lab3$ ls
access.log  err.txt  out.txt  play
```

<p align="center"><em>Figure 3.1 – stdout and stderr in separate files</em></p>


```
vboxuser@Ubuntu:~/os-lab3$ ls /etc /nope > all.txt 2>&1 ; tail -2 all.txt
xml
zsh_command_not_found
vboxuser@Ubuntu:~/os-lab3$ echo first > notes.md ; echo second >> notes.md ; cat notes.md
first
second
vboxuser@Ubuntu:~/os-lab3$ wc -l < access.log
500
```

### The order of redirections

```
vboxuser@Ubuntu:~/os-lab3$ ls /etc /nope > all.txt 2>&1
vboxuser@Ubuntu:~/os-lab3$ head -2 all.txt
ls: cannot access '/nope': No such file or directory
/etc:
vboxuser@Ubuntu:~/os-lab3$ ls /etc /nope 2>&1 > all.txt
ls: cannot access '/nope': No such file or directory
vboxuser@Ubuntu:~/os-lab3$ head -2 all.txt
/etc:
ModemManager
```

<p align="center"><em>Figure 3.2 – Comparing the order of redirections</em></p>


### The operators `;` `&&` `||`

```
vboxuser@Ubuntu:~/os-lab3$ ls /nope 2> /dev/null || echo "failed with code $?"
failed with code 2
vboxuser@Ubuntu:~/os-lab3$ mkdir -p tmp && cd tmp && echo "I am in $(pwd)" ; cd ~/os-lab3
I am in /home/vboxuser/os-lab3/tmp
```

<p align="center"><em>Figure 3.3 – The ; && || operators</em></p>


### A background job

```
vboxuser@Ubuntu:~/os-lab3$ sleep 100 &
[1] 21770
vboxuser@Ubuntu:~/os-lab3$ jobs ; kill %1 ; jobs
[1]+  Running                    sleep 100 &
[1]+  Running                    sleep 100 &
[1]+  Terminated                 sleep 100
```

<p align="center"><em>Figure 3.4 – Running a background job and stopping it</em></p>


The second `jobs` still showed `Running`, because it ran immediately after `kill` and the shell had not yet noticed that the process ended. The message `Terminated` appeared right after.

### Observe

**1. Which file got the error message, and which got the listing?**
In `ls /etc /nope > out.txt 2> err.txt` the listing of `/etc` went to `out.txt` (238 lines) and the error message `cannot access '/nope'` went to `err.txt`. The number 1 stream (stdout) is redirected with `>` and the number 2 stream (stderr) with `2>`.

**2. What is the difference between `> all.txt 2>&1` and `2>&1 > all.txt`?**
The shell reads the redirections from left to right.
- In `> all.txt 2>&1`: first stdout goes to the file, then stderr is sent to the same place where stdout goes now, which is the file. So both the error and the listing are in `all.txt` (the first lines of the file were the error and `/etc:`).
- In `2>&1 > all.txt`: first stderr is sent to the place where stdout goes now, which is still the terminal, and only after that stdout is moved to the file. So the error was printed on the screen, and only the listing is in `all.txt` (the first lines were `/etc:` and `ModemManager`).

**3. Why does `wc -l < access.log` print only a number, while `wc -l access.log` also prints the file name?**
With `<` the shell opens the file and gives it to `wc` as its standard input, so `wc` only sees a stream of text and does not know any file name. With `wc -l access.log` the name is an argument, `wc` opens the file itself and prints the name next to the number.

**4. The operators `;`, `&&`, `||` in one sentence each.**
- `;` runs the next command always, when the first one is finished (it does not matter if it worked or not).
- `&&` runs the next command only if the first one succeeded (exit code 0), like in `mkdir -p tmp && cd tmp && echo ...`.
- `||` runs the next command only if the first one failed (exit code not 0), like in `ls /nope 2> /dev/null || echo "failed with code $?"`, which printed `failed with code 2`.


## Part 4. Answer questions with one-liners

I answered every question with one command line and saved all of them in the file `oneliners.sh`.

```
vboxuser@Ubuntu:~/os-lab3$ cat > oneliners.sh <<'EOF'
# Q1. How many requests failed?
grep -c FAIL access.log
# Q2. How many different pages were requested, and which?
cut -d' ' -f5 access.log | sort -u | tee /dev/stderr | wc -l
# Q3. How many requests did each page get?
cut -d' ' -f5 access.log | sort | uniq -c
# Q4. Which user or users have the most failed requests, and how many?
grep FAIL access.log | cut -d' ' -f3 | sort | uniq -c | sort -rn | awk 'NR==1{m=$1} $1==m'
# Q5. How many requests did user3 make, and how many of them failed?
grep ' user3 ' access.log | cut -d' ' -f4 | sort | uniq -c
# Q6. Print the last 3 failed requests, showing only time and user.
grep FAIL access.log | tail -3 | cut -d' ' -f2,3
# Q7. Which login shells appear in /etc/passwd, and how many accounts use each?
cut -d: -f7 /etc/passwd | sort | uniq -c | sort -rn
# Q8. Why do ls /etc | wc -l and ls -l /etc | wc -l differ by exactly one?
ls /etc | wc -l ; ls -l /etc | wc -l
# Bonus. Lines where the minute ends in 0 and the request failed.
grep -E '10:[0-5]0 user[0-9] FAIL' access.log | wc -l
EOF
```

### Q1. How many requests failed?

```
grep -c FAIL access.log
100
```

<p align="center"><em>Figure 4.1 – Number of failed requests</em></p>


`grep -c` counts the lines that have the word FAIL: 100 requests failed.

### Q2. How many different pages were requested, and which?

```
cut -d' ' -f5 access.log | sort -u | tee /dev/stderr | wc -l
/page0
/page1
/page2
/page3
4
```

<p align="center"><em>Figure 4.2 – Different pages</em></p>


`cut` takes the 5th field (the page), `sort -u` leaves every page once. `tee /dev/stderr` shows the list of pages on the screen and lets the same list go on to `wc -l`, which counts them. There are 4 different pages.

### Q3. How many requests did each page get?

```
cut -d' ' -f5 access.log | sort | uniq -c
    125 /page0
    125 /page1
    125 /page2
    125 /page3
```

<p align="center"><em>Figure 4.3 – Requests per page</em></p>


Check: 4 × 125 = 500, so the counts add up to the number of lines of the log.

### Q4. Which user or users have the most failed requests, and how many?

```
grep FAIL access.log | cut -d' ' -f3 | sort | uniq -c | sort -rn | awk 'NR==1{m=$1} $1==m'
     15 user5
     15 user3
```

<p align="center"><em>Figure 4.4 – Users with the most failed requests</em></p>


Two users have the most failures, `user3` and `user5`, 15 each. I used `awk` to keep every line whose count is equal to the count of the first line, so a tie is shown too. Check: the failures of all users from `report.sh` are 14 + 14 + 14 + 15 + 14 + 15 + 14 = 100, the same as Q1.

### Q5. How many requests did user3 make, and how many of them failed?

```
grep ' user3 ' access.log | cut -d' ' -f4 | sort | uniq -c
     15 FAIL
     57 OK
```

<p align="center"><em>Figure 4.5 – Requests of user3</em></p>


`user3` made 15 + 57 = 72 requests, and 15 of them failed.

### Q6. Print the last 3 failed requests, showing only time and user.

```
grep FAIL access.log | tail -3 | cut -d' ' -f2,3
10:10 user0
10:15 user5
10:20 user3
```

<p align="center"><em>Figure 4.6 – The last 3 failed requests</em></p>


### Q7. Which login shells appear in /etc/passwd, and how many accounts use each?

```
cut -d: -f7 /etc/passwd | sort | uniq -c | sort -rn
     40 /usr/sbin/nologin
      6 /bin/false
      2 /bin/bash
      1 /bin/sync
```

<p align="center"><em>Figure 4.7 – Login shells in /etc/passwd</em></p>


The shell is the 7th field and the separator in `/etc/passwd` is `:`. Most accounts (40) are system accounts with `nologin`, and only 2 accounts use `/bin/bash`.

### Q8. Why do `ls /etc | wc -l` and `ls -l /etc | wc -l` differ by exactly one?

My prediction: `ls -l /etc | wc -l` will be bigger because of the `total` line at the top.

```
ls /etc | wc -l ; ls -l /etc | wc -l
237
238
```

<p align="center"><em>Figure 4.8 – ls and ls -l with wc</em></p>


`ls /etc` prints one name per line when the output goes into a pipe, so `wc -l` counts 237 entries. `ls -l` prints the same 237 lines, one for each entry, and one more line at the top, `total ...`, with the size of the files in blocks. That is why the second number is bigger by exactly one.

I checked it by looking at the first lines of `ls -l /etc`:

```
vboxuser@Ubuntu:~/os-lab3$ ls -l /etc | head -3
total 1156
drwxr-xr-x  5 root                 root                  4096 Aug 26 05:30 ModemManager
drwxr-xr-x  8 root                 root                  4096 Sep 29 16:30 NetworkManager
```

The first line is `total 1156`, and it is the extra line that `wc -l` counts.

### Bonus. Lines where the minute ends in 0 and the request failed

```
grep -E '10:[0-5]0 user[0-9] FAIL' access.log | wc -l
50
```

There are 50 such lines. It is also logical: the minute ends in 0 when the line number is a multiple of 10, and then the number is also a multiple of 5, so the request is always a FAIL. Between 1 and 500 there are 50 multiples of 10.

### Full run

```
vboxuser@Ubuntu:~/os-lab3$ bash oneliners.sh
100
/page0
/page1
/page2
/page3
4
    125 /page0
    125 /page1
    125 /page2
    125 /page3
     15 user5
     15 user3
     15 FAIL
     57 OK
10:10 user0
10:15 user5
10:20 user3
     40 /usr/sbin/nologin
      6 /bin/false
      2 /bin/bash
      1 /bin/sync
237
238
50
```

<p align="center"><em>Figure 4.9 – Full run of oneliners.sh</em></p>



## Part 5. Your first script

I created `report.sh` and completed the TODO with a loop over the users from `user0` to `user6`.

```
vboxuser@Ubuntu:~/os-lab3$ cat > report.sh <<'EOF'
#!/bin/bash
# usage: ./report.sh access.log
log=$1
if [ ! -f "$log" ]; then
echo "no such file: $log" >&2
exit 1
fi
total=$(wc -l < "$log")
fails=$(grep -c FAIL "$log")
echo "$total requests, $fails failed"
for n in 0 1 2 3 4 5 6; do
echo "user$n: $(grep FAIL "$log" | grep -c "user$n ") failures"
done
EOF
```

<p align="center"><em>Figure 5.1 – Contents of report.sh</em></p>


I tried to run it before `chmod +x`, and then after it:

```
vboxuser@Ubuntu:~/os-lab3$ ./report.sh access.log
bash: ./report.sh: Permission denied
vboxuser@Ubuntu:~/os-lab3$ chmod +x report.sh
vboxuser@Ubuntu:~/os-lab3$ ./report.sh access.log
500 requests, 100 failed
user0: 14 failures
user1: 14 failures
user2: 14 failures
user3: 15 failures
user4: 14 failures
user5: 15 failures
user6: 14 failures
```

<p align="center"><em>Figure 5.2 – Running the script on access.log</em></p>


```
vboxuser@Ubuntu:~/os-lab3$ ./report.sh nope.log ; echo "exit code: $?"
no such file: nope.log
exit code: 1
```

<p align="center"><em>Figure 5.3 – Error handling for a missing file</em></p>


### Observe

**1. What does the first line `#!/bin/bash` do, and what happens if I run `./report.sh` before `chmod +x`?**
The first line (it is called shebang) tells the system which program must run the file: here it is `/bin/bash`. Before `chmod +x` the file did not have the permission to execute, so the shell refused to start it and printed `bash: ./report.sh: Permission denied`. After `chmod +x` it worked.

**2. Why must I type `./report.sh` and not just `report.sh`?**
When I type a name without a slash, the shell looks for the program only in the directories of the `PATH`. As I saw in Part 1, the current directory is not in my `PATH`, so `report.sh` would give `command not found`. With `./` I give the path to the file myself (the dot means the current directory), so the shell does not need to search.

**3. Why does the script write the error with `>&2` and end with `exit 1`?**
`>&2` sends the message to stderr (stream 2) and not to stdout, so the error can be separated from the normal output, for example with `2> err.txt`, like in Part 3. `exit 1` ends the script with a non-zero exit code. The exit code 0 means success and everything else means a problem. Other commands and scripts check it with `$?`, `&&` and `||`, so they know that the script failed (in my test `echo "exit code: $?"` printed 1).


## Conclusion

In this work I learned to use the command line faster: Tab completion, the history search with Ctrl+R, and the commands `type`, `which`, `man` and `help` to find where a command comes from and how to use it. I saw that the shell does a lot of work before a program starts: it expands `*`, `?`, `[a-b]`, `{1..5}`, `$HOME` and `$(...)`, and it splits the line at the spaces, which is why I need quotes for `alfa beta.txt`. I redirected stdout and stderr to different files and understood that the order of `> file` and `2>&1` is important, because the shell reads it from left to right. With pipes and the filters `grep`, `cut`, `sort`, `uniq`, `wc`, `head` and `tail` I answered eight questions about the log of 500 lines, and checked the numbers (the counts of the pages add up to 500, and the failures of all users add up to 100). At the end I wrote my first script `report.sh`, and learned about the shebang line, the execute permission, `./`, stderr and exit codes.
