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
std::array<std::array<int, SIZE>, SIZE> curboard;
std::vector<Point> next_valid_spots;
enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
};

const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
}};

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
        disc_count[EMPTY] -= disc_count[WHITE] + disc_count[BLACK];
        cur_player = preMove.cur_player;
        next_valid_spots = preMove.next_valid_spots;
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
        if(find(next_valid_spots.begin(), next_valid_spots.end(), Point(0, 0)) != next_valid_spots.end())
                value += 2;
        if(find(next_valid_spots.begin(), next_valid_spots.end(), Point(0, 7)) != next_valid_spots.end())
            value += 2;
        if(find(next_valid_spots.begin(), next_valid_spots.end(), Point(7, 0)) != next_valid_spots.end())
            value += 2;
        if(find(next_valid_spots.begin(), next_valid_spots.end(), Point(7, 7)) != next_valid_spots.end())
            value += 2;
        if(cur_player == 1){ // black or O
            if(board[0][0] == BLACK) value += 3;
            if(board[0][7] == BLACK) value += 3;
            if(board[7][0] == BLACK) value += 3;
            if(board[7][7] == BLACK) value += 3;
        }
        else{ // white or X
            if(board[0][0] == WHITE) value += 3;
            if(board[0][7] == WHITE) value += 3;
            if(board[7][0] == WHITE) value += 3;
            if(board[7][7] == WHITE) value += 3;
        }
    }
};

int minimax(Status curMove, int depth, int a, int b){
    if(depth == 0){
        curMove.setValue();
        return curMove.value;
    }
    int value;
    curMove.get_valid_spots();
    if(curMove.cur_player == player){
        value = INF;
        for(Point step : curMove.next_valid_spots){
            Status nxtMove = curMove;
            nxtMove.flip_discs(step);
            nxtMove.cur_player = 3-curMove.cur_player;
            value = std::min(value, minimax(nxtMove, depth-1, a, b));
            a = std::max(a, value);
            if(a >= b) break;
        }
    }
    else{
        value = -INF;
        for(Point step : curMove.next_valid_spots){
            Status nxtMove = curMove;
            nxtMove.flip_discs(step);
            nxtMove.cur_player = 3-curMove.cur_player;
            value = std::max(value, minimax(nxtMove, depth-1, a, b));
            b = std::min(b, value);
            if(a >= b) break;
        }
    }

    return value;
}


void read_board() {
    std::cin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            std::cin >> curboard[i][j];
        }
    }
}

void read_valid_spots() {
    int n_valid_spots;
    std::cin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        std::cin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot() {
    Status curMove;
    curMove.disc_count[EMPTY] = 64;
    curMove.disc_count[BLACK] = 0;
    curMove.disc_count[WHITE] = 0;
    curMove.cur_player = player;
    for (int i = 0; i < SIZE; i++) {
        // curMove.next_valid_spots[i] = next_valid_spots[i];
        for (int j = 0; j < SIZE; j++) {
            curMove.board[i][j] = curboard[i][j];
            if(curboard[i][j] == BLACK) curMove.disc_count[BLACK]++;
            else if(curboard[i][j] == WHITE) curMove.disc_count[WHITE]++;
        }
    }
    int value = -INF;
    for(Point step : next_valid_spots){
        /*int n_valid_spots = next_valid_spots.size();
        srand(time(NULL));
        int index = (rand() % n_valid_spots);
        Point p = next_valid_spots[index];
        fout << p.x << " " << p.y << std::endl;
        fout.flush();*/
        Status nxtMove = curMove;
        nxtMove.flip_discs(step);
        nxtMove.cur_player = 3-curMove.cur_player;
        int v = minimax(nxtMove, 0, -INF, INF);
        // printf("v: %d\n", v);
        if(v > value){
            value = v;
            std::cout << step.x << " " << step.y << std::endl;
        }
    }
}

int main(int, char** argv) {
    read_board();
    read_valid_spots();
    write_valid_spot();
    return 0;
}

