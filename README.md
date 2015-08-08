# win32-console-hook-lib

##### Background #####

We often interact with console applications with standard input, standard output and standard error, though it has a lot of drawbacks.

For example, when the output has not been well organized, we have to do a lot of coding to parse the output. When the application accesses the console directly with Windows APIs (e.g., WriteConsole, SetConsoleCursorPosition, etc.), it becomes impossible to get the desired output with standard output. For another example, see the code below:

```c++
#include <stdio.h>
#include <windows.h>

int main(int argc, char *argv[])
{
    printf("Hello world!\n");
    Sleep(10000);
    return 0;
}
```

This is a console application that generates some output but does not call fflush() or exit immediately. When launched by another GUI program, it means that the directed output would return nothing in 10 seconds! In a lot of occasions, it may be impossible or too expensive to modify the target console application. What's the solution then?

Win32 Console Hook Library is just here for you. With the library, it is easy to:

 * Process every function calls that generate console outputs without having to parsing the stream
 * Deal with those applications that use native Windows console APIs
 * Fix the "pipe delay" problem

##### About the Library #####

To achieve the goals mentioned above, Win32 Console Hook Library uses the "API Hook" techniques. The first release is based on the Detours Express 1.5 from Microsoft Research, which has been included in the source code. So you don't have to download it elsewhere.

Win32 Console Hook Library (the dynamic link library itself) is written in C. But it's also easy to use in other development environments.

The first release hooks only one function

 * WriteConsole

But it works for a list of functions that generate outputs:

 * printf (C)
 * vprintf (C)
 * putchar (C)
 * puts (C)
 * fputc (C)
 * fputs (C)
 * std::cout (C++)
 * Console.Write (C#)
 * Console.WriteLine (C#)
 * ... (many many more)

Win32 Console Hook Library is released under the terms of **GNU Lesser General Public License version 3 (LGPLv3)**. You're welcome to develop your own application with the library.

##### Install #####

Git clone the latest code, and compile with Visual Studio. The project is based on Visual C++ 2008, but it's also available for later versions after a conversion.

##### Demo Code #####

A demo project called AutoPlink is also included in the source. It works with the plink.exe (from putty) and adds the "auto reconnect" feature to the original program. Unfortunately, because of the "pipe delay" problem, this goal is not likely to be achieved by reading redirected output.

The source code of AutoPlink includes every thing you need to know about the hook library. It's fairly simple and easy to use.
