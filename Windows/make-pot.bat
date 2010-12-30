rem --- create uget-filelist.txt
dir /b ..\uglib\*.c ..\uglib\*.h ..\uget-gtk\*.c ..\uget-gtk\*.h > uget-filelist.txt

rem --- GetText for Windows
rem --- http://sourceforge.net/projects/gnuwin32/
rem --- http://gnuwin32.sourceforge.net/packages/gettext.htm
xgettext -C -s -D ..\src -D ..\uget-gtk -f uget-filelist.txt --keyword=_ --keyword=N_ -o ..\po\uget.pot

rem --- merge files
rem msgmerge ..\po\de.po    ..\po\uget.pot
rem msgmerge ..\po\es.po    ..\po\uget.pot
rem msgmerge ..\po\zh_CN.po ..\po\uget.pot
rem msgmerge ..\po\zh_TW.po ..\po\uget.pot

pause

