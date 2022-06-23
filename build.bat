gcc main.c -Wl,--subsystem,windows -ISDL2/inc -ISDL2_ttf/inc -LSDL2/lib -LSDL2_ttf/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -o a_star
pause