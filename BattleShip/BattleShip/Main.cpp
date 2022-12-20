#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include "graphics.h"


const float CELL_SIZE = 100;

const float GRID_Y_OFFSET = 2 * CELL_SIZE;
const float PIXEL_WIDTH = 5 * CELL_SIZE;
const float PIXEL_HEIGHT = 6 * CELL_SIZE;
const float CANVAS_WIDTH = 10 * CELL_SIZE;
const float CANVAS_HEIGHT = 12 * CELL_SIZE;

const float lineWidth = 10;

class CursorPosition { //convert pixel to canvas units
public:
    float x;
    float y;

    CursorPosition(int x, int y) {
        this->x = (x / PIXEL_WIDTH) * CANVAS_WIDTH;
        this->y = (y / PIXEL_HEIGHT) * CANVAS_HEIGHT;
    }
};

class Cell {
public:
    float center_x, center_y, width, height;
    float lineWidth = 10;
    bool highlighted = false;
    bool hasShip = false;
    bool miss = false;
    bool hit = false;

    Cell(float cx, float cy, float w, float h) {
        center_x = cx;
        center_y = cy;
        width = w;
        height = h;
    }

    bool toggleHighlight() {
        highlighted = !highlighted;
        return !highlighted;
    }

    void draw() {
        graphics::Brush br;
        br.outline_width = lineWidth;
        br.fill_opacity = 0.0f;
        if (highlighted) {
            graphics::Brush br_hl;
            br_hl.fill_color[0] = 1.0f;
            br_hl.fill_color[1] = 1.0f;
            br_hl.fill_color[2] = 0.0f;
            br_hl.fill_opacity = 0.5f;
         
            graphics::drawRect(center_x, center_y, width, height, br_hl);
        }
        else {
            graphics::Brush br_hl;
            br_hl.fill_opacity = 0.5f;

            if (hit) {
                br_hl.fill_color[0] = 1.0f;
                br_hl.fill_color[1] = 0.0f;
                br_hl.fill_color[2] = 0.0f;
                graphics::drawRect(center_x, center_y, width, height, br_hl);
            }
            else if (miss) {
                br_hl.fill_color[0] = 1.0f;
                br_hl.fill_color[1] = 1.0f;
                br_hl.fill_color[2] = 1.0f;
                br_hl.fill_opacity = 0.5f;
                graphics::drawRect(center_x, center_y, width, height, br_hl);
            }
        }
        graphics::drawRect(center_x, center_y, width, height, br);
    }
};


class Grid {
public:
    std::vector<std::vector<Cell*>> grid;
    int highlightedCnt = 0;
    int hits = 0;

    Grid(float cs, float cw, float offset)
    {
        for (int j = cs / 2; j < cw; j += cs) {
            std::vector<Cell*> row;
            for (int i = cs / 2; i < cw; i += cs) {
                Cell* cell = new Cell(i, j + offset, cs, cs);
                row.push_back(cell);
            }
            grid.push_back(row);
        }
    }

    void toggleCellHighlight(CursorPosition pos) { //find from canvas units to index
        int x = pos.x / CELL_SIZE;
        if (pos.y > GRID_Y_OFFSET) {
            int y = (pos.y - GRID_Y_OFFSET) / CELL_SIZE;
            if (grid[y][x]->toggleHighlight()) { //open cell or close cell
                highlightedCnt--;
            }
            else {
                highlightedCnt++;
            }
        }
    }

    std::vector<std::vector<int>> getSelectedCells() {
        std::vector<std::vector<int>> selected;
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++) {
                if (grid[i][j]->highlighted) {
                    std::vector<int> v;
                    v.push_back(i);
                    v.push_back(j);
                    selected.push_back(v);
                }
            }
        }
        return selected;
    }

    void deselectAll() {
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++) {
                if (grid[i][j]->highlighted) {
                    grid[i][j]->toggleHighlight();
                }
            }
        }
        highlightedCnt = 0;
    }

    bool placeShip(int c1i, int c1j, int c2i, int c2j) { //Grid must know where are the ship 
        if (c1i == c2i) {
            int i = c1i;
            int j_min = std::min(c1j, c2j); 
            int j_max = std::max(c1j, c2j);
            for (int j = j_min; j <= j_max; j++) {
                if (grid[i][j]->hasShip) { //if cell hasShip to know grid and make an intersection check
                    return false;
                }
            }
            for (int j = j_min; j <= j_max; j++) {
                grid[i][j]->hasShip = true;
            }
        }
        else {
            int i_min = std::min(c1i, c2i);
            int i_max = std::max(c1i, c2i);
            int j = c1j;
            for (int i = i_min; i <= i_max; i++) {
                if (grid[i][j]->hasShip) {
                    return false;
                }
            }
            for (int i = i_min; i <= i_max; i++) {
                grid[i][j]->hasShip = true;
            }
        }
        return true;
    }

    void draw() {
        for (int j = 0; j < 10; j++) {
            for (int i = 0; i < 10; i++) {
                grid[i][j]->draw();
            }
        }
    }
};


class Ship {
public:
    float center_x, center_y, width, height;
    bool vertical;
    
    Ship(float cx, float cy, float w, float h, bool vertical) {
        center_x = cx;
        center_y = cy;
        width = w;
        height = h;
        this->vertical = vertical;
    }

    void draw() {
        graphics::Brush br;
        br.texture = "assets\\ship1.png";
        if (vertical)
            graphics::setOrientation(90);
        graphics::drawRect(center_x, center_y, width, height, br);
        if (vertical)
            graphics::resetPose();
    }
};


struct Game {
    Grid* grids[2];
    Ship* ships[2][3];

    std::string text;

    bool player1 = true;
    bool waitingNextPlayer = false;
    bool playerShot = false;
    bool waitingMove = true;

    bool gameOver = false;

    void init() {
        graphics::setFont("assets\\orange juice 2.0.ttf");
        grids[0] = new Grid(CELL_SIZE, CANVAS_WIDTH, GRID_Y_OFFSET);
        grids[1] = new Grid(CELL_SIZE, CANVAS_WIDTH, GRID_Y_OFFSET);
        for (int i = 0; i <2; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                ships[i][j] = nullptr;
            }
        }
    }

    void changePlayer() {
        player1 = !player1;
        waitingMove = true;
    }

    bool shoot() { //first highlited and then you set selected Cell that is x,y  and then shoot 
        int pid = player1 ? 0 : 1;
        int other_pid = player1 ? 1 : 0;
        std::vector<int> cellCoords = grids[pid]->getSelectedCells()[0];
        int x = cellCoords[0];
        int y = cellCoords[1];
        Cell* cell = grids[pid]->grid[x][y];
        Cell* otherCell = grids[other_pid]->grid[x][y];
        if (!cell->miss && !cell->hit) { //check other player grid if cell hasShip,then cell if hit or miss draw green or red
            if (otherCell->hasShip) {
                cell->hit = true;
                grids[pid]->hits++;
            }
            else
                cell->miss = true;
            return true;
        }
        return false;
    }

    std::string getText() {
        int pid = player1 ? 1 : 2;
        if (waitingNextPlayer)
            pid = pid % 2 + 1;
        return "Player " + std::to_string(pid) + ": " + text;
    }
};


// The custom callback function that the library calls 
// to check for and set the current application state.
void update(float ms)
{
    Game* game = (Game*)graphics::getUserData();

    int pid = game->player1 ? 0 : 1;

    if (game->waitingNextPlayer) {
        game->text = "Click anywhere to play";
    }
    // player must place ship 0
    else if (game->ships[pid][0] == nullptr) {
        int selected = game->grids[pid]->highlightedCnt; //is only for the start of the game when you click to set ship
        // player has not selected any cell
        if (selected == 0)
            game->text = "Select starting cell for 2-cell ship";
        // player has selected one cell
        else if (selected == 1)
            game->text = "Select ending cell for 2-cell ship";
        // player has selected 2 cells; try to place ship
        else if (selected == 2) {
            std::vector<std::vector<int>> sc = game->grids[pid]->getSelectedCells();
            std::vector<int> c1 = sc[0];
            std::vector<int> c2 = sc[1];
            Cell* cell1 = game->grids[pid]->grid[c1[0]][c1[1]];
            Cell* cell2 = game->grids[pid]->grid[c2[0]][c2[1]];
            float cx = (cell1->center_x + cell2->center_x) / 2;
            float cy = (cell1->center_y + cell2->center_y) / 2;

            if ((c1[0] == c2[0]) && (abs(c1[1] - c2[1]) == 1)) { //same x and length of ship is 2
                game->ships[pid][0] = new Ship(cx, cy, 2 * CELL_SIZE, CELL_SIZE, false);
                game->grids[pid]->placeShip(c1[0], c1[1], c2[0], c2[1]);
            } else if ((c1[1] == c2[1]) && (abs(c1[0] - c2[0]) == 1)) { //same y and length of ship is 2
                game->ships[pid][0] = new Ship(cx, cy, 2 * CELL_SIZE, CELL_SIZE, true);
                game->grids[pid]->placeShip(c1[0], c1[1], c2[0], c2[1]);
            }

            game->grids[pid]->deselectAll(); //deselect all yellow cells after place ship
        }
    }
    // player must place ship 1
    else if (game->ships[pid][1] == nullptr) {
        int selected = game->grids[pid]->highlightedCnt;
        // player has not selected any cell
        if (selected == 0)
            game->text = "Select starting cell for 3-cell ship";
        // player has selected one cell
        else if (selected == 1)
            game->text = "Select ending cell for 3-cell ship";
        // player has selected 2 cells; try to place ship
        else if (selected == 2) {
            std::vector<std::vector<int>> sc = game->grids[pid]->getSelectedCells();
            std::vector<int> c1 = sc[0];
            std::vector<int> c2 = sc[1];
            Cell* cell1 = game->grids[pid]->grid[c1[0]][c1[1]];
            Cell* cell2 = game->grids[pid]->grid[c2[0]][c2[1]];
            float cx = (cell1->center_x + cell2->center_x) / 2;
            float cy = (cell1->center_y + cell2->center_y) / 2;

            if ((c1[0] == c2[0]) && (abs(c1[1] - c2[1]) == 2)) {
                if (game->grids[pid]->placeShip(c1[0], c1[1], c2[0], c2[1]))//first check if you have intersection and then set
                    game->ships[pid][1] = new Ship(cx, cy, 3 * CELL_SIZE, CELL_SIZE, false);
            }
            else if ((c1[1] == c2[1]) && (abs(c1[0] - c2[0]) == 2)) { //if vertical
                if (game->grids[pid]->placeShip(c1[0], c1[1], c2[0], c2[1]))
                    game->ships[pid][1] = new Ship(cx, cy, 3 * CELL_SIZE, CELL_SIZE, true); //set orientation(90) 
            }

            game->grids[pid]->deselectAll();
        }
    }
    // player must place ship 2
    else if (game->ships[pid][2] == nullptr) {
        int selected = game->grids[pid]->highlightedCnt;
        // player has not selected any cell
        if (selected == 0)
            game->text = "Select starting cell for 4-cell ship";
        // player has selected one cell
        else if (selected == 1)
            game->text = "Select ending cell for 4-cell ship";
        // player has selected 2 cells; try to place ship
        else if (selected == 2) {
            std::vector<std::vector<int>> sc = game->grids[pid]->getSelectedCells();
            std::vector<int> c1 = sc[0];
            std::vector<int> c2 = sc[1];
            Cell* cell1 = game->grids[pid]->grid[c1[0]][c1[1]];
            Cell* cell2 = game->grids[pid]->grid[c2[0]][c2[1]];
            float cx = (cell1->center_x + cell2->center_x) / 2;
            float cy = (cell1->center_y + cell2->center_y) / 2;

            if ((c1[0] == c2[0]) && (abs(c1[1] - c2[1]) == 3)) {
                if (game->grids[pid]->placeShip(c1[0], c1[1], c2[0], c2[1]))
                    game->ships[pid][2] = new Ship(cx, cy, 4 * CELL_SIZE, CELL_SIZE, false);
            }
            else if ((c1[1] == c2[1]) && (abs(c1[0] - c2[0]) == 3)) {
                if (game->grids[pid]->placeShip(c1[0], c1[1], c2[0], c2[1]))
                    game->ships[pid][2] = new Ship(cx, cy, 4 * CELL_SIZE, CELL_SIZE, true);
            }

            game->grids[pid]->deselectAll();
            if (game->ships[pid][2] != nullptr) { //middleware to play second player
                game->waitingNextPlayer = true;
            }
        }
    }

    graphics::MouseState mouse;
    graphics::getMouseState(mouse);
    if (game->ships[pid][2] != nullptr) {
        /* THE FOLLOWING IS THE GAME LOOP AFTER INIT STUFF */
        if (game->gameOver) {
            game->text = "Congratulations, you won!";
        }
        else if (mouse.button_left_released) //is true every time you click.
        {
            if (game->waitingNextPlayer) {
                game->waitingNextPlayer = false;
                game->changePlayer();
            }
            else if (game->waitingMove) {
                CursorPosition pos(mouse.cur_pos_x, mouse.cur_pos_y);
                game->grids[pid]->toggleCellHighlight(pos);
                bool valid = game->shoot();
                if (valid) {
                    game->waitingMove = false;
                    game->playerShot = true;
                    if (game->grids[pid]->hits == 9) {
                        game->gameOver = true;
                    }
                    game->text = "Click anywhere to end turn";
                }
                game->grids[pid]->deselectAll();
            }
            else if (game->playerShot) { //one state also to show the cell hit or miss
                game->playerShot = false;
                game->waitingMove = false;
                game->grids[pid]->deselectAll();
                game->waitingNextPlayer = true;
            }
        }
        else if (!game->waitingNextPlayer && !game->playerShot) {
            game->text = "Shoot at a cell!";
        }
    }
    else {
        if (mouse.button_left_released) {
            CursorPosition pos(mouse.cur_pos_x, mouse.cur_pos_y);
            game->grids[pid]->toggleCellHighlight(pos);
        }
    }
}

void drawBackground() {
    graphics::Brush br;
    br.texture = "assets\\sea.png";
    br.outline_opacity = 0.0f;
    br.fill_opacity = 0.5f;
    graphics::setOrientation(90);
    graphics::drawRect(PIXEL_WIDTH, PIXEL_HEIGHT, CANVAS_HEIGHT, CANVAS_WIDTH, br);
    graphics::resetPose();
}

void drawText() {
    Game* game = (Game*)graphics::getUserData();

    graphics::Brush br;
    br.fill_secondary_color[0] = 1.0f;
    br.fill_secondary_color[1] = 1.0f;
    br.fill_secondary_color[2] = 1.0f;
    graphics::drawText(0, CELL_SIZE, 40, game->getText(), br);
}

void drawGrid() {
    Game* game = (Game*)graphics::getUserData();

    if (!game->waitingNextPlayer && !game->gameOver) {
        if (game->player1)
            game->grids[0]->draw();
        else
            game->grids[1]->draw();
    }
}

void drawShips() {
    Game* game = (Game*)graphics::getUserData();

    if (!game->waitingNextPlayer && !game->gameOver) {
        if (game->player1)
            for (Ship* ship : game->ships[0])
            {
                if (ship != nullptr) ship->draw();
            }
        else {
            for (Ship* ship : game->ships[1])
            {
                if (ship != nullptr) ship->draw();
            }
        }
    }
}

// The window content drawing function.
void draw()
{
    drawBackground();
    drawText();
    drawGrid();
    drawShips();
}

int main()
{
    Game game;

    graphics::createWindow(PIXEL_WIDTH, PIXEL_HEIGHT, "Board Game");

    game.init();

    graphics::setUserData(&game);
    graphics::setDrawFunction(draw);
    graphics::setUpdateFunction(update);

    graphics::setCanvasSize(CANVAS_WIDTH, CANVAS_HEIGHT);
    graphics::setCanvasScaleMode(graphics::CANVAS_SCALE_FIT);

    graphics::startMessageLoop();
    graphics::destroyWindow();

    return 0;
}
