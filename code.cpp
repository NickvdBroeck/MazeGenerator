#include <vector>
#include <iostream>
#include <sstream>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <ctime>

//const size_t grid_width = 5;
//const size_t grid_height = 5;
//const size_t nrCells = grid_width*grid_height;

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
typedef std::vector<std::wstring> Visualization;

Visualization CreateEmptyGrid(const std::size_t grid_height, const std::size_t grid_width)
{
  const std::size_t lastPixel_r = 2*grid_height; // 2 because: 1*wall + 1*center
  const std::size_t lastPixel_c = 3*grid_width; // 3 becaus: 1*wall + 2*center

  Visualization vis(lastPixel_r+1); 
  for(auto& row : vis)
  {
    row.resize(lastPixel_c+1, ' '); 
  }

  // outer corners
  vis[0][0] = L'\x250c';
  vis[0][lastPixel_c] = L'\x2510';
  vis[lastPixel_r][0] = L'\x2514';
  vis[lastPixel_r][lastPixel_c] = L'\x2518';

  // Horizontal walls
  for(int r=0; r<=lastPixel_r; r+=2)
  {
    for(int c=1; c<=lastPixel_c; c+=3)
    {
      vis[r][c] = L'\x2500';
      vis[r][c+1] = L'\x2500';
    }
  }

  // Vertical walls
  for(int r=1; r<=lastPixel_r; r+=2)
  {
    for(int c=0; c<=lastPixel_c; c+=3)
    {
      vis[r][c] = L'\x2502';
    }
  }

  // remaining corners
  for(int r=2; r<=lastPixel_r-1; r+=2)
  {
    vis[r][0] = L'\x251c';
    vis[r][lastPixel_c] = L'\x2524';
    for(int c=3; c<=lastPixel_c-1; c+=3)
    {
      vis[r][c] = L'\x253c';
    }  
  }
  for(int c=3; c<=lastPixel_c-1; c+=3)
  {
    vis[0][c] = L'\x252c';
    vis[lastPixel_r][c] = L'\x2534';
  }

  return vis;
}

void RemoveWalls(Visualization& vis, const Cell& cell, const Loc pos)
{
  if(cell.wallOpen_Down)
  {
    vis[pos.first][pos.second+1] = ' ';
    vis[pos.first][pos.second+2] = ' ';
  }
  if(cell.wallOpen_Up)
  {
    vis[pos.first+2][pos.second+1] = ' ';
    vis[pos.first+2][pos.second+2] = ' ';
  }
  if(cell.wallOpen_Left)
  {
    vis[pos.first+1][pos.second] = ' ';
  }
  if(cell.wallOpen_Right)
  {
    vis[pos.first+1][pos.second+3] = ' ';
  }
}

wchar_t GetCorner(bool N, bool E, bool S, bool W)
{
  if (!N && !E && !S && W) return L'\x2574';
  if (N && !E && !S && !W) return L'\x2575';
  if (!N && E && !S && !W) return L'\x2576';
  if (!N && !E && S && !W) return L'\x2577';

  // 2 empty
  if (N && E && !S && !W) return L'\x2514';
  if (N && !E && S && !W) return L'\x2502';
  if (N && !E && !S && W) return L'\x2518';
  if (!N && E && S && !W) return L'\x250c';
  if (!N && E && !S && W) return L'\x2500';
  if (!N && !E && S && W) return L'\x2510';

  // 1 empty
  if (!N && E && S && W) return L'\x252c';
  if (N && !E && S && W) return L'\x2524';
  if (N && E && !S && W) return L'\x2534';
  if (N && E && S && !W) return L'\x251c';

  return L'\x253c';
}

void CorrectCorners(Visualization& vis)
{ 
  const std::size_t lastPixel_r = vis.size()-1;
  const std::size_t lastPixel_c = vis[0].size()-1;
  // inner corners
  for(int r=0; r<=lastPixel_r; r+=2)
  {
    for(int c=0; c<=lastPixel_c; c+=3)
    {
      // N == taken,  !N == empty
      const bool N = (r != 0) ? vis[r-1][c] != L' ' : false;
      const bool S = (r != lastPixel_r) ? vis[r+1][c] != L' ' : false;
      const bool W = (c !=0) ? vis[r][c-1] != L' ' : false;
      const bool E = (c != lastPixel_c) ? vis[r][c+1] != L' ' : false;

      vis[r][c] = GetCorner(N, E, S, W);
    }  
  }
}

void MarkTrajectory(Visualization& vis, const Maze& maz)
{
  const std::size_t grid_height = maz.size();
  const std::size_t grid_width = maz[0].size();
  const wchar_t trajectory = L'\x2591';
  for(int r=0; r<grid_height; ++r)
  {
    for(int c=0; c<grid_width; ++c)
    {
      if(maz[r][c].isRoute)
      {
        vis[1+2*r][1+3*c] = trajectory;
        vis[1+2*r][2+3*c] = trajectory;
        if(maz[r][c].wallOpen_Up && r+1 < grid_height)
        {
          vis[2+2*r][1+3*c] = trajectory;
          vis[2+2*r][2+3*c] = trajectory;
        }
        if(maz[r][c].wallOpen_Right && c+1 < grid_width)
        {
          vis[1+2*r][3*(c+1)] = trajectory;
        }
      }
    }
  }
}

std::wstringstream MazeToString(const Maze& maz)
{
  
  const std::size_t grid_height = maz.size();
  const std::size_t grid_width = maz[0].size();
  auto vis = CreateEmptyGrid(grid_height, grid_width);

  for(int r=0; r<grid_height; ++r)
  {
    for(int c=0; c<grid_width; ++c)
    {
      RemoveWalls(vis, maz[r][c], Loc(2*r,3*c));
    }
  }

  CorrectCorners(vis);

  MarkTrajectory(vis, maz);

  std::wstringstream ss;
  for(auto row : vis)
  {
    ss << row << std::endl;
  }

  return ss;
}


Neighbors GetListOfUnvisitedNeighbors(const Maze& maz, Loc current)
{
  Neighbors nn;
  const std::size_t grid_height = maz.size();
  const std::size_t grid_width = maz[0].size();
  
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

  //std::wcout << "  Moving from: " << current.first << "," << current.second 
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
  
  //std::wcout << " | Which is: " << dir << std::endl;

  return nnLoc;
};

Loc BackTrace(Maze& maz, Trajectory& track)
{
  while(!track.empty())
  {
    if(!GetListOfUnvisitedNeighbors(maz, track.back()).empty())
    {
      //std::wcout << "  Backtracking to:" << track.back().first << "," << track.back().second << std::endl;
      return track.back();
    }
    track.pop_back();
  }
  return Loc(-1,-1);
};

Maze InitializeMaze(const std::size_t grid_height, std::size_t grid_width)
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
  for(int r=0; r<maz.size(); ++r)
  {
    for(int c=0; c<maz[r].size(); ++c)
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
  const std::size_t grid_height = maz.size();
  const std::size_t grid_width = maz[0].size();
  const std::size_t nrCells = grid_width * grid_height;
  std::size_t nrVisited = 1;
  Loc currentLoc(0,0);
  Trajectory track;
  track.reserve(nrCells);
  track.push_back(currentLoc);
  maz[currentLoc.first][currentLoc.second].visited = true;
  while(nrVisited <= nrCells)
  {
    auto nn = GetListOfUnvisitedNeighbors(maz, currentLoc);
    //std::wcout << "Visiting (" << currentLoc.first
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
          //std::wcout << "All visited cells have no free nn" << std::endl;
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

std::size_t CalculateDifficulty(const Maze& maz)
{
  std::size_t difficulty = 0;
  for(const auto& row : maz)
  {
    for(const auto & cell : row)
    {
      if(cell.isRoute)
      {
        int edges = 0;
        edges += cell.wallOpen_Down ? 1 : 0;
        edges += cell.wallOpen_Up ? 1 : 0;
        edges += cell.wallOpen_Left ? 1 : 0;
        edges += cell.wallOpen_Right ? 1 : 0;
        if(edges > 2)
        {
          ++difficulty;
        }
      }
    }
  }
  return difficulty;
}

void PrintHelp()
{
  std::cout << "MAZE generator help" << std::endl << std::endl;
   std::cout << "Provide two positive, non-zero integers to generate a maze of those dimensions." << std::endl;
   std::cout << " - arg1 = height" << std::endl;
   std::cout << " - arg2 = width" << std::endl;
   std::cout << " - arg3 = print solution (optional)" << std::endl;
   std::cout << " - arg4 = seed (optional)" << std::endl;
   std::cout << std::endl;
   std::cout << "Examples: " << std::endl;
   std::cout << "   'maze.exe'                 creates default size maze, no solution and random seed " << std::endl;
   std::cout << "   'maze.exe 50 50'           creates 50 x 50 maze, no solution and random seed " << std::endl;
   std::cout << "   'maze.exe 50 50 true'      same as above, but displays solution as well." << std::endl;
   std::cout << "   'maze.exe 50 50 false 125' No solution, but use this seed." << std::endl;
}

int main(int argc, char *argv[])
{
  // read args
  std::size_t grid_height = 10;
  std::size_t grid_width = 30;
  bool printSolution = false;
  unsigned int seed = std::time({});
  if(argc >= 3)
  {
    grid_height = std::stoi(argv[1]);
    grid_width = std::stoi(argv[2]);
  }
  if(argc >= 4)
  {
    printSolution = std::string(argv[3]).compare("true") == 0;
  }
  if(argc >= 5)
  {
    seed = std::stoi(argv[4]);
  }
  if(argc == 2 | argc >= 6)
  {
    PrintHelp();
    return EXIT_FAILURE;
  }

  if(grid_width == 0 || grid_height == 0)
  {
    std::cerr << "Invalid grid size. Please provide two positive, non-zero integers." << std::endl;
    return EXIT_FAILURE;
  }

  // construct maze (and solution)
  auto maz = InitializeMaze(grid_height, grid_width);
  Trajectory solution;
  std::srand(seed); // set rand seed
  ConstructMaze(maz,solution);
  maz[0][0].wallOpen_Down = true; // start
  maz[grid_height-1][grid_width-1].wallOpen_Up = true; // end

  // Display
  _setmode(_fileno(stdout), _O_U16TEXT);
  std::wcout << std::endl << L"START" << std::endl;
  std::wcout << MazeToString(maz).str();
  std::wcout << std::wstring( 3*(grid_width-1), L' ' ) << L"END" << std::endl;

  IndicateSolution(maz,solution);

  // Display solution
  if(printSolution)
  {
    std::wcout << std::wstring( 10, L'\n' ) << std::endl;
    std::wcout << MazeToString(maz).str();
  }
  std::wcout << "seed: " << seed << std::endl;
  std::wcout << "Difficulty: " << CalculateDifficulty(maz) << std::endl;

  return EXIT_SUCCESS;
}