<h1 align="center">
    Hyperreflex
</h1>

<p align="center">
    Smoothing of Curves on Polyhedral Surface Meshes by using Penalty Potentials and Geodesics
</p>

## Development Status

<p align="center">
    <img src="https://img.shields.io/github/languages/top/lyrahgames/hyperreflex.svg?style=for-the-badge">
    <img src="https://img.shields.io/github/languages/code-size/lyrahgames/hyperreflex.svg?style=for-the-badge">
    <img src="https://img.shields.io/github/repo-size/lyrahgames/hyperreflex.svg?style=for-the-badge">
    <a href="COPYING.md">
        <img src="https://img.shields.io/github/license/lyrahgames/hyperreflex.svg?style=for-the-badge&color=blue">
    </a>
</p>

<b>
<table align="center">
    <tr>
        <td>
            master
        </td>
        <td>
            <a href="https://github.com/lyrahgames/hyperreflex">
                <img src="https://img.shields.io/github/last-commit/lyrahgames/hyperreflex/master.svg?logo=github&logoColor=white">
            </a>
        </td>
        <!-- <td>
            <a href="https://circleci.com/gh/lyrahgames/hyperreflex/tree/master"><img src="https://circleci.com/gh/lyrahgames/hyperreflex/tree/master.svg?style=svg"></a>
        </td> -->
        <!-- <td>
            <a href="https://codecov.io/gh/lyrahgames/hyperreflex">
              <img src="https://codecov.io/gh/lyrahgames/hyperreflex/branch/master/graph/badge.svg" />
            </a>
        </td> -->
        <td>
            <a href="https://ci.stage.build2.org/?builds=lyrahgames-xstd&pv=&tc=*&cf=&mn=&tg=&rs=*">
                <img src="https://img.shields.io/badge/b|2 ci.stage.build2.org-Click here!-blue">
            </a>
        </td>
    </tr>
    <!-- <tr>
        <td>
            develop
        </td>
        <td>
            <a href="https://github.com/lyrahgames/hyperreflex/tree/develop">
                <img src="https://img.shields.io/github/last-commit/lyrahgames/hyperreflex/develop.svg?logo=github&logoColor=white">
            </a>
        </td>
        <td>
            <a href="https://circleci.com/gh/lyrahgames/hyperreflex/tree/develop"><img src="https://circleci.com/gh/lyrahgames/hyperreflex/tree/develop.svg?style=svg"></a>
        </td>
        <td>
            <a href="https://codecov.io/gh/lyrahgames/hyperreflex">
              <img src="https://codecov.io/gh/lyrahgames/hyperreflex/branch/develop/graph/badge.svg" />
            </a>
        </td>
    </tr> -->
    <tr>
        <td>
        </td>
    </tr>
    <tr>
        <td>
            Current
        </td>
        <td>
            <a href="https://github.com/lyrahgames/hyperreflex">
                <img src="https://img.shields.io/github/commit-activity/y/lyrahgames/hyperreflex.svg?logo=github&logoColor=white">
            </a>
        </td>
        <!-- <td>
            <img src="https://img.shields.io/github/release/lyrahgames/hyperreflex.svg?logo=github&logoColor=white">
        </td>
        <td>
            <img src="https://img.shields.io/github/release-pre/lyrahgames/hyperreflex.svg?label=pre-release&logo=github&logoColor=white">
        </td> -->
        <td>
            <img src="https://img.shields.io/github/tag/lyrahgames/hyperreflex.svg?logo=github&logoColor=white">
        </td>
        <td>
            <img src="https://img.shields.io/github/tag-date/lyrahgames/hyperreflex.svg?label=latest%20tag&logo=github&logoColor=white">
        </td>
        <!-- <td>
            <a href="https://queue.cppget.org/xstd">
                <img src="https://img.shields.io/website/https/queue.cppget.org/xstd.svg?down_message=empty&down_color=blue&label=b|2%20queue.cppget.org&up_color=orange&up_message=running">
            </a>
        </td> -->
    </tr>
</table>
</b>

## Requirements
<b>
<table>
    <tr>
        <td>Language Standard:</td>
        <td>C++20</td>
    </tr>
    <tr>
        <td>Compiler:</td>
        <td>
            GCC
        </td>
    </tr>
    <tr>
        <td>Build System:</td>
        <td>
            <a href="https://build2.org/">build2</a>
        </td>
    </tr>
    <tr>
        <td>Operating System:</td>
        <td>
            Linux
        </td>
    </tr>
    <tr>
        <td>Dependencies:</td>
        <td>
            Geometry Central<br>
            Libigl<br>
            Assimp<br>
            GLM<br>
            SFML<br>
            glbinding
        </td>
    </tr>
</table>
</b>

## Building
Currently, only the development mode is supported.

### Clone the Repository
For the development and the proper handling of multiple configurations, it is recommend to clone the repository into its own directory.

    mkdir hyperreflex && cd hyperreflex
    git clone https://github.com/lyrahgames/hyperreflex.git
    cd hyperreflex

### Initialization
To create a basic target configuration with the name `default` with the system's default GCC compiler and enabled optimization flags, run the following command.

    bdep init -C \                                # Create a new configuration for initialization.
        @default \                                # Name the configuration 'default'.
        cc \                                      # Load the `cc` module to properly handle C and C++.
        config.cxx=g++ \                          # Use the system's default GCC C++ compiler.
        config.cxx.coptions="-O3 -march=native" \ # Enable optimization.
        -- "sys:assimp/*"                         # Mark 'assimp' as system dependency.

During the creation and initialization of a new target configuration all dependencies except `assimp` are fetched and downloaded from package repositories.
The `assimp` dependency needs to be fulfilled by the system itself.
As the compilation of some dependencies, such as `glbinding` and `sfml-graphics`, takes some time, you might consider marking these as system dependencies, too, by either appending `"?sys:glbinding/*"` and `"?sys:libsfml-graphics/*"` to the previous command or running the following commands after a successful initialization.

    bdep sync "?sys:glbinding/*"
    bdep sync "?sys:libsfml-graphics/*"

### Compilation
The first compilation will also compile all dependencies and will require more time.
To compile the program run one of the following commands.

    b
    bdep update @default

### Clean
If you need to clean up after a compilation attempt, run one of the following commands.

    b clean
    bdep clean @default

## Usage
If the compilation was successful it should be possible to run the following command with a surface mesh file.

    hyperreflex/hyperreflex <surface mesh file>

### Keybindings
