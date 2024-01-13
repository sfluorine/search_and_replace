# Search and Replace

This will match strings and replace them with your desired string.

## How it works...

Actually, when I'm in the toilet I'm thinking about how text editors
handles searching and replacing strings. 

The idea is pretty simple actually, it match every strings and puts the pointers into the matched table.
Then it traverse the file content character by character (pointer),
if you come across a pointer that match with the matched table, it will append the replaced string instead.
If not, then it wil just append the unchanged character.

## How to use it...

Your machine will need a C compiler (clang, gcc, etc...) and a build system called ```tup```.

Assuming you have installed it, just ```cd``` into the root of project directory and type ```tup```
it will build the project.

Command line arguments:

```bash
usage: snr <filepath> <search_string> <replace_string>
```

Example:

```bash
./snr Tupfile clang gcc
```
