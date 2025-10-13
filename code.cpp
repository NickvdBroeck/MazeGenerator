#include <vector>
#include <iostream>

const size_t grid_width = 45;
const size_t grid_height = 15;
const size_t nrCells = grid_width*grid_height;

typedef std::pair<int, int> Loc; // up-down ; L-R

class Cell
{
  public:
  Cell(){};

  Loc loc{0,0};

  bool visited{false};

  bool wallOpen_Up{false};
  bool wallOpen_Down{false};
  bool wallOpen_Left{false};
  bool wallOpen_Right{false};

  bool isRoute{false};
};

enum Dir
{
  up,
  down,
  left,
  right
};

typedef std::vector<std::vector<Cell>> Maze;
typedef std::vector<Loc> Trajectory;
typedef std::vector<std::pair<Dir,Cell>> Neighbors;

void PrintCell1(const Cell& cell, bool printLast)
{
  std::cout << "+" << (cell.wallOpen_Down?"  ":"--") << (printLast?"+":"");
}
void PrintCell2(const Cell& cell, bool printLast)
{
  std::cout << (cell.wallOpen_Left?" ":"|") << (cell.isRoute?"xx":"  ") << (printLast?(cell.wallOpen_Right?" ":"|"):"");
}
void PrintCell3(const Cell& cell, bool printLast)
{
  std::cout << "+" << (cell.wallOpen_Up?"  ":"--") << (printLast?"+":"");
}


void PrintMazeToStd(const Maze& maz)
{
  for(int r=0; r<grid_height; ++r)
  {
    for(int c=0; c<grid_width; ++c)
    {
      PrintCell1(maz[r][c], c+1 ==grid_width );
    }
    std::cout << std::endl;
    for(int c=0; c<grid_width; ++c)
    {
      PrintCell2(maz[r][c],c+1 ==grid_width);
    }
    std::cout << std::endl;
    if(r+1 == grid_height)
    {
      for(int c=0; c<grid_width; ++c)
      {
        PrintCell3(maz[r][c],c+1 ==grid_width);
      }
    std::cout << std::endl;
    }
  }
  std::cout << std::endl;
}

Neighbors GetListOfUnvisitedNeighbors(const Maze& maz, Loc current)
{
  Neighbors nn;
  
  // Down
  if(current.first -1 >=0 && !maz[current.first -1][current.second].visited)
    nn.push_back({down,maz[current.first -1][current.second]});
  // Up
  if(current.first +1 < grid_height && !maz[current.first +1][current.second].visited)
    nn.push_back({up,maz[current.first +1][current.second]});
  // Left
  if(current.second -1 >=0 && !maz[current.first][current.second-1].visited)
    nn.push_back({left,maz[current.first ][current.second-1]});
  // Right
  if(current.second +1 <grid_width && !maz[current.first][current.second+1].visited)
    nn.push_back({right,maz[current.first][current.second+1]});

  return nn;
}

Loc MoveForward(Maze& maz, Trajectory& track, const Neighbors& nn)
{
  Loc current = track.back();
  int selected = std::rand() % nn.size();
  Loc nnLoc = nn[selected].second.loc;
  maz[nnLoc.first][nnLoc.second].visited = true;
  track.push_back(nnLoc);

  //std::cout << "  Moving from: " << current.first << "," << current.second 
  //  << " to:" << nnLoc.first << "," << nnLoc.second;

  //std::string dir = "none";
  switch(nn[selected].first)
  {
  case up:
    maz[current.first][current.second].wallOpen_Up = true;
    maz[nnLoc.first][nnLoc.second].wallOpen_Down = true;
    //dir = "up";
    break;
  case down:
    maz[current.first][current.second].wallOpen_Down = true;
    maz[nnLoc.first][nnLoc.second].wallOpen_Up = true;
    //dir = "down";
    break;
  case left:
    maz[current.first][current.second].wallOpen_Left = true;
    maz[nnLoc.first][nnLoc.second].wallOpen_Right = true;
    //dir = "left";
    break;
  case right:
    maz[current.first][current.second].wallOpen_Right = true;
    maz[nnLoc.first][nnLoc.second].wallOpen_Left = true;
    //dir = "right";
    break;
  }
  
  //std::cout << " | Which is: " << dir << std::endl;

  return nnLoc;
};

Loc BackTrace(Maze& maz, Trajectory& track)
{
  while(!track.empty())
  {
    if(!GetListOfUnvisitedNeighbors(maz, track.back()).empty())
    {
      //std::cout << "  Backtracking to:" << track.back().first << "," << track.back().second << std::endl;
      return track.back();
    }
    track.pop_back();
  }
  return Loc(-1,-1);
};

Maze InitializeMaze()
{
  Maze maz;
  maz.resize(grid_height);

  for(int r=0; r<grid_height; ++r)
  {
    maz[r].resize(grid_width);
    for(int c=0; c<grid_width; ++c)
    {
      maz[r][c].loc = Loc(r,c);
    }
  }
  return maz;
};

Loc FindAnyVisitedCellWithNn(const Maze& maz)
{
  for(int r=0; r<grid_height; ++r)
  {
    for(int c=0; c<grid_width; ++c)
    {
      if(maz[r][c].visited && !GetListOfUnvisitedNeighbors(maz,Loc(r,c)).empty())
      {
        return Loc(r,c);
      }
    }
  }
  return Loc(-1,-1);
}

void ConstructMaze(Maze& maz, Trajectory& solution)
{
  std::size_t nrVisited = 1;
  Loc currentLoc(0,0);
  Trajectory track;
  track.reserve(nrCells);
  track.push_back(currentLoc);
  maz[currentLoc.first][currentLoc.second].visited = true;
  while(nrVisited <= nrCells)
  {
    auto nn = GetListOfUnvisitedNeighbors(maz, currentLoc);
    //std::cout << "Visiting (" << currentLoc.first
    //  << ", " << currentLoc.second << ") with "
    //  << nn.size() << " neighbors." << std::endl;
    if(nn.size() > 0)
    {
      currentLoc = MoveForward(maz, track, nn);
      ++nrVisited;
      if(currentLoc.first+1 == grid_height && currentLoc.second+1 == grid_width)
      {
        solution = track;
      }
    }
    else
    {
      currentLoc = BackTrace(maz, track);
      if(currentLoc.first == -1)
      {
        currentLoc = FindAnyVisitedCellWithNn(maz);
        if(currentLoc.first == -1)
        {
          //std::cout << "All visited cells have no free nn" << std::endl;
          return;
        }
      }
    }
  }
};

void IndicateSolution(Maze& maz,const Trajectory& solution)
{
  for(Loc l:solution)
  {
    maz[l.first][l.second].isRoute = true;
  }
}

int main()
{
  auto maz = InitializeMaze();
  Trajectory solution;
  std::srand(std::time({})); // change rand seed
  ConstructMaze(maz,solution);
  maz[0][0].wallOpen_Down = true; // start
  maz[grid_height-1][grid_width-1].wallOpen_Up = true; // end

  std::cout << std::endl << "START" << std::endl;
  PrintMazeToStd(maz);
  std::cout << std::string( 3*(grid_width-1), ' ' ) << "END" << std::endl;
  
  std::cout << std::string( 10, '\n' ) << std::endl;
  IndicateSolution(maz,solution);
  PrintMazeToStd(maz);
  return 0;
}