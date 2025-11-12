To play a game agaisnt the AI:

TO COMPILE: (USING LINUX) 
g++ main.cpp -o connect4 -lsfml-graphics -lsfml-window -lsfml-system

If you do not have SFML run:
sudo apt update
sudo apt install libsfml-dev

TO RUN: ENABLE?(1) DIABLE(0) Alpha-Beta pruning, Mid-Row first, Early-Win

This would run all optimizations
./connect4 1 1 1 

This would run alpha-beta ONLY
./connect4 1 0 0

This would run Mid-Row ONLY
./connect4 0 1 0

If you are running NO optimizations I recommend a depth of 6 max 9-11(takes long)
If you are running ALL optimizations I recommend a depth of 8 (under 1 second generally) and max of 12-15(will take very long).
You can change depth in the macros on the top of main.cpp. If you want font labels for each col you need to specify location of your font on computer and 
enable the font macro

To test runtime on a certain game state: 

TO COMPILE: (USING LINUX)
g++ tester.cpp -o tester -std=c++17

TO RUN: 
./tester <boardName.txt>

output will be in results.csv or the name you input to the CSV_NAME macro. If the tester fails to run after a minute lower depth, reload your terminal, recompile and run

IMPORTANT: Tester was created by chatGPT. I inputted my main.cpp (I created entirely myself) and asked it to remove the SFML option 
and output a csv with the runtimes for a specific gamestate. The orginal output was very buggy and removed all my comments but i went ahead and fixed the glitches
and edited the output. I also fixed some of the loops it changed for runtime sake. I know some professors are heavily agaisnt AI but if you are not using AI 
in this way you will fall behind engineers that are. I alrealdy completed all the critcial thinking by designing and implementing solid algorithms and drawing a board. But rather than spending an hour
redesinging into a tester file, which would be no critical thinking but rather tedious design work, I delegated it to AI which could do it in seconds. This is how I think
the modern software engineer should operate. 