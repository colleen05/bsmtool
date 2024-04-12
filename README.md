# bsmtool - A command-line tool for creating and modifying BSM files.
**bsmtool** provides functionality for manipulating BSM files (see: [bsmlib on GitHub](https://github.com/colleen05/bsmlib)) with easy-to-use syntax.

## :warning: :rotating_light: DISCONTINUED C++ PROJECT :rotating_light: :warning:
I'm no longer using C++ for personal projects, thus this project's development has been discontinued.
Check my GitHub or [**my website**](https://colleen05.me/) for projects coming in the future.

## Syntax and Using bsmtool
Executing `bsm --help` will yield help on syntax and general use of bsmtool:
```
$ bsm --help
bsmtool v1.0.0 by Colleen (colleen05 on GitHub).

Usage: bsm file (list | dump | get [keys] | remove [keys] | set {options})
	- When using 'list', bsmtool will list all keys and their values.
	- When using 'dump', bsmtool will dump 'raw' keys to appropriately named files.
	- When using 'get', bsmtool will list specified keys.
	- When using 'remove', bsmtool will remove (delete) specified keys.

Options:
	-i <name> <value>    Set integer value.
	-f <name> <value>    Set float value.
	-s <name> <value>    Set string value.
	-r <name> <file>     Set raw value using bytes from given file.

Universal options (other arguments will be ignored):
	--help or -h       Display help.
	--version or -v    Display version info.
```
