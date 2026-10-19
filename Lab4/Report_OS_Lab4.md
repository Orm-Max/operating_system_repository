# Lab 4: The xv6 Shell from the Outside

**MINISTRY OF EDUCATION, CULTURE AND RESEARCH**  
**OF THE REPUBLIC OF MOLDOVA**  

**Technical University of Moldova**  
**Faculty of Computers, Informatics and Microelectronics**  
**Department of Software Engineering and Automation**  


**MAXIM ORMANJI FAF-242**

Checked by: Reițman Petricia

Chișinău, 2026


## Purpose of the work
 
The purpose of this work is to see what a shell really has to do and what a bigger shell like bash adds on top. I use the xv6 shell and bash side by side and compare them. I also write a small program that prints its arguments, to see who expands `*` and who removes the quotes. Before that I make sure that my `sleep` program builds on the current xv6.
 
 
## Part 1. Fix sleep for the current xv6
 
In August 2025 the xv6 authors renamed the system call `sleep()` to `pause()`, so my `sleep` program from Lab 2 may not build any more. First I checked which name my xv6 tree uses.
 
```
vboxuser@Ubuntu:~/xv6-riscv$ grep -n "pause\|sleep" user/user.h
25:int pause(int);
```
 
<p align="center"><em>Figure 1.1 – The grep result in user.h</em></p>
 
`user.h` declares `pause`, so I had to change the call in `user/sleep.c`. This is the updated program:
 
```c
#include "kernel/types.h"
#include "user/user.h"
 
int
main(int argc, char *argv[])
{
  if(argc != 2){
    fprintf(2, "usage: sleep <ticks>\n");
    exit(1);
  }
  pause(atoi(argv[1])); // was sleep() before Aug 2025
  exit(0);
}
```
 
<p align="center"><em>Figure 1.2 – The updated sleep.c</em></p>
 
Then I checked that `_sleep` is in `UPROGS` in the Makefile:
 
```
vboxuser@Ubuntu:~/xv6-riscv$ grep -n "_sleep" Makefile
135:	$U/_sleep\
```
 
After that I built and started xv6 with `make qemu`. The build finished without errors (`user/_sleep` was linked and added to `fs.img`) and xv6 booted. Then I tested the program:
 
```
xv6 kernel is booting
 
hart 2 starting
hart 1 starting
init: starting sh
$ sleep 20
$ sleep
usage: sleep <ticks>
$
```
 
<p align="center"><em>Figure 1.3 – sleep running in xv6, with and without an argument</em></p>
`sleep 20` waits 20 ticks and prints nothing, and `sleep` without an argument prints the usage message.
 
 
 
## Part 2. bash versus the xv6 shell
 
I ran every command twice: in bash on Linux (in an empty test folder `~/shtest` that has only a file `README`) and at the xv6 `$` prompt.
 
### Commands in bash
 
```
vboxuser@Ubuntu:~$ rm -rf ~/shtest
vboxuser@Ubuntu:~$ mkdir ~/shtest
vboxuser@Ubuntu:~$ cd ~/shtest
vboxuser@Ubuntu:~/shtest$ touch README
vboxuser@Ubuntu:~/shtest$ echo hello > f
vboxuser@Ubuntu:~/shtest$ cat < f
hello
vboxuser@Ubuntu:~/shtest$ ls | grep READ
README
vboxuser@Ubuntu:~/shtest$ (echo a; echo b) | wc
      2       2       4
vboxuser@Ubuntu:~/shtest$ echo one
one
vboxuser@Ubuntu:~/shtest$ echo two
two
vboxuser@Ubuntu:~/shtest$ echo bg & wait
bg
vboxuser@Ubuntu:~/shtest$ ls *
README	f
vboxuser@Ubuntu:~/shtest$ echo $HOME
/home/vboxuser
vboxuser@Ubuntu:~/shtest$ echo "a b"
a b
vboxuser@Ubuntu:~/shtest$ mkdir d
vboxuser@Ubuntu:~/shtest$ cd d
vboxuser@Ubuntu:~/shtest/d$ ls ..
README	d  f
```
 
<p align="center"><em>Figure 2.1 – The commands run in bash</em></p>
Here every command is typed on its own line. I first ran all of them in one line with `bash -x`, and then bash also printed every command after the expansion (the lines with `+`). For example `ls *` became `+ ls f README` and `echo $HOME` became `+ echo /home/vboxuser`, so I can see what the shell really gave to the program.
 
 
### Commands in xv6
 
```
$ echo hello > f
$ cat < f
hello
$ ls | grep READ
README         2 2 2441
$ (echo a; echo b) | wc
2 2 4 
$ echo one
one
$ echo two
two
$ echo bg &
bg
$ ls *
ls: cannot open *
$ echo $HOME
$HOME
$ echo "a b"
"a b"
$ mkdir d
$ cd d
$ ls ..
exec ls failed
```
 
<p align="center"><em>Figure 2.2 – The same commands run in xv6</em></p>
 
### Comparison table
 
| Command | Tests | bash | xv6 | Why |
|---|---|---|---|---|
| `echo hello > f ; cat < f` | redirection | `hello` | `hello` | `<` and `>` are done by the shell itself: it opens the file and puts it in place of stdin/stdout before running the program. |
| `ls \| grep READ` | pipe | `README` | `README         2 2 2441` | Both shells have pipes. In xv6 `ls` prints more (type, inode number and size), so the line looks different. |
| `(echo a; echo b) \| wc` | grouping, list | `2 2 4` | `2 2 4` | Both shells understand `( )` and `;`. |
| `echo one ; echo two` | sequence | `one` and `two` | `one` and `two` | `;` runs the commands one after another in both shells. |
| `echo bg &` | background | `bg` | `bg` | `&` starts the command and the shell does not wait for it. |
| `ls *` | globbing | `README f` | `ls: cannot open *` | bash replaces `*` with the file names. The xv6 shell does not, so `ls` gets a real `*` and there is no file with this name. |
| `echo $HOME` | variables | `/home/vboxuser` | `$HOME` | xv6 has no environment variables, so the text stays as it is. |
| `echo "a b"` | quotes | `a b` | `"a b"` | bash removes the quotes. xv6 keeps them in the text. |
| `mkdir d ; cd d ; ls ..` | PATH | `README d f` | `exec ls failed` | xv6 has no PATH. It looks for the file `ls` in the current folder `d`, and it is not there. |
| arrow up, Tab, Ctrl+R | history, completion | all three work: arrow up brought back `ls README`, Tab completed `ls RE` to `ls README`, Ctrl+R found the old command (`reverse-i-search`) | none of them work | bash uses a line editor (readline) that remembers the commands and completes the names. The xv6 shell only reads a line of text from the keyboard, so these keys do nothing useful. |
 
### Observe
 
**1. Features that both shells have and features that only bash has.**
Both shells can run programs, redirect input and output with `<` and `>`, make pipes with `|`, run commands in a sequence with `;`, group commands with `( )` and run a command in the background with `&`. Only bash has globbing (`*`), variables (`$HOME`), quotes, `PATH`, command history (arrow up), Tab completion and Ctrl+R search. I checked the last three by hand: they work in bash and do nothing in xv6. Bash also has other things that I did not test here, for example `&&`, `||` and scripts.
 
**2. Why `ls` stops working after `cd d`, and how to run it anyway.**
xv6 has no `PATH`. When I type `ls`, the shell calls `exec("ls", ...)`, and `exec` looks for a file called `ls` in the current folder. All xv6 programs live in the root folder `/`. After `cd d` I am in `d`, where there is no `ls` file, so I get `exec ls failed`. To run it anyway I have to give the path to the program, for example `/ls ..`:
 
```
$ cd d
$ ls ..
exec ls failed
$ /ls ..
.              1 1 1024
..             1 1 1024
README         2 2 2441
cat            2 3 36728
echo           2 4 35592
sleep          2 5 35280
forktest       2 6 18136
grep           2 7 44184
init           2 8 35976
kill           2 9 35512
ln             2 10 35304
ls             2 11 42880
mkdir          2 12 35568
rm             2 13 35552
sh             2 14 58312
stressfs       2 15 36416
usertests      2 16 206800
grind          2 17 51824
wc             2 18 37728
zombie         2 19 34848
logstress      2 20 37560
forphan        2 21 36352
dorphan        2 22 35800
sync           2 23 34944
console        3 24 0
f              2 25 6
d              1 26 32
$
```
 
<p align="center"><em>Figure 2.3 – ls failing after cd d, and the working alternative</em></p>
It works, and the list shows that all the programs (`cat`, `echo`, `ls`, `sleep`, `sh` ...) are in `/`, and the folder `d` and the file `f` are there too. `../ls ..` should also work, because `..` from `d` is `/`.
 
 
**3. Which missing feature I would miss most, and which is the easiest to add.**
I would miss the command history (arrow up) the most, because without it I have to type every command again. The easiest to add is the search in `PATH`, I think: when `exec` does not find the command, the shell could try again with `/` in front of the name. It needs only a few lines in the shell. Globbing is harder, because the shell would have to read the folder and compare the names with the pattern, and variables are harder still, because they need a whole place to store them.
 
 
## Part 3. Who expands `*`? A program that prints its argv
 
### The programs
 
`user/args.c` for xv6, with its line in `UPROGS` in the Makefile:
 
```c
#include "kernel/types.h"
#include "user/user.h"
 
int
main(int argc, char *argv[])
{
  printf("argc = %d\n", argc);
  for(int i = 0; i < argc; i++)
    printf("argv[%d] = [%s]\n", i, argv[i]);
  exit(0);
}
```
 
```
vboxuser@Ubuntu:~/xv6-riscv$ grep -n "_args" Makefile
133:	$U/_args\
```
 
<p align="center"><em>Figure 3.1 – args.c and the UPROGS line</em></p>
 
The same program for Linux, `args_linux.c` (it uses `<stdio.h>` and `return 0`):
 
```c
#include <stdio.h>
 
int
main(int argc, char *argv[])
{
  printf("argc = %d\n", argc);
  for(int i = 0; i < argc; i++)
    printf("argv[%d] = [%s]\n", i, argv[i]);
  return 0;
}
```
 
I compiled it in a folder `~/shtest3` that has the files `a.c`, `b.c` and `c.c`:
 
```
vboxuser@Ubuntu:~$ rm -rf ~/shtest3
vboxuser@Ubuntu:~$ mkdir ~/shtest3
vboxuser@Ubuntu:~$ cd ~/shtest3
vboxuser@Ubuntu:~/shtest3$ touch a.c b.c c.c
vboxuser@Ubuntu:~/shtest3$ gcc -o args ~/args_linux.c
```
 
### Output in xv6
 
```
$ args one   two three
argc = 4
argv[0] = [args]
argv[1] = [one]
argv[2] = [two]
argv[3] = [three]
$ args *.c "x y"
argc = 4
argv[0] = [args]
argv[1] = [*.c]
argv[2] = ["x]
argv[3] = [y"]
$ args $HOME
argc = 2
argv[0] = [args]
argv[1] = [$HOME]
$ args 'single quoted'
argc = 3
argv[0] = [args]
argv[1] = ['single]
argv[2] = [quoted']
```
 
<p align="center"><em>Figure 3.2 – args output in xv6</em></p>
 
### Output in bash
 
```
vboxuser@Ubuntu:~/shtest3$ ./args one   two three
argc = 4
argv[0] = [./args]
argv[1] = [one]
argv[2] = [two]
argv[3] = [three]
vboxuser@Ubuntu:~/shtest3$ ./args *.c "x y"
argc = 5
argv[0] = [./args]
argv[1] = [a.c]
argv[2] = [b.c]
argv[3] = [c.c]
argv[4] = [x y]
vboxuser@Ubuntu:~/shtest3$ ./args $HOME
argc = 2
argv[0] = [./args]
argv[1] = [/home/vboxuser]
vboxuser@Ubuntu:~/shtest3$ ./args 'single quoted'
argc = 2
argv[0] = [./args]
argv[1] = [single quoted]
```
 
<p align="center"><em>Figure 3.3 – args_linux output in bash</em></p>
 
Side by side, the line `args *.c "x y"` gave this:
 
| | bash | xv6 |
|---|---|---|
| argc | 5 | 4 |
| arguments | `./args`, `a.c`, `b.c`, `c.c`, `x y` | `args`, `*.c`, `"x`, `y"` |
 
### Observe
 
**1. How many arguments did each program receive for `args *.c "x y"`?**
The program in bash received 5 arguments (`argc = 5`): its own name, `a.c`, `b.c`, `c.c` and `x y`. The program in xv6 received 4 (`argc = 4`): its own name, `*.c`, `"x` and `y"`. The first argument `argv[0]` is always the name of the program, so it is also counted.
 
**2. Who turns `*.c` into file names and removes the quotes: the program, the kernel or the shell?**
It is the shell. The program gets the ready array `argv` and only prints it, and the kernel only passes this array to the program when `exec` is called. The proof is the difference between the two outputs: it is the same program idea, but in xv6 the characters `*.c`, `"x` and `$HOME` arrive without any change, because the xv6 shell cannot expand them. In bash it is the same: when I first ran the four lines with `bash -x`, bash printed `+ ./args a.c b.c c.c 'x y'`, and this shows that the expansion was already done before `args` was started.
 
**3. What happened to the extra spaces in `one   two three`?**
They disappeared in both shells. The shell uses spaces only to separate the words, and several spaces in a row work like one. So `argv` has three clean words (`one`, `two`, `three`) and `argc = 4`, because the program name is counted too. To keep spaces inside one argument I need quotes, but in xv6 the quotes do not work.
 
 
## Part 4. Read the shell
 
I ran these commands inside `~/xv6-riscv` and read the places in `user/sh.c` that they show.
 
```
vboxuser@Ubuntu:~/xv6-riscv$ wc -l user/sh.c
499 user/sh.c
vboxuser@Ubuntu:~/xv6-riscv$ grep -n "#define" user/sh.c
8:#define EXEC  1
9:#define REDIR 2
10:#define PIPE  3
11:#define LIST  4
12:#define BACK  5
14:#define MAXARGS 10
```
 
<p align="center"><em>Figure 4.1 – The command kinds defined in sh.c</em></p>
The file has 499 lines, which fits the "about 500 lines" from the task.
 
 
```
vboxuser@Ubuntu:~/xv6-riscv$ grep -n "case " user/sh.c
75:  case EXEC:
83:  case REDIR:
93:  case LIST:
101:  case PIPE:
125:  case BACK:
284:  case 0:
286:  case '|':
287:  case '(':
288:  case ')':
289:  case ';':
290:  case '&':
291:  case '<':
294:  case '>':
391:    case '<':
394:    case '>':
397:    case '+': // >>
469:  case EXEC:
475:  case REDIR:
481:  case PIPE:
487:  case LIST:
493:  case BACK:
```
 
<p align="center"><em>Figure 4.2 – Where each kind is run</em></p>
The lines 75 to 125 are in the function `runcmd`, so this is where each kind of command is run. The lines 284 to 294 are the characters that the parser treats as special, and the lines 469 to 493 are another `switch` over the same five kinds.
 
 
```
vboxuser@Ubuntu:~/xv6-riscv$ grep -n "cd" user/sh.c | head
170:        fprintf(2, "cannot cd %s\n", cmd + 3);
```
 
The search found only the error message, because the check for `cd` is written with single characters (`cmd[0] == 'c' && cmd[1] == 'd'`), so the word `cd` is not in one piece there. That is why I printed the lines around it:
 
```
vboxuser@Ubuntu:~/xv6-riscv$ sed -n '160,180p' user/sh.c
  while (getcmd(buf, sizeof(buf)) >= 0) {
    char *cmd = buf;
    while (*cmd == ' ' || *cmd == '\t')
      cmd++;
    if (*cmd == '\n') // is a blank command
      continue;
    if (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == ' ') {
      // Chdir must be called by the parent, not the child.
      cmd[strlen(cmd) - 1] = 0; // chop \n
      if (chdir(cmd + 3) < 0)
        fprintf(2, "cannot cd %s\n", cmd + 3);
    } else {
      if (fork1() == 0)
        runcmd(parsecmd(cmd));
      wait(0);
    }
  }
  exit(0);
}
 
void
```
 
<p align="center"><em>Figure 4.3 – The code that handles cd</em></p>
Here the shell checks if the line starts with `cd ` and then calls `chdir` itself. Only if it is not `cd`, it calls `fork1()` and runs the command in the child with `runcmd`.
 
 
I also searched `sh.c` for `*` and `$` with `grep -n "'\*'\|'\$'\|\"\*\"" user/sh.c`, and it printed nothing.
 
### Observe
 
**1. The five kinds of command and the character that produces each.**
 
| Kind | What it is | Character |
|---|---|---|
| `EXEC` | a simple command with arguments | just words separated by spaces |
| `REDIR` | redirection of input or output | `<`, `>` (and `>>`) |
| `PIPE` | pipe between two commands | `\|` |
| `LIST` | commands one after another | `;` |
| `BACK` | command in the background | `&` |
 
The brackets `( )` are not a sixth kind. They only group commands when the parser reads the line.
 
**2. Where is `cd` handled, and why is it not a separate program?**
`cd` is handled in the shell itself, in the main loop of `main` that reads the lines (`while (getcmd(...))`, line 160). The check `cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == ' '` is on line 166, and `chdir(cmd + 3)` is called right there, in the shell process. Only the `else` branch calls `fork1()`. The comment in the code says it directly: "Chdir must be called by the parent, not the child". The reason is that the current folder belongs to a process. If `cd` was a separate program, the shell would start it as a child process, the child would change its own folder and exit, and the shell would stay in the old folder. So the shell has to call `chdir` itself.
 
**3. Is there code for `*` or `$` in `sh.c`? What does it explain about Part 2?**
No, the search found nothing. The special characters of the parser are only `|`, `(`, `)`, `;`, `&`, `<` and `>` (lines 286–294), and `+` for `>>` (line 397). This explains Part 2 and Part 3: in xv6 `ls *` looks for a file that is really named `*`, and `$HOME` stays the text `$HOME`, because the shell has no code that can expand them.
 
 
## Conclusion
 
In this work I used the xv6 shell and bash side by side. First I fixed my `sleep` program for the new name of the system call (`pause` instead of `sleep`) and checked it in xv6. Then I compared the two shells and saw that the xv6 shell can do the basic things (programs, redirection, pipes, `;`, `( )` and `&`), but it has no globbing, variables, quotes, `PATH` and history. With my `args` program I found out that the expansion of `*` and the removing of quotes are done by the shell, not by the program or the kernel: the kernel only gives `argv` to the program. At the end I read `user/sh.c` (499 lines) and saw the five kinds of commands (`EXEC`, `REDIR`, `PIPE`, `LIST`, `BACK`) and that `cd` must be done by the shell itself, because it changes the folder of the shell process. So a shell is quite a small program, and a lot of what we are used to in bash is an extra on top of it.
 
