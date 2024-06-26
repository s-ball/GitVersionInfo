# BuildVersionInfo
Automatically build VERSIONINFO resource from Git tags.

## Current status

This program is currently a beta version. All required features are
implemented and most of them are tested.
Its full source is available from [GitHUB](https://github.com/s-ball/BuildVersionInfo).

## Goals

VERSIONINFO resources are a great way to provide version information for Windows programs. `BuildVersionInfo` aims at
automatically maintain such a resource by extracting the version numbers from git tags.
It expects tags to use a standard semantic versioning Major.Minor.Patch

## Usage:

`BuildVersionInfo` builds a *partial* resource file containing a VERSIONINFO
resource from a simpler .ini file. That partial resource file is expected to
be included from a standard `resource.rc` file.

### Syntax:

    BuildVersionInfo.exe appfilename [/I:inifile] [/O:outfile] [outfile [inifile]]

builds outfile (default versioninfo.rc2) from inifile (default version.ini)

    BuildVersionInfo {/h|/?}

displays a help message for the syntax of the command

    BuildVersioniInfo /v

displays the version of the BuildVersionInfo program

    BuildVersionInfo [/I:inifile] /S [statusfile]

uses a statusfile (default version.bin) to check whether the version should
change.

### Input file structure:

The input `version.ini` file is a standard `.ini` file with *section*
containing *key = value* pairs. Lines starting with a semicolon (`;`) are
ignored and can be used as comments.

Here is a commented example:

    ; GIT special section can be used to set the path for the git command
    ; and declare a special regex that will be used to parse the tags
    [git]
    ; default command is git and relies on the path to be found
    command = d:\path\to\git.exe
    ; a special processing for tags using a Release major-minor-patch pattern
    version_template = (d+)-(d+)-(d+)
    ; the template must return up to 3 groups for respectively major, minor
    ; and patch value of a semantic versioning. Non returned values as taken
    ; as 0

    ; the FIXED section is used to force values in the fixed-info bloc
    ; by default, the FILEVERSION and PRODUCT version are extracted from
    ; git tags, and other fields are set for a WINDOWS32 application
    ; keys are the ones used in a true VERSIONINFO resource
    [fixed]
    productversion = 1,2,0,0

    ; other sections declare StringFileInfo blocs
    ; the first one will be used to provide default values to other ones
    [US]
    ; standard LANGID, charset pair for US English, unicode
    ; default is 0,1200 for language neutral, unicode
    Translation = 0x409, 1200
    companyname = ACME Company
    LegalCopyright= Copyright ACME 2021
    ; missing required keys will receive reasonable values based on the
    ;  application name

    [french]
    Translation = 0x40c, 1200
    CompanyName = Compagnie ACME
    ; the missing LegalCopyright will be extracted from the first bloc

### git processing

The version will be based on the last git tag. The highest 3 words will
be the major, minor, patch values extracted from the tag, and the last
one will be the number of commits since the tag.

If the repository is *dirty* (uncommited changes), the last number will
be increased and the `VS_FF_PRERELEASE` flag will be added to `FILEFLAGS`.

The `FileVersion` in all `StringFileInfo` blocs will be the tag name with
`-delta` added where delta is the 4th element of the FILEVERSION.

### StringFileInfo processing

The first section in the ini file is special: its fields will be used for
default values in all other sections. And if another section contains a
field that was not present in the first section, it will be added there
too.

Finally if any section contains a `SpecialBuild` (resp. `PrivateBuild`)
field (nota: it will be also present in the first StringFileInfo bloc...)
then the corresponding flag is added to `FILEFLAGS` in the fixed info bloc.

## Installation

### To develop on BuildVersionInfo using VisualStudio 2019

You can simply clone https://github.com/s-ball/BuildVersionInfo , either
with `git` or directly from VisualStudio. As the repository contains the
project and solution files you can directly build the executable locally.

The *not so nice* point is that the build process internally uses a previous
version of itself to build a `versioninfo.rc2` file from the `version.ini`
one. That means that the first build will build and use an empty file for
`versioninfo.rc2`. You have then to manually:

1. compile `version.ini` from the VisualStudio Solution Explorer to generate
a correct `versioninfo.rc2` file
2. generate again `BuildVersionInfo.exe` using that new `versioninfo.rc2`

That modus operandi has to be repeated each time you clean the project,
because if `BuildVersionInfo.exe` is not available, you get an empty
`versioninfo.rc2`.

### To use BuildVersionInfo in other projects

You have to get a BuildVersionInfo.exe on your system. You can either
download the source from GitHUB (prefere a release version, untagged
ones may be unstable...) and build the application localy, or directly
download a compiled version from the *packages*. Most releases should be
published there starting with the 0.7.0 one.

You must write a `version.ini` file according to the documentation above.
and set a custom build step on it using your installed `BuildVersionInfo.exe`
to generate a `versioninfo.rc2` file. Then open your `resource.rc` file (or
create one with Visual Studio) and use the *Resources include* tool from the
resource editor or resource display (right click) to add

    #include "versioninfo.rc2"

in the *Compilation directives* block (third one).

Finally, you must add a prebuild event step to remove the versionini.rc2
file each time the version should change. It should write:

    [path\to\]BuildVersionInfo [/I:version.ini] /S [version.bin] || del versioninfo.rc2

### BEWARE:

In the current versions (0.8.1 or 0.9.0), `BuildVersionInfo` can be unable to
find the ini file if you give a relative path. Maybe I will fix that in a 
future version, but anyway, in case of problem you should first
try to give an absolute path for `version.ini`.

## Contributing

As this project is developped on my free time, I cannot guarantee very fast feedbacks. Anyway, I shall be glad to receive issues or pull requests on GitHUB. 

## Versioning

This project uses a standard Major.Minor.Patch versioning pattern. Inside a major version, public API stability is expected (at least after 1.0.0 version will be published).

## License

This project is licensed under the MIT License - see the LICENSE.txt file for details
