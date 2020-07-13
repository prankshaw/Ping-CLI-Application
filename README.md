# Ping-CLI-Application  <a href="../../issues"><img alt="Contributions Welcome" src="https://img.shields.io/badge/contributions-welcome-blue??style=flat"></a>
### Replicating the ping command using C language  
[![License: MIT](https://img.shields.io/badge/License-MIT-orange.svg)](https://opensource.org/licenses/MIT)
<a href="../../issues"><img alt="Issues Open" src="https://img.shields.io/github/issues//prankshaw/Ping-CLI-Application?color=pink"></a>
<a href="../../issues"><img alt="Forks" src="https://img.shields.io/github/forks//prankshaw/Ping-CLI-Application?color=purple"></a>
<a href="../../issues"><img alt="Stars" src="https://img.shields.io/github/stars//prankshaw/Ping-CLI-Application?color=yellow"></a>
[![Twitter URL](https://img.shields.io/twitter/url/https/twitter.com/fold_left.svg?style=social&label=Follow%20%40mepranjal31)](https://twitter.com/mepranjal31)


## Features

The Ping CLI Application covers various features like:- 
<ul>
<li> Accepting as a positional terminal argument a hostname or IP address. </li>
<li> Showcases equivalent IP Address for the hostname argument
<li> Showcases the resolved reverse lookup domain value, i.e., converts dot notation IP address to hostname.
<li> Emitting requests with a periodic delay as long as the program is running.
<li> Report of packets sent lost and received as well as RTT times and total time.
<li> Features like TTL as argument and time exceeded alert if the TTL value of a packet is greater than the specified value and prints information baout that packet (At least information about one packet is always printed). Many more features are present.
</ul>

## Folder Contents

<li><b>Ping_Cli_Application.c</b> - This file contains the source code of the Ping Application developed using C language.
<li><b>makefile</b> - This file is used to compile the code and produce an executable file.
<li><b>.gitignore</b> - To ignore the executable file generated during compilation.

## How to run this program
The program can be run either using <b>makefile</b> or by compiling <b>Ping_Cli_Application.c</b> 

Install essential packages using Linux terminal or WSL on windows.

```
$ sudo apt-get install build-essential
```

<li><b>By Compiling</b>- Compile the program by typing the following commands into the terminal:-

``` Bash
$ gcc Ping_Application_CLI.c -o pingcli 
$ sudo ./pingcli <argument1> <argument2> 
```

<li><b>Using make file</b>- cd into the directory containing the makefile. Now type following commands into the terminal:-

```
$ make
$ sudo ./pingcli <argument1> <argument2>
```

## Arguments 
It accepts 2 diiferent argumants as follows:-

<li><b> Hostname/IP Address</b>- The first argument consists of a Hostname (google.com) or an IP Address (216.58.203.46) and is compulsory to be provided.
<li><b> TTL value</b>- The second argument is optional in nature and is an integer value in the range -32,768 to 32,767 (Supported by 2-bit ‘int’ datatype in C). Any value which is not an integer in nature will result in “Not an Integer” alert.

If number of arguments is less than 2 or more than 3 it will result in an alert.

