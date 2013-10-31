@Echo off
reg add "HKEY_CLASSES_ROOT\*\shell\SciTE" /ve /t REG_SZ /d "Edit with SciTE"  /f
reg add "HKEY_CLASSES_ROOT\*\shell\SciTE\command" /ve /t REG_SZ /d "\"%CD%\\..\\..\\SciTE.exe\" \"%%1\"" /f
Pause
