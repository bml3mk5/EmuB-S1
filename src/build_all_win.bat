rem @echo off

set app=mbs1
set mmf=mmf_loader
set path=%path%;"C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE";C:\Windows\Microsoft.NET\Framework\v4.0.30319

if not "%1"=="" set tag=/t:%1

svn update

cd source\VC++2010
: VCExpress.exe %app%.vcxproj %tag% /build Release

msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release;Platform=Win32
msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release;Platform=x64

msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release_Dbgr;Platform=Win32
msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release_Dbgr;Platform=x64

: msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release_Z80B;Platform=Win32
: msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release_Z80B;Platform=x64

msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release_Z80B_Dbgr;Platform=Win32
msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release_Z80B_Dbgr;Platform=x64

msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release_M68K_Dbgr;Platform=Win32
msbuild.exe %app%.vcxproj %tag% /p:Configuration=Release_M68K_Dbgr;Platform=x64

msbuild.exe %mmf%.vcxproj %tag% /p:Configuration=DLLRelease;Platform=Win32
msbuild.exe %mmf%.vcxproj %tag% /p:Configuration=DLLRelease;Platform=x64

cd ..\..
