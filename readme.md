# 135tk

This repo is a set of tools used to read and edit the files used by the Touhou games using the th135 engine: Hopeless Masquerade, Urban Legend in Limbo and Antinomy of Common Flowers.
Some of them are directly copy-pasted from Riatre's 135tk, some of them are upgraded versions, some of them are rewritten, and some of them are totally new.

Most of them are written in c/cpp and compile on MinGW-w64 with either `gcc *.c`, `g++ *.cpp` or `make`. Some of them will require aditional libraries. Anyway, they all have a tiny build.sh script file, just look at it.
Also, some of these tools will also compile on Linux with the same commands.  
*These tools are tested under MinGW-w64 MINGW32 and MinGW-w64 MINGW64, some of these tools does __not__ compile under the MinGW-w64 MSYS environment.*

When you clone the repository, eiher use the `--recursive` command line switch or run `git submodule init && git submodule update`. To build 


Global note about the tools: you would use `./tool_name` only if you run them from an unix-like shell (like MinGW-w64). If you run them from a Windows command prompt, replace every occurence of `./tool_name` in this readme with just `tool_name`.

## th145arc
A tool to extract and repack the pak files from Touhou 14.5 and Touhou 15.5.  
To extract files, run `./th145arc /x th145.pak`. To repack them, run `./th145arc /p th145.pak`.  
*The /x and /p switches may be interpreted as paths on MinGW/cygwin, but the unix-style -x and -p also work.*

Archives created with /p will only be usable by the Touhou 14.5 English patch, the original game won't be able to open them. And there is currently no way to use the archives created by this tool in Touhou 15.5.

## read_pat
`read_pat` can be used to inspect and edit pat files. Run it with `./read_pat pat_file.pat [json_file.json]`.  
It outputs the PAT datas in 2 different formats:
- A plaintext format on the standard output
- A JSON format in `json_file.json` if provided.

`json_file.json` is used both as input and output. When `read_pat` parses the pat file, it will read the current JSON object in the JSON file. If this JSON object exists, it will be written to the pat file. Otherwise, it will be read from the pat file and written to the JSON file.  
Most of the time, if you want to edit a pat file, you will run it in 2 steps:
- First, you run it with a path to a nonexistent JSON file. `read_pat` will create this file and fill it with the content of the pat file.
- Then, you run it again with the same parameters. `read_pat` will take the values from the JSON file and write them all to the pat file.
You can also run it with a partial JSON file. For example, if the JSON file you give it contains only `{ "part1": { "version": 10 } }`, `read_pat` will replace the version number in the pat file, and it will fill the JSON file with all the other fields from the pat file.  
*This behavior seems confusing to most users and may change in future releases (probably by adding a -x or -p switch to lock the program into reading/writing).*

## TFBMTool-alt
A C rewrite of Riatre's TFBMTool, with support for 8-bits images with palette. But this one doesn't support repacking yet.  
Since it doesn't support repacking, it doesn't need a read/write switch for now. Also, it always overwrites the TFBM file in place.  
Usage: `./TFBMTool tfbm_file.[bmp|png] [palette_XXX.bmp]`  
The palette is optional for 24-bpp and 32-bpp TFBM files. It is mandatory for 8-bpp TFBM files. And actually, due to a bug in the current version, if you try to convert a 8-bpp TFBM file without providing a valid palette, the TFBM file's content will be erased.
