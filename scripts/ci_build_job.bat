@echo off
setlocal

set "build_folder=build"

echo Working directory is %CD%

if not exist "%build_folder%" (
    mkdir "%build_folder%"
    if errorlevel 1 (
        echo Failed to create folder "%build_folder%"
        goto error
    )
)

cd %build_folder%

cmake -DCAVE_BUILD_ASSIMP=OFF -DCAVE_BUILD_UNIT_TESTS=ON .. || goto error
cmake --build . --config %1 || goto error

endlocal
exit /b 0

:error
endlocal
exit /b 1
