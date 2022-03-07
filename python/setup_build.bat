@echo off
::cl /LD rpnnmodule.c

set python=C:\Users\Lubko\AppData\Local\Programs\Python\Python39\python.exe

del /Q /S /F build\*
%python% rpnnsetup.py build

timeout 5
