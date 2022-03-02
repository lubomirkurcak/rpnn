@echo off
::cl /LD spammodule.c

set python=C:\Users\Lubko\AppData\Local\Programs\Python\Python39\python.exe

del /Q /S /F build\*
%python% spamsetup.py build

timeout 5
