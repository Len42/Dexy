""" make-binary-inc-file - Convert a binary file to C-compatible data.

Usage: make-binary-inc-file.py <input-filename> ...

For each input file, write an output file with '.h' appended to the name.
The output file contains the binary data from the input file, formatted in a way
that can be #included in a C or C++ declaration.
"""

import sys
import os

cmdName, *inputFiles = sys.argv
# Set outputDir to the same directory as this script - that is where output files
# will be written.
# If that's not desired, set outputDir = '' to write output files next to the input files.
outputDir = os.path.dirname(cmdName)
#outputDir = ''
cmdName = os.path.basename(cmdName)
print(f'{cmdName}: {outputDir=}')
for inputFile in inputFiles:
    outputFile = inputFile + '.h'
    print(f'{cmdName}: {inputFile=} {outputFile=}')
    if outputDir:
        outputFile = os.path.join(outputDir, os.path.basename(outputFile))
    try:
        with open(inputFile, 'rb') as input:
            try:
                with open(outputFile, 'w') as output:
                    i = 0
                    while char := input.read(1):
                        i += 1
                        if i % 16 == 0:
                            endLine = '\n'
                        else:
                            endLine = ''
                        print(f'{ord(char):3}, ', end=endLine, file=output)
                    print(file=output)
            except Exception as ex:
                print(f'{cmdName}: {ex}')
    except Exception as ex:
        print(f'{cmdName}: {ex}')
