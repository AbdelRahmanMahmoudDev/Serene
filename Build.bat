@echo off

set CommonCompilerFlags=-DSERENE_DEBUG=1 -DSERENE_INTERNAL=1 -DSERENE_WIN32=1 -Gm- -MTd -nologo -EHa- -GR- -Od -Oi -WX -W4 -wd4505 -wd4201 -wd4100 -wd4101 -wd4189 -FC -Z7 
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib Gdi32.lib Winmm.lib Opengl32.lib

IF NOT EXIST build mkdir build
pushd build
del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags%  ..\code\Serene_Game.cpp -FmSerene_Game.map -LD /link -incremental:no -opt:ref -PDB:Serene_Game_%Random%.pdb /EXPORT:GameGenerateAudio /EXPORT:GameUpdate
cl %CommonCompilerFlags%  ..\code\Serene_Win32.cpp -FmSerene_Win32.map /link %CommonLinkerFlags%
popd