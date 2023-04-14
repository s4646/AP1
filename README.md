324205160

## Usage examples:
```ruby
ubuntu@ubuntu-ASUS-TUF:~/Documents/AP1$ make
gcc -Wall -Werror -g -c myshell.c
gcc -Wall -Werror -g -c memory.c
gcc myshell.o memory.o -o myshell
ubuntu@ubuntu-ASUS-TUF:~/Documents/AP1$ ./myshell 
hello: date > myfile
hello: cat myfile
Fri 14 Apr 2023 10:29:10 PM IDT
hello: date -u >> myfile
hello: cat myfile
Fri 14 Apr 2023 10:29:10 PM IDT
Fri 14 Apr 2023 07:29:20 PM UTC
hello: wc -l < myfile
2
hello: prompt = hi
hi: mkdir mydir
hi: cd mydir
hi: pwd
/home/ubuntu/Documents/AP1/mydir
hi: touch file1 file2 file3
hi: ls
file1  file2  file3
hi: !!
file1  file2  file3
hi: echo abc xyz
abc xyz 
hi: ls
file1  file2  file3
hi: echo $?
0
hi: ls no_such_file
ls: cannot access 'no_such_file': No such file or directory
hi: echo $?
512
hi: ls no_such_file 2> file
hi: ^C
You typed Control-C!
hi: cat > colors.txt   
blue
black
red
red
green
blue
green
red
red
blue
hi: cat colors.txt
blue
black
red
red
green
blue
green
red
red
blue
hi: cat colors.txt | cat | cat | cat
blue
black
red
red
green
blue
green
red
red
blue
hi: sort colors.txt | uniq -c | sort -r | head -3
      4 red
      3 blue
      2 green
hi: quit
ubuntu@ubuntu-ASUS-TUF:~/Documents/AP1$
```
## Important notes

### Section 2 (Redirection)
It is unclear if multiple redirections in a single command should be supported. Therefore, only one redirection per command is allowed.

### Section 10 (Variables)
It is unclear whether there is a need to recall the variable's value when executing something else than `echo $variable`. Therefore, a valid use of variables is:
```
hello: $person = David          
hello: echo $person
David
```
While executing something else will NOT provide the value of the variable:
```
hello: echo hello $person
hello $person 
```

### Section 11 (read command)
It is unclear whether you need to use "echo Enter a string" first. Therefore, in order to use the read command, you must enter "echo Enter a string" first.
```
hello: echo Enter a string
read name
hello
hello: echo $name
hello
hello: 
```

### Section 12 (Arrow Navigation)
It is unclear how to implement the arrows up and down. Therefore, in order to use the arrows, you need to press the desired arrow and then Enter:
```
hello: ls
Makefile  memory.c  memory.h  memory.o	myshell  myshell.c  myshell.h  myshell.o  README.md  task1.pdf
hello: echo hey
hey 
hello: ^[[A     # press enter afterwards
```
In addition, once a command is chosen, you cannot delete the command from stdin unless you use the arrows again or execute clear after executing the command.