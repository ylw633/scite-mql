# Version 1.5.0 #
  * Update Scite to 3.3.6
  * Update metalang.exe to Build 509

# Version 1.4.2 #
  * Update MT4 refresher program so it finds MT instance more robust
  * Update editor so "find next" on search strip does not interfere with ALT+F key. The shortcut for the button has been moved to "I".

# Version 1.4.1 #
Add coloring enhancements:
  * by clicking on a word, all other occurrences are easier to be identified.
  * highlight current line in default theme
  * made change to dark theme so it's more eye appealing



# Version 1.4 #
  * New MT4 notifier program for better performance
  * More completed MQL4 reference document and fixed keyword list
  * Some UI and behavior tweak
  * New coloring on syntax highlighting
  * Incorporated new side bar from scite-ru



# Version 1.3.2 #
Mainly a bug fix version. MT4 notifier should be working now, and "Go" command will compile the program as well as force MT4 to reload EA. "Compile" and "Build" will only compile without notifying MT4.



# Version 1.3 #
  * Warning message from compiler now becomes orange instead of red like error message
  * Add Astyle code beautifier integration (java style though, change it in SciTEGlobal.properties to suit your own need)
  * Once the code is compiled, MT4 will be notified for the change and updates itself, thx to fx1 for providing the info!
  * Add shortcut Ctrl+Alt+F to toggle all fold/unfold of blocks
  * Integrate select\_block.lua so by clicking on the line number of first line of the block, the whole block is selected. Same effect will be on when you use keyboard to highlight the whole first line of block.



# Version 1.2.1 #

  * Auto completion is now case insensitive
  * More keywords set colors, therefore it's easier to distinguish built-in sets of keywords
  * Better MetaLanguage help document
  * Adjust some color scheme
  * "Document Comment" can be activated by "///" (3 slashes) so it's easier to find sections of code
  * Document comment keyword is activated by prefixing a "@" to it, currently default supported keywords are TODO, FIXME, OPTIMIZE. It can be used to notify later work

Please see screen shot below to see the added feature
![http://wiki.scite-mql.googlecode.com/hg/images/scite-mql-s6.png](http://wiki.scite-mql.googlecode.com/hg/images/scite-mql-s6.png)