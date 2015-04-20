#Introduction
An editor based on scite-for-php and enhanced for MQL4 language. The goal is to be the de-facto editor in writing MT4 based code.

Currently it supports:
 - MQL syntax highlighting
 - Compiler integration
 - Auto-update MT4 upon compilation
 - Function listing
 - Autocomplete
 - Parameter hint
 - Help integration (press F1 on keyword brings up Help window)
 - Bracket matching
 - Block collapse / expand
 - Snippet, abbreviation support
 - Theming


The project is based on [scite-for-php](http://code.google.com/p/scite-for-php/) and partly on [scite-ru](http://code.google.com/p/scite-ru/) which both are based on [SciTE](http://www.scintilla.org). I added MQL4 support to it and although scite-for-php already supports a few languages such as C/C++/PHP/Javascript/HTML/CSS but this project will only target MQL specifically.<br>


#Download
All new versions can be downloaded [here](https://drive.google.com/folderview?id=0B5yYuvScBRONRkREWm9QdTVKMU0&usp=sharing)

[What's changed](https://github.com/ylw633/scite-mql/blob/wiki/WhatsNew.md)


#Screen Shots
more screen shots can be found <a href='https://github.com/ylw633/scite-mql/blob/wiki/screenshot.md'>HERE</a>
<img src='http://wiki.scite-mql.googlecode.com/hg/images/scite-mql-s1.png' />


#FAQ
**How do I compile the source?**<br/>
There are some [build instructions](https://github.com/ylw633/scite-mql/blob/wiki/BuildInstruction.md)

**How do I switch theme?**<br/>
Open SciTEGlobal.properties and at the very bottom of the document it specifies which theme to become active, uncomment (remove #) to activate the theme of your choice, if you comment all the themes then default theme will be used.
