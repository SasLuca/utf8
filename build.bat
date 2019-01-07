call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

if "%1"=="release" (
  set params=-Ofast
) else if "%1"=="debug" (
  set params=-Z7
) else (
  set params=0
)

if %params%==0 (
  echo Please provide release or debug as command line argument 1
) else (
  clang-cl %params% -c extern/utf8proc/utf8proc.c -Fobuild/utf8proc_%1 -DUTF8PROC_STATIC
  clang-cl %params% -c -std:c++17 utf8.cc -Fobuild/utf8_%1
    
  lib /OUT:.\build\utf8_%1.lib .\build\utf8proc_%1.obj .\build\utf8_%1.obj
)