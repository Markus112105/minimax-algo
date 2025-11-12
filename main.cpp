#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>
#include <chrono> // for timing MiniMax

const int ROWS = 6; // 6 x 7 connect 4 board
const int COLS = 7;
const int CELL_SIZE = 100;
const int RADIUS = 40;
const int WIN_SCORE = 100000;
const int MAX_DEPTH = 9; // This is insanely important. This is how far ahead the minimax algo will look. Anything past 16 will take too long to run.
const bool DRAW_COL_LABELS = false;
const int TOP_MARGIN = DRAW_COL_LABELS ? 40 : 10; //offset is higher if we draw labels
const int WIDTH = COLS * CELL_SIZE;
const int HEIGHT = ROWS * CELL_SIZE + TOP_MARGIN;
const char* ARIAL = "/usr/share/fonts/truetype/msttcorefonts/Arial.ttf"; //change with the location to your font (if you enable labels)

enum Player
{
    NONE = 0,
    HUMAN,
    AI
};
using Board = std::vector<std::vector<int>>; // board is 2D vector. So the datastructure we are using is a matrix.

// order to check columns for move ordering (center-first)
static const int COLUMN_ORDER[COLS] = {3, 2, 4, 1, 5, 0, 6};

// Feature toggles (set via command-line: 0=disable, 1=enable)
static bool USE_ALPHA_BETA = true;
static bool USE_MOVE_ORDER = true;
static bool USE_EARLY_WIN = true;
static std::vector<int> columnOrder; // actual order based on toggle

sf::Color getColor(int player)
{
    if (player == HUMAN)
        return sf::Color::Red;
    if (player == AI)
        return sf::Color::Yellow;
    return sf::Color::White;
}

void drawBoard(sf::RenderWindow &window, const Board &board)
{
    window.clear(sf::Color(0, 0, 200));

    // Draw column numbers above the board for the sake of the Demo. I allow you to toggle this on and off because you are going to need to update the
    // location of your Arial.ttf
    if (DRAW_COL_LABELS)
    {
        for (int c = 0; c < COLS; ++c)
        {
            sf::Text label;
            sf::Font font;
            if (!font.loadFromFile(ARIAL))
            {
                std::cerr << "Failed to load Arial\n";
            }

            label.setFont(font);
            label.setString(std::to_string(c));
            label.setCharacterSize(24);
            label.setFillColor(sf::Color::White);
            label.setPosition(c * CELL_SIZE + TOP_MARGIN, 5);
            window.draw(label);
        }
    }
    // Draw the circles
    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            sf::CircleShape circle(RADIUS);
            circle.setFillColor(getColor(board[r][c]));
            circle.setPosition(c * CELL_SIZE + 10, r * CELL_SIZE + TOP_MARGIN); // Shift down to make space for labels
            window.draw(circle);
        }
    }

    window.display();
}

bool isValidMove(const Board &board, int col)
{ // you really dont need the other col checks but its incase you get a bad col from some weird col math. Not expesive test could save a weird edge case
    return col >= 0 && col < COLS && board[0][col] == NONE;
}

bool makeMove(Board &board, int col, int player)
{
    for (int r = ROWS - 1; r >= 0; --r)
    {
        if (board[r][col] == NONE)
        {
            board[r][col] = player;
            return true;
        }
    }
    return false;
}

bool checkWin(const Board &board, int player)
{ // This version uses a cool "directional" matrix math to correctly check 4 cells in less written loops using a sliding window
    // although time complexity is worse.
    const int directions[4][2] = {
        {0, 1}, // Horizontal (right)
        {1, 0}, // Vertical (down)
        {1, 1}, //  Diagonal down-right
        {-1, 1} // Diagonal up-right
    };

    // Loop through every cell in the matrix as a potential starting point
    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {

            // For each cell, try all 4 directions
            for (auto [dr, dc] : directions)
            {
                int count = 0;

                // Slide a 4-cell window from (r, c) in the current direction
                for (int k = 0; k < 4; ++k)
                {
                    int nr = r + k * dr; // next row
                    int nc = c + k * dc; // next column

                    // Check if within bounds and if it matches the player
                    if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS &&
                        board[nr][nc] == player)
                    {
                        count++;
                    }
                    else
                    {
                        break; // interrupt if out of bounds or not matching
                    }
                }

                // If all 4 cells match, it's a win
                if (count == 4)
                    return true;
            }
        }
    }

    // No win found after sliding all windows
    return false;
}

int evaluateWindow(const std::vector<int> &window)
{
    int score = 0;
    int countAI = std::count(window.begin(), window.end(), AI);
    int countHUMAN = std::count(window.begin(), window.end(), HUMAN);
    int countEmpty = std::count(window.begin(), window.end(), NONE);

    if (countAI > 0 && countHUMAN > 0)
        return 0; // Mixed window, blocked (equal here)

    // if (countAI == 4) score += WIN_SCORE; don't need this cuz we check in the outer mini-max loop.
    else if (countAI == 3 && countEmpty == 1)
        score += 51; // 3 unblocked is really good
    else if (countAI == 2 && countEmpty == 2)
        score += 17; // 2 unblocked is good

    // if (countHUMAN == 4) score -= WIN_SCORE;
    else if (countHUMAN == 3 && countEmpty == 1)
        score -= 51; // substract for opponet.
    else if (countHUMAN == 2 && countEmpty == 2)
        score -= 17;

    return score;
}

int evaluateBoard(const Board &board)
{ // this version also uses a sliding window but without the directional matrix math. So we have more written loops less overall looping
    int score = 0;
    int centerCol = COLS / 2;
    for (int r = 0; r < ROWS; ++r)
    {
        if (board[r][centerCol] == AI)
            score += 18; // having center column is great.
        else if (board[r][centerCol] == HUMAN)
            score -= 18;
        if (board[r][centerCol - 1] == AI)
            score += 6; // being near center col is good
        else if (board[r][centerCol - 1] == HUMAN)
            score -= 6;
        if (board[r][centerCol + 1] == AI)
            score += 6;
        else if (board[r][centerCol + 1] == HUMAN)
            score -= 6;
        if (board[r][centerCol - 2] == AI)
            score += 2; // being 2 away center col is ok honestly this might be a waste of runtime but its ok not much overhead
        else if (board[r][centerCol - 2] == HUMAN)
            score -= 2;
        if (board[r][centerCol + 2] == AI)
            score += 2;
        else if (board[r][centerCol + 2] == HUMAN)
            score -= 2;
    }

    for (int r = 0; r < ROWS; ++r)
    { // pass in all possible wins to the evalutor which will see if you cant win there, or there is 2 or 1 empty spots there
        for (int c = 0; c <= COLS - 4; ++c)
        {
            std::vector<int> window = {board[r][c], board[r][c + 1], board[r][c + 2], board[r][c + 3]}; // columns
            score += evaluateWindow(window);
        }
    }
    for (int c = 0; c < COLS; ++c)
    {
        for (int r = 0; r <= ROWS - 4; ++r)
        {
            std::vector<int> window = {board[r][c], board[r + 1][c], board[r + 2][c], board[r + 3][c]}; // rows
            score += evaluateWindow(window);
        }
    }
    for (int r = 0; r <= ROWS - 4; ++r)
    {
        for (int c = 0; c <= COLS - 4; ++c)
        {
            std::vector<int> window = {board[r][c], board[r + 1][c + 1], board[r + 2][c + 2], board[r + 3][c + 3]}; // SE
            score += evaluateWindow(window);
        }
    }
    for (int r = 3; r < ROWS; ++r)
    {
        for (int c = 0; c <= COLS - 4; ++c)
        {
            std::vector<int> window = {board[r][c], board[r - 1][c + 1], board[r - 2][c + 2], board[r - 3][c + 3]}; // NE
            score += evaluateWindow(window);
        }
    }
    return score;
}

// Recursive minimax function with alpha-beta pruning, midRow, earlyWin
// Returns a score representing how good the current board is for the AI.
// depth: how many moves ahead to search
// alpha: the best already explored option for the maximizer
// beta:  the best already explored option for the minimizer
// maximizingPlayer: true if it's AI's turn, false if it's HUMAN's turn
int minimax(Board board, int depth, int alpha, int beta, bool maximizingPlayer)
{
    // Base case: if someone has won, return a large positive or negative score
    // Use depth to reward faster wins (higher depth left = win is sooner)
    if (checkWin(board, AI))
        return WIN_SCORE + depth;
    if (checkWin(board, HUMAN))
        return -WIN_SCORE - depth;

    // If no one has won and we're at max depth, return the leaf node strength
    if (depth == 0)
        return evaluateBoard(board);

    if (maximizingPlayer)
    {
        int maxEval = std::numeric_limits<int>::min();

        // Early win detection: if any move wins immediately, return WIN_SCORE + depth
        if (USE_EARLY_WIN)
        {
            for (int idx = 0; idx < COLS; ++idx)
            {
                int col = columnOrder[idx];
                if (!isValidMove(board, col))
                    continue;
                Board temp = board;
                makeMove(temp, col, AI);
                if (checkWin(temp, AI))
                {
                    return WIN_SCORE + depth;
                }
            }
        }

        // Try every valid column in center-first order for better pruning
        for (int idx = 0; idx < COLS; ++idx)
        {
            int col = columnOrder[idx];
            if (!isValidMove(board, col))
                continue;

            Board temp = board;
            makeMove(temp, col, AI);

            // Recursively evaluate this move with the opponent's turn next
            int score = minimax(temp, depth - 1, alpha, beta, false);

            // Update maxEval to track the best score so far
            maxEval = std::max(maxEval, score);

            // Update alpha to represent the best option available to the maximizer so far
            if (USE_ALPHA_BETA)
            {
                alpha = std::max(alpha, score);
                // Alpha-Beta Pruning:
                // If alpha is greater than or equal to beta, we can prune this branch
                if (beta <= alpha)
                    break;
            }
        }
        return maxEval;
    }
    else
    {
        int minEval = std::numeric_limits<int>::max();

        // Early loss detection: if opponent can win immediately, return -WIN_SCORE - depth
        if (USE_EARLY_WIN)
        {
            for (int idx = 0; idx < COLS; ++idx)
            {
                int col = columnOrder[idx];
                if (!isValidMove(board, col))
                    continue;
                Board temp = board;
                makeMove(temp, col, HUMAN);
                if (checkWin(temp, HUMAN))
                {
                    return -WIN_SCORE - depth;
                }
            }
        }

        for (int idx = 0; idx < COLS; ++idx)
        {
            int col = columnOrder[idx];
            if (!isValidMove(board, col))
                continue;

            Board temp = board;
            makeMove(temp, col, HUMAN);

            int score = minimax(temp, depth - 1, alpha, beta, true);

            minEval = std::min(minEval, score);
            if (USE_ALPHA_BETA)
            {
                beta = std::min(beta, score);
                if (beta <= alpha)
                    break;
            }
        }
        return minEval;
    }
}

int getBestMove(Board board, int depth = MAX_DEPTH) // calls minimax for each valid move to find best spot
{
    int bestScore = std::numeric_limits<int>::min();
    int bestCol = 0;

    // Check for immediate winning move
    if (USE_EARLY_WIN)
    {
        for (int idx = 0; idx < COLS; ++idx) // run for each col
        {
            int col = columnOrder[idx];
            if (!isValidMove(board, col))
                continue;
            Board temp = board;
            makeMove(temp, col, AI);
            if (checkWin(temp, AI))
            {
                std::cout << "AI moves to column " << col << ": immediate win" << std::endl;
                return col;
            }
        }
    }

    for (int idx = 0; idx < COLS; ++idx)
    {
        int col = columnOrder[idx];
        if (!isValidMove(board, col))
            continue;
        Board temp = board;
        makeMove(temp, col, AI);
        int score = minimax(temp, depth - 1,
                            std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false);
        if (score > 99999)
            std::cout << "Col " << col << ": " << score << " Ai wins in " << MAX_DEPTH - score + 100000 << " turns " << std::endl;
        else if (score < -99999)
            std::cout << "Col " << col << ": " << score << " Ai loses in " << MAX_DEPTH + score + 100000 << " turns " << std::endl;
        else
            std::cout << "Col " << col << ": " << score << std::endl;
        if (score > bestScore)
        {
            bestScore = score;
            bestCol = col;
        }
        if (score == WIN_SCORE + MAX_DEPTH - 1)
            break; // if you won just quit. Why check other moves..
    }
    return bestCol;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <alpha-beta> <mid-col> <early-win> (0 or 1 each)\n";
        return 1;
    }
    USE_ALPHA_BETA = (argv[1][0] == '1');
    USE_MOVE_ORDER = (argv[2][0] == '1');
    USE_EARLY_WIN = (argv[3][0] == '1');

    // build columnOrder based on toggle
    columnOrder.clear();
    if (USE_MOVE_ORDER)
    {
        for (int i = 0; i < COLS; ++i)
            columnOrder.push_back(COLUMN_ORDER[i]);
    }
    else
    {
        for (int i = 0; i < COLS; ++i)
            columnOrder.push_back(i);
    }
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Connect 4 SFML",
                            sf::Style::Titlebar | sf::Style::Close); // Disallow resizing (causes drop bugs) and allow closing

    // Center window on screen (normally spawns on bottom)
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.setPosition(sf::Vector2i(
        (desktop.width - WIDTH) / 2,
        (desktop.height - HEIGHT) / 2));
    Board board(ROWS, std::vector<int>(COLS, NONE));
    bool gameOver = false;
    int currentPlayer = HUMAN;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (!gameOver && currentPlayer == HUMAN && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                int col = event.mouseButton.x / CELL_SIZE;
                if (isValidMove(board, col))
                {
                    makeMove(board, col, HUMAN);
                    if (checkWin(board, HUMAN))
                    {
                        std::cout << "You win!\n";
                        gameOver = true;
                    }
                    else
                    {
                        currentPlayer = AI;
                    }
                }
            }
        }

        if (!gameOver && currentPlayer == AI)
        {
            auto start = std::chrono::steady_clock::now(); // start timing MiniMax
            int aiCol = getBestMove(board);
            auto end = std::chrono::steady_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "MiniMax runtime: " << ms << " ms\n";

            makeMove(board, aiCol, AI);
            std::cout << "AI moves to column " << aiCol << "\n";
            if (checkWin(board, AI))
            {
                std::cout << "AI wins!\n";
                gameOver = true;
            }
            currentPlayer = HUMAN;
        }

        drawBoard(window, board);
    }

    return 0;
}
