# File Splitter
Splits file into blocks and joins them back into normal file. Can be useful if you need to upload big file by parts.

# Usage

* FileSplitter.exe h - show help
* FileSplitter.exe <file name> split						- split file into blocks of 1 Mb each	
* FileSplitter.exe <file name> split <block size in Kb>	- split file into blocks of custom size
* FileSplitter.exe <file name> join						- join all blocks into one file, you need to specify only first (_0) file name