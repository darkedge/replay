@echo off
call "%VS140COMNTOOLS:~0,-14%VC\vcvarsall.bat" amd64
start cmd.exe
start sublime_text.exe
