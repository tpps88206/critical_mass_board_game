#include <iostream>
#include <vector>
#include <queue>
#include <cstring>

const std::string red("\033[0;91m");
const std::string blue("\033[0;94m");
const std::string red_player("\033[0;31m");
const std::string blue_player("\033[0;34m");
const std::string white("\033[0;97m");
const std::string black("\033[0;90m");
const std::string reset("\033[0m");

using namespace std;

enum Color {
    White, // Initial 0
    Blue, // Player One
    Red, // Player Two
    Black // Explosion
};

static int Depth = 4;
static int Round = 100;

class Student {
public:
    void makeMove(int Record[5][6], int Max[5][6], Color color[5][6], Color inputColor) {
        // 將整個棋盤複製一個到 virtual_board 上
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 6; j++) {
                virtual_board[i][j] = Record[i][j];
                virtual_Max[i][j] = Max[i][j];
                virtual_color[i][j] = color[i][j];
            }
        }

        // 使用 minimax 更改 private 的 x, y 值
        int temp = minimax(Record, color, Depth, inputColor);
    }

    int evaluate(int virtual_board[5][6], Color color[5][6]) {
        // 角落分數最高, 再來邊上, 再來中央, 分數為該格子顏色乘以該格子的權重再乘以該格子的棋子數目
        // 顏色定義為藍色1, 紅色-1, 藍色希望越高越好, 紅色希望越低越好
        int corner_weight = 4, edge_weight = 2, center_weight = 1, total_weight = 0;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 6; j++) {
                if (i == 0 || i == 4) {
                    if (j == 0 || j == 5)
                        total_weight += virtual_board[i][j] * corner_weight * player_color(color[i][j]);
                    else
                        total_weight += virtual_board[i][j] * edge_weight * player_color(color[i][j]);
                } else {
                    if (j == 0 || j == 5)
                        total_weight += virtual_board[i][j] * edge_weight * player_color(color[i][j]);
                    else
                        total_weight += virtual_board[i][j] * center_weight * player_color(color[i][j]);
                }
            }
        }
        return total_weight;
    }

    int player_color(Color inputColor) {
        // return 玩家顏色的數字以方便計算分數, 藍色為1, 紅色為-1, 黑色或白色為0
        if (inputColor == Black || inputColor == White)
            return 0;
        else
            return (inputColor == Blue) ? 1 : -1;
    }

    int minimax(int board[5][6], Color board_color[5][6], int depth, Color inputColor) {
        // 目前檯面的評分值, 希望能找到比這值更好的
        int value;
        // 對藍色玩家來說希望越大越好, 對紅色玩家來說希望越小越好, 目前初始值為藍色：-10000, 紅色：10000
        int best_value = -10000 * player_color(inputColor);
        // 當棋子下在virtual_board上, 遇到已經爆炸(黑色), 或敵對顏色時, return一個很差的值, 使得minimax選值的時候不會選到他們
        // 對 Blue 來說會是-10001, 對 Red 來說是10001
        int illegal_value = -10001 * player_color(inputColor);
        // 每當有一步能直接清空對面所有棋子時, 給出一個最好的值, 對Blue來說是10000, 對Red來說是-10000
        int killing_value = 10000 * player_color(inputColor);

        // 終止條件, 如果檯面遊戲已經結束, return 最好的值, 或因為 minimax 深度到0了直接對檯面評分
        if (depth == 0 || game_is_over(board, board_color, inputColor)) {
            if (game_is_over(board, board_color, inputColor))
                return killing_value;
            else
                return evaluate(virtual_board, virtual_color);
        }

        // 對 virtual_board 上每個點下一次, 先判斷我們是藍色還是紅色, 在判斷這步是不是合法的, 若否則得到很差的值
        // 若是則從此節點繼續 minimax 直到深度為零或遊戲結束, 藍色方則會從所有可能中挑分數最高的走, 紅色方會挑所有可能中分數最低的走
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 6; j++) {
                if (inputColor == Blue) {
                    if (is_valid_move(i, j, inputColor)) {
                        move(i, j, Blue, virtual_board, virtual_color);
                        value = minimax(virtual_board, virtual_color, depth - 1, Red);
                        reset_board(board, board_color);
                    } else
                        value = illegal_value;
                    best_value = max(best_value, value);
                } else {
                    if (is_valid_move(i, j, inputColor)) {
                        move(i, j, Red, virtual_board, virtual_color);
                        value = minimax(virtual_board, virtual_color, depth - 1, Blue);
                        reset_board(board, board_color);
                    } else
                        value = illegal_value;
                    best_value = min(best_value, value);
                }
                if (value == best_value && depth == Depth) {
                    x = i;
                    y = j;
                }
            }
        }
        return best_value;
    }

    void explosion(int x, int y, Color inputColor, int board[5][6], Color board_color[5][6]) {
        // 當 x, y 點發生爆炸, 此點變為黑色, 且上下左右各走一步該顏色
        board_color[x][y] = Black;
        move(x + 1, y, inputColor, board, board_color);
        move(x - 1, y, inputColor, board, board_color);
        move(x, y - 1, inputColor, board, board_color);
        move(x, y + 1, inputColor, board, board_color);

    }

    void move(int x, int y, Color inputColor, int board[5][6], Color board_color[5][6]) {
        // 呼叫 move 時該點不能為對手顏色, 因為此函式為了和 explodsion 互相呼叫, 會在 move 該點時自動更新該點顏色為 input 的顏色
        if (x < 5 && x > -1 && y < 6 && y > -1) {
            if (board_color[x][y] != Black) {
                board[x][y]++;
                board_color[x][y] = inputColor;
                if (board[x][y] == virtual_Max[x][y])
                    explosion(x, y, inputColor, board, board_color);
            }
        }
    }

    bool is_valid_move(int x, int y, Color inputColor) {
        // 確認此步是否合法, 非法情況有：1.下在格子外 2.下在黑色(已爆炸)格子上 3.下在對手顏色上, 合法 return true 若非法則 false
        if (x < 5 && x > -1 && y < 6 && y > -1) {
            if ((virtual_color[x][y] == inputColor
                || virtual_color[x][y] == White)
                && virtual_board[x][y] + 1 <= virtual_Max[x][y]
            )
                return true;
        }
        return false;
    }

    void reset_board(int board[5][6], Color board_color[5][6]) {
        // minimax 會將所有情況下過一遍, 故每下了一種情況要將 virtual_board 回復到真正的 Record 的情況才能繼續往下 minimax
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 6; j++) {
                virtual_board[i][j] = board[i][j];
                virtual_color[i][j] = board_color[i][j];
            }
        }
    }

    bool game_is_over(int board[5][6], Color board_color[5][6], Color inputColor) {
        // 兩個條件遊戲結束, 1.檯面上不存在 inputcolor 對手的顏色 2.檯面全部的棋子總和必須至少有兩顆
        // 另一個條件應該用不到 inputcolor 對手沒地方下了
        int counter = 0;
        bool whip_out = true;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 6; j++) {
                counter += board[i][j];
                if (board_color[i][j] == opponent_color(inputColor)) whip_out = false;
            }
        }

        if (counter > 2 && whip_out == true)
            return true;
        else
            return false;
    }

    Color opponent_color(Color inputColor) {
        // return inputcolor 對手的顏色
        return (inputColor == Blue) ? Red : Blue;
    }


    int getX() {
        return x;
    }

    int getY() {
        return y;
    }

private:
    int x;
    int y;
    int virtual_board[5][6];
    int virtual_Max[5][6];
    Color virtual_color[5][6];
};

int main() {
    int x = -1;
    int y = -1;
    int game_board[5][6];
    int game_board_max[5][6];
    Color game_board_color[5][6];
    Color play_one = Blue;
    Color play_two = Red;
    Color current_player = play_one;
    Student player;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            game_board[i][j] = 0;
            game_board_max[i][j] = 3;
            game_board_color[i][j] = White;
        }
    }

    game_board_max[0][0] = game_board_max[0][5] = game_board_max[4][0] = game_board_max[4][5] = 2;
    for (int i = 1; i < 4; i++) {
        for (int j = 1; j < 5; j++) {
            game_board_max[i][j] = 4;
        }
    }

    for (int i = 0; i < Round; i++) {
        // check game status
        if (player.game_is_over(game_board, game_board_color, Blue)) {
            cout << "=====GAME OVER=====\n";
            cout << "=====藍色玩家獲勝=====\n";
            break;
        }

        if (player.game_is_over(game_board, game_board_color, Red)) {
            cout << "=====GAME OVER=====\n";
            cout << "=====紅色玩家獲勝=====\n";
            break;
        }

        // play
        player.makeMove(game_board, game_board_max, game_board_color, current_player);
        x = player.getX();
        y = player.getY();
        player.move(x, y, current_player, game_board, game_board_color);

        // print result
        cout << "[第" << i+1 << "回合] ";
        switch(current_player) {
            case Blue:
                cout << blue_player << "藍色" << reset << "玩家下: " << x << " " << y << " \n";
                break;
            case Red:
                cout << red_player << "紅色" << reset << "玩家下: " << x << " " << y << " \n";
                break;
            default:
                break;
        }

        // print board
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 6; j++) {
                switch(game_board_color[i][j]) {
                    case White:
                        cout << white << game_board[i][j] << reset << " ";
                        break;
                    case Black:
                        cout << black << game_board[i][j] << reset << " ";
                        break;
                    case Blue:
                        cout << blue << game_board[i][j] << reset << " ";
                        break;
                    case Red:
                        cout << red << game_board[i][j] << reset << " ";
                        break;
                    default:
                        break;
                }
            }
            cout << "\n";
        }
        cout << "\n";

        // change player
        if (current_player == play_one)
            current_player = play_two;
        else
            current_player = play_one;
    }

    return 0;
}
