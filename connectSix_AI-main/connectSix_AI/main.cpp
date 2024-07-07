#include <iostream>
#include <vector>
#include<algorithm>
#include <chrono>

#define GRIDSIZE 15
#define grid_blank 0
#define grid_black 1
#define grid_white (-1)
const int INF = std::numeric_limits<int>::max();//无穷大

// 设置搜索最长时间为 1 秒
const int kMaxSearchTime = 999; // 单位：毫秒
// 记录搜索开始时间
auto start_time = std::chrono::high_resolution_clock::now();
// 判断是否超时
bool TimeIsUp() {
    auto now = std::chrono::high_resolution_clock::now();
    auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    return time_elapsed >= kMaxSearchTime;
}

using namespace std;

//棋盘边界
int upper_boundary = GRIDSIZE;//上边界
int lower_boundary = -1;//下边界
int left_border = GRIDSIZE;//左边界
int right_border = -1;//右边界

int currBotColor; // 本方所执子颜色（1为黑，-1为白，棋盘状态亦同）
int gridInfo[GRIDSIZE][GRIDSIZE] = { 0 }; // 先x后y，记录棋盘状态

//棋步
struct Move {
    int x;
    int y;
};
//初始棋步及其评分
struct MoveWithValue {
    int value;//棋步对应的初始值
    Move move;//棋步
};
vector<MoveWithValue> legal_move;//可能棋步
Move best_move[2];//最终棋步
vector<Move> simulated_move;//模拟落子的棋步

// 判断是否在棋盘内
inline bool inMap(int x, int y) {
    if (x < 0 || x >= GRIDSIZE || y < 0 || y >= GRIDSIZE)
        return false;
    return true;
}
// 在坐标处落子，检查是否合法或模拟落子
bool ProcStep(int x0, int y0, int x1, int y1, int grid_color, bool check_only) {
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
//初始评估
int InitialReview(Move move, int player) {
    ///设置敌我双方各种路分数
    //我方分数
    int my_value[5] = { 20, 45, 50, 10000000, 10000000 };
    //敌方分数
    int opponent_value[5] = { 1, 15, 30, 9000000, 9000000 };

    ///各道分值
    //竖道分值
    int vertical_line_value = 0;
    vector<int> vertical_line;
    for (int i = upper_boundary; i <= lower_boundary; i++) {
        if (abs(move.y - i) < 6) {
            vertical_line.push_back(gridInfo[move.x][i]);
        }
    }
    //求出竖道上敌我双方各种路的数量
    int num = (int)vertical_line.size();
    while (num < 6) {
        vertical_line.push_back(2);
        num++;
    }
    for (int i = 0; i <= num - 6; i++) {
        //求出一条路中的各种棋子数量
        int my_nums = 0;
        int opponent_nums = 0;
        for (int j = i; j < i + 6; j++) {
            if (vertical_line[j] == player) {
                my_nums++;
            }
            else if (vertical_line[j] == -player) {
                opponent_nums++;
            }
        }
        //我方价值
        if (my_nums != 0 && opponent_nums == 0) {
            vertical_line_value += my_value[my_nums - 1];
        }
        //对方威胁
        if (my_nums == 0 && opponent_nums != 0) {
            vertical_line_value += opponent_value[opponent_nums - 1];
        }
    }
    //横道分值
    int horizontal_line_value = 0;
    vector<int> horizontal_line;
    for (int i = left_border; i <= right_border; i++) {
        if (abs(move.x - i) < 6) {
            horizontal_line.push_back(gridInfo[i][move.y]);
        }
    }
    //求出横道上敌我双方各种路的数量
    num = (int)horizontal_line.size();
    while (num < 6) {
        horizontal_line.push_back(2);
        num++;
    }
    for (int i = 0; i <= num - 6; i++) {
        //求出一条路中的各种棋子数量
        int my_nums = 0;
        int opponent_nums = 0;
        for (int j = i; j < i + 6; j++) {
            if (horizontal_line[j] == player) {
                my_nums++;
            }
            else if (horizontal_line[j] == -player) {
                opponent_nums++;
            }
        }
        //我方价值
        if (my_nums != 0 && opponent_nums == 0) {
            horizontal_line_value += my_value[my_nums - 1];
        }
        //对方威胁
        if (my_nums == 0 && opponent_nums != 0) {
            horizontal_line_value += opponent_value[opponent_nums - 1];
        }
    }
    //左上右下道分值
    int leftup_rightdown_line_value = 0;
    vector<int> leftup_rightdown_line;
    int left_up = max(move.y - upper_boundary, move.x - left_border);
    int right_down = max(lower_boundary - move.y, right_border - move.x);
    for (int i = -left_up; i <= right_down; i++) {
        if (abs(i) < 6) {
            int x = move.x + i;
            int y = move.y + i;
            if (y >= upper_boundary && y <= lower_boundary && x >= left_border && x <= right_border) {
                leftup_rightdown_line.push_back(gridInfo[x][y]);
            }
        }
    }
    //求出左上右下道上敌我双方各种路的数量
    num = (int)leftup_rightdown_line.size();
    while (num < 6) {
        leftup_rightdown_line.push_back(2);
        num++;
    }
    for (int i = 0; i <= num - 6; i++) {
        //求出一条路中的各种棋子数量
        int my_nums = 0;
        int opponent_nums = 0;
        for (int j = i; j < i + 6; j++) {
            if (leftup_rightdown_line[j] == player) {
                my_nums++;
            }
            else if (leftup_rightdown_line[j] == -player) {
                opponent_nums++;
            }
        }
        //我方价值
        if (my_nums != 0 && opponent_nums == 0) {
            leftup_rightdown_line_value += my_value[my_nums - 1];
        }
        //对方威胁
        if (my_nums == 0 && opponent_nums != 0) {
            leftup_rightdown_line_value += opponent_value[opponent_nums - 1];
        }
    }
    //左下右上道分值
    int leftdown_rightup_line_value = 0;
    vector<int> leftdown_rightup_line;
    int left_down = max(lower_boundary - move.y, move.x - left_border);
    int right_up = max(move.y - upper_boundary, right_border - move.x);
    for (int i = -left_down; i <= right_up; i++) {
        if (abs(i) < 6) {
            int x = move.x + i;
            int y = move.y - i;
            if (y >= upper_boundary && y <= lower_boundary && x >= left_border && x <= right_border) {
                leftdown_rightup_line.push_back(gridInfo[x][y]);
            }
        }
    }
    //求出左下右上道上敌我双方各种路的数量
    num = (int)leftdown_rightup_line.size();
    while (num < 6) {
        leftdown_rightup_line.push_back(2);
        num++;
    }
    for (int i = 0; i <= num - 6; i++) {
        //求出一条路中的各种棋子数量
        int my_nums = 0;
        int opponent_nums = 0;
        for (int j = i; j < i + 6; j++) {
            if (leftdown_rightup_line[j] == player) {
                my_nums++;
            }
            else if (leftdown_rightup_line[j] == -player) {
                opponent_nums++;
            }
        }
        //我方价值
        if (my_nums != 0 && opponent_nums == 0) {
            leftdown_rightup_line_value += my_value[my_nums - 1];
        }
        //对方威胁
        if (my_nums == 0 && opponent_nums != 0) {
            leftdown_rightup_line_value += opponent_value[opponent_nums - 1];
        }
    }
    ///总分值
    return vertical_line_value + horizontal_line_value + leftup_rightdown_line_value + leftdown_rightup_line_value;
}
//按value从大到小排序
bool cmp(const MoveWithValue& a, const MoveWithValue& b) {
    return a.value > b.value;
}
//生成所有合法棋步
void GenerateLegalMoves(int player) {
    //将合法棋步存入legal_move数组
    for (int y = upper_boundary; y <= lower_boundary; y++) {
        for (int x = left_border; x <= right_border; x++) {
            if (gridInfo[x][y] == grid_blank) {
                Move move{};
                move.x = x;
                move.y = y;
                int value = InitialReview(move, player);
                MoveWithValue temp{};
                temp.move = move;
                temp.value = value;
                legal_move.push_back(temp);
            }
        }
    }
    //将合法棋步按评分由高到低排序
    sort(legal_move.begin(), legal_move.end(), cmp);
}
//一条道上的总分
int LineValue(vector<int>& line, int player) {
    ///设置敌我双方各种路分数
    //我方分数
    int my_value[6] = { 1, 25, 50, 4000, 4000, 1000000 };
    int my_pre_value = 0;
    int my_after_value = 0;
    //敌方分数
    int opponent_value[6] = { 1, 20, 40, 200, 7000, 900000 };
    int opponent_pre_value = 0;
    int opponent_after_value = 0;
    ///求出敌我双方各种路的数量
    int num = (int)line.size();
    while (num < 6) {
        line.push_back(2);
        num++;
    }
    for (int i = 0; i <= num - 6; i++) {
        //求出一条路中的各种棋子数量
        int my_nums = 0;
        int opponent_nums = 0;
        for (int j = i; j < i + 6; j++) {
            if (line[j] == player) {
                my_nums++;
            }
            else if (line[j] == -player) {
                opponent_nums++;
            }
        }
        //对手棋数为0
        if (opponent_nums == 0) {
            //若nums-1>0（即原先便有我方棋子），更新原先与此时的我方价值
            if (my_nums - 1 > 0) {
                my_pre_value += my_value[my_nums - 2];
                my_after_value += my_value[my_nums - 1];
            }
                //若my_nums>0（即此时有我方棋子），更新此时我方价值
            else if (my_nums > 0) {
                my_after_value += my_value[my_nums - 1];
            }
        }
            //对手棋数非0
        else {
            //若my_nums=0（即原先此时都只有对手的棋），更新原先与此时的对手威胁值
            if (my_nums == 0) {
                opponent_after_value += opponent_value[opponent_nums - 1];
                opponent_pre_value += opponent_value[opponent_nums - 2];
            }
                //若my_nums-1=0（即原先只有对手的棋），更新原先对手威胁
            else if (my_nums - 1 == 0) {
                opponent_pre_value += opponent_value[opponent_nums - 2];
            }
        }
    }
    ///求出该道上的总分，并返回
    return my_after_value - opponent_after_value - my_pre_value + opponent_pre_value;
}
//竖道分值
int VerticalLineValue(Move move, int player) {
    //落子所在竖道
    vector<int> vertical_line;
    for (int i = upper_boundary; i <= lower_boundary; i++) {
        if (abs(move.y - i) < 6) {
            vertical_line.push_back(gridInfo[move.x][i]);
        }
    }
    //计算该道分值
    return LineValue(vertical_line, player);
}
//横道分值
int HorizontalLineValue(Move move, int player) {
    //落子所在横道
    vector<int> horizontal_line;
    for (int i = left_border; i <= right_border; i++) {
        if (abs(move.x - i) < 6) {
            horizontal_line.push_back(gridInfo[i][move.y]);
        }
    }
    //计算该路分道
    return LineValue(horizontal_line, player);
}
//左上右下道分值
int LeftupRightdownLineValue(Move move, int player) {
    //落子所在左上右下道
    vector<int> leftup_rightdown_line;
    int left_up = max(move.y - upper_boundary, move.x - left_border);
    int right_down = max(lower_boundary - move.y, right_border - move.x);
    for (int i = -left_up; i <= right_down; i++) {
        if (abs(i) < 6) {
            int x = move.x + i;
            int y = move.y + i;
            if (y >= upper_boundary && y <= lower_boundary && x >= left_border && x <= right_border) {
                leftup_rightdown_line.push_back(gridInfo[x][y]);
            }
        }
    }
    //计算该道分值
    return LineValue(leftup_rightdown_line, player);
}
//左下右上道分值
int LeftdownRightupLineValue(Move move, int player) {
    //落子所在左下右上道
    vector<int> leftdown_rightup_line;
    int left_down = max(lower_boundary - move.y, move.x - left_border);
    int right_up = max(move.y - upper_boundary, right_border - move.x);
    for (int i = -left_down; i <= right_up; i++) {
        if (abs(i) < 6) {
            int x = move.x + i;
            int y = move.y - i;
            if (y >= upper_boundary && y <= lower_boundary && x >= left_border && x <= right_border) {
                leftdown_rightup_line.push_back(gridInfo[x][y]);
            }
        }
    }
    //计算该道分值
    return LineValue(leftdown_rightup_line, player);
}
//评估函数
int Evaluate(int player) {
    int result = 0;
    int nums = (int)simulated_move.size();
    for (int i = 0; i < nums; i++) {
        Move move = simulated_move[i];
        result = result
                 + VerticalLineValue(move, player)
                 + HorizontalLineValue(move, player)
                 + LeftupRightdownLineValue(move, player)
                 + LeftdownRightupLineValue(move, player);
    }
    return result;
}
//Alpha_Beta剪枝
int AlphaBetaPruning(int alpha, int beta, int depth, int player) {
    // 超时直接返回当前已经搜索到的最优解
    if (TimeIsUp()) {
        return 0;
    }
    //达到搜索深度，返回评估值
    if (depth == 0) {
        return Evaluate(player);
    }
    //从合法棋步中搜索
    int nums = (int)legal_move.size();
    for (int i = 0; i < nums && i < 12; i++) {
        //模拟第一步落子
        Move move1 = legal_move[i].move;
        if (gridInfo[move1.x][move1.y] != grid_blank) {
            continue;
        }
        gridInfo[move1.x][move1.y] = player;
        simulated_move.push_back(move1);
        for (int j = 0; j < nums && j < 12; j++) {
            //模拟第二步落子
            Move move2 = legal_move[j].move;
            if (gridInfo[move2.x][move2.y] != grid_blank) {
                continue;
            }
            gridInfo[move2.x][move2.y] = player;
            simulated_move.push_back(move2);
            //评估局面
            int value = -AlphaBetaPruning(-beta, -alpha, depth - 1, -player);
            //撤回第二步落子
            gridInfo[move2.x][move2.y] = 0;
            simulated_move.pop_back();
            //剪枝
            if (value >= beta) {
                //撤回第一步落子
                gridInfo[move1.x][move1.y] = 0;
                simulated_move.pop_back();
                return beta;
            }
            if (value > alpha) {
                alpha = value;
                if (depth == 2) {
                    best_move[0] = move1;
                    best_move[1] = move2;
                }
            }
        }
        //撤回第一步落子
        gridInfo[move1.x][move1.y] = 0;
        simulated_move.pop_back();
    }
    //返回找到的最佳分数
    return alpha;
}
int main()
{
    ///恢复棋盘状态,并找出边界和记录最近4步棋
    int x0, y0, x1, y1;
    // 分析自己收到的输入和自己过往的输出，并恢复棋盘状态
    int turnID;
    cin >> turnID;
    currBotColor = grid_white; // 先假设自己是白方
    // 根据这些输入输出逐渐恢复状态到当前回合，并找出边界和记录最近4步棋
    for (int i = 0; i < turnID; i++)
    {
        cin >> x0 >> y0 >> x1 >> y1;
        //确定棋色
        if (x0 == -1) {
            currBotColor = grid_black;//第一回合收到坐标是-1, -1，说明我是黑方
        }
        // 模拟对方落子，并查找边界和记录对方最近两步棋
        if (x0 >= 0) {
            //模拟落子
            ProcStep(x0, y0, x1, y1, -currBotColor, false);
            //检测第一步是否为边界
            upper_boundary = min(upper_boundary, y0);
            lower_boundary = max(lower_boundary, y0);
            left_border = min(left_border, x0);
            right_border = max(right_border, x0);
            //检测第二步是否为边界
            if (i != 0 || -currBotColor == grid_white) {
                upper_boundary = min(upper_boundary, y1);
                lower_boundary = max(lower_boundary, y1);
                left_border = min(left_border, x1);
                right_border = max(right_border, x1);
            }
        }
        // 模拟己方落子，并查找边界和记录自己最近两步棋
        if (i < turnID - 1) {
            cin >> x0 >> y0 >> x1 >> y1;
            if (x0 >= 0) {
                //模拟落子
                ProcStep(x0, y0, x1, y1, currBotColor, false);
                //检测第一步是否为边界
                upper_boundary = min(upper_boundary, y0);
                lower_boundary = max(lower_boundary, y0);
                left_border = min(left_border, x0);
                right_border = max(right_border, x0);
                //检测第二步是否为边界
                if (i != 0 || currBotColor == grid_white) {
                    upper_boundary = min(upper_boundary, y1);
                    lower_boundary = max(lower_boundary, y1);
                    left_border = min(left_border, x1);
                    right_border = max(right_border, x1);
                }
            }
        }
    }
    ///扩展边界，并生成所有合法棋步
    //第一步
    if (turnID == 1) {
        //黑棋第一步直接输出中间位置
        if (currBotColor == grid_black) {
            best_move[0].x = 7;
            best_move[0].y = 7;
            best_move[1].x = -1;
            best_move[1].y = -1;
            cout << best_move[0].x << ' ' << best_move[0].y << ' ' << best_move[1].x << ' ' << best_move[1].y << endl;
            return 0;
        }
            //白棋第一步只需扩展一格
        else {
            //向上扩展
            if (upper_boundary - 1 >= 0) {
                upper_boundary = upper_boundary - 1;
            }
            //向下扩展
            if (lower_boundary + 1 < GRIDSIZE) {
                lower_boundary = lower_boundary + 1;
            }
            //向左扩展
            if (left_border - 1 >= 0) {
                left_border = left_border - 1;
            }
            //向右扩展
            if (right_border + 1 < GRIDSIZE) {
                right_border = right_border + 1;
            }
        }
    }
        //第二步起均扩展两格
    else {
        //向上扩展
        if (upper_boundary - 2 >= 0) {
            upper_boundary = upper_boundary - 2;
        }
        else if (upper_boundary - 1 >= 0) {
            upper_boundary = upper_boundary - 1;
        }
        //向下扩展
        if (lower_boundary + 2 < GRIDSIZE) {
            lower_boundary = lower_boundary + 2;
        }
        else if (lower_boundary + 1 < GRIDSIZE) {
            lower_boundary = lower_boundary + 1;
        }
        //向左扩展
        if (left_border - 2 >= 0) {
            left_border = left_border - 2;
        }
        else if (left_border - 1 >= 0) {
            left_border = left_border - 1;
        }
        //向右扩展
        if (right_border + 2 < GRIDSIZE) {
            right_border = right_border + 2;
        }
        else if (right_border + 1 < GRIDSIZE) {
            right_border = right_border + 1;
        }
    }
    //生成合法棋步
    GenerateLegalMoves(currBotColor);
    ///进行决策
    //开局直接下
    if (turnID == 1) {
        best_move[0] = legal_move[0].move;
        best_move[1] = legal_move[2].move;
    }
        //对博弈树剪枝，找出最佳棋步
    else {
        best_move[0] = legal_move[0].move;
        best_move[1] = legal_move[1].move;
        AlphaBetaPruning(-INF, INF, 2, currBotColor);
    }

    /// 决策结束，向平台输出决策结果
    cout << best_move[0].x << ' ' << best_move[0].y << ' ' << best_move[1].x << ' ' << best_move[1].y << endl;

    return 0;
}