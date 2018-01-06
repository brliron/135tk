== 135tk ==

This repo is a set of tools used to read and edit the files used by the Touhou games using the th135 engine: Hopeless Masquerade, Urban Legend in Limbo and Antinomy of Common Flowers.
Some of them are directly copy-pasted from Riatre's 135tk, some of them are upgraded versions, some of them are rewritten, and some of them are totally new. Most of them are written in c/cpp and compile on MinGW-w64 with either gcc *.c, g++ *.cpp or make. Some of them will require aditional libraries (I can think of zlib, libpng and jansson). Some of them will also compile on Linux with the same commands.

== th145arc ==
A tool to extract and repack the pat files from Touhou 14.5. You will need the lib miracl to compile it: https://github.com/miracl/MIRACL (I will set up a git submodule someday to make compilation easier). Compile it with ./cc.sh (why did I use --enable-stdcall-fixup for something compiled with g++ ??)
To extract files, run ./th145arc /x th145.pak. To repack them, run ./th145arc /p th145.pak.
The /x and /p switches may be interpreted as paths on MinGW/cygwin, but the unix-style -x and -p also work.
Archives created with /p will only be usable by the English patch.

== th155arc ==
Like th145arc, but for th155. I even forgot to change "th145" to "th155" in some places, just ignore them.
The /p option works like the th145 one, but there is currently no English patch able to read the files created with it.

== read_pat ==
read_pat can be used to inspect and edit pat files. Compile it with gcc *.c -ljansson. run it with ./read_pat pat_file.pat [json_file.json].
It outputs the PAT datas in 2 different formats:
- A plaintext format on the standard output
- A JSON format in json_file.json if provided.
json_file.json is used both as input and output. When read_pat parses the pat file, it will read the current JSON object in the JSON file. If this JSON object exists, it will be written to the pat file. Otherwise, it will be read from the pat file and written to the JSON file.
Most of the time, if you want to edit a pat file, you will run it in 2 steps:
- First, you run it with a path to a nonexistent JSON file. read_pat will create this file and fill it with the content of the pat file.
- Then, you run it again with the same parameters. read_pat will take the values from the JSON file and write them all to the pat file.
You can also run it with a partial JSON file. For example, if the JSON file you give it contains only { "part1": { "version": 10 } }, read_pat will replace the version number in the pat file, and it will fill the JSON file with all the other fields from the pat file.