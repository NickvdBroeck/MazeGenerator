# Maze Generator

![c++](https://img.shields.io/badge/language-c%2B%2B-blue)

## Intro

Based on a session I followed at SocratesBE 2025 Unconference.
Session was hosted by Vincent.
He gave us the algorithm to get started.
This repo was created just for fun.

## Algorithm

The base algorithm is as follows:

1. Create a grid of cells
2. Each cell has a number of walls and starts as 'unvisited'
3. Select random edge cell to start from (break the outside wall)
4. Find all neighboring cells that are 'unvisited':
   - If any such cell exists then:
     - Select one such neighbor at random
     - Break wall from previous cell to the selected neighbor (2 breaks, one per cell)
     - Move to new cell
     - Add new cell to 'trajectory'
     - Restart from step 4.
   - If no cells are available then:
     - backtrace to a previous cell that has free neighbors.
5. When all cells are visited, the algorithm ends.
6. Select another edge cell and break the wall. This acts as 'exit'.

This algorithm is guaranteed to visit all cells.
It is also guaranteed to not create loops and to create a single track from start to finish.
This algorithm can handle any shape of grid, including grids with holes and none rectangular grids.

## Solution

The algorithm always keeps the trajectory from start to the current cell.
Hence, if the current cell equals the end cell, then the trajectory equals the solution.
Store it before backtracing.
