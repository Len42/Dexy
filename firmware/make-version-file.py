""" make-version-file - Write a file with inserted version information.

Usage: make-version-file.py <version-info-filename> <output-filename>
  <version-info-filename> is a version info file written by update-version-info.py
  <output-filename> will be written with the latest version info
A template file named "<output-filename>.template" is expected, containing text
with placeholders for the version info. The supported placeholders are:
  {verMajor}
  {verMinor}
  {verRevision}
  {verBuild}
  {verCommit}
  {verIsDevBuild}
  {verString}
  {verTimestamp}
  {verDatestamp}
"""

import sys
import os
import datetime

cmdName, verInfoFile, *outputFiles = sys.argv
cmdName = os.path.basename(cmdName)
try:
    with open(verInfoFile, 'r') as file:
        versionGit = file.readline().strip()
except:
    versionGit = '0.0.0-0-unknown-dirty'
# Parse the git version info.
verMajor = 0
verMinor = 0
verRevision = 0
verBuild = 0
verCommit = ''
verIsDevBuild = False
# versionGit comes from 'git describe --tags'.
# (Tags in the repo are expected to be version numbers.)
# We interpret it like this: <major>.<minor>.<rev>-<build>-<commit>-<dirty>
# Some of those items may be missing.
# Examples of what it may look like:
#   g12345
#   0
#   0.0
#   0.0.0
#   0.0-2-g12345
#   0.0-2-g12345-dirty ('-dirty' indicates that there are un-committed changes)
# Version tag may start with a 'v', e.g. 'v1.2'. Remove it.
if versionGit.startswith('v'):
    versionGit = versionGit[1:]
parts = versionGit.split('-')
if parts[-1] == 'dirty':
    verIsDevBuild = True
    del parts[-1]
# Parse the tag into numbers
nums = parts[0].split('.')
verMajor = int(nums[0]) if nums[0].isdigit() else 0
if len(nums) > 1:
    verMinor = int(nums[1]) if nums[1].isdigit() else 0
    if len(nums) > 2:
        verRevision = int(nums[2]) if nums[2].isdigit() else 0
# Parse the other parts (there should be zero or two parts)
if len(parts) > 2:
    # Expect a number of commits (which we use as a build number)
    # and a commit hash
    verBuild = int(parts[1]) if parts[1].isdigit() else 0
    verCommit = parts[2]
verString = '{verMajor}.{verMinor}.{verRevision}.{verBuild}{commit}{isDevBuild}'\
    .format(\
        verMajor = verMajor,\
        verMinor = verMinor,\
        verRevision = verRevision,\
        verBuild = verBuild,\
        commit = ('-' + verCommit) if verCommit else '',\
        isDevBuild = '-dev' if verIsDevBuild else '')
tNow = datetime.datetime.now(datetime.timezone.utc)
verTimestamp = tNow.isoformat()
verDatestamp = tNow.date().isoformat()
# Write the output file, based on a template file.
# str.format() will replace the given placeholders with the corresponding
# version info items.
for outputFile in outputFiles:
    templateFile = outputFile + '.template'
    try:
        with open(templateFile, 'r') as file:
            input = file.read()
    except:
        # If there's no template file, output some generic text.
        print(f'{cmdName}: No template file was found for {outputFile}')
        input = 'Version {verString} updated {verDatestamp}\n'
    output = input.format(\
        verMajor = verMajor,\
        verMinor = verMinor,\
        verRevision = verRevision,\
        verBuild = verBuild,\
        verCommit = verCommit,\
        verIsDevBuild = 1 if verIsDevBuild else 0,\
        verString = verString,\
        verTimestamp = verTimestamp,\
        verDatestamp = verDatestamp)
    with open(outputFile, 'w') as file:
        file.write(output)
