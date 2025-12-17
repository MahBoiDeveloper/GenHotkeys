@echo off
chcp 65001 > nul

echo Program: GenHotkeys
echo Authors: mah_boi, nikitvs
echo.

echo Configuring MinGW Make...
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=C:\Qt\Tools\mingw_64\\bin\gcc.exe -DCMAKE_CXX_COMPILER:FILEPATH=C:\Qt\Tools\mingw_64\bin\g++.exe -S.\ -B.\build -G "MinGW Makefiles"
echo Configuring MinGW Make done
echo.

echo Compilation and linking executables...
cmake --build .\build --config Release --target all -j 4
echo Compilation and linking executables done
echo.

echo Starting editor in 5 seconds
timeout /t 5
cd .\build\exe\
start "" GenHotkeys.exe
