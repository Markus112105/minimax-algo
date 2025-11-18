# Connect 4 Minimax AI

An interactive Connect 4 game powered by an optimized minimax search. The SFML front end lets you play against the AI while toggling the major pruning heuristics from the command line. A separate tester lets you benchmark individual board states and capture runtime statistics.

## Features
- SFML-powered Connect 4 board with click-to-drop controls and optional column labels.
- Minimax search with adjustable depth, alpha-beta pruning, center-first move ordering, and early-win detection.
- Heuristic scoring tuned for center control and sliding-window pattern evaluation.
- Lightweight benchmarking harness (`tester.cpp`) that sweeps every optimization combo and records runtimes to CSV.
- Sample board states (`firstBoard.txt`, `midRow.txt`, `nearWin.txt`) for quick regression checks.

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
Use the provided `Makefile` (defaults to `build/` as the output directory and `-std=c++17`).

```bash
# Build both the SFML client and the CLI tester
make all

# or build a single target
make connect4
make tester
```

Executables are placed under `build/` by default (`build/connect4`, `build/tester`). Override `BIN_DIR` when invoking `make` if you prefer a different location, e.g. `make BIN_DIR=. all`.

## Run the game
Launch the executable with three feature toggles (1 = enable, 0 = disable):
```bash
./build/connect4 <alpha-beta> <mid-row-first> <early-win>
```

| Arg | Toggle | Description |
| --- | --- | --- |
| `alpha-beta` | Alpha-Beta Pruning | Cuts off branches once upper/lower bounds meet to reduce search nodes. |
| `mid-row-first` | Mid-Row / Move Ordering | Explores center columns first (`3,2,4,1,5,0,6`) to find strong lines earlier. |
| `early-win` | Early-Win Detection | Checks for immediate wins/losses before deepening the tree. |

Examples:
- All optimizations: `./build/connect4 1 1 1`
- Alpha-Beta only: `./build/connect4 1 0 0`
- Mid-row ordering only: `./build/connect4 0 1 0`

Recommended depths (`main.cpp:MAX_DEPTH` macro):
- **No optimizations:** Depth 6 for responsive play, absolute max ~9–11 (very slow).
- **All optimizations enabled:** Depth 8 stays under a second on typical hardware; depth 12–15 is possible but long-running.

## Tuning search depth & labels
`main.cpp` exposes key macros near the top of the file:
- `MAX_DEPTH` – global search depth for the interactive AI.
- `DRAW_COL_LABELS` – set to `true` to render column numbers above the board.
- `ARIAL` – absolute path to the font used when labels are enabled.

Update the values and rebuild to experiment with deeper searches or on-screen hints. If you enable labels, be sure `ARIAL` points to a font that exists on your system.

## Benchmark specific board states
1. Describe a board as six rows of seven integers using the encoding `0 = empty`, `1 = human`, `2 = AI`. Examples live in `*.txt` files in this repo.
2. Build the tester (`g++ tester.cpp -o tester -std=c++17`).
3. Run it with the board file:
   ```bash
   ./tester <boardName.txt>
   ```

What you get:
- The program times the AI decision for every combination of optimization toggles (alpha-beta, mid-row ordering, early-win) and writes rows to `results.csv` (or whatever you set in `CSV_NAME`).
- The final line records the best column choice plus its heuristic score.

Troubleshooting tips:
- If a run hangs for ~1 minute, lower `MAX_DEPTH` inside `tester.cpp`, restart the terminal, rebuild, and retry.
- Change the CSV destination by editing the `CSV_NAME` macro at the top of `tester.cpp`.

## Project layout
- `main.cpp` – SFML game loop + minimax AI.
- `tester.cpp` – headless benchmarking harness.
- `*.txt` – sample boards for reproducing positions.
- `results.csv` – default output from the tester.

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
