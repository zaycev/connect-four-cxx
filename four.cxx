#include <algorithm>
#include <iostream>
#include <set>
#include <optional>
#include <vector>
#include <tuple>

using namespace std;

// Some typedefs.
using ColorType = size_t;
using IdType = size_t;
using ErrMsgType = string;

static const ColorType EMPTY_COLOR = 0;
static const size_t line_len = 4;
static const size_t line_offset = line_len - 1;
static const size_t line_steps = line_len * 2 - 1;

/**
 * GameState represents current game state.
 **/
struct GameState {
    vector<vector<ColorType>> grid;
    size_t grid_width = 0;
    size_t grid_height = 0;
    vector<tuple<int, int, IdType>> history;

    /**
     * GameState returns initial state for given grid size.
     **/
    static optional<GameState> init_state(size_t width, size_t height);

    /**
      * Returns ID of a player who won the game if game is in the terminal state
      * otherwise returns nothing.
      */
    [[nodiscard]] optional<IdType> is_terminal() const;
};


optional<GameState> GameState::init_state(size_t width, size_t height) {

    if (width == 0 || height == 0) {
        return nullopt;
    }

    GameState state;

    state.grid_width = width;
    state.grid_height = height;
    state.history = vector<tuple<int, int, IdType>>();
    state.grid = vector<vector<ColorType>>();

    for (auto i = 0; i < height; ++i) {
        state.grid.emplace_back(width, EMPTY_COLOR);
    }

    return {state};
}

/**
 * Checks if a line starting at start_row_i,start_col_i contains line_len number of
 * tokens of a same color consecutively.
 *
 * TODO: simplify params.
 */
bool check_line(
        const GameState &state,
        int start_row_i,
        int start_col_i,
        int inc_row,
        int inc_col,
        size_t steps,
        IdType player_id) {

    size_t token_counter = 0;
    int row_id = start_row_i;
    int col_id = start_col_i;

    for (int step_i = 0; step_i < steps; ++step_i) {

        if (state.grid[row_id][col_id] == player_id) {
            token_counter++;
        } else {
            token_counter = 0;
        }

        if (token_counter == line_len) {
            return true;
        }

        col_id += inc_col;
        row_id += inc_row;

        if (row_id > state.grid_height - 1 || row_id < 0) {
            return false;
        }
        if (col_id > state.grid_width - 1 || col_id < 0) {
            return false;
        }

    }

    return false;
}

optional<IdType> GameState::is_terminal() const {

    if (this->history.empty()) {
        return {};
    }

    // Extract last turn data.
    auto last_turn = this->history[this->history.size() - 1];
    int token_row = get<0>(last_turn);
    int token_col = get<1>(last_turn);
    IdType player_id = get<2>(last_turn);

    // Check vertical.
    int v_start_row = max(int(token_row - line_offset), 0);
    int v_start_col = token_col;
    if (check_line(*this, v_start_row, v_start_col, 1, 0, line_steps, player_id)) {
        return {player_id};
    }

    // Check horizontal.
    int h_start_row = token_row;
    int h_start_col = max(int(token_col - line_offset), 0);
    if (check_line(*this, h_start_row, h_start_col, 0, 1, line_steps, player_id)) {
        return {player_id};
    }

    // Check diagonal 1.
    int d1_start_row = max(int(token_row - line_offset), 0);
    int d1_start_col = max(int(token_col - line_offset), 0);

    if (check_line(*this, d1_start_row, d1_start_col, 1, 1, line_steps, player_id)) {
        return {player_id};
    }

    // Check diagonal 2.
    int d2_start_row = max(int(token_row - line_offset), 0);
    int d2_start_col = min(int(token_col + line_offset), int(this->grid_width - 1));

    if (check_line(*this, d2_start_row, d2_start_col, 1, -1, line_steps, player_id)) {
        return {player_id};
    }

    return {};
}

/**
 * Internal function for debugging game state.
 */
void debug_print_state(const GameState &state) {

    cout << "size:     " << state.grid_width << " x " << state.grid_height << endl;
    cout << "turn:     " << state.history.size() << endl;

    if (auto term_result = state.is_terminal()) {
        cout << "terminal: " << "YES (player " << *term_result << " is a winner)" << endl;
    } else {
        cout << "terminal: NO" << endl;
    }

    cout << endl;

    for (auto row_i = 0; row_i < state.grid_height; ++row_i) {
        for (auto col_i = 0; col_i < state.grid_width; ++col_i) {
            auto cell_color = state.grid[row_i][col_i];
            if (cell_color == EMPTY_COLOR) {
                cout << "⬜️";
            } else if (cell_color == 1) {
                cout << "🟢";
            } else if (cell_color == 2) {
                cout << "🔴";
            }
            cout << " ";
        }
        cout << endl;
    }
    cout << endl;
}

/**
 * Traces row index for next token in a given column.
 */
optional<size_t> trace_row_coordinate(GameState &state, size_t col_i) {
    if (col_i < 0 || col_i >= state.grid_width) {
        return {};
    }

    for (int row_i = int(state.grid_height - 1); row_i >= 0; --row_i) {
        if (state.grid[row_i][col_i] == EMPTY_COLOR) {
            return {row_i};
        }
    }
    return {};
}


/**
 *  Updates state of the game. Returns error message in case of invalid input.
 */
optional<ErrMsgType> make_turn(GameState &state, size_t col_i, IdType player_id) {
    // Validate input.
    if (col_i < 0 || col_i >= state.grid_width) {
        return {"column index is outside of the grid range"};
    }

    // Compute coordinates for the new token.
    size_t row_i = 0;
    if (auto row_i_opt = trace_row_coordinate(state, col_i)) {
        row_i = *row_i_opt;
    } else {
        return {"token cannot be placed in a given column as it's full or does not exist"};
    }

    // Update state.
    state.grid[row_i][col_i] = player_id;
    state.history.emplace_back(row_i, col_i, player_id);

    return {};
}

int main() {

    if (auto state_result = GameState::init_state(10, 10)) {
        GameState state = *state_result;

        int turn_counter = 0;
        vector<int> players = {1, 2};

        while (true) {

            int player_id = players[turn_counter % 2];
            int token_col;

            turn_counter++;
            cin >> token_col;

            if (auto err = make_turn(state, token_col, player_id)) {
                cout << "error: " << *err << endl;
                return 0;
            }

            debug_print_state(state);
            if (state.is_terminal()) {
                cout << "gg" << endl;
                break;
            }

        }

    } else {
        cout << "failed to initialize game state";
        return 1;
    }


    return 0;
}
