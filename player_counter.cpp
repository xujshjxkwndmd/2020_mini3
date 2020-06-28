#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <stdio.h>
#define INF 2147483647

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

int player;
const int SIZE = 8;
#define corner 500
#define edge 15
#define inEgde 0
#define central 0
#define midEdge 0
#define danger -30
int score[8][8] = {
    { corner, danger, edge, edge, edge, edge, danger, corner},
    { danger, danger, midEdge, midEdge, midEdge, midEdge, danger, danger},
    { edge, midEdge, inEgde, inEgde, inEgde, inEgde, midEdge, edge},
    { edge, midEdge, inEgde, central, central, inEgde, midEdge, edge},
    { edge, midEdge, inEgde, central, central, inEgde, midEdge, edge},
    { edge, midEdge, inEgde, inEgde, inEgde, inEgde, midEdge, edge},
    { danger, danger, midEdge, midEdge, midEdge, midEdge, danger, danger},
    { corner, danger, edge, edge, edge, edge, danger, corner},
};
std::array<std::array<int, SIZE>, SIZE> curboard;
std::vector<Point> next_valid_spots;
enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
};

const std::array<Point, 8> directions{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
};


class Status{
public:

    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    int value;

    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    Status() {
        reset();
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8*8-4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        value = 0;
    }
    Status(const Status& preMove){
        disc_count[EMPTY] = 64;
        disc_count[BLACK] = 0;
        disc_count[WHITE] = 0;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = preMove.board[i][j];
                if(board[i][j] == BLACK) disc_count[BLACK]++;
                else if(board[i][j] == WHITE) disc_count[WHITE]++;
            }
        }
        disc_count[WHITE] = preMove.disc_count[WHITE];
        disc_count[BLACK] = preMove.disc_count[BLACK];
        disc_count[EMPTY] -= disc_count[WHITE] + disc_count[BLACK];
        cur_player = preMove.cur_player;
        // next_valid_spots = get_valid_spots();
        value = 0;
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    void setValue(){
        value = 0;
        /* game end */
        if(disc_count[EMPTY] == 0){
            if(disc_count[cur_player] > disc_count[3-cur_player])
                value = 100000;
            else if(disc_count[cur_player] < disc_count[3-cur_player])
                value = -100000;
            return;
        }

        /* next value */
        // available moves
        if(next_valid_spots.size() == 0){
            value = -100000;
            return;
        }
        value += next_valid_spots.size()/2;
        // find corner
        if(find(next_valid_spots.begin(), next_valid_spots.end(), Point(0, 0)) != next_valid_spots.end())
            value += 50;
        if(find(next_valid_spots.begin(), next_valid_spots.end(), Point(0, 7)) != next_valid_spots.end())
            value += 50;
        if(find(next_valid_spots.begin(), next_valid_spots.end(), Point(7, 0)) != next_valid_spots.end())
            value += 50;
        if(find(next_valid_spots.begin(), next_valid_spots.end(), Point(7, 7)) != next_valid_spots.end())
            value += 50;
        // find edges
        /*for(Point step : next_valid_spots){
            if(i == 0 || i == 7) value += 10;
            if(j == 0 || j == 7) value += 10;
        }
            if(step.x == 0 || step.x == 7) value += 10;
            if(step.y == 0 || step.y == 7) value += 10;
        }*/

        /* now value */
        // location value
        for(int i = 0; i < 8; ++i){
            for(int j = 0; j < 8; ++j){
                if(board[i][j] == cur_player){
                    // if(i == 0 || i == 7) value += 10;
                    // if(j == 0 || j == 7) value += 10;
                    value += score[i][j];
                }
            }
        }
        // find wall
        if(board[0][0] == cur_player){
            for(int i = 1; i < 8; ++i){
                if(board[0][i] != cur_player)
                    break;
                value += 30;
            }
            for(int i = 1; i < 8; ++i){
                if(board[i][0] != cur_player)
                    break;
                value += 30;
            }
            for(int i = 1; i < 8; ++i){
                if(board[i][i] != cur_player)
                    break;
                value += 50;
            }
        }
        if(board[0][7] == cur_player){
            for(int i = 1; i < 8; ++i){
                if(board[0][i] != cur_player)
                    break;
                value += 30;
            }
            for(int i = 1; i < 8; ++i){
                if(board[i][7] != cur_player)
                    break;
                value += 30;
            }
            for(int i = 1; i < 8; ++i){
                if(board[i][7-i] != cur_player)
                    break;
                value += 50;
            }
        }
        if(board[7][0] == cur_player){
            for(int i = 1; i < 8; ++i){
                if(board[7][i] != cur_player)
                    break;
                value += 30;
            }
            for(int i = 1; i < 8; ++i){
                if(board[i][0] != cur_player)
                    break;
                value += 30;
            }
            for(int i = 1; i < 8; ++i){
                if(board[7-i][i] != cur_player)
                    break;
                value += 50;
            }
        }
        if(board[7][7] == cur_player){
            for(int i = 1; i < 8; ++i){
                if(board[7][i] != cur_player)
                    break;
                value += 30;
            }
            for(int i = 1; i < 8; ++i){
                if(board[i][7] != cur_player)
                    break;
                value += 30;
            }
            for(int i = 1; i < 8; ++i){
                if(board[7-i][7-i] != cur_player)
                    break;
                value += 50;
            }
        }
        // discs amount
        value += disc_count[cur_player];
        /*if(disc_count[EMPTY] > 29){
            value -= disc_count[cur_player];
        }
        else{
            value += disc_count[cur_player]*2;
        }*/

    }
};

int minimax(Status curMove, int depth, int a, int b){
    if(curMove.disc_count[EMPTY] == 0){
        if(curMove.disc_count[curMove.cur_player] > curMove.disc_count[3-curMove.cur_player])
            return INF;
        else if(curMove.disc_count[curMove.cur_player] < curMove.disc_count[3-curMove.cur_player])
            return -INF;
        return 0;
    }
    curMove.next_valid_spots = curMove.get_valid_spots();
    if(depth == 0){
        curMove.setValue();
        return curMove.value;
    }
    int value;
    if(curMove.cur_player == player){
        value = INF;
        for(Point step : curMove.next_valid_spots){
            Status nxtMove = curMove;
            nxtMove.flip_discs(step);
            nxtMove.board[step.x][step.y] = curMove.cur_player;
            nxtMove.cur_player = 3-curMove.cur_player;
            value = std::min(value, minimax(nxtMove, depth-1, a, b));
            a = std::max(a, value);
            if(a >= b) break;
        }
        if(curMove.next_valid_spots.size() == 0)
            value = -INF;
    }
    else{
        value = -INF;
        for(Point step : curMove.next_valid_spots){
            Status nxtMove = curMove;
            nxtMove.flip_discs(step);
            nxtMove.board[step.x][step.y] = curMove.cur_player;
            nxtMove.cur_player = 3-curMove.cur_player;
            value = std::max(value, minimax(nxtMove, depth-1, a, b));
            b = std::min(b, value);
            if(a >= b) break;
        }
    }

    return value;
}

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> curboard[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream& fout) {
    Status curMove;
    curMove.disc_count[EMPTY] = 64;
    curMove.disc_count[BLACK] = 0;
    curMove.disc_count[WHITE] = 0;
    curMove.cur_player = player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            curMove.board[i][j] = curboard[i][j];
            if(curboard[i][j] == BLACK) curMove.disc_count[BLACK]++;
            else if(curboard[i][j] == WHITE) curMove.disc_count[WHITE]++;
        }
    }
    int value = INF;
    int x, y;
    /*for(Point step : next_valid_spots){
        Status nxtMove = curMove;
        nxtMove.flip_discs(step);
        nxtMove.cur_player = 3-curMove.cur_player;
        int v = minimax(nxtMove, 1, -INF, INF);
        if(v >= value){
            value = v;
            x = step.x, y = step.y;
            // fout << step.x << " " << step.y << std::endl;
            // fout.flush();
        }
    }*/
    for(Point step : next_valid_spots){
        Status nxtMove = curMove;
        nxtMove.flip_discs(step);
        nxtMove.board[step.x][step.y] = curMove.cur_player;
        nxtMove.cur_player = 3-curMove.cur_player;
        int v = minimax(nxtMove, 3, -INF, INF);
        if(v <= value){
            value = v;
            x = step.x, y = step.y;
            // fout << step.x << " " << step.y << std::endl;
            // fout.flush();
        }
    }
    fout << x << " " << y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
