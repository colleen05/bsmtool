# bsmtool - A command-line tool for creating and modifying BSM files.
**bsmtool** provides functionality for manipulating BSM files (see: [bsmlib on GitHub](https://github.com/colleen05/bsmlib)) with easy-to-use syntax.

## Syntax
Executing `bsm --help` will yield help on syntax:
```
$ bsm --help
bsmtool v1.0.0 by Colleen

Usage: bsm file (get [keys] | remove [keys] | set [options] | --help)
    - When using 'get', bsmtool will list specified keys.
    - When using 'remove', bsmtool will remove (delete) specified keys.
    - When using '--help', bsmtool will display this help message (and all other arguments will be ignored).

Options:
    -i <name> <value>   Set integer value.
    -f <name> <value>   Set float value.
    -s <name> <value>   Set string value.
    -r <name> <file>    Set raw value using bytes from given filename.
```