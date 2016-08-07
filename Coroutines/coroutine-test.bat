@echo off

cl /Zi /Od /DBNS_DEBUG ../vector.cpp ../assert.cpp ../strings.cpp ../lexer.cpp coroutines.cpp /Fecoroutines.exe

coroutines.exe

cl /Zi /Od /DBNS_DEBUG ../assert.cpp coroutines_example_use.cpp /Fecoroutines_example_use.exe

coroutines_example_use.exe