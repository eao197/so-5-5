setlocal
set PATH=%PATH%;${CMAKE_INSTALL_PREFIX}\bin
cmd /c ctest --force-new-ctest-process --output-on-failure
if %errorlevel% neq 0 exit /b 1
endlocal
