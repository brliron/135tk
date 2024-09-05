# 135tk

This repo is a set of tools used to read and edit the files used by the Touhou games using the th135 engine: Hopeless Masquerade, Urban Legend in Limbo and Antinomy of Common Flowers.
Some of them are directly copy-pasted from Riatre's 135tk, some of them are upgraded versions, some of them are rewritten, and some of them are totally new.

Most of them are written in c/cpp and compile on MinGW-w64 with either `gcc *.c`, `g++ *.cpp` or `make`. Some of them will require aditional libraries. Anyway, they all have a tiny build.sh script file, just look at it.
Also, some of these tools will also compile on Linux with the same commands.  
*These tools are tested under MinGW-w64 MINGW32 and MinGW-w64 MINGW64, some of these tools does __not__ compile under the MinGW-w64 MSYS environment.*

When you clone the repository, eiher use the `--recursive` command line switch or run `git submodule init && git submodule update`. To build 


Global notes about the tools:
- You would use `./tool_name` only if you run them from an unix-like shell (like MinGW-w64). If you run them from a Windows command prompt, replace every occurence of `./tool_name` in this readme with just `tool_name`.
- Some tools use Windows-style parameters (for example `/x file`). They may be interpreted as paths on MinGW/cygwin, but most of these tools will also support the unix-style variant (for example `-x file`).

## Act/Nut lib
Act/Nut lib is a library to parse and edit the Act and Nut files. Nut files are compiled [Squirrel 3](https://github.com/albertodemichelis/squirrel) scripts that can be read with the [Squirrel `sq_readclosure` function](https://github.com/albertodemichelis/squirrel/blob/453a9668903238ec18da1e7fd1f91c60d42ab502/include/squirrel.h#L359). And I don't exactly know what are Act files, they don't seem to belong to a standard file format.

If you're only interested by reading the content of a file, you can use print-act-nut: `./print-act-nut [--no-print-file] [--print-full-names] file.(act|nut)`  
Each member of every structure will be displayed on one line. embeeded structures will be indented in order to make a tree view. If you specify `--print-full-names`, each line will contain the full path to a member, with all its parents, instead of only the member name. It's less readable, but you can copy-paste a path directly to use it with the getChild() functon (see below).  
The `--no-print-files` suppresses the normal output and keeps only the errors. It is used mostly for debugging the library.

If you want to change the Act or Nut file, you will need to either edit [main.cpp](https://github.com/thpatch/Act-Nut-lib/blob/master/main.cpp) to add your modifications ([right after the file parsing](https://github.com/thpatch/Act-Nut-lib/blob/4b246aca9267ae1057b0f6bb0da0c00dc6775892/main.cpp#L58) is a good place), or link to `libactnut.dll` yourself. The easiest is to use [`Object::getChild()`](https://github.com/thpatch/Act-Nut-lib/blob/4b246aca9267ae1057b0f6bb0da0c00dc6775892/Object.hpp#L66) and the `operator=` overloads.  
You have an example in [the thcrap source code](https://github.com/thpatch/thcrap/blob/7794aae8978ec07839dce29c21abc2b3a152207a/thcrap_tasofro/src/act-nut.cpp#L40). In this example, the `key` and `text` variables are pulled from [files like this](https://github.com/thpatch/thcrap-tsa/blob/master/script_latin/th155/data/script/talk/talk_balloon.nut.jdiff).

Look at [main.cpp](https://github.com/thpatch/Act-Nut-lib/blob/master/main.cpp) and the header files for more examples.

The Makefile builds 2 copies of the lib and executable: print-act-nut and libactnut are for 32-bits nut files (most of them), and print-act-nut-64 and libactnut64 are for 64-bits nut files (from the PS4 version of ULiL). Note that print-act-nut-64 and libactnut64 can be used on a 32-bits computer.

## bmpfont (by brliron)
Touhou 15.5 decided to use bitmap fonts instead of rendering texts with a TTF font. The bitmap font provided by the game supports only Japanese and some latin characters, so we have to provide a different font for translations.

This project contains 3 programs: `bmpfont_extract`, `bmpfont_convert` and `bmpfont_create`. `bmpfont_extract` and `bmpfont_convert` are easy to use, let's start with them.

### `bmpfont_extract`
This tool displays the informations stored at the end of the bmp file.  
Usage: `./bmpfont_extract.exe in.bmp [out.txt]`  
If you don't provide an output file, the standard output will be used instead.  
This tool doesn't do any encoding conversion, and assumes the input uses shift-jis.

### `bmpfont_convert`
Convert the metadatas of a font from shift-jis to UCS-2LE.  
Usage: `./bmpfont_convert file.bmp`  
The file is modified in place.

### `bmpfont_create`
Create a bitmap font usable by thcrap (this tool doesn't support shift-jis and will always add UCS-2LE metadatas to the fonts so you can't use them with the original game).  
When you create a font, you want to control how it looks. This tool try to provide some options about this, and so it is more complicated than the others. The main program doesn't take care of text rendering, it is done using plugins. For now, I wrote 2 text rendering plugins: one using GDI, and the other using GDI+. But for now, let's look at the main program.

`./bmpfont_create options...`
Run `./bmpfont_create --help` for a list of options. `--format`, `--out` and `--plugin` are required.

The GDI plugin uses the GDI function DrawText to display the text. To use it, run `./bmpfont_create --plugin bmpfont_create_gdi.dll options...`. For a list of options supported by the plugin, run `./bmpfont_create --plugin bmpfont_create_gdi.dll --help`

The GDI+ plugin uses the GDI+ functions `GraphicsPath::AddString`, `Graphics::DrawPath` and `Graphics::FillPath` to render the characters. It supports adding an outline around the characters. On the other hand, the rendered characters seems to be too big, or `GraphicsPath::GetBounds` adds a margin around the characters, or I don't know what, but the result is that there is a lot of space between the characters.  
To use it, run `./bmpfont_create --plugin bmpfont_create_gdiplus.dll options...`. For a list of options supported by the plugin, run `./bmpfont_create --plugin bmpfont_create_gdiplus.dll --help`

You can also create your own plugins. The `--plugin` option takes any dll exporting the `graphics_init`, `graphics_free` and `graphics_put_char` functions (see [`bmpfont_create.h`](https://github.com/brliron/135tk/blob/master/bmpfont/bmpfont_create.h)). Do whatever you want in `graphics_init` and `graphics_free`, and when `graphics_put_char` is called, fill the `dest` array with a nice rendered character. `graphics_help` is optionnal, you can display the options supported by your plugin in it.  
Also, the language in which you write your plugin doesn't matter as long as you can compile it to a DLL and your functions can be called from C code.

## nhtextool (by brliron)
A tool to unpack and edit nhtex files.

Usage: `./nhtextool (/x|/p) file.nhtex`  
`./nhtextool /x file.nhtex` will extract the image in file.nhtex to either file.nhtex.png (if file.nhtex contains a PNG file), file.nhtex.dds (if file.nhtex contains a DDS file), or file.nhtex.bin (if it couldn't guess the file type).

`./nhtextool /p file.nhtex` will replace the image in file.nhtex with either file.nhtex.png, file.nhtex.dds or file.nhtex.bin (using the algorithm above).

## orig_135tk (by Riatre)
A set of tools made by Riatre. TFBMTool and TFCS are made in Python, and I don't know the language used for ACT1Tool and cnutool. They are used to extract/edit TFBM files (usually with the png extension), TFCS files (usually with the csv extension), act files and nut files.  
For more details, run each tool without arguments.

The dump/extract bat scripts run the corresponding program for each file in the current directory and all its subdirectories, with the dump/extract command line switch.  
The write/repack bat scripts will do the same thing, but using the write/repack command line switch.

## read_pat (by brliron)
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

## TFBMTool-alt (original by Riatre in Python, rewritten in C by brliron)
A C rewrite of Riatre's TFBMTool, with support for 8-bits images with palette.  
This one always overwrites the TFBM file in place, because I've never seen anyone actually using the original in another way.  

Usage (extraction): `./TFBMTool-alt /x tfbm_file.[bmp|png] [palette_XXX.bmp]`  
The palette is optional (and not used) for 24-bpp and 32-bpp TFBM files. It is mandatory for 8-bpp TFBM files, and the extracted file will be a 8-bpp PNG file with a palette.  

Usage (repacking): `./TFBMTool-alt /p tfbm_file.[bmp|png] [palette_XXX.bmp]`  
The palette is not used for 24-bpp and 32-bpp PNG files. For 8-bpp PNG files, it is optional, and if specified, the PNG file's palette is dumped into it.

### extractBM-alt
A C rewrite of Riatre's extractBM using TFBMTool-alt. It searches for every png and bmp files in the current directory and its subdirectories (recursively), and calls TFBMTool-alt on them.  
Because it uses TFBMTool-alt, it supports 8-bits images with palette, using the palette000.bmp file from the image file's directory.
Also, it should be significantly faster when going through all the file of a game, because it is entierly written in C, even the file search. Handling a file doesn't result on a new processus, only on a function call. Errors don't throw exceptions, they only return from a few functions. It display only the file names and the errors (and there will be way fewer errors, because we support 8-bits files with palette).
Usage: just double-click on it (or run `./extractBM-alt`).

## th145arc (original by Riatre, updated by brliron)
A tool to extract and repack the pak files from Touhou 14.5 and Touhou 15.5.  
To extract files, run `./th145arc /x th145.pak`. To repack them, run `./th145arc /p th145.pak`.  

Archives created with /p will only be usable by the Touhou 14.5 English patch, the original game won't be able to open them. And there is currently no way to use the archives created by this tool in Touhou 15.5.

## th135arc-alt (by brliron)
Yet another tool to extract and repack the pak files from the 2nd generation Tasofro fighters (Touhou 13.5, Touhou 14.5 and Touhou 15.5).  
Note that the repacking have the same drawbacks as th145arc.

Why making yet another tool for that?
- First, th135arc supports only Touhou 13.5, and th145arc supports only Touhou 14.5 and Touhou 15.5. This one supports the 3 games (at least for unpacking).
- Next, it works on Linux, if you care about that.
- And finally, this one does not *have* to extract everything from the archive. It does that, but it can easily be modified to extract only a part of it. And I want to use that feature with the palette editor.

Usage:  
`./th135arc-alt.exe -x th135.pak` to extract files from `th135.pak` to the `th135` directory, or  
`./th135arc-alt.exe -p th135` to repack files from the `th135` directory to `th135.pak`.

## th175arc (by brliron)
A tool to extract and repack the cga files from the Touhou 17.5 1st beta (might work with the other betas and the release, I don't know yet).
To extract files, run `./th175arc -x data.cga data`. To repack them, run `./th175arc -p data data.cga`.  
Also, note that game.exe is __both__ an exe file and a cga file. When you run `-x` on the exe file, the output directory will also contain a `game.exe`, which contains only the exe part of game.exe. And when you run `-p` on a directory with a game.exe file, the output file will be a working exe file.
