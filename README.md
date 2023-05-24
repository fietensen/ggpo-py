![version](https://img.shields.io/github/license/fietensen/ggpo-py?color=blue&style=plastic)

# ggpo-py - Good Game, Peace Out Rollback Network SDK Python binds

## What is ggpo-py

ggpo-py creates Python bindings to GGPO.net



## Requirements

Supported Python Versions: 3.9 - 3.11

Requires: CMake>=3.25

## Installation

Installing via Git:

```bash
  git clone https://github.com/fietensen/ggpo-py.git
  cd ggpo-py
  git submodule update --init
  python setup.py install
```
    
## Documentation

Perhaps I'll add an indepth documentation on how the SDK works
later. For now you can check the [demo](#demo) for a brief overview of the Python binds.

To get further information on GGPO, I can only recommend going through `ggpo/src/include/ggponet.h` as it contains lots of useful
documentation of the functions and structures.
## Demo

I've written a small demo for ggpo-py using [pygame](https://pygame.org). This means you'll have to install the pygame module on your computer like this:

```bash
pip install pygame
```

After this you may now start two instances of the `examples/test_game.py` from your console with connection information as arguments:

```bash
python examples/test_game.py 8001 local 127.0.0.1:8002 
```

and in another shell:

```bash
python examples/test_game.py 8002 127.0.0.1:8001 local
```

You may now move around the two rectangles in their respective game instances using your arrow keys.
## License

[MIT](https://choosealicense.com/licenses/mit/)

