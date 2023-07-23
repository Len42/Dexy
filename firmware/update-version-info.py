""" update-version-info - Update a file with version information derived from a git release tag.

Usage: make-version-file.py <version-info-filename> <git-describe-filename>
  <version-info-filename> is a file used to save version info
  <git-describe-filename> is a file containing the output from 'git describe --tags --always --dirty'
<version-info-filename> will be created or updated if necessary with the latest
version information.
The output file will only be written if necessary, i.e. if it doesn't exist or
if the version info needs to be updated. This will prevent code from being
re-compiled unnecessarily due to the version.h file being written every time.
This utility gets the version info from the most recent git tag which should
be a version number like '1.2.3'.
"""
import sys
import os

cmdName, fnameSavedVer, fnameGitVer = sys.argv
cmdName = os.path.basename(cmdName)
try:
    with open(fnameGitVer, 'r') as file:
        versionGit = file.readline().strip()
except:
    versionGit = '0.0.0-0-unknown-dirty'
try:
    with open(fnameSavedVer, 'r') as file:
        versionSaved = file.readline().strip()
except:
    versionSaved = ''
# Only update the output file if necessary, to avoid unnecessary recompilations.
if versionGit == versionSaved:
    pass # no need to update the version info
else:
    print(f'{cmdName}: Updating version info')
    with open(fnameSavedVer, 'w') as file:
        file.write(versionGit + '\n')
