#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <chrono>
#include <array>
#include <fstream>
#include <cmath>
#include <random>
#include <set>
#include <sstream>
#include "json.h"
#include "json-forwards.h"

/*alphabeta part*/
#define GRIDSIZE 15
#define grid_blank 0
#define grid_black 1 
#define grid_white -1

using namespace std;
//����������
const int INF = numeric_limits<int>::max();

//����dege
int up_edge = GRIDSIZE;
int down_edge = -1;
int left_edge = GRIDSIZE;
int right_edge = -1;

int currBotColor;
int gridInfo[GRIDSIZE][GRIDSIZE] = { 0 };

class AlphabetaAI {
public:
    struct Step {
        int x;
        int y;
    };
    static int ChessPatternAnalysis(string line);
    static int PointEvaluation(Step step);
    static void AllLegalSteps();
    static int SimulatedStepEvaluate(int player);
    static int LineValue(Step step, int player);
    static int AlphaBeta(int alpha, int beta, int player, int depth);
};





struct LegalSteps {
    AlphabetaAI::Step step;
    int value = 0;
};
vector<LegalSteps> legal_step;//���µ�
vector<AlphabetaAI::Step> simulate_step;//ģ���µ�
AlphabetaAI::Step decided2steps[2];

//ʱ������
auto BeginTime = chrono::high_resolution_clock::now();
bool TimeOut() {
    auto RightNow = chrono::high_resolution_clock::now();
    auto Duration = chrono::duration_cast<chrono::milliseconds>(RightNow - BeginTime).count();
    return Duration >= 998;//����ʱ��Ϊ998ms
}

inline bool inMap(int x, int y);
bool ProcStep(int x0, int y0, int x1, int y1, int grid_color, bool check_only);

void ExpandEdge();
bool cmp(LegalSteps& a, LegalSteps& b);

inline bool inMap(int x, int y)
{
    if (x < 0 || x >= GRIDSIZE || y < 0 || y >= GRIDSIZE)
        return false;
    return true;
}
bool ProcStep(int x0, int y0, int x1, int y1, int grid_color, bool check_only)
{
    if (x1 == -1 || y1 == -1) {
        if (!inMap(x0, y0) || gridInfo[x0][y0] != grid_blank)
            return false;
        if (!check_only) {
            gridInfo[x0][y0] = grid_color;
        }
        return true;
    }
    else {
        if ((!inMap(x0, y0)) || (!inMap(x1, y1)))
            return false;
        if (gridInfo[x0][y0] != grid_blank || gridInfo[x1][y1] != grid_blank)
            return false;
        if (!check_only) {
            gridInfo[x0][y0] = grid_color;
            gridInfo[x1][y1] = grid_color;
        }
        return true;
    }
}

int AlphabetaAI::ChessPatternAnalysis(string line) {//���ͷ���
    int myscores[5] = { 140, 315, 350, 70000, 70000 };//���ҷ���ֵ
    int enemyscores[5] = { 7, 105, 210, 65000, 65000 };//�з���вֵ
    int value = 0;
    int length = line.length();

    while (length < 6) {
        line += 'x';
        length++;
    }
    for (int i = 0; i <= length - 6; ++i) {//ÿ6��Ϊһ��
        int my = 0;
        int enemy = 0;
        for (int j = i; j < i + 6; ++j) {
            if (line[j] == 'i') {//I
                my++;
            }
            else if (line[j] == 'e') {//Enemy
                enemy++;
            }
        }
        if (my != 0 && enemy == 0) {//��Ϊ0
            value += myscores[my - 1];
        }
        if (my == 0 && enemy != 0) {//��Ϊ0
            value += enemyscores[enemy - 1];
        }
    }
    return value;
}


int AlphabetaAI::PointEvaluation(Step step) {//��point�������֣��������ͣ�����string�󴫸����η�������ChessPatternAnalysis
    //����
    string shu_line;
    for (int i = up_edge; i <= down_edge; ++i) {
        if ((step.y - i) < 6 && (step.y - i) > -6) {
            if (gridInfo[step.x][i] == currBotColor) {
                shu_line += 'i';//I
            }
            if (gridInfo[step.x][i] == -currBotColor) {
                shu_line += 'e';//Enemy
            }
            if (gridInfo[step.x][i] == grid_blank) {
                shu_line += 'o';//��
            }
        }
    }
    //����
    string heng_line;
    for (int i = left_edge; i <= right_edge; ++i) {
        if ((step.x - i) < 6 && (step.x - i) > -6) {
            if (gridInfo[i][step.y] == currBotColor) {
                heng_line += 'i';//I
            }
            if (gridInfo[i][step.y] == -currBotColor) {
                heng_line += 'e';//Enemy
            }
            if (gridInfo[i][step.y] == grid_blank) {
                heng_line += 'o';//��
            }
        }
    }

    string na_line;//����
    int left_up = (step.y - up_edge > step.x - left_edge) ? step.y - up_edge : step.x - left_edge;
    int right_down = (down_edge - step.y > right_edge - step.x) ? down_edge - step.y : right_edge - step.x;
    for (int i = -left_up; i <= right_down; ++i) {
        if (i < 6 && i>-6) {
            int x = step.x + i;
            int y = step.y + i;
            if (y >= up_edge && y <= down_edge && x >= left_edge && x <= right_edge) {
                if (gridInfo[x][y] == currBotColor) {
                    na_line += 'i';//I
                }
                if (gridInfo[x][y] == -currBotColor) {
                    na_line += 'e';//Enemy
                }
                if (gridInfo[x][y] == grid_blank) {
                    na_line += 'o';//��
                }
            }

        }
    }

    string pie_line;//Ʋ��
    int left_down = (down_edge - step.y > step.x - left_edge) ? down_edge - step.y : step.x - left_edge;
    int right_up = (step.y - up_edge > right_edge - step.x) ? step.y - up_edge : right_edge - step.x;
    for (int i = -left_down; i <= right_up; ++i) {
        if (i < 6 && i>-6) {
            int x = step.x + i;
            int y = step.y - i;
            if (y >= up_edge && y <= down_edge && x >= left_edge && x <= right_edge) {
                if (gridInfo[x][y] == currBotColor) {
                    pie_line += 'i';//I
                }
                if (gridInfo[x][y] == -currBotColor) {
                    pie_line += 'e';//Enemy
                }
                if (gridInfo[x][y] == grid_blank) {
                    pie_line += 'o';//��
                }
            }

        }
    }

    return ChessPatternAnalysis(shu_line) + ChessPatternAnalysis(heng_line) + ChessPatternAnalysis(na_line) + ChessPatternAnalysis(pie_line);

}



bool cmp(LegalSteps& a, LegalSteps& b) {
    return a.value > b.value;
}

void AlphabetaAI::AllLegalSteps() {//�ڱ߽������ɳ����п��µĵط���Ȼ������
    for (int x = left_edge; x <= right_edge; ++x) {
        for (int y = up_edge; y <= down_edge; ++y) {
            if (gridInfo[x][y] == grid_blank) {
                Step step{ x,y };
                LegalSteps temp;
                temp.value = PointEvaluation(step);
                temp.step = step;
                legal_step.push_back(temp);
            }
        }
    }
    sort(legal_step.begin(), legal_step.end(), cmp);
}






int AlphabetaAI::LineValue(Step step, int player) {//alphabetaģ�����������
    int my_value[6] = { 7, 175, 350, 28000, 28000, 7000000 };//�ҷ���ֵ

    int enemy_value[6] = { 7, 140, 280, 1400, 49000, 6300000 };//�з���в

    int values[4] = { 0 };

    string heng_line;
    string shu_line;
    string pie_line;
    string na_line;

    vector<string> lines;

    for (int i = left_edge; i <= right_edge; ++i) {
        if ((step.x - i) < 6 && (step.x - i) > -6) {
            if (gridInfo[i][step.y] == player) {
                heng_line += 'i';//I
            }
            if (gridInfo[i][step.y] == -player) {
                heng_line += 'e';//Enemy
            }
            if (gridInfo[i][step.y] == grid_blank) {
                heng_line += 'o';//��
            }
        }
    }
    lines.push_back(heng_line);


    for (int i = up_edge; i <= down_edge; ++i) {
        if ((step.y - i) < 6 && (step.y - i) > -6) {
            if (gridInfo[step.x][i] == player) {
                shu_line += 'i';//I
            }
            if (gridInfo[step.x][i] == -player) {
                shu_line += 'e';//Enemy
            }
            if (gridInfo[step.x][i] == grid_blank) {
                shu_line += 'o';//��
            }
        }
    }

    lines.push_back(shu_line);



    int left_down = (down_edge - step.y > step.x - left_edge) ? down_edge - step.y : step.x - left_edge;
    int right_up = (step.y - up_edge > right_edge - step.x) ? step.y - up_edge : right_edge - step.x;
    for (int i = -left_down; i <= right_up; ++i) {
        if (i < 6 && i>-6) {
            int x = step.x + i;
            int y = step.y - i;
            if (y >= up_edge && y <= down_edge && x >= left_edge && x <= right_edge) {
                if (gridInfo[x][y] == player) {
                    pie_line += 'i';//I
                }
                if (gridInfo[x][y] == -player) {
                    pie_line += 'e';//Enemy
                }
                if (gridInfo[x][y] == grid_blank) {
                    pie_line += 'o';//��
                }
            }

        }
    }
    lines.push_back(pie_line);


    int left_up = (step.y - up_edge > step.x - left_edge) ? step.y - up_edge : step.x - left_edge;
    int right_down = (down_edge - step.y > right_edge - step.x) ? down_edge - step.y : right_edge - step.x;
    for (int i = -left_up; i <= right_down; ++i) {
        if (i < 6 && i>-6) {
            int x = step.x + i;
            int y = step.y + i;
            if (y >= up_edge && y <= down_edge && x >= left_edge && x <= right_edge) {
                if (gridInfo[x][y] == player) {
                    na_line += 'i';//I
                }
                if (gridInfo[x][y] == -player) {
                    na_line += 'e';//Enemy
                }
                if (gridInfo[x][y] == grid_blank) {
                    na_line += 'o';//��
                }
            }

        }
    }
    lines.push_back(na_line);

    //�Ѵ�õ�4��stringѭ�������
    for (int x = 0; x < 4; ++x) {

        int n = lines[x].length();
        while (n < 6) {
            lines[x] += 'x';
            n++;
        }
        for (int i = 0; i <= n - 6; ++i) {
            int my_amount = 0;
            int enemy_amount = 0;
            for (int j = i; j < i + 6; ++j) {
                if ((lines[x])[j] == 'i') {
                    my_amount++;
                }
                else if ((lines[x])[j] == 'e') {
                    enemy_amount++;
                }
            }
            if (enemy_amount == 0) {
                if (my_amount > 1) {
                    values[x] = values[x] - my_value[my_amount - 2] + my_value[my_amount - 1];
                }
                else if (my_amount > 0) {
                    values[x] = values[x] + my_value[my_amount - 1];
                }
            }
            else {
                if (my_amount == 0) {
                    values[x] = values[x] - enemy_value[enemy_amount - 1] + enemy_value[enemy_amount - 2];
                }
                else if (my_amount - 1 == 0) {
                    values[x] = values[x] + enemy_value[enemy_amount - 2];
                }
            }
        }
    }
    return values[0] + values[1] + values[2] + values[3];
}

int AlphabetaAI::SimulatedStepEvaluate(int player) {
    int nums = (int)simulate_step.size();
    int score = 0;
    for (int i = 0; i < nums; ++i) {
        Step step = simulate_step[i];
        score = score + LineValue(step, player);
    }
    return score;
}


int AlphabetaAI::AlphaBeta(int alpha, int beta, int player, int depth) {
    if (TimeOut()) {//ʱ�䵽�˾��˳�
        return 0;
    }
    if (depth == 0) {//���
        return SimulatedStepEvaluate(player);
    }

    int num = (int)legal_step.size();
    for (int i = 0; i < 11 && i < num; ++i) {//��֧�����������ƣ���ѧ11
        //��һ������ģ��
        Step step1 = legal_step[i].step;

        if (gridInfo[step1.x][step1.y] == grid_black || gridInfo[step1.x][step1.y] == grid_white) {
            continue;
        }

        simulate_step.push_back(step1);//����
        gridInfo[step1.x][step1.y] = player;

        for (int j = i; j < 11 && j < num; ++j) {//��ѧ���η�֧��Ϊ11
            //�ڶ���ģ������
            Step step2 = legal_step[j].step;
            if (gridInfo[step2.x][step2.y] == grid_black || gridInfo[step2.x][step2.y] == grid_white) {
                continue;
            }

            simulate_step.push_back(step2);//����
            gridInfo[step2.x][step2.y] = player;

            int value = -AlphaBeta(-beta, -alpha, -player, depth - 1);//���-1

            //���� �ڶ���
            gridInfo[step2.x][step2.y] = grid_blank;
            simulate_step.pop_back();

            //���⵽�������֦
            if (value > beta || value == beta) {
                //���˵�һ��
                gridInfo[step1.x][step1.y] = grid_blank;
                simulate_step.pop_back();

                return beta;
            }
            if (value > alpha) {
                alpha = value;
                if (depth == 2) {
                    decided2steps[0] = step1;
                    decided2steps[1] = step2;
                }
            }
        }
        //���˵�һ������
        simulate_step.pop_back();
        gridInfo[step1.x][step1.y] = grid_blank;
    }
    //return���
    return alpha;
}


void ExpandEdge() {
    if (up_edge == 1) {
        up_edge = up_edge - 1;
    }
    else if (up_edge > 1) {
        up_edge = up_edge - 2;
    }
    if (down_edge == 13) {
        down_edge = down_edge + 1;
    }
    else if (down_edge < 13) {
        down_edge = down_edge + 2;
    }
    if (left_edge == 1) {
        left_edge = left_edge - 1;
    }
    else if (left_edge > 1) {
        left_edge = left_edge - 2;
    }
    if (right_edge == 13) {
        right_edge = right_edge + 1;
    }
    else if (right_edge < 13) {
        right_edge = right_edge + 2;
    }
}


/*alphabeta part*/



/*MCTS part*/
#define LENGTH 15
#define WIN 314159265357
#define DEPTH 12
#define C 1.5
#define A 0.3
#define K 0.2
#define TIMELIMIT 980
#define VIGILANCELIMIT 1000.0
using namespace std;
typedef long long ll;
static constexpr int BREADTH[DEPTH + 1] = { 2, 3, 3, 4, 4, 5, 5, 6, 6, 8, 8, 12, 12 };
double dynamic_C = C;

//�޿ռ�������
static constexpr ll VIGILANCE_Self[6][3] = { {1, 1, 1},
                                             {1, 1, 1},
                                             {1, 1, 3},
                                             {1, 3, 12},
                                             {1, 100, 10030},
                                             {1, 10080, 10080} };

//�޿ռ�������
static constexpr ll VIGILANCE_Opponent[6][3] = { {1, 1, 1},
                                                 {1, 1, 1},
                                                 {1, 1, 2},
                                                 {1, 4, 10},
                                                 {1, 110, 10100},
                                                 {1, 10050, 10100} };

//���пռ�������
static constexpr ll HOPEDVIGILANCE_Self[6][3] = { {1, 1, 1},
                                                  {1, 1, 1},
                                                  {1, 1, 1},
                                                  {1, 3, 5},
                                                  {1, 115, 120},
                                                  {900, 960, 1050} };


//���пռ�������
static constexpr ll HOPEDVIGILANCE_Opponent[6][3] = { {1, 1, 1},
                                                      {1, 1, 1},
                                                      {1, 1, 1},
                                                      {1, 2, 4},
                                                      {1, 110, 120},
                                                      {900, 940, 1050} };

//�������
enum PlayerType {
    Self = 0,
    Opponent = 1,
    Blank = 2,
    Outside = 3,
};

//����
class Stone {
public:
    Stone() {}
    Stone(const int& x = -1, const int& y = -1) : x(x), y(y) {}
    Stone(const Stone& tmp) :x(tmp.x), y(tmp.y) {}

    Stone operator+(const Stone& tmp) const {
        return Stone(x + tmp.x, y + tmp.y);
    }
    Stone operator-(const Stone& tmp) const {
        return Stone(x - tmp.x, y - tmp.y);
    }
    Stone operator-() const {
        return Stone(-x, -y);
    }
    Stone operator*(const int& k) const {
        return Stone(x * k, y * k);
    }
    friend Stone operator*(const int& k, const Stone& tmp) {
        return tmp * k;
    }
    bool operator==(const Stone& tmp) const {
        return (x == tmp.x && y == tmp.y);
    }
    bool operator!=(const Stone& tmp) const {
        return (x != tmp.x || y != tmp.y);
    }
    bool operator<(const Stone& tmp) const {
        return x == tmp.x ? y < tmp.y : x < tmp.x;
    }

    int x, y;
};

//����·��˼·����״̬����
class State {
public:
    enum left_or_right { left = 0, right = 1 };
    //0Ϊ��1Ϊ��

    PlayerType player;

    int connected_len[2];
    //��ʾ�ڸ÷�����������ͬһ������ӵ�������
    int hoped_connected_len[2];
    // ��ʾ���˿��ܴ���һ���հ׵�Ԫ���⣬�÷�����������ͬһ������ӵ�������hoped����
    bool is_len_next_blank[2];
    //��ʾ��÷�����������ͬһ����������ڵ���һ����Ԫ���Ƿ�Ϊ�հף�Blank)��
    bool is_hopedlen_next_blank[2];
    //��ʾ��÷�����hoped������ͬһ����������ڵ���һ����Ԫ���Ƿ�Ϊ�հף�Blank)��

public:
    State() {
        player = Blank;
        for (int i = 0; i < 2; i++) {
            connected_len[i] = hoped_connected_len[i] = 0;
            is_len_next_blank[i] = is_hopedlen_next_blank[i] = false;
        }
    }

    State(const State& tmp) {
        player = tmp.player;
        for (int i = 0; i < 2; i++) {
            connected_len[i] = tmp.connected_len[i];
            hoped_connected_len[i] = tmp.hoped_connected_len[i];
            is_len_next_blank[i] = tmp.is_len_next_blank[i];
            is_hopedlen_next_blank[i] = tmp.is_hopedlen_next_blank[i];
        }
    }

    State operator=(const State& tmp) {
        return State(tmp);
    }

    void update_state(const State& neighbour, const PlayerType& neighbour_player, const left_or_right& lr) {
        if (neighbour_player == Blank) {
            connected_len[lr] = 0;
            hoped_connected_len[lr] = neighbour.connected_len[lr] + 1;
            is_len_next_blank[lr] = true;
            is_hopedlen_next_blank[lr] = neighbour.is_len_next_blank[lr];
        }
        else if (neighbour_player == player) {
            connected_len[lr] = neighbour.connected_len[lr] + 1;
            hoped_connected_len[lr] = neighbour.hoped_connected_len[lr] + 1;
            is_len_next_blank[lr] = neighbour.is_len_next_blank[lr];
            is_hopedlen_next_blank[lr] = neighbour.is_hopedlen_next_blank[lr];
        }
        else if (neighbour_player == (player ^ 1)) {
            connected_len[lr] = hoped_connected_len[lr] = 0;
            is_len_next_blank[lr] = is_hopedlen_next_blank[lr] = false;
        }
    }

    ll evaluate_weight(const PlayerType& player) const {
        const int total_connect_len = connected_len[left] + connected_len[right] + 1;
        if (total_connect_len >= 6) { return WIN; }
        const int left_hoped_len = hoped_connected_len[left] + connected_len[right];
        const int right_hoped_len = hoped_connected_len[right] + connected_len[left];

        if (player == Self) {
            return max({ VIGILANCE_Self[total_connect_len][is_len_next_blank[left] + is_len_next_blank[right]],
                         HOPEDVIGILANCE_Self[(left_hoped_len < 5) ? left_hoped_len : 5][is_len_next_blank[right] + is_hopedlen_next_blank[left]],
                         HOPEDVIGILANCE_Self[(right_hoped_len < 5) ? right_hoped_len : 5][is_len_next_blank[left] + is_hopedlen_next_blank[right]] });
        }
        else {
            return max({ VIGILANCE_Opponent[total_connect_len][is_len_next_blank[left] + is_len_next_blank[right]],
                         HOPEDVIGILANCE_Opponent[(left_hoped_len < 5) ? left_hoped_len : 5][is_len_next_blank[right] + is_hopedlen_next_blank[left]],
                         HOPEDVIGILANCE_Opponent[(right_hoped_len < 5) ? right_hoped_len : 5][is_len_next_blank[left] + is_hopedlen_next_blank[right]] });

        }
    }

    void state_init(const left_or_right& lr, const bool& extra = false) {
        is_len_next_blank[lr] = true;
        hoped_connected_len[lr] = 1;
        if (extra) {
            is_hopedlen_next_blank[lr] = true;
        }
    }
};

//����
class Board {
public:
    const Stone DIR[4] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };

    struct Move {
        Stone stone;
        ll weight;
        int distance_from_center;

        int get_distance_from_center(const Stone& stone) {
            int center = LENGTH >> 1;
            int x_distance = abs(stone.x - center) + 1;
            int y_distance = abs(stone.y - center) + 1;
            return x_distance * y_distance;
        }

        Move(const Stone& stone, const ll& weight) :
            stone(stone), weight(weight) {
            distance_from_center = get_distance_from_center(stone);
        }

        Move(Board* board, const Stone& stone) :
            Move(stone, max(board->get_evaluation(stone, Self), board->get_evaluation(stone, Opponent))) {
        }

        //����<,set������
        bool operator<(const Move& tmp) const {
            if (weight != tmp.weight) {
                return weight > tmp.weight;
            }
            else if (distance_from_center != tmp.distance_from_center) {
                return distance_from_center < tmp.distance_from_center;
            }
            else {
                return stone < tmp.stone;
            }
        }
    };

    struct Node {
        PlayerType player;
        //��ʾ��ǰ�ڵ��Ӧ����ң����ڸýڵ��¸���ҽ�����һ������
        PlayerType end;
        //��ʾ��Ϸ����ʱ��״̬����ʼ��Ϊ Blank����ʾ��Ϸδ����
        int visited_times;
        //��¼�ýڵ㱻���ʵĴ���
        int win_times;
        //��¼�ڸýڵ��л�ʤ�Ĵ���
        Board::Move move1;
        //�洢��ǰ�ڵ�ĵ�һ���ƶ�
        Board::Move move2;
        //ָ��ǰ�ڵ�ĵڶ����ƶ�
        Node* parent;
        vector<Node* > children;

        Node(const PlayerType& player, const Board::Move& move1, const Board::Move& move2, Node* parent = nullptr, const PlayerType& end = Blank)
            : player(player), end(end), visited_times(0), win_times(0), move1(std::move(move1)), move2(std::move(move2)), parent(parent) {}


        //���½ڵ�ķ��ʴ����ͻ�ʤ����
        void update_node(const PlayerType& loser) {
            visited_times += 2;
            if (loser == Blank) {
                win_times++;
            }
            else if (loser == (player ^ 1)) {
                win_times += 2;
            }
        }

        double UCB() const {
            return (double)win_times / visited_times + dynamic_C * sqrt(log(parent->visited_times) / visited_times);
        }

    };

    explicit Board() :start_time(chrono::steady_clock::now()) {
        int x_index, y_index, dir;
        for (x_index = 0; x_index < LENGTH; x_index++) {
            for (y_index = 0; y_index < LENGTH; y_index++) {
                //initialization
                allstones_current_players[x_index][y_index] = Blank;
                const Stone current_stone(x_index, y_index);

                //�ĸ�����
                for (dir = 0; dir < 4; dir++) {
                    State& self = get_state(current_stone, dir)[Self];
                    State& opp = get_state(current_stone, dir)[Opponent];
                    self.player = Self;
                    opp.player = Opponent;

                    if (is_stone_inboard(current_stone - DIR[dir])) {
                        if (is_stone_inboard(current_stone - 2 * DIR[dir])) {
                            self.state_init(State::left, true);
                            opp.state_init(State::left, true);
                        }
                        else {
                            self.state_init(State::left);
                            opp.state_init(State::left);
                        }
                    }

                    if (is_stone_inboard(current_stone + DIR[dir])) {
                        if (is_stone_inboard(current_stone + 2 * DIR[dir])) {
                            self.state_init(State::right, true);
                            opp.state_init(State::right, true);
                        }
                        else {
                            self.state_init(State::right);
                            opp.state_init(State::right);
                        }

                    }

                    evaluate_state(current_stone, Self);
                    evaluate_state(current_stone, Opponent);
                    scenario_moves_set.emplace(this, current_stone);
                }
            }
        }
    }

private:
    array<array<PlayerType, LENGTH>, LENGTH> allstones_current_players;
    //��¼ÿ��λ�õ�player

    array<array<array<array<State, 2>, 4>, LENGTH>, LENGTH> allstones_current_state;
    //��¼ÿ��λ�����ĸ������ϵ�state����Ϊself��opponent

    array<array<array<ll, 2>, LENGTH>, LENGTH> allstones_current_evaluation;
    //��¼ÿ��λ�õ�����state��evaluate_weight���,��Ϊself��opponent

    set<Move> scenario_moves_set;
    //feasible�����ƶ�

    chrono::time_point<chrono::steady_clock> start_time;
    //��¼��ʼ��ʱ��

public:
    //�ж�����λ���Ƿ���������
    static bool is_stone_inboard(const Stone& stone) {
        int x = stone.x;
        int y = stone.y;
        return (x >= 0 && x < LENGTH && y >= 0 && y < LENGTH);
    }

    //�õ����λ�õ�stone���ڵ�player
    PlayerType get_player(const Stone& stone) const {
        if (is_stone_inboard(stone)) {
            return allstones_current_players[stone.x][stone.y];
        }
        else {
            return Outside;
        }
    }

    //����player�õ�ÿ��state��evaluate_weight���
    ll get_evaluation(const Stone& stone, const PlayerType& player) const {
        return allstones_current_evaluation[stone.x][stone.y][player];
    }

    //evaluate_weight��ֵ
    void evaluate_state(const Stone& stone, const PlayerType& player) {
        ll value = 1;
        int dir;
        const int& x = stone.x;
        const int& y = stone.y;
        auto& a = allstones_current_state[x][y];
        for (dir = 0; dir < 4; dir++) {
            const ll weight = a[dir][player].evaluate_weight(player);
            if (weight == WIN) {
                value = WIN;
                break;
            }
            value *= weight;
        }

        allstones_current_evaluation[x][y][player] = value;
    }

    //��ͬ��stone�ĸ������array<State, 2>,��Ϊself��opponent
    array<State, 2>& get_state(const Stone& stone, const int& direction) {
        return allstones_current_state[stone.x][stone.y][direction];
    }

    void update_board(const Stone& stone, const PlayerType& player) {

        int x = stone.x;
        int y = stone.y;

        //���֣����⴦��
        if (x == -1) return;

        const PlayerType current_player = get_player(stone);

        //�ظ��������޸�
        if (current_player == player) return;

        //ɾȥ��move
        if (current_player == Blank) {
            scenario_moves_set.erase({ this,stone });
        }

        static int changed_stones[LENGTH][LENGTH] = { 0 };
        static int change_times = 0;
        vector<Move> changed_state_list;

        //����player
        allstones_current_players[x][y] = player;

        change_times++;


        int dir, p, lr;
        for (dir = 0; dir < 4; dir++) {
            for (p = 0; p < 2; p++) {
                const State& current_state = get_state(stone, dir)[p];
                for (lr = 0; lr < 2; lr++)
                {
                    const Stone& delt = lr ? DIR[dir] : -DIR[dir];
                    // d�Ǹ���λƫ��ʸ��
                    // ���ݵ�ǰ���Ҳ�ı�� lr��ȷ���ƶ��ķ��� d�����Ϊ��ǰ�����Ҳ�Ϊ��ǰ����ķ���

                    const Stone& destination = stone + (current_state.hoped_connected_len[lr] +
                        current_state.is_hopedlen_next_blank[lr] + 1) * delt;
                    //�����ƶ����յ�λ�� destination����λ���ǵ�ǰλ�� stone ���ϵ�ǰ�����ϵ�����״̬����
                    //���Ͽ��ܵ�һ���հ׵�Ԫ���ƫ�ƺ��λ�á�
                    //����delt����ƫ����
                    Stone tmp = stone + delt;
                    while (tmp != destination) {
                        if (get_player(tmp) == Blank &&
                            changed_stones[tmp.x][tmp.y] != change_times) {
                            //�����ǰλ���ǿհף�Blank��������״̬û�б����¹�
                            //��changed_stones����Ϊ��ǰ״̬�仯����change_times
                            //������ǰλ�ü��뵽״̬�仯�б���
                            changed_stones[tmp.x][tmp.y] = change_times;
                            changed_state_list.emplace_back(this, tmp);
                        }
                        //�������
                        get_state(tmp, dir)[p].update_state(get_state(tmp - delt, dir)[p],
                            get_player(tmp - delt),
                            (State::left_or_right)(lr ^ 1));
                        tmp = tmp + delt;
                    }
                }
            }
        }

        //ɾ���޸Ĺ�state��move
        for (const Move& tmp : changed_state_list) {
            scenario_moves_set.erase(tmp);
        }

        //�����޸�stautus���move
        for (const Move& tmp : changed_state_list) {
            evaluate_state(tmp.stone, Self);
            evaluate_state(tmp.stone, Opponent);
            scenario_moves_set.emplace(this, tmp.stone);
        }

        //���ڻ�ԭ
        if (player == Blank) {
            scenario_moves_set.emplace(this, stone);
        }
    }

    //��ʱ
    bool timeout() const {
        using namespace chrono;
        return duration_cast<milliseconds>(steady_clock::now() - start_time).count() > TIMELIMIT;
    }

    void addmove(vector<Move>& moves, const int& depth) {
        const ll min_weight = min(VIGILANCELIMIT, sqrt(scenario_moves_set.begin()->weight));

        for (const Move& move : scenario_moves_set) {
            if ((int)moves.size() >= max(2, BREADTH[depth] / 2) &&
                (move.weight < min_weight || (int)moves.size() >= BREADTH[depth]))
                break;
            moves.emplace_back(move);
        }
    }

    PlayerType MCTS(Node* node, int depth = DEPTH) {
        dynamic_C = A * exp(-K * depth);
        PlayerType nodeplayer = node->player;
        //exploration
        if (node->visited_times == 0) {
            //�о֣�δֹ
            if (node->end == Blank && depth != 0) {
                vector<Move> moves1;
                addmove(moves1, depth);

                set<pair<Stone, Stone>> visited;

                for (int i = 0; i < (int)moves1.size(); i++) {
                    //����ɸѡ����ƶ����� moves1
                    const Move& move_one = moves1[i];

                    //���ִ�е�ǰ�ƶ����Ƿ��Ѿ�����ʤ��������ǣ�˵����ǰ�ڵ��Ѿ�����ֹ�ڵ�
                    // ֱ�Ӵ���һ���½ڵ��ʾʤ��״̬������ֹ��������
                    if (get_evaluation(move_one.stone, nodeplayer) == WIN) {
                        Node* winchild = new Node(PlayerType(nodeplayer ^ 1), move_one, moves1[i == 0 ? 1 : i - 1], node,
                            nodeplayer);
                        node->children.push_back(winchild);
                        break;
                    }

                    //ģ��ִ�е�ǰ�ƶ���ģ�⵱ǰ����״̬�ı仯
                    update_board(move_one.stone, nodeplayer);

                    int count = 0;
                    bool win = false;

                    //�Ե�ǰ�����µ����п��ܵ�move_two���б���
                    for (const Move& move_two : scenario_moves_set) {

                        //��鵱ǰ�ƶ�����Ƿ��Ѿ����ʹ�������ǣ���������ǰѭ��
                        if (visited.insert({ min(move_one.stone, move_two.stone),
                                             max(move_one.stone, move_two.stone) }).second) {
                            count++;
                            //����ѭ��������ȷ��������Ԥ��������������

                            if (count > (BREADTH[depth] - i) / 2 + 1)//���޸�
                                break;

                            //���move_twoִ�е�ǰ�ƶ����Ƿ��Ѿ�����ʤ��
                            if (get_evaluation(move_two.stone, nodeplayer) == WIN) {
                                win = true;
                                Node* winchild = new Node(PlayerType(nodeplayer ^ 1), move_one, move_two, node,
                                    nodeplayer);
                                node->children.push_back(winchild);
                                break;
                            }
                            else {
                                //���δ����ʤ��������һ���½ڵ��ʾ��ǰ����״̬��������ӵ���ǰ�ڵ���ӽڵ��б���
                                Node* child = new Node(PlayerType(nodeplayer ^ 1), move_one, move_two, node);
                                node->children.push_back(child);
                            }
                        }
                    }

                    //������ǰ�ƶ����ָ���ǰ����״̬
                    update_board(move_one.stone, Blank);

                    if (win) break;
                }
            }

        }

        //�����ǰ�ڵ�û���ӽڵ㣬���ʾ�ýڵ���Ҷ�ӽڵ㣬ֱ�Ӹ��µ�ǰ�ڵ��ʤ��״̬������
        if (node->children.empty()) {
            node->update_node(node->end);
            return node->end;
        }

        Node* decision = nullptr;
        double max_UCB = 0;

        for (Node* child : node->children) {
            //exploition
            if (child->visited_times == 0) {
                decision = child;
                break;
            }
            if (child->UCB() > max_UCB) {
                decision = child;
                max_UCB = child->UCB();
            }
        }

        update_board(decision->move1.stone, nodeplayer);
        update_board(decision->move2.stone, nodeplayer);
        //��ģ������У�ִ��ѡ�������������ģ�⵱ǰ�ڵ�ľ���仯

        const PlayerType loser = MCTS(decision, depth - 1);
        //�ݹ���� MCTS �㷨����ѡ��Ľڵ����ģ�����������������Ϊ depth - 1

        update_board(decision->move2.stone, Blank);
        update_board(decision->move1.stone, Blank);
        //������ǰ�ڵ�Ķ������ָ���ǰ�ڵ�ľ���״̬

        node->update_node(loser);
        //����ģ��Ľ�����µ�ǰ�ڵ��ʤ��״̬

        return loser;

    }

    Json::Value MCTS()
    {
        auto* root = new Node(Self, { {-1, -1}, 0 }, { {-1, -1}, 0 }, nullptr);

        while (!timeout()) MCTS(root);

        const auto best =
            *max_element(root->children.begin(), root->children.end(), [](Node* lhs, Node* rhs) {
            return lhs->visited_times < rhs->visited_times;
                });

        ostringstream debug;
        for (auto v : root->children)
        {
            debug << v->move1.stone.x << ',' << v->move1.stone.y << ' ' << v->move1.weight << ' ';
            debug << v->move2.stone.x << ',' << v->move2.stone.y << ' ' << v->move2.weight << ' ';
            debug << v->win_times << '/' << v->visited_times - v->win_times << "    ";
        }

        Json::Value output;
        output["response"]["x0"] = best->move1.stone.x;
        output["response"]["y0"] = best->move1.stone.y;
        output["response"]["x1"] = best->move2.stone.x;
        output["response"]["y1"] = best->move2.stone.y;
        return output;
    }
};
/*MCTS part*/

int main()
{
    string str;
    getline(cin, str);
    Json::Value input;
    Json::Reader().parse(str, input);
    const int turnID = input["requests"].size();
    const bool isBlack = input["requests"][0u]["x0"].asInt() == -1;


    //�غ���С��5��ʹ��alphabeta��֦
    if (turnID <= 5) {
        Json::Value output2;
        int x0, y0, x1, y1;

        currBotColor = grid_white; //�����ǰ׷�
        for (int i = 0; i < turnID; ++i)
        {
            x0 = input["requests"][i]["x0"].asInt();
            y0 = input["requests"][i]["y0"].asInt();
            x1 = input["requests"][i]["x1"].asInt();
            y1 = input["requests"][i]["y1"].asInt();
            if (x0 == -1) {
                currBotColor = grid_black;//x1x2=-1����Ϊ��
            }
            // ģ������
            if (x0 >= 0) {
                //ģ������
                ProcStep(x0, y0, x1, y1, -currBotColor, false);
                up_edge = (up_edge < y0) ? up_edge : y0;
                down_edge = (down_edge > y0) ? down_edge : y0;
                left_edge = (left_edge < x0) ? left_edge : x0;
                right_edge = (right_edge > x0) ? right_edge : x0;

                if (i != 0 || -currBotColor == grid_white) {
                    up_edge = (up_edge < y1) ? up_edge : y1;
                    down_edge = (down_edge > y1) ? down_edge : y1;
                    left_edge = (left_edge < x1) ? left_edge : x1;
                    right_edge = (right_edge > x1) ? right_edge : x1;
                }
            }
            if (i < turnID - 1) {
                x0 = input["responses"][i]["x0"].asInt();
                y0 = input["responses"][i]["y0"].asInt();
                x1 = input["responses"][i]["x1"].asInt();
                y1 = input["responses"][i]["y1"].asInt();
                if (x0 >= 0) {
                    ProcStep(x0, y0, x1, y1, currBotColor, false);
                    up_edge = (up_edge < y0) ? up_edge : y0;
                    down_edge = (down_edge > y0) ? down_edge : y0;
                    left_edge = (left_edge < x0) ? left_edge : x0;
                    right_edge = (right_edge > x0) ? right_edge : x0;
                    if (i != 0 || currBotColor == grid_white) {
                        up_edge = (up_edge < y1) ? up_edge : y1;
                        down_edge = (down_edge > y1) ? down_edge : y1;
                        left_edge = (left_edge < x1) ? left_edge : x1;
                        right_edge = (right_edge > x1) ? right_edge : x1;
                    }
                }
            }
        }

        if (turnID == 1 && currBotColor == grid_black) {
            output2["response"]["x0"] = 7;
            output2["response"]["y0"] = 7;
            output2["response"]["x1"] = -1;
            output2["response"]["y1"] = -1;
            cout << Json::FastWriter().write(output2) << endl;
            return 0;
        }
        else {
            ExpandEdge();
        }

        AlphabetaAI::AllLegalSteps();

        if (turnID == 1) {
            decided2steps[0] = legal_step[0].step;
            decided2steps[1] = legal_step[1].step;
        }
        else {
            decided2steps[0] = legal_step[0].step;
            decided2steps[1] = legal_step[1].step;
            AlphabetaAI::AlphaBeta(-INF, INF, currBotColor, 2);
        }

        output2["response"]["x0"] = decided2steps[0].x;
        output2["response"]["y0"] = decided2steps[0].y;
        output2["response"]["x1"] = decided2steps[1].x;
        output2["response"]["y1"] = decided2steps[1].y;
        cout << Json::FastWriter().write(output2) << endl;
        return 0;
    }



    Board Board;

    for (int i = 0; i < turnID; i++) {
        Board.update_board({ input["requests"][i]["x0"].asInt(), input["requests"][i]["y0"].asInt() }, Opponent);
        Board.update_board({ input["requests"][i]["x1"].asInt(), input["requests"][i]["y1"].asInt() }, Opponent);
        if (i == turnID - 1) break;
        Board.update_board({ input["responses"][i]["x0"].asInt(), input["responses"][i]["y0"].asInt() }, Self);
        Board.update_board({ input["responses"][i]["x1"].asInt(), input["responses"][i]["y1"].asInt() }, Self);
    }

    Json::Value output;

    if (turnID == 1 && isBlack) {
        output["response"]["x0"] = LENGTH / 2;
        output["response"]["y0"] = LENGTH / 2;
        output["response"]["x1"] = -1;
        output["response"]["y1"] = -1;
    }
    else {
        output = Board.MCTS();
    }

    cout << Json::FastWriter().write(output) << endl;

    return 0;

}