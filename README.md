# Description

This project was created as a linux sample code for [this report](https://www.youtube.com/watch?v=L0sEnA1fazk). It demonstrates some of the hook's capabilities using Tetris game as a target.

**Warning:** This code does not the same code from the presentation

## What does this SO do?

- randomly sets the position and rotation when creating shapes
- print the type (number) of the event that occurred

## Links

- [Windows version](https://github.com/osogi/hook-example-windows)
- [Challenge yourself](https://ny2022.forkbomb.ru/tasks)
- [Presentation](https://docs.google.com/presentation/d/1VHTaCe5Modzr4dvWYWGVWrc0xC3bIg_JINU-e9TXUWA/edit?usp=sharing)

# Using

## Launch game
```bash
make tetris
```
  
## Launch with so injection
```bash
make inject
```

# Tetris source
The source code of the game was taken from [here](https://github.com/mmatyas/openblok)
