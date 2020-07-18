# Octave-GSoC-JSON
JavaScript Object Notation​, in short JSON, is a very common human readable and structured data format. Unfortunately, GNU Octave still lacks builtin support for that data format. Having JSON support, Octave can improve for example it's web service functions, which often exchange JSON data these days. 

This repo contains the code of my GSoC project that aims to support JSON decoding and encoding in GNU Octave.

## How to compile and run tests on the code
Right now, the code is treated as an external *.oct file. The integration of the code into Octave's build system will be done at the end of the project. To compile it:
* `cd` into the repo's directory.
* run `mkoctfile` command using the file name (eg. jsondecode.cc) as an argument.

Octave test files are provided for each function. For example, you can run the one that tests `jsondecode` by running this command:
```
test('test/jsondecodetest.m','quiet','test/log-jsondecode.txt')
```
The log file "log-jsondecode.txt" in "test" in your repo's directory will have the data of the failed tests.
