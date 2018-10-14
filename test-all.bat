@echo off

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT all_tests.cpp /Feall_tests
if %ERRORLEVEL% NEQ 0 goto somethingbad

all_tests.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

pushd .

cd Coroutines
call coroutine-test.bat
if %ERRORLEVEL% NEQ 0 goto somethingbad

popd

echo Success!
goto :EOF
:somethingbad
echo Something Bad Happened.