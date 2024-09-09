// Project Identifier: 950181F63D0A883F183EC0A5CC67B19928FE896A
#include <iostream>
#include <vector>
#include <getopt.h>
#include <algorithm>
#include <deque>
#include <map>
using namespace std;


class getFastestRoute   {

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

        // Fill in the double quotes, to match the mode and help options.
        while ((choice = getopt_long(argc, argv, "sqo:h", long_options, &index)) != -1) {
            switch (choice) {
                case 's':
                    useStack = true;

                case 'q': 
                    useStack = false;

                case 'o':   {
                    string arg{optarg};

                    if (arg != "M" && arg != "L") {
                        //TODO CHECK IF Exit SHOULD BE USED
                        // The first line of error output has to be a 'fixed' message
                        // for the autograder to show it to you.
                        cerr << "Error: invalid mode" << endl;
                        // The second line can provide more information, but you
                        // won't see it on the AG.
                        cerr << "  I don't recognize: " << arg << endl;
                        exit(1);
                    }  // if ..arg valid
                    
                    if (arg == "L") { // Maybe use ''
                        isMapOutput = false;
                    }
                    else    {
                        isMapOutput = true;
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
    }  // getMode()
    

    //TODO IMPLEMENT
    void takeInData()   {

        //Put in main
        std::ios::sync_with_stdio(false);

        string layoutType;
        string numLevels;
        string floorSize;

        //Maybe cin >> layoutType >> numLevels >> floorSize; ?
        getline(cin,layoutType);
        getline(cin, numLevels);
        getline(cin, floorSize);

        int numLevelsInt = stoi(numLevels);
        int floorSizeInt = stoi(floorSize);

        d.floors = numLevelsInt;
        d.rows = floorSizeInt;

        if (layoutType[0] == 'L')   {
            takeInDataList(numLevelsInt, floorSizeInt);
        }
        else{
            takeInDataMap(floorSizeInt, numLevelsInt);
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
        storePath(hangar);
    } 

    //TODO IMPLEMENT
    void outputInstructions()   {
        if (isMapOutput)    {
            cout << "Start in level " << start.floor <<", row " << start.row << ", column "<< start.col << "\n";
            outputInstructionsM();
        }
        else    {
            cout << "//path taken";
            outputInstructionsL();
        }
    }

    private:
        bool useStack;
        bool isMapOutput;
        int fileIndex;

        struct coord    {
            int floor;
            int row;
            int col;

            coord(): floor(-1), row(-1), col(-1){}
            coord(int f, int r, int c): floor(f), row(r), col(c){}
        };

        struct location {
            bool discovered = false;
            char val;
            location* prev;
            location* next;

            coord myCoord;

            location(char c): val(c){}
        };

        struct dimensions   {
            int rows;
            int floors;
        };

        //Level, row, column
        vector<vector<vector<location>>> layout;
        coord start;
        deque<location> searchContainer;
        dimensions d;
        location hangar;
        vector<location> path;


        void takeInDataMap(int floorSize, int numFloors)    {
            layout.reserve(numFloors);
            vector<vector<location>> floorData(floorSize);
            string fileData;
            int counter = 0;
            int floorCount = 0;


            while (getline(cin, fileData))  {
                vector<location> oneRow(floorSize);
                if (fileData[0] == '/') {
                    continue;
                }
                for (int i = 0; i < fileData.size(); ++i) {
                    oneRow.push_back(location(fileData[i]));

                    if (fileData[i] == 'S')   {
                        start.col = i;
                        start.row = counter;
                        start.floor = floorCount;
                    }
                }
                floorData.push_back(oneRow);
                ++counter;

                if (counter == floorSize)   {
                    layout.push_back(floorData);
                    floorData.clear();
                    ++floorCount;
                }

            }

        }

        void takeInDataList(int floorSize, int numFloors)   {

            //Fill Layout Grid with . with correct sizing
            vector<vector<location>> floorData(floorSize, vector<location>(floorSize, location('.')));
            layout.resize(numFloors, floorData);

            string lineData;
            while(getline(cin, lineData))  {
                if (lineData[0] != '(') {
                    continue;
                }

                char level = lineData[2];
                char row = lineData[4];
                char col = lineData[6];
                char val = lineData[8];

                if (val == 'S') {
                    start.col = col;
                    start.row = row;
                    start.floor = level;
                }

                layout[level][row][col] = location(val);
            }
        }

        //Stack LIFO: push_back, pop_back
        //Returns start location if hanger not found. 
        //Return of this function.myCoord = start means hanger not found
        //TODO build function to take out code duplication
        location findFastestRouteStack()    {
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

                if (currentLoc.val == 'E')  {
                    for (int i = 0; i < d.floors; ++i)  {
                        if (currCoord.floor != i)   {
                            location& loc = layout[i][currCoord.row][currCoord.col];
                            loc.myCoord = currCoord;
                            loc.myCoord.floor = i;
                            loc.discovered = true;
                            loc.prev = &currentLoc;
                            searchContainer.push_back(loc);
                        }
                    }
                }

                if (currCoord.row > 0) {
                    location& north = layout[currCoord.floor][currCoord.row - 1][currCoord.col];

                    if (!north.discovered && north.val != '#')  {
                        north.prev = &currentLoc;
                        searchContainer.push_back(north);
                        north.myCoord = currCoord;
                        north.discovered = true;
                        --north.myCoord.row;

                        if (north.val == 'H')   {
                            hangar = north;
                            return north;
                        }
                    }
                }
                if (currCoord.row < d.rows - 1)    {
                    location& south = layout[currCoord.floor][currCoord.row + 1][currCoord.col];

                    if (!south.discovered && south.val != '#')  {
                        south.prev = &currentLoc;
                        searchContainer.push_back(south);
                        south.myCoord = currCoord;
                        south.discovered = true;
                        ++south.myCoord.row;

                        if (south.val == 'H')   {
                            hangar = south;
                            return south;
                        }
                    }
                }
                if (currCoord.col > 0)  {
                    location& west = layout[currCoord.floor][currCoord.row][currCoord.col - 1];


                    if (!west.discovered && west.val != '#')  {
                        west.prev = &currentLoc;
                        searchContainer.push_back(west);
                        west.myCoord = currCoord;
                        west.discovered = true;
                        --west.myCoord.col;

                        if (west.val == 'H')   {
                            hangar = west;
                            return west;
                        }
                    }
                }
                if (currCoord.col < d.rows - 1) {
                    location& east = layout[currCoord.floor][currCoord.row][currCoord.col + 1];

                    if (!east.discovered && east.val != '#')  {
                        east.prev = &currentLoc;
                        searchContainer.push_back(east);
                        east.myCoord = currCoord;
                        east.discovered = true;
                        ++east.myCoord.col;

                        if (east.val == 'H')   {
                            hangar = east;
                            return east;
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
                if (currentLoc.val == 'E')  {
                    for (int i = 0; i < d.floors; ++i)  {
                        if (currCoord.floor != i)   {
                            location& loc = layout[i][currCoord.row][currCoord.col];
                            loc.myCoord = currCoord;
                            loc.myCoord.floor = i;
                            loc.discovered = true;
                            loc.prev = &currentLoc;
                            searchContainer.push_back(loc);
                        }
                    }
                }

                if (currCoord.row > 0) {
                    location& north = layout[currCoord.floor][currCoord.row - 1][currCoord.col];

                    if (!north.discovered && north.val != '#')  {
                        north.prev = &currentLoc;
                        searchContainer.push_back(north);
                        north.myCoord = currCoord;
                        north.discovered = true;
                        --north.myCoord.row;

                        if (north.val == 'H')   {
                            hangar = north;
                            return north;
                        }
                    }
                }
                if (currCoord.row < d.rows - 1)    {
                    location& south = layout[currCoord.floor][currCoord.row + 1][currCoord.col];

                    if (!south.discovered && south.val != '#')  {
                        south.prev = &currentLoc;
                        searchContainer.push_back(south);
                        south.myCoord = currCoord;
                        south.discovered = true;
                        ++south.myCoord.row;

                        if (south.val == 'H')   {
                            hangar = south;
                            return south;
                        }
                    }
                }
                if (currCoord.col > 0)  {
                    location& west = layout[currCoord.floor][currCoord.row][currCoord.col - 1];


                    if (!west.discovered && west.val != '#')  {
                        west.prev = &currentLoc;
                        searchContainer.push_back(west);
                        west.myCoord = currCoord;
                        west.discovered = true;
                        --west.myCoord.col;

                        if (west.val == 'H')   {
                            hangar = west;
                            return west;
                        }
                    }
                }
                if (currCoord.col < d.rows - 1) {
                    location& east = layout[currCoord.floor][currCoord.row][currCoord.col + 1];

                    if (!east.discovered && east.val != '#')  {
                        east.prev = &currentLoc;
                        searchContainer.push_back(east);
                        east.myCoord = currCoord;
                        east.discovered = true;
                        ++east.myCoord.col;

                        if (east.val == 'H')   {
                            hangar = east;
                            return east;
                        }
                    }
                }
            }
            hangar = startLoc;
            return startLoc;
        }
    
        void storePath(location goal)    {
            if (goal.prev == nullptr) {
                return;
            }
            if (goal.myCoord.col > goal.prev->myCoord.col)  {
                goal.prev->val = 'e';
                goal.prev->next = &goal;
            }
            else if (goal.myCoord.col < goal.prev->myCoord.col)  {
                goal.prev->val = 'w';
                goal.prev->next = &goal;
            }
            else if (goal.myCoord.row > goal.prev->myCoord.row)  {
                goal.prev->val = 's';
                goal.prev->next = &goal;
            }
            else if (goal.myCoord.row < goal.prev->myCoord.row)  {
                goal.prev->val = 'n';
                goal.prev->next = &goal;
            }
            else {
                goal.prev->val = goal.myCoord.floor;
                goal.prev->next = &goal;
            }
            return storePath(*goal.prev);
        }

        void outputInstructionsM()  {
            for (int i = 0; i < d.floors; ++i)  {
                cout << "//level " << i << "\n";
                for (int j = 0; j < d.rows; ++j)    {
                    for (int k = 0; k < d.rows; ++k)    {
                        cout << layout[i][j][k].val;
                    }
                    cout << "\n";
                }
            }
        }
        
        void outputInstructionsL()  {
            location curr = layout[start.floor][start.row][start.col];
            while (curr.next != nullptr)    {
                cout << "(" << curr.myCoord.floor << "," << curr.myCoord.row << "," 
                << curr.myCoord.col << "," << curr.val << ")\n";
                curr = *curr.next;
            }
        }
    
};

