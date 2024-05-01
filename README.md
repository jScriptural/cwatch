# **NAME**
_cwatch_ - files change monitoring tool.

# **SYNOPSIS** 
```cwatch -f <file1> <file2> ... -d <dir1> <dir2> ... [-e <ext1> <ext2> ...] -c <cmd>```


# **DESCRIPTION**
_cwatch_  is a simple command-line tool written in C to monitor changes in files and directories.
_cwatch_  can monitor up to 50 files. Files beyond this limit are ignored. 

It accepts four options:
* -f:  Specify files to watch for changes. You can provide multiple files as arguments. 
If this option is provided, the first file specified serves as the entry point of the project.Both option f and option d cannot be absent.
* -d: Specify directories to watch for changes. You can provide up to 5 directories as arguments.
If option f is not included, the entry point is taken randomly from the first directory read.
* -e: Specify file extensions to watch for within directories specified with -d. You can provide up to 5 file extensions.
* -c: Compulsory option to specify the command to run on file changes. The argument to -c is executed with the entry point file. For example, if the argument to -c is /bin/node and the entry point is app.js, then the command /bin/node app.js is executed on every file change.

### **AUTHOR**
Written by  __Isonguyo John__ <isonguyojohndeveloper@gmail.com>

###  **REPORTING BUGS**
Open an issue at [github.com/cwatch/issues](https://github.com/jscriptural/cwatch/issues)

