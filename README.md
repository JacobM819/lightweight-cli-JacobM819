# Minishell  

A lightweight Bash-like shell built in **C**.  
Minishell provides a text-based interface to interact with the operating system, execute programs, and manage processes. It supports core shell features including I/O redirection, pipes, environment variables, and conditional operators.  

---

## 🚀 Features  

- **Filesystem navigation**: `cd`, `pwd`  
- **Process execution**: run executables via relative or absolute paths  
- **Environment variables**: define and expand variables at runtime  
- **I/O redirection**: `<`, `>`, `>>`, `2>`, `2>>`, `&>`  
- **Operators**:  
  - `;` — sequential execution  
  - `&` — parallel execution  
  - `|` — piping  
  - `&&`, `||` — conditional execution  
- **Exit commands**: `exit` or `quit`  
- **Error handling**: gracefully handles invalid input and missing paths  

---

## 📖 Examples  

```sh
# Filesystem navigation
> pwd
/home/user
> cd minishell
> pwd
/home/user/minishell

# Run executables
> ./sum 2 4 1
7
> /usr/bin/echo "Hello, world!"
Hello, world!

# Pipes
> echo "world" | ./reverse_input
dlrow

# Operators
> echo "Hello"; echo "World!"
Hello
World!
