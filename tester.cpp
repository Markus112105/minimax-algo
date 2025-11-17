//chatGPT adaption of main.cpp to allow you to run a certain game state 
#include <vector>
#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>
#include <chrono>
#include <climits>

const int ROWS = 6, COLS = 7;
const int WIN_SCORE = 100000;
const int MAX_DEPTH = 5;
const char* CSV_NAME = "results.csv";

enum Player { NONE=0, HUMAN, AI };
using Board = std::vector<std::vector<int>>;

// center-first ordering
static const int COLUMN_ORDER[COLS] = {3,2,4,1,5,0,6};
static bool USE_ALPHA_BETA = true;
static bool USE_MOVE_ORDER = true;
static bool USE_EARLY_WIN  = true;
static std::vector<int> columnOrder;

// drop a piece and tell the caller the row so we can undo without copying the whole board
int dropPiece(Board &b, int c, int p) {
    for(int r=ROWS-1; r>=0; --r)
        if(b[r][c]==NONE) { b[r][c]=p; return r; }
    return -1;
}

void undoMove(Board &b, int c, int r) {
    if(r>=0 && r<ROWS) b[r][c]=NONE;
}

bool hasValidMove(const Board &b) {
    for(int c=0;c<COLS;++c)
        if(b[0][c]==NONE) return true;
    return false; // lets minimax return 0 for draws instead of propagating bogus scores
}

// valid?
bool isValidMove(const Board &b, int c) {
    return c>=0 && c<COLS && b[0][c]==NONE;
}

// win?
bool checkWin(const Board &b, int p) {
    const int dirs[4][2]={{0,1},{1,0},{1,1},{-1,1}};
    for(int r=0;r<ROWS;++r)for(int c=0;c<COLS;++c){
        for(auto &d:dirs){
            int cnt=0;
            for(int k=0;k<4;++k){
                int nr=r+k*d[0], nc=c+k*d[1];
                if(nr<0||nr>=ROWS||nc<0||nc>=COLS||b[nr][nc]!=p) break;
                cnt++;
            }
            if(cnt==4) return true;
        }
    }
    return false;
}

// window score
int evaluateWindow(int a,int b,int c,int d) {
    int vals[4]={a,b,c,d};
    int score=0, ai=0, hu=0, em=0;
    for(int cell:vals){
        if(cell==AI) ++ai;
        else if(cell==HUMAN) ++hu;
        else ++em;
    }
    if(ai&&hu) return 0;
    if(ai==3&&em==1) score+=51; else if(ai==2&&em==2) score+=17;
    if(hu==3&&em==1) score-=51; else if(hu==2&&em==2) score-=17;
    return score;
}

// board heuristic
int evaluateBoard(const Board &b) {
    int sc=0, cx=COLS/2;
    for(int r=0;r<ROWS;++r){
        if(b[r][cx]==AI)    sc+=18;
        else if(b[r][cx]==HUMAN) sc-=18;
        if(b[r][cx-1]==AI) sc+=6; else if(b[r][cx-1]==HUMAN) sc-=6;
        if(b[r][cx+1]==AI) sc+=6; else if(b[r][cx+1]==HUMAN) sc-=6;
        if(b[r][cx-2]==AI) sc+=2; else if(b[r][cx-2]==HUMAN) sc-=2;
        if(b[r][cx+2]==AI) sc+=2; else if(b[r][cx+2]==HUMAN) sc-=2;
    }
    // horiz
    for(int r=0;r<ROWS;++r)
      for(int c=0;c<=COLS-4;++c)
        sc += evaluateWindow(b[r][c],b[r][c+1],b[r][c+2],b[r][c+3]); // avoid per-window vector allocations in the tight loop
    // vert
    for(int c=0;c<COLS;++c)
      for(int r=0;r<=ROWS-4;++r)
        sc += evaluateWindow(b[r][c],b[r+1][c],b[r+2][c],b[r+3][c]);
    // diag SE
    for(int r=0;r<=ROWS-4;++r)
      for(int c=0;c<=COLS-4;++c)
        sc += evaluateWindow(b[r][c],b[r+1][c+1],b[r+2][c+2],b[r+3][c+3]);
    // diag NE
    for(int r=3;r<ROWS;++r)
      for(int c=0;c<=COLS-4;++c)
        sc += evaluateWindow(b[r][c],b[r-1][c+1],b[r-2][c+2],b[r-3][c+3]);
    return sc;
}

// minimax
int minimax(Board &b,int depth,int alpha,int beta,bool maxP) {
    if(checkWin(b,AI))    return WIN_SCORE+depth;
    if(checkWin(b,HUMAN)) return -WIN_SCORE-depth;
    if(depth==0) return evaluateBoard(b);
    if(!hasValidMove(b)) return 0;

    if(maxP) {
        int best=INT_MIN;
        if(USE_EARLY_WIN) {
            for(int i=0;i<COLS;++i){
                int c=columnOrder[i]; if(!isValidMove(b,c)) continue;
                int row=dropPiece(b,c,AI);
                bool win=checkWin(b,AI);
                undoMove(b,c,row);
                if(win) return WIN_SCORE+depth;
            }
        }
        for(int i=0;i<COLS;++i){
            int c=columnOrder[i]; if(!isValidMove(b,c)) continue;
            int row=dropPiece(b,c,AI);
            int v=minimax(b,depth-1,alpha,beta,false);
            undoMove(b,c,row);
            best=std::max(best,v);
            if(USE_ALPHA_BETA){
                alpha=std::max(alpha,v);
                if(beta<=alpha) break;
            }
        }
        return best;
    } else {
        int best=INT_MAX;
        if(USE_EARLY_WIN) {
            for(int i=0;i<COLS;++i){
                int c=columnOrder[i]; if(!isValidMove(b,c)) continue;
                int row=dropPiece(b,c,HUMAN);
                bool win=checkWin(b,HUMAN);
                undoMove(b,c,row);
                if(win) return -WIN_SCORE-depth;
            }
        }
        for(int i=0;i<COLS;++i){
            int c=columnOrder[i]; if(!isValidMove(b,c)) continue;
            int row=dropPiece(b,c,HUMAN);
            int v=minimax(b,depth-1,alpha,beta,true);
            undoMove(b,c,row);
            best=std::min(best,v);
            if(USE_ALPHA_BETA){
                beta=std::min(beta,v);
                if(beta<=alpha) break;
            }
        }
        return best;
    }
}

// pick best column (timed only)
std::pair<int,int> getBestMove(Board b) {
    int bestScore=INT_MIN, bestCol=-1;
    bool moveFound=false;
    for(int i=0;i<COLS;++i){
        int c=columnOrder[i]; if(!isValidMove(b,c)) continue;
        moveFound=true;
        int row=dropPiece(b,c,AI);
        int v = minimax(b,MAX_DEPTH-1,INT_MIN,INT_MAX,false);
        undoMove(b,c,row);
        if(v>bestScore){ bestScore=v; bestCol=c; }
    }
    if(!moveFound) return {-1,0};
    return {bestCol,bestScore};
}
int main(int argc, char* argv[]){
    std::pair<int,int> colScore;
    if(argc!=2){
        std::cerr<<"Usage: "<<argv[0]<<" <board_file>\n";
        return 1;
    }
    std::ifstream in(argv[1]);
    if(!in){
        std::cerr<<"Error opening file: "<<argv[1]<< "\n";
        return 1;
    }

    Board b(ROWS, std::vector<int>(COLS));
    for(int r=0;r<ROWS;++r){
        for(int c=0;c<COLS;++c){
            if(!(in>>b[r][c])){
                std::cerr<<"Invalid board format at r="<<r<<", c="<<c<<"\n";
                return 1;
            }
        }
    }
    in.close();

    std::ofstream out(CSV_NAME);
    out<<"alpha_beta,mid_col,early_win,runtime_ms,config\n";
    for(int a=0;a<2;++a)for(int m=0;m<2;++m)for(int e=0;e<2;++e){
        USE_ALPHA_BETA=a;
        USE_MOVE_ORDER=m;
        USE_EARLY_WIN =e;
        columnOrder.clear();
        for(int i=0;i<COLS;++i)
            columnOrder.push_back(m?COLUMN_ORDER[i]:i);

        auto t1=std::chrono::steady_clock::now();
        colScore = getBestMove(b);
        auto t2=std::chrono::steady_clock::now();
        long ms=std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
        out<<a<<","<<m<<","<<e<<","<<ms<<",\""
           <<a<<","<<m<<","<<e<<"\"\n";
    }
    out << "Best Col:" <<colScore.first<<" With Score/Win:"<<colScore.second<<"\n";
    out.close();
    std::cout<<"Benchmark written to csv\n";
    return 0;
}
