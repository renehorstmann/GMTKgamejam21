# GMTK Game Jam 2021

# Swarm
A mini game in which you should survive with your small fish swarm.
To survive you need at least three fishs in the swarm.
Eat food to get more score points.

## Livedemo
Emscripten compiled [live demo](https://renehorstmann.github.io/gmtk21/)

## engine
Based on [some](http://github.com/renehorstmann/some) framework.


# compile in emscripten:
`emcc -I../include/ -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s FULL_ES3=1 -s SDL2_IMAGE_FORMATS='["png"]' --preload-file ../res -s ALLOW_MEMORY_GROWTH=1 -DOPTION_GLES -DOPTION_SDL ../src/e/*.c ../src/p/*.c ../src/r/*.c ../src/u/*.c ../src/*.c -o index.html`
