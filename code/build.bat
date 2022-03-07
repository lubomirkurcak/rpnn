@echo off

:: C++ build (MSVC)
set CommonCompilerFlags=-Od -MTd -nologo -fp:fast -GS- -Gs9999999 -Gm- -GR- -EHa- -Oi -wd4723 -wd4101 -wd4302 -wd4311 -wd4127 -wd4456 -wd4201 -wd4100 -wd4189 -wd4505 -wd4010 -wd4312 -wd4003 -FC -Z7 -Zo
set BuildCompilerFlags=-DINTERNAL_BUILD=1 -DSLOW_BUILD=1 -DWIN32_BUILD=1 %CommonCompilerFlags%

pushd ..\build
del /Q /S /F *
cl %BuildCompilerFlags% -DTEMPORARY=1 -D_CRT_SECURE_NO_WARNINGS=1 -DCOMPILER_MSVC=1 ..\code\rpnn.cpp
popd

:: Python build
pushd ..\python
del /Q /S /F build\*
:: NOTE: %python% needs to be set as an environmental variable path to python interpreter
%python% rpnnsetup.py build
::%python% rpnnsetup.py install
popd
