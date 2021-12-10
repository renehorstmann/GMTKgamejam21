# GMTK Game Jam 2021

# Swarm
A mini game in which you should survive with your small fish swarm.
To survive you need at least three fishs in the swarm.
Eat food to get more score points.

## Livedemo
[Emscripten compiled game](https://renehorstmann.github.io/swarm/)

## engine
Based on [some](http://github.com/renehorstmann/some) framework.


## Highscore
The highscore system is run by the [HighscoreServer](https://github.com/renehorstmann/HighscoreServer) App.
To use it, set your own HIGHSCORE_SECRET_KEY and SHOWSCORE_URL.
The default will use your local running Highscoreserver (http://127.0.0.1:1000/api/swarm) with the key: 12345

## Compiling for Web

Using Emscripten:
```sh
mkdir web && cd web
```

```sh
emcc -I../include/ -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s FULL_ES3=1 -s EXPORTED_FUNCTIONS='["_main", "_e_io_idbfs_synced"]' -s SDL2_IMAGE_FORMATS='["png"]'  --preload-file ../res -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY=1 -s EXIT_RUNTIME=1 -s FETCH=1 -lidbfs.js -DOPTION_GLES -DOPTION_SDL -DOPTION_FETCH ../src/e/*.c ../src/p/*.c ../src/r/*.c ../src/u/*.c ../src/*.c -o index.html
```

Copy the icons into the webpage

````shell
cp ../icon/* .
````

Add the following changes to the generated index.html (into the head...):

```html

<link rel="shortcut icon" href="favicon.ico" type="image/x-icon">
<link rel="apple-touch-icon" href="icon180.png">
<link rel="shortcut icon" href="icon196.png" sizes="196x196">

<style>
    #canvas {
        position: absolute;
        top: 0px;
        left: 0px;
        margin: 0px;
        width: 100%;
        height: 100%;
        overflow: hidden;
        display: block;
    }
</style>
<script>
    function set_error_img() {
        var newContent = '<!DOCTYPE html><html><body style="background-color:black;"><h1 style="color:white;">Potato Browsers are not supported!</h1><p style="color:silver;">Full WebGL2.0 is needed!</p></body></html>';
        document.open();
        document.write(newContent);
        document.close();
    }
</script>
```
This will let Emscripten run in fullscreen and display an error text, if the app / game is not able to run (WebGL2.0 support missing)


## Author
René Horstmann

## Licence
- The game and its assets are licenced under GPLv3, see LICENCE.
- The [some](https://github.com/renehorstmann/some) framework is under MIT licence, see someLICENCE.
- Have a look at [some_examples](https://github.com/renehorstmann/some_examples) for some shared modules, under the MIT licence
