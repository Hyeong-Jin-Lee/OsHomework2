#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

// -------------------------
// Constants & Definitions
// -------------------------
#define COLS 7
#define ROWS 6
#define MAX_DEPTH 6             // Maximum search depth (adjust as needed)

// Board state structure (State)
// - board: ROWS x COLS, each cell: 0 (empty), 1 or 2 (player stone)
// - top: next index (row) where a stone will be placed in each column (0-based)
// - player: the player who is about to move (1 or 2)
typedef struct {
    int board[ROWS][COLS];
    int top[COLS];
    int player;
} State;

// -------------------------
// Functions Related to State
// -------------------------

// Deep copy of state: copy src state to dest state
void copy_state(const State* src, State* dest) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            dest->board[i][j] = src->board[i][j];
        }
    }
    for (int j = 0; j < COLS; j++) {
        dest->top[j] = src->top[j];
    }
    dest->player = src->player;
}

// Save the valid moves (columns where a stone can be placed) in the moves array,
// and return the number of valid moves.
int get_valid_moves(const State* s, int moves[]) {
    int count = 0;
    for (int j = 0; j < COLS; j++) {
        if (s->top[j] < ROWS) {
            moves[count++] = j;
        }
    }
    return count;
}

// Apply a move on state s given a column (move from 0 to COLS-1).
// The stone is placed at s->board[s->top[move]][move],
// then top[move] is incremented by 1 and the player is switched.
void apply_move(State* s, int move) {
    int row = s->top[move];
    s->board[row][move] = s->player;
    s->top[move] += 1;
    s->player = 3 - s->player;
}

// Winner checking function
// Return value: 0 = game still in progress,
// 1 or 2 = victory for that player,
// -1 = draw (board full)
int check_winner(const State* s) {
    // For every cell that is not empty, check in all directions for 4 in a row.
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int player = s->board[i][j];
            if (player == 0) continue;
            // Horizontal check
            if (j + 3 < COLS &&
                player == s->board[i][j+1] &&
                player == s->board[i][j+2] &&
                player == s->board[i][j+3])
                return player;
            // Vertical check
            if (i + 3 < ROWS &&
                player == s->board[i+1][j] &&
                player == s->board[i+2][j] &&
                player == s->board[i+3][j])
                return player;
            // Right diagonal check
            if (i + 3 < ROWS && j + 3 < COLS &&
                player == s->board[i+1][j+1] &&
                player == s->board[i+2][j+2] &&
                player == s->board[i+3][j+3])
                return player;
            // Left diagonal check
            if (i + 3 < ROWS && j - 3 >= 0 &&
                player == s->board[i+1][j-1] &&
                player == s->board[i+2][j-2] &&
                player == s->board[i+3][j-3])
                return player;
        }
    }
    // If the board is full, return draw (-1)
    int full = 1;
    for (int j = 0; j < COLS; j++) {
        if (s->top[j] < ROWS) { full = 0; break; }
    }
    if (full) return -1;
    return 0;
}

// Returns 1 if the state is terminal (end of game), otherwise 0.
int is_terminal(const State* s) {
    return (check_winner(s) != 0);
}

// -------------------------
// Evaluation Function
// -------------------------
// (1) If the state is terminal, return a very high score depending on win or loss.
// (2) Otherwise, simply evaluate by the difference in the number of stones between players.
// This is a simple example; you can improve the evaluation function for a more refined assessment.
int evaluate_state(const State* s, int root_player) {
    int winner = check_winner(s);
    if (winner == root_player)
        return 100000;  // Root player's win
    else if (winner == 3 - root_player)
        return -100000; // Opponent's win
    else if (winner == -1)
        return 0;       // Draw

    // For non-terminal state, simply evaluate by stone count difference.
    int count_root = 0, count_opp = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (s->board[i][j] == root_player)
                count_root++;
            else if (s->board[i][j] == (3 - root_player))
                count_opp++;
        }
    }
    return count_root - count_opp;
}

// -------------------------
// Alpha-Beta Pruning (Minimax)
// -------------------------
// Recursively search the game tree up to a given depth.
// The function returns the evaluated score using alpha-beta pruning.
int alphabeta(State* s, int depth, int alpha, int beta, int maximizing, int root_player) {
    int winner = check_winner(s);
    if (depth == 0 || winner != 0) {
        return evaluate_state(s, root_player);
    }

    int moves[COLS];
    int num_moves = get_valid_moves(s, moves);
    if (num_moves == 0) {  // No valid moves available
        return evaluate_state(s, root_player);
    }

    if (maximizing) {
        int value = INT_MIN;
        for (int i = 0; i < num_moves; i++) {
            State child;
            copy_state(s, &child);
            apply_move(&child, moves[i]);
            int score = alphabeta(&child, depth - 1, alpha, beta, 0, root_player);
            if (score > value) {
                value = score;
            }
            if (value > alpha) {
                alpha = value;
            }
            if (alpha >= beta) {  // Beta cutoff
                break;
            }
        }
        return value;
    } else {
        int value = INT_MAX;
        for (int i = 0; i < num_moves; i++) {
            State child;
            copy_state(s, &child);
            apply_move(&child, moves[i]);
            int score = alphabeta(&child, depth - 1, alpha, beta, 1, root_player);
            if (score < value) {
                value = score;
            }
            if (value < beta) {
                beta = value;
            }
            if (alpha >= beta) {  // Alpha cutoff
                break;
            }
        }
        return value;
    }
}

// From the given state (root), perform alpha-beta search for each valid move,
// and return the move (column number) with the highest evaluation.
int alphabeta_search(State* root, int depth, int root_player) {
    int best_move = -1;
    int best_value = INT_MIN;
    int moves[COLS];
    int num_moves = get_valid_moves(root, moves);
    for (int i = 0; i < num_moves; i++) {
        State child;
        copy_state(root, &child);
        apply_move(&child, moves[i]);
        int value = alphabeta(&child, depth - 1, INT_MIN, INT_MAX, 0, root_player);
        // Debug: print each move and its evaluation
        // printf("Move %d evaluated as %d\n", moves[i], value);
        if (value > best_value) {
            best_value = value;
            best_move = moves[i];
        }
    }
    return best_move;
}

// -------------------------
// Helper: Convert column number to character (A~G)
// -------------------------
char stack_name(int i) {
    return 'A' + i;
}

// -------------------------
// Main: Agent Execution (Reads player number and board state from parent)
// -------------------------
int main() {
    srand(time(NULL));
    
    int this_player;
    if (scanf("%d", &this_player) != 1) {
        fprintf(stderr, "Error: Failed to read player number\n");
        return EXIT_FAILURE;
    }
    if (this_player != 1 && this_player != 2) {
        fprintf(stderr, "Error: Invalid player number %d\n", this_player);
        return EXIT_FAILURE;
    }
    
    // Initialize the state to be used by the agent (read board state)
    State root_state;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (scanf("%d", &root_state.board[i][j]) != 1) {
                fprintf(stderr, "Error: Failed to read board at [%d][%d]\n", i, j);
                return EXIT_FAILURE;
            }
        }
    }
    // Initialize the top array: Count how many stones are already in each column (0-based)
    for (int j = 0; j < COLS; j++) {
        root_state.top[j] = 0;
        for (int i = 0; i < ROWS; i++) {
            if (root_state.board[i][j] != 0)
                root_state.top[j]++;
        }
    }
    // Set the current player
    root_state.player = this_player;
    
    // Use alpha-beta pruning to determine the best move (column number from 0 to COLS-1)
    int best_move = alphabeta_search(&root_state, MAX_DEPTH, this_player);
    if (best_move < 0) {
        fprintf(stderr, "Error: No valid move found.\n");
        return EXIT_FAILURE;
    }
    
    // Convert the selected column number to a character (e.g., 0 -> 'A') and print it
    printf("%c", stack_name(best_move));
    fflush(stdout);
    return EXIT_SUCCESS;
}
