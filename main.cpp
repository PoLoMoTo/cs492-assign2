#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
using namespace std;

vector <array<int, 3>> getPageTable(string line, int totalPages, int pageSize){
    int pid = stoi(line.substr(0, line.find(' ')));
    int locations = stoi(line.substr(line.find(' ') + 1, line.length()));
    vector <array<int, 3>> pageTable;

    for (int i = totalPages; i < totalPages + (locations/pageSize); i++){
        pageTable.push_back({i, 0, 0});
    }

    return pageTable;
}

int main(int argc, char const *argv[]){
    if (argc != 6){
        cout << "Ur drunk go home m8";
        return 1;
    }

    string plistFile = argv[1];
    string ptraceFile = argv[2];
    int pageSize = stoi(argv[3]);
    string algorithm = argv[4];
    string paging = argv[5]; // + = Pre-paging     - = Demand paging
    string line;
    int mainPages = 512/pageSize;
    int mainMemory[mainPages];
    fill_n(mainMemory, mainPages, -1);
    vector<vector<array<int, 3>>> pageTables;
    int totalPrograms;
    int totalPages = 0;
    int counter = 1;
    
    ifstream plist (plistFile + ".txt");
    ifstream ptrace (ptraceFile + ".txt");

    
    if (plist.is_open()){
        while (getline(plist,line)){
            vector<array<int, 3>> newTable = getPageTable(line, totalPages, pageSize);
            pageTables.push_back(newTable);
            totalPages += newTable.size();
            // for (int i = 0; i < newTable.size(); i++){
            //     cout << newTable.at(i)[0] << '\n';
            // }
        }
        plist.close();
    }

    totalPrograms = pageTables.size();

    for (int i = 0; i < totalPrograms; i++){
        for (int j = 0; j < mainPages/totalPrograms && j < pageTables.at(i).size(); j++){
            pageTables.at(i).at(j)[1] = 1;
            mainMemory[((mainPages/totalPrograms)*i)+j] = pageTables.at(i).at(j)[0];
        }
    }

    // for (int i = 0; i < mainPages; i++)
    //     cout << mainMemory[i] << '\n';
    // for (int i = 0; i < pageTables.at(0).size(); i++)
    //     cout << pageTables.at(0).at(i)[0] << ", " << pageTables.at(0).at(i)[1] << '\n';

    int pagefaults = 0;

    if (ptrace.is_open()){
        while (getline(ptrace,line)){
            int pid = stoi(line.substr(0, line.find(' ')));
            int location = stoi(line.substr(line.find(' ') + 1, line.length()))-1;
            cout << pid << ", " << location << ", " << (location/pageSize) << '\n';
            if (!pageTables.at(pid).at(location/pageSize)[1]){
                pagefaults++;
            }
            counter++;
        }
        ptrace.close();
    }

    cout << pagefaults;
}