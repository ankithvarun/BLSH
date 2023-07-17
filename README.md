# Operating System and Networks
## Assignment 2 - Shell Part 2

<br>

Implementation of a user-defined interactive shell in C that mimics the behaviour of *bash* shell. The following is a brief overview about the contents of each file and implementation of various required features of the shell.

<br>

**shell.h :**

A basic module that contains all environment variables and built-in command prototypes for the independent execution of the shell. It also contains the driver prototype for the *ls* command. All system details and path names have been limited to a maximum of 500 characters.

<br>

**process.h :**

A module that deals with system commands which are executed as foreground or background processes. It contains a data structure named *Proc* which stores the details of a background process such as pid and process name. Prototypes of various process-related functions have also been included.

<br>

**history.h :**

A module that keeps track of previous commands that have been executed over all instances of the shell. A maximum of 20 commands are stored at any time.

<br>

**shell.c :**

This code contains an *init()* function to initialise the various environment variables and data structures defined in the above modules. It also contains the implementations of built-in commands *echo*, *pwd* and *cd*.

<br>

**execute.c :**

This code contains the primary driver function *execute_command()* which is responsible to execute pre-defined and system commands by passing them to their respective command functions. The *repeat* command is implemented implicitly in this function which recursively calls itself to repeat an instruction a certain number of times. It also contains the *tokenize()* function which parses inputs and commands into arguments. The implicit implementations for input-output redirections of commands have been added to *execute_command()* function. The function *implement_pipe()* that handles and, as the name suggests, implements command pipelines has also been added.
Multiple functionalities to handle the user-defined commands *jobs*, *sig*, *fg* and *bg* have been added as required. As bonus, implcit implementation of replay command in *execute_command()* function has been added as well.

<br>

**process.c :**

The code that executes system commands either as a foreground process or as a background process. The *exec_process()* function achieves this purpose using forks. This code contains the implementation of the *pinfo* command that displays the details of any process wiz pid, status, virtual memory and executable path of the process by accessing the */proc/\<pid\>* directory. It also contains a *check_child()* function to handle finished background processes. This is done using the *Proc* data structure which is implemented as a linked list in order to ease the insertion and deletion of background processes when they are created and terminated respectively.

<br>

**history.c :**

This code deals with the addition, storage and retrieval of previous commands entered in the shell prompt. A record of these commands are stored in a file named *history.data* in the home directory of the file system which are constantly retrieved and modified by *add_command()* and *put_commands()* functions using an array of strings named *commands_log*. It also contains the implementation of the *history* command.

<br>

**ls.c :**

Code that implements the *ls* command with(or without) a combination of *-a* and *-l* flags. It also contains a *ls_driver()* function and *check_flag()* function to parse the command. The file mode specifier under permissions field of *-l* functionality is limited to only normal files and directories, however rest of the fields are in accordance with bash behaviour.

<br>

**signalhandler.c :**

This code primarily deals with defining signal handlers for various software interrupt signals such as **SIGINT**, **SIGTSTP** and **SIGCHLD**. It also defines a custom signal handler in order to ignore signals when the main shell process is running. 

<br>

**main.c :**

The primary code that displays the prompt of the shell as well as takes input of commands in order to execute it by calling *execute_command()*. Each input and command have been limited to a maximum of 1000 and 100 characters each respectively. It also calls *init()*, *check_child()* and *add_command()* functions for the maintenance of the shell.

<br>

**NOTE :** All commands, excluding user-defined commands, try to mimic shell behaviour of bash to the maximum extent. However, there may be variations in error handling and other functionalitites.