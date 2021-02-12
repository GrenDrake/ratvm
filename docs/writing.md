---
Writing Games
---

# Writing Games


Every RatVM game is made from one or more source files.
Regardless of how large or small the game is, however, there are a few things every game will need to include.
(Though note that these are required per-project, not per-source file.)
An example of a truly minimal "game" is as follows.

```
declare TITLE   "Minimal Sample Game";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "56735BDD-C5EE-4837-911D-BBE70676C89A";

function main() {
}
```

There are two forms of construct in this minimal project.
The `declare` statements create a new named constant and define its value.
The four constants `TITLE`, `AUTHOR`, `VERSION`, and `GAMEID` are required in all projects.
`TITLE` and `AUTHOR` are quoted strings; the expected content should be apparent from the name.
`VERSION` is an integer and should be incremented for each release.
The `GAMEID` is a string containing the [IFID](http://www.ifwiki.org/index.php/IFID) used to uniquely identify this game from all other works of interactive fiction.
IFIDs can be [generated online](http://tads.org/ifidgen/ifidgen)

The other construct in this project is the declaration of the main function.
