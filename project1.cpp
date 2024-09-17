// Project Identifier: 950181F63D0A883F183EC0A5CC67B19928FE896A
#include <iostream>
#include <vector>
#include <getopt.h>
#include <algorithm>
#include <deque>
#include <map>
#include <cassert>
using namespace std;


class getFastestRoute   {
    public:
        getFastestRoute(): useStack(false), rSchemeProvided(false), isMapOutput(true), foundRoute(false){}

        void printHelp()    {
            //TODO IMPLEMENT
            return;
        }
        
        void getMode(int argc, char * argv[]) {
            // These are used with getopt_long()
            opterr = false; // Let us handle all error output for command line options
            int choice;
            int index = 0;
            option long_options[] = {
                {"stack", no_argument, nullptr, 's'},
                {"queue", no_argument, nullptr, 'q'},
                {"output", required_argument, nullptr, 'o'},
                {"help", no_argument, nullptr, 'h'},
                { nullptr, 0, nullptr, '\0' },
            };  // long_options[]

            // Check for:
            //illegal input characters, such as 'f'
            //for than one '-stack' or '-queue' on command line
                //i.e. ./ship --queue -s < infile > outfile
            //no option specified for stack or queue
            //two options specified for stack and queue
            //after -o will be correct (no need to check for other characters than O or M in output, or files to output to)
            int counter = 0;
            rSchemeProvided = false;

            while ((choice = getopt_long(argc, argv, "sqo:h", long_options, &index)) != -1) {
                switch (choice) {
                    case 's':
                        useStack = true;
                        rSchemeProvided = true;
                        if (counter > 0)    {
                            cerr << "Multiple routing modes specified" << endl;
                            exit(1);
                        }
                        ++counter;
                        break;
                    case 'q': 
                        useStack = false;
                        rSchemeProvided = true;
                        if (counter > 0)    {
                            cerr << "Multiple routing modes specified" << endl;
                            exit(1);
                        }
                        ++counter;
                        break;
                    case 'o':   {
                        if (!optarg)  {
                            isMapOutput = true;
                            break;
                        }
                        char arg = *optarg;

                        if (arg != 'M' && arg != 'L') {
                            // The first line of error output has to be a 'fixed' message
                            // for the autograder to show it to you.
                            cerr << "Invalid output mode specified" << endl;
                            exit(1);
                        }  // if ..arg valid
                        
                        if (arg == 'L') { // Maybe use ''
                            isMapOutput = false;
                            break;
                        }
                        else    {
                            isMapOutput = true;
                            break;
                        }
                    }
                    case 'h':
                        printHelp();
                        exit(0);
                    default:
                        cerr << "Error: invalid option" << endl;
                        exit(1);
                }  // switch ..choice
            }  // while
            if (!rSchemeProvided)   {
                cerr << "No routing mode specified" << endl;
                exit(1);
            }
        }  // getMode()
        

        void takeInData()   {

            //Put in main
            std::ios::sync_with_stdio(false);

            string layoutType;
            string numLevels;
            string floorSize;

            getline(cin, layoutType);
            getline(cin, numLevels);
            getline(cin, floorSize);

            cout << "LayoutType: " << layoutType << endl;
            cout << "numLevels: " << numLevels << endl;
            cout << "floorSize: " << floorSize << endl;



            size_t numLevelsInt = static_cast<size_t>(stoul(numLevels));
            size_t floorSizeInt = static_cast<size_t>(stoul(floorSize));

            d.floors = numLevelsInt;
            d.rows = floorSizeInt;

            if (layoutType[0] == 'L')   {
                takeInDataList();
            }
            else{
                takeInDataMap();
            }
        } //layout type M


        //TODO IMPLEMENT
        void findFastestRoute() {
            if (useStack)   {
                findFastestRouteStack();
            }
            else    {
                findFastestRouteQueue();
            }
            storePath(&hangar);
        } 

        //TODO IMPLEMENT
        void outputInstructions()   {
        if (isMapOutput)    {
            cout << "Start in level " << start.floor <<", row " << start.row << ", column "<< start.col << "\n";
            outputInstructionsM();
        }
        else    {
            cout << "//path taken";
            if (foundRoute) {
                outputInstructionsL();
            }
            else    {
                cerr << "Path not found" << endl;
            }
        }
    }

    private:
        bool useStack;
        bool rSchemeProvided;
        bool isMapOutput;

        struct coord    {
            size_t floor;
            size_t row;
            size_t col;

            coord(): floor(0), row(0), col(0){}
            coord(size_t f, size_t r, size_t c): floor(f), row(r), col(c){}
        };

        struct location {
            bool discovered;
            char val;
            location* prev;
            location* next;

            coord myCoord;

            location(): discovered(false), val('a'), prev(nullptr), next(nullptr){}
            location(char c): discovered(false), val(c){}
        };

        struct dimensions   {
            size_t rows;
            size_t floors;
        };

        //Level, row, column
        vector<vector<vector<location>>> layout;
        coord start;
        deque<location> searchContainer;
        dimensions d;
        location hangar;
        bool foundRoute;


        //check that each char is valid
        void takeInDataMap()    {
            layout.reserve(d.floors);
            vector<vector<location>> floorData;
            string fileData;
            size_t counter = 0;
            size_t floorCount = 0;


            while (getline(cin, fileData))  {
                vector<location> oneRow;
                if (fileData[0] == '/' || fileData.empty()) {
                    continue;
                }
                for (size_t i = 0; i < fileData.size(); ++i) {
                    if (!isMapCharValid(fileData[i]))  {
                        cerr << "Invalid map character" << endl;
                        exit(1);
                    }

                    oneRow.push_back(location(fileData[i]));

                    if (fileData[i] == 'S')   {
                        start.col = i;
                        start.row = counter;
                        start.floor = floorCount;
                    }
                }
                floorData.push_back(oneRow);
                ++counter;

                if (counter == d.rows)   {
                    layout.push_back(floorData);
                    floorData.clear();
                    ++floorCount;
                    counter = 0;
                }
            }
        }
        
        //ERROR CHECK: check that for each coordinate, the level row and column are valid
        //check that each val is valid
        void takeInDataList()   {
            //Fill Layout Grid with . with correct sizing
            vector<vector<location>> floorData(d.rows, vector<location>(d.rows, location('.')));
            layout.resize(d.floors, floorData);

            string lineData;
            vector<string> line;

            while(getline(cin, lineData))  {
                if (lineData[0] != '(') {
                    continue;
                }
                assert(lineData.size() >= 9);

                int counter = 0;
                string stLevel = "";
                string stRow = "";
                string stCol = "";
                char val;
                for (auto l : lineData) {
                    if (l != ',' && l != '(' && l != ')')   {
                        if (counter == 0)   {
                            stLevel += l;
                        }
                        else if (counter == 1)  {
                            stRow += l;
                        }
                        else if (counter == 2)  {
                            stCol += l;
                        }
                        else    {
                            val = l;
                        }
                    }
                    if (l == ',')   {
                        ++counter;
                    }
                }

                
                int level = stoi(stLevel);
                int row = stoi(stRow);
                int col = stoi(stCol);
                

                if (level >= (int)d.floors || level < 0) {
                    cerr << "Invalid map level" << endl;
                    exit(1);
                }
                if (row >= (int)d.rows || row < 0)   {
                    cerr << "Invalid map row" << endl;
                    exit(1);
                }
                if (col >= (int)d.rows || col < 0)   {
                    cerr << "Invalid map column" << endl;
                    exit(1);
                }

                size_t sLevel = static_cast<size_t>(level);
                size_t sRow = static_cast<size_t>(row);
                size_t sCol = static_cast<size_t>(col);

                if (!isMapCharValid(val))    {
                     cerr << "Invalid map character" << endl;
                    exit(1);
                }


                if (val == 'S') {
                    start.col = sCol;
                    start.row = sRow;
                    start.floor = sLevel;
                }
                location l(val);
                layout[sLevel][sRow][sCol] = l;
            }
        }


        bool isMapCharValid(char val)   {
            if (val == 'S') {
                return true;
            }
            if (val == 'E') {
                return true;
            }
            if (val == 'H') {
                return true;
            }
            if (val == '.') {
                return true;
            }
            if (val == '#') {
                return true;
            }
            return false;    
        }

        //Stack LIFO: push_back, pop_back
        //Returns start location if hanger not found. 
        //Return of this function.myCoord = start means hanger not found
        //TODO build function to take out code duplication
        location findFastestRouteStack()    {
            if (layout.empty() || layout[start.floor].empty() || layout[start.floor][start.row].empty()) {
                cerr << "Layout vector is not properly initialized or accessed out of bounds" << endl;
                exit(1);
            }
            
            location& startLoc = layout[start.floor][start.row][start.col];
            startLoc.myCoord = start;
            startLoc.prev = nullptr;
            searchContainer.push_back(startLoc);
            startLoc.discovered = true;

            while(!searchContainer.empty()) {
                location currentLoc = searchContainer.back();
                coord currCoord = currentLoc.myCoord;
                searchContainer.pop_back();

                if (currentLoc.val == 'H')  {
                    hangar = currentLoc;
                    return currentLoc;
                }

                if (currCoord.row > 0) {
                    location& north = layout[currCoord.floor][currCoord.row - 1][currCoord.col];

                    if (!north.discovered && north.val != '#')  {
                        north.prev = &currentLoc;
                        searchContainer.push_back(north);
                        north.myCoord = {currCoord.floor, currCoord.row - 1, currCoord.col};
                        north.discovered = true;

                        if (north.val == 'H')   {
                            hangar = north;
                            foundRoute = true;
                            return north;
                        }
                    }
                }

                if (currCoord.col < d.rows - 1) {
                    location& east = layout[currCoord.floor][currCoord.row][currCoord.col + 1];

                    if (!east.discovered && east.val != '#')  {
                        east.prev = &currentLoc;
                        searchContainer.push_back(east);
                        east.myCoord = {currCoord.floor, currCoord.row, currCoord.col + 1};
                        east.discovered = true;

                        if (east.val == 'H')   {
                            hangar = east;
                            foundRoute = true;
                            return east;
                        }
                    }
                }

                if (currCoord.row < d.rows - 1)    {
                    location& south = layout[currCoord.floor][currCoord.row + 1][currCoord.col];

                    if (!south.discovered && south.val != '#')  {
                        south.prev = &currentLoc;
                        searchContainer.push_back(south);
                        south.myCoord = {currCoord.floor, currCoord.row + 1, currCoord.col};
                        south.discovered = true;

                        if (south.val == 'H')   {
                            hangar = south;
                            foundRoute = true;
                            return south;
                        }
                    }
                }

               if (currCoord.col > 0)  {
                    location& west = layout[currCoord.floor][currCoord.row][currCoord.col - 1];


                    if (!west.discovered && west.val != '#')  {
                        west.prev = &currentLoc;
                        searchContainer.push_back(west);
                        west.myCoord = {currCoord.floor, currCoord.row, currCoord.col - 1};
                        west.discovered = true;

                        if (west.val == 'H')   {
                            hangar = west;
                            foundRoute = true;
                            return west;
                        }
                    }
                }
                if (currentLoc.val == 'E')  {
                    for (size_t i = 0; i < d.floors; ++i)  {
                        location& loc = layout[i][currCoord.row][currCoord.col];
                        if (currCoord.floor != i && !loc.discovered
                            && loc.val == 'E')   {
                            loc.myCoord = {i, currCoord.row, currCoord.col};
                            loc.discovered = true;
                            loc.prev = &currentLoc;
                            searchContainer.push_back(loc);
                        }
                    }
                }
            }
            hangar = startLoc;
            return startLoc;
        }
        

        //Queue FIFO: push_back, pop_front
        //TODO build function to take out code duplication
        //TODO CHECK ELEVATORS
        location findFastestRouteQueue()    {
            location& startLoc = layout[start.floor][start.row][start.col];
            startLoc.myCoord = start;
            startLoc.prev = nullptr;
            searchContainer.push_back(startLoc);
            startLoc.discovered = true;


            while(!searchContainer.empty()) {
                location currentLoc = searchContainer.front();
                coord currCoord = currentLoc.myCoord;
                searchContainer.pop_front();

                if (currentLoc.val == 'H')  {
                    hangar = currentLoc;
                    return currentLoc;
                }

                if (currCoord.row > 0) {
                    location& north = layout[currCoord.floor][currCoord.row - 1][currCoord.col];

                    if (!north.discovered && north.val != '#')  {
                        north.prev = &currentLoc;
                        searchContainer.push_back(north);
                        north.myCoord = {currCoord.floor, currCoord.row - 1, currCoord.col};
                        north.discovered = true;

                        if (north.val == 'H')   {
                            hangar = north;
                            foundRoute = true;
                            return north;
                        }
                    }
                }

                if (currCoord.col < d.rows - 1) {
                    location& east = layout[currCoord.floor][currCoord.row][currCoord.col + 1];

                    if (!east.discovered && east.val != '#')  {
                        east.prev = &currentLoc;
                        searchContainer.push_back(east);
                        east.myCoord = {currCoord.floor, currCoord.row, currCoord.col + 1};
                        east.discovered = true;

                        if (east.val == 'H')   {
                            hangar = east;
                            foundRoute = true;
                            return east;
                        }
                    }
                }

                if (currCoord.row < d.rows - 1)    {
                    location& south = layout[currCoord.floor][currCoord.row + 1][currCoord.col];

                    if (!south.discovered && south.val != '#')  {
                        south.prev = &currentLoc;
                        searchContainer.push_back(south);
                        south.myCoord = {currCoord.floor, currCoord.row + 1, currCoord.col};
                        south.discovered = true;

                        if (south.val == 'H')   {
                            hangar = south;
                            foundRoute = true;
                            return south;
                        }
                    }
                }

               if (currCoord.col > 0)  {
                    location& west = layout[currCoord.floor][currCoord.row][currCoord.col - 1];


                    if (!west.discovered && west.val != '#')  {
                        west.prev = &currentLoc;
                        searchContainer.push_back(west);
                        west.myCoord = {currCoord.floor, currCoord.row, currCoord.col - 1};
                        west.discovered = true;

                        if (west.val == 'H')   {
                            hangar = west;
                            foundRoute = true;
                            return west;
                        }
                    }
                }
                if (currentLoc.val == 'E')  {
                    for (size_t i = 0; i < d.floors; ++i)  {
                        location& loc = layout[i][currCoord.row][currCoord.col];
                        if (currCoord.floor != i && !loc.discovered
                            && loc.val == 'E')   {
                            loc.myCoord = {i, currCoord.row, currCoord.col};
                            loc.discovered = true;
                            loc.prev = &currentLoc;
                            searchContainer.push_back(loc);
                        }
                    }
                }
            }
            hangar = startLoc;
            return startLoc;
        }
    
        void storePath(location* goal)    {
            if (goal->prev == nullptr)  {
                return;
            }

            if (goal->myCoord.col > goal->prev->myCoord.col)  {
                goal->prev->val = 'e';
            }
            else if (goal->myCoord.col < goal->prev->myCoord.col)  {
                goal->prev->val = 'w';
            }
            else if (goal->myCoord.row > goal->prev->myCoord.row)  {
                goal->prev->val = 'n';
            }
            else if (goal->myCoord.row < goal->prev->myCoord.row)  {
                goal->prev->val = 's';
            }
            else {
                goal->prev->val = static_cast<char>(goal->myCoord.floor);
            }

            return storePath(goal->prev);
        }

        void outputInstructionsM()  {
            for (size_t i = 0; i < d.floors; ++i)  {
                cout << "//level " << i << "\n";
                for (size_t j = 0; j < d.rows; ++j)    {
                    for (size_t k = 0; k < d.rows; ++k)    {
                        cout << layout[i][j][k].val;
                    }
                    cout << "\n";
                }
            }
        }
        
        void outputInstructionsL()  {
            location curr = layout[start.floor][start.row][start.col];
            while (curr.val != 'H')    {
                cout << "(" << curr.myCoord.floor << "," << curr.myCoord.row << "," 
                << curr.myCoord.col << "," << curr.val << ")\n";
                
                if (curr.val == 'n')    {
                    curr = layout[curr.myCoord.floor][curr.myCoord.row + 1][curr.myCoord.col];
                }
                if (curr.val == 's')    {
                    curr = layout[curr.myCoord.floor][curr.myCoord.row - 1][curr.myCoord.col];
                }
                if (curr.val == 'e')    {
                    curr = layout[curr.myCoord.floor][curr.myCoord.row][curr.myCoord.col + 1];
                }
                if (curr.val == 'w')    {
                    curr = layout[curr.myCoord.floor][curr.myCoord.row][curr.myCoord.col - 1];
                }
                else    {
                    cerr << curr.val << endl;
                    int floor = int(curr.val);
                    size_t sFloor = static_cast<size_t>(floor);
                    curr = layout[sFloor][curr.myCoord.row][curr.myCoord.col];
                }
            }
        }
    
};

int main(int argc, char * argv[])   {
    getFastestRoute tester;
    tester.getMode(argc, argv);
    tester.takeInData();
    tester.findFastestRoute();
    tester.outputInstructions();
}