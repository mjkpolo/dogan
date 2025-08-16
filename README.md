# Dogan - TUI Catan clone built with notcurses

It's like Catan but Dog...

## Build

Make sure notcurses is cloned:

```bash
git submodule update --init --recursive
```

Run the build script:

```bash
./b
```

## Run

```bash
./build/dogan
```

## Debug

add `-DUSE_ASAN=on` to [b](./b)

To see the logs redirect to a debug file:

```bash
./build/dogan 2> debug.log
```
