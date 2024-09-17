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
        getFastestRoute(): useStack(false), rSchemeProvided(false), isMapOutput(true), start(0,0,0), hangar(0,0,0), foundRoute(false){}

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

            string layoutType;
            string numLevels;
            string floorSize;

            getline(cin, layoutType);
            getline(cin, numLevels);
            getline(cin, floorSize);

            
            uint32_t numLevelsInt = static_cast<uint32_t>(stoul(numLevels));
            uint32_t floorSizeInt = static_cast<uint32_t>(stoul(floorSize));

            d.floors = numLevelsInt;
            d.rows = floorSizeInt;

            if (layoutType[0] == 'L')   {
                takeInDataList();
            }
            else{
                takeInDataMap();
            }
        } 


        void findFastestRoute() {
            if (useStack)   {
                findFastestRouteStack();
            }
            else    {
                findFastestRouteQueue();
            }
            if (foundRoute) {
                storePath(hangar);
            }
        } 

        void outputInstructions()   {
            if (isMapOutput)    {
                outputInstructionsM();
            }
            else    {
                cout << "//path taken" << endl;
                if (foundRoute) {
                    outputInstructionsL();
                }
            }
        }

    private:
        bool useStack;
        bool rSchemeProvided;
        bool isMapOutput;

        struct coord    {
            uint32_t floor;
            uint32_t row;
            uint32_t col;

            coord(): floor(0), row(0), col(0){}
            coord(uint32_t f, uint32_t r, uint32_t c): floor(f), row(r), col(c){}
            coord(const coord& copy):  floor(copy.floor), row(copy.row), col(copy.col){};

            bool operator!=(coord& rhs) const {
                return !(floor == rhs.floor && row == rhs.row && col == rhs.col);
            }
            bool operator==(coord& rhs) const {
                return floor == rhs.floor && row == rhs.row && col == rhs.col;
            }

            coord& operator=(const coord& rhs) {
                if (this != &rhs)   {
                    floor = rhs.floor;
                    row = rhs.row;
                    col = rhs.col;
                }
                return *this;
            }
            
        };

        struct location {
            bool discovered;
            char val;
            char prev;
            
            location(): discovered(false), val('a'), prev('\n'){}
            location(char c): discovered(false), val(c), prev('\n'){}
            location(const location& copy): discovered(copy.discovered),
             val(copy.val), prev(copy.prev){}

            location& operator=(const location& rhs) {
                if (this != &rhs)   {
                    discovered = rhs.discovered;
                    val = rhs.val;
                    prev = rhs.prev;
                }
                return *this;
            };
        };

        struct dimensions   {
            uint32_t rows;
            uint32_t floors;
        };

        //Level, row, column
        vector<vector<vector<location>>> layout;
        coord start;
        coord hangar;
        deque<coord> searchContainer;
        dimensions d;
        bool foundRoute;


        //check that each char is valid
        void takeInDataMap()    {
            layout.reserve(d.floors);
            vector<vector<location>> floorData;
            string fileData;
            uint32_t counter = 0;
            uint32_t floorCount = 0;


            while (getline(cin, fileData))  {
                vector<location> oneRow;
                if (fileData[0] == '/' || fileData.empty()) {
                    continue;
                }
                for (uint32_t i = 0; i < fileData.size(); ++i) {
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

            while(getline(cin, lineData))  {
                if (lineData[0] != '(') {
                    continue;
                }
                assert(lineData.size() >= 9);

                int counter = 0;
                string stLevel = "";
                string stRow = "";
                string stCol = "";
                char val = '\n';
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

                uint32_t sLevel = static_cast<uint32_t>(level);
                uint32_t sRow = static_cast<uint32_t>(row);
                uint32_t sCol = static_cast<uint32_t>(col);

                if (!isMapCharValid(val))    {
                     cerr << "Invalid map character" << endl;
                    exit(1);
                }


                if (val == 'S') {
                    start.col = sCol;
                    start.row = sRow;
                    start.floor = sLevel;
                }
                layout[sLevel][sRow][sCol] = location(val);
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
        void findFastestRouteStack()    {            
            layout[start.floor][start.row][start.col].discovered = true;
            searchContainer.push_back(start);

            while(!searchContainer.empty() && !foundRoute) {
                coord currentLocC = searchContainer.back();
                searchContainer.pop_back();

                foundRoute = checkNESW(currentLocC);
            }
        }
        

        //Queue FIFO: push_back, pop_front
        void findFastestRouteQueue()    {
            layout[start.floor][start.row][start.col].discovered = true;
            searchContainer.push_back(start);

            while(!searchContainer.empty() && !foundRoute) {
                coord currentLocC = searchContainer.front();
                searchContainer.pop_front();

                foundRoute = checkNESW(currentLocC);
            }
        }
    
    
        bool checkNESW(coord currCoord)   {
            if (currCoord.row > 0
                &&!layout[currCoord.floor][currCoord.row - 1][currCoord.col].discovered 
                && layout[currCoord.floor][currCoord.row - 1][currCoord.col].val != '#')  {

                location& north = layout[currCoord.floor][currCoord.row - 1][currCoord.col];
                north.prev = 's';
                north.discovered = true;
                searchContainer.push_back(coord(currCoord.floor, currCoord.row - 1, currCoord.col));

                if (north.val == 'H')   {
                    hangar = coord(currCoord.floor, currCoord.row - 1, currCoord.col);
                    
                    return true;
                }
            }

            if (currCoord.col < d.rows - 1
                &&!layout[currCoord.floor][currCoord.row][currCoord.col + 1].discovered 
                && layout[currCoord.floor][currCoord.row][currCoord.col + 1].val != '#')  {

                location& east = layout[currCoord.floor][currCoord.row][currCoord.col + 1];
                east.prev = 'w';
                east.discovered = true;
                searchContainer.push_back(coord(currCoord.floor, currCoord.row, currCoord.col + 1));

                if (east.val == 'H')   {
                    hangar = coord(currCoord.floor, currCoord.row, currCoord.col + 1);
                    return true;
                }
            }

            if (currCoord.row < d.rows - 1 
                && !layout[currCoord.floor][currCoord.row + 1][currCoord.col].discovered 
                && layout[currCoord.floor][currCoord.row + 1][currCoord.col].val != '#')  {

                location& south = layout[currCoord.floor][currCoord.row + 1][currCoord.col];
                south.prev = 'n';
                south.discovered = true;
                searchContainer.push_back(coord(currCoord.floor, currCoord.row + 1, currCoord.col));

                if (south.val == 'H')   {
                    hangar = coord(currCoord.floor, currCoord.row + 1, currCoord.col);
                    return true;
                }
            }

           if (currCoord.col > 0  
                && !layout[currCoord.floor][currCoord.row][currCoord.col - 1].discovered 
                && layout[currCoord.floor][currCoord.row][currCoord.col - 1].val != '#')  {

                location& west = layout[currCoord.floor][currCoord.row][currCoord.col - 1];
                west.discovered = true;
                west.prev = 'e';
                searchContainer.push_back(coord(currCoord.floor, currCoord.row, currCoord.col - 1));

                if (west.val == 'H')   {
                    hangar = coord(currCoord.floor, currCoord.row, currCoord.col - 1);
                    return true;
                }
            }

            if (layout[currCoord.floor][currCoord.row][currCoord.col].val == 'E')  {
                for (uint32_t i = 0; i < d.floors; ++i)  {
                    location& loc = layout[i][currCoord.row][currCoord.col];
                    if (currCoord.floor != i && !loc.discovered
                        && loc.val == 'E')   {
                        loc.discovered = true;
                        loc.prev = static_cast<char>('0' + currCoord.floor);
                        searchContainer.push_back(coord(i, currCoord.row, currCoord.col));
                     }
                 }
            }
            return false;
        };



        void storePath(coord goal)    {
            //if the previous place is to the __
 
            while(goal != start)   {
                location cLoc = layout[goal.floor][goal.row][goal.col];
                switch(cLoc.prev)   {
                    case 's':
                        layout[goal.floor][goal.row + 1][goal.col].val = 'n';
                        ++goal.row;
                        break;
                    case 'n':   
                        layout[goal.floor][goal.row - 1][goal.col].val = 's';
                        --goal.row;
                        break;
                    case 'e':
                        layout[goal.floor][goal.row][goal.col + 1].val = 'w';
                        ++goal.col; 
                        break;
                    case 'w':
                        layout[goal.floor][goal.row][goal.col - 1].val = 'e';
                        --goal.col; 
                        break;
                    default:
                        uint32_t floor1 = static_cast<uint32_t>(cLoc.prev - '0');
                        char cFloor = static_cast<char>('0' + goal.floor);
                        layout[floor1][goal.row][goal.col].val = cFloor;
                        goal.floor = floor1;
                        break;
                }
            }
        }

        void outputInstructionsM()  {
            cout << "Start in level " << start.floor 
            << ", row " << start.row << ", column " << start.col << endl;
            for (uint32_t i = 0; i < d.floors; ++i)  {
                cout << "//level " << i << "\n";
                for (uint32_t j = 0; j < d.rows; ++j)    {
                    for (uint32_t k = 0; k < d.rows; ++k)    {
                        cout << layout[i][j][k].val;
                    }
                    cout << "\n";
                }
            }
        }
        
        void outputInstructionsL()  {
            coord curr = start;
            while (curr != hangar)    {
                location cLoc = layout[curr.floor][curr.row][curr.col];
                cout << "(" << curr.floor << "," << curr.row << "," 
                << curr.col << "," << cLoc.val << ")\n";
                switch(cLoc.val){
                    case 'n':
                        --curr.row;
                        break;
                    case 's':
                        ++curr.row;
                        break;
                    case 'e':
                        ++curr.col;
                        break;
                    case 'w':
                        --curr.col;
                        break;
                    default:
                        curr.floor = static_cast<uint32_t>(cLoc.val - '0');
                        break;
                }
            }
        }
};

int main(int argc, char * argv[])   {
    std::ios::sync_with_stdio(false);

    getFastestRoute tester;
    tester.getMode(argc, argv);
    tester.takeInData();
    tester.findFastestRoute();
    tester.outputInstructions();
}