# Connect 4 Minimax AI

An interactive Connect 4 game powered by an optimized minimax search. The SFML front end lets you play against the AI while toggling the major pruning heuristics from the command line.
## Simple Algo Explanation

Minimax is an algorithm that assumes both players play perfectly—one trying to win, the other trying to ruin that win—and picks the move that leads to the best possible outcome even if the opponent plays optimally. So you are maximizing your score and minimizing the opponent score.
NOTE: only designed for zero sum games because of the assumption with minimax that "What is good for me is equally bad for my opponent."

## Features
- SFML-powered Connect 4 board with click-to-drop controls and optional column labels.
- Minimax search with adjustable depth, alpha-beta pruning, center-first move ordering, and early-win detection.
- Heuristic scoring tuned for center control and sliding-window pattern evaluation.

## Prerequisites
- Linux or macOS environment with a C++17-capable compiler (tested with `g++`).
- SFML graphics libraries. Install via your package manager and ensure `pkg-config` is available so the Makefile can locate headers/libraries automatically:  
  **Debian/Ubuntu**
  ```bash
  sudo apt update
  sudo apt install libsfml-dev pkg-config
  ```
  **macOS (Homebrew)**
  ```bash
  brew install sfml pkg-config
  ```

## Build
Use the provided `Makefile` (`-std=c++17` by default). It now writes the executable directly to the repository root as `./connect4`.

```bash
# Build the SFML client
make all

# or explicitly
make connect4
```

## Run the game
Launch the executable (produced in the project root) with three feature toggles (1 = enable, 0 = disable):
```bash
./connect4 <alpha-beta> <mid-row-first> <early-win>
```

| Arg | Toggle | Description |
| --- | --- | --- |
| `alpha-beta` | Alpha-Beta Pruning | Cuts off branches once upper/lower bounds meet to reduce search nodes. |
| `mid-row-first` | Mid-Row / Move Ordering | Explores center columns first (`3,2,4,1,5,0,6`) to find strong lines earlier. |
| `early-win` | Early-Win Detection | Checks for immediate wins/losses before deepening the tree. |

Examples:
- All optimizations: `./connect4 1 1 1`
- Alpha-Beta only: `./connect4 1 0 0`
- Mid-row ordering only: `./connect4 0 1 0`

Recommended depths (`main.cpp:MAX_DEPTH` macro):
- **No optimizations:** Depth 6 for responsive play, absolute max ~9–11 (very slow).
- **All optimizations enabled:** Depth 8 stays under a second on typical hardware; depth 12–15 is possible but long-running.

## Tuning search depth & labels
`main.cpp` exposes key macros near the top of the file:
- `MAX_DEPTH` – global search depth for the interactive AI.
- `DRAW_COL_LABELS` – set to `true` to render column numbers above the board.
- `ARIAL` – absolute path to the font used when labels are enabled.

Update the values and rebuild to experiment with deeper searches or on-screen hints. If you enable labels, be sure `ARIAL` points to a font that exists on your system.

## Project layout
- `main.cpp` – SFML game loop + minimax AI.

## Minimax Pseudocode

```
MINIMAX-VALUE(state, depth, maximizingPlayer)
1  if gameOver(state) or depth = 0 //base cases
2      return evalStrength(state)
3  if maximizingPlayer //recursive step (maximizingPlayer)
4      value = -INF
5      for each child in SUCCESSORS(state)
6          value = MAX(value, MINIMAX-VALUE(child, depth - 1, FALSE))
7      return value
8  else               //recursive step (mimizingPlayer)
9      value = +INF
10     for each child in SUCCESSORS(state)
11         value = MIN(value, MINIMAX-VALUE(child, depth - 1, TRUE))
12     return value
```
