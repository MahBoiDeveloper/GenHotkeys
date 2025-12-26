@echo off
chcp 65001 > nul

:: Print header
echo Program: GenHotkeys
echo Authors: mah_boi, nikitvs
echo.

:: Set up cmake config folder for the project
echo Configuring MinGW Make...
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=C:\Qt\Tools\mingw_64\\bin\gcc.exe -DCMAKE_CXX_COMPILER:FILEPATH=C:\Qt\Tools\mingw_64\bin\g++.exe -S.\ -B.\build -G "MinGW Makefiles"
echo Configuring MinGW Make done
echo.

:: Start compiling
echo Compilation and linking executables...
cmake --build .\build --config Release --target all -j 4
echo Compilation and linking executables done
echo.

:: Copy translation tools
mkdir build\exe\Resources\Translations\Tools
copy /y C:\Qt\5.15.2\mingw81_64\bin\lrelease.exe build\exe\Resources\Translations\Tools > nul
copy /y C:\Qt\5.15.2\mingw81_64\bin\linguist.exe build\exe\Resources\Translations\Tools > nul
copy /y src\GUI\Translations\ru.ts               build\exe\Resources\Translations\Tools > nul
echo lrelease.exe -- CLI Qt compilation tool to compile *.ts files to the *.qm. Run lrelease.exe --help to see more info about how to use it. >> build\exe\Resources\Translations\Tools\readme.txt
echo linguist.exe -- GUI Qt translation tool to edit *.ts files. >> build\exe\Resources\Translations\Tools\readme.txt
echo ru.ts        -- Translation file example. >> build\exe\Resources\Translations\Tools\readme.txt

:: Add any file to "Logs" folder for the next shipping
mkdir build\exe\Logs >nul 2> nul
cd build\exe\Logs
echo Do not delete this folder, please. Or program will can not be start. > readme.txt
echo.

echo Compiling finished. Press enter to exit...
timeout /t 30 > nul
