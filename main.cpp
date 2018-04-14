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

    for (int i = totalPages; i < totalPages + ((locations + pageSize - 1)/pageSize); i++){
        pageTable.push_back({i, 0, 0});
    }

    return pageTable;
}

int main(int argc, char const *argv[]){
    if (argc != 6){
        cout << "Ur drunk go home m8" << endl;
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
    unsigned long counter = 1;
    int firstIn = 0;
    
    ifstream plist (plistFile + ".txt");
    ifstream ptrace (ptraceFile + ".txt");

    
    if (plist.is_open()){
        while (getline(plist,line)){
            vector<array<int, 3>> newTable = getPageTable(line, totalPages, pageSize);
            pageTables.push_back(newTable);
            totalPages += newTable.size();
            // for (int i = 0; i < newTable.size(); i++){
            //     cout << newTable.at(i)[0] << ", " << i << endl;
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
    //     cout << mainMemory[i] << endl;
    // for (int i = 0; i < pageTables.at(0).size(); i++)
    //     cout << pageTables.at(0).at(i)[0] << ", " << pageTables.at(0).at(i)[1] << endl;

    int pagefaults = 0;

    if (ptrace.is_open()){
        while (getline(ptrace,line)){
            int pid = stoi(line.substr(0, line.find(' ')));
            int location = stoi(line.substr(line.find(' ') + 1, line.length()))-1;
            // cout << pid << ", " << location << ", " << (location/pageSize) << endl;
            if (!pageTables.at(pid).at(location/pageSize)[1]){
                pagefaults++;
                if (algorithm == "FIFO"){
                    for (int i = 0; i < totalPrograms; i++){
                        if (pageTables.at(i).at(0)[0] <= mainMemory[firstIn] && mainMemory[firstIn] <= pageTables.at(i).at(pageTables.at(i).size() - 1)[0]){
                            pageTables.at(i).at(mainMemory[firstIn] - pageTables.at(i).at(0)[0])[1] = 0;
                            break;
                        }
                    }
                    mainMemory[firstIn] = pageTables.at(pid).at(location/pageSize)[0];
                    pageTables.at(pid).at(location/pageSize)[1] = 1;
                    firstIn = (mainPages-1) ? 0 : firstIn + 1;
                } else if (algorithm == "LRU"){
                    unsigned long lowest = 0;
                    int pidpage[3]; // PID, Program Page, Main Page
                    for (int i = 0; i < mainPages && mainMemory[i] != -1; i++){
                        for (int j = 0; j < totalPrograms; j++){
                            // cout << pageTables.at(j).at(0)[0] << ',' << mainMemory[i] << ',' << pageTables.at(j).at(pageTables.at(j).size() - 1)[0] << endl;
                            if (pageTables.at(j).at(0)[0] <= mainMemory[i] && mainMemory[i] <= pageTables.at(j).at(pageTables.at(j).size() - 1)[0]){
                                if (i == 0 || pageTables.at(j).at(mainMemory[i] - pageTables.at(j).at(0)[0])[2] < lowest){
                                    lowest = pageTables.at(j).at(mainMemory[i] - pageTables.at(j).at(0)[0])[2];
                                    pidpage[0] = j;
                                    pidpage[1] = mainMemory[i] - pageTables.at(j).at(0)[0];
                                    pidpage[2] = i;
                                }
                                break;
                            }
                        }
                    }

                    pageTables.at(pidpage[0]).at(pidpage[1])[1] = 0;
                    mainMemory[pidpage[2]] = pageTables.at(pid).at(location/pageSize)[0];
                    pageTables.at(pid).at(location/pageSize)[1] = 1;
                }
            }
            pageTables.at(pid).at(location/pageSize)[2] = counter;
            counter++;
        }
        ptrace.close();
    }

    cout << pagefaults << endl;
}