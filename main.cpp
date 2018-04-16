#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
using namespace std;

// Function to create the page table for a program
vector <array<unsigned long, 3>> getPageTable(string line, int totalPages, int pageSize){
    int pid = stoi(line.substr(0, line.find(' ')));
    int locations = stoi(line.substr(line.find(' ') + 1, line.length()));
    vector <array<unsigned long, 3>> pageTable;

    for (int i = totalPages; i < totalPages + ((locations + pageSize - 1)/pageSize); i++){
        pageTable.push_back({i, 0, 0});
    }

    return pageTable;
}

int main(int argc, char const *argv[]){
    // Incorrect number of arguements
    if (argc != 6){
        cout << "Ur drunk go home m8" << endl;
        return 1;
    }

    string plistFile = argv[1]; // Name of the plist file without the extension
    string ptraceFile = argv[2]; // Name of the ptrace file without the extension
    int pageSize = stoi(argv[3]); // Size of the pages
    string algorithm = argv[4]; // Paging algorithm to use
    int paging; // + = Pre-paging     - = Demand paging
    if (argv[5][0] == '+'){
        paging = 1;
    } else if (argv[5][0] == '-'){
        paging = 0;
    } else {
        cout << "Ur drunk go home m8" << endl;
        return 1;
    }
    string line;
    int mainPages = 512/pageSize; // Get the number of pages to use
    int mainMemory[mainPages]; // Create the main memory page array
    fill_n(mainMemory, mainPages, -1);
    int Rbit[mainPages]; // Create the array of Rbits for use in Clock
    fill_n(Rbit, mainPages, 0);
    vector<vector<array<unsigned long, 3>>> pageTables; // Create the vector for the page tables for each program
    int totalPrograms; // Storage for the total number of programs
    int totalPages = 0; // Storage for the total number of pages
    unsigned long counter = 0; // Global counter
    int firstIn = 0; // FIFO and Clock algorithm pointer
    unsigned long pagefaults = 0; // Counter for the number of page faults
    
    ifstream plist (plistFile + ".txt"); // Open the plist file
    ifstream ptrace (ptraceFile + ".txt"); // Open the ptrace file

    
    if (plist.is_open()){
        while (getline(plist,line)){
            // Create the program's page table and add it to the vector
            vector<array<unsigned long, 3>> newTable = getPageTable(line, totalPages, pageSize);
            pageTables.push_back(newTable);
            totalPages += newTable.size();
        }
        plist.close();
    }

    // Get the total number of programs, this might be unnecessary
    totalPrograms = pageTables.size();

    // Load in the initial pages
    for (int i = 0; i < totalPrograms; i++){
        for (int j = 0; j < mainPages/totalPrograms && j < pageTables.at(i).size(); j++){
            pageTables.at(i).at(j)[1] = 1;
            mainMemory[((mainPages/totalPrograms)*i)+j] = pageTables.at(i).at(j)[0];
        }
    }

    // Get to the real meat and potatoes, reading ptrace for the instructions
    if (ptrace.is_open()){
        while (getline(ptrace,line)){
            // Parse the pid and mem location from the file
            int pid = stoi(line.substr(0, line.find(' ')));
            int location = stoi(line.substr(line.find(' ') + 1, line.length()))-1;
            counter++; // increment the global counter

            // Page fault! Do the stuff!!!
            if (!pageTables.at(pid).at(location/pageSize)[1]){
                pagefaults++;

                // If there are empty frames in the main mem fill those first.
                if (mainMemory[mainPages-1] == -1){
                    for (int i = 0; i < mainPages; i++){
                        if (mainMemory[i] == -1){
                            pageTables.at(pid).at(location/pageSize)[1] = 1;
                            mainMemory[i] = pageTables.at(pid).at(location/pageSize)[0];
                            Rbit[i] = 1;
                            break;
                        }
                    }
                } else if (algorithm == "FIFO"){ // First in first out algorithm
                    // Find the page for the first in page to swap out
                    for (int i = 0; i < totalPrograms; i++){
                        if (pageTables.at(i).at(0)[0] <= mainMemory[firstIn] && mainMemory[firstIn] <= pageTables.at(i).at(pageTables.at(i).size() - 1)[0]){
                            pageTables.at(i).at(mainMemory[firstIn] - pageTables.at(i).at(0)[0])[1] = 0;
                            break;
                        }
                    }

                    // Do the swappy swappy and increment the first in pointer
                    mainMemory[firstIn] = pageTables.at(pid).at(location/pageSize)[0];
                    pageTables.at(pid).at(location/pageSize)[1] = 1;
                    firstIn = (mainPages-1) ? 0 : firstIn + 1;
                } else if (algorithm == "LRU"){ // Least recently used algorithm
                    unsigned long lowest = 0; // Hold the lowest last access time
                    int pidpage[3]; // PID, Program Page, Main Page

                    // Loop through all of the main memory pages checking their last access time and saving the lowest
                    // Also set the lowest to the first on the first check
                    for (int i = 0; i < mainPages; i++){
                        for (int j = 0; j < totalPrograms; j++){
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

                    // Do the swappy swappy
                    pageTables.at(pidpage[0]).at(pidpage[1])[1] = 0;
                    mainMemory[pidpage[2]] = pageTables.at(pid).at(location/pageSize)[0];
                    pageTables.at(pid).at(location/pageSize)[1] = 1;
                } else if (algorithm == "Clock"){ // Clock algorithm

                    // Find the next main mem page with an rbit of 0
                    while (Rbit[firstIn] != 0){
                        Rbit[firstIn] = 0;
                        if (firstIn >= mainPages - 1){
                            firstIn = 0;
                        } else {
                            firstIn++;
                        }
                    }

                    // Swap the page in
                    Rbit[firstIn] = 1;
                    for (int i = 0; i < totalPrograms; i++){
                        if (pageTables.at(i).at(0)[0] <= mainMemory[firstIn] && mainMemory[firstIn] <= pageTables.at(i).at(pageTables.at(i).size() - 1)[0]){
                            pageTables.at(i).at(mainMemory[firstIn] - pageTables.at(i).at(0)[0])[1] = 0;
                            break;
                        }
                    }
                    mainMemory[firstIn] = pageTables.at(pid).at(location/pageSize)[0];
                    pageTables.at(pid).at(location/pageSize)[1] = 1;

                    // Increment the hand
                    if (firstIn >= mainPages - 1){
                        firstIn = 0;
                    } else {
                        firstIn++;
                    }
                }
                pageTables.at(pid).at(location/pageSize)[2] = counter;

                // If pre-paging is on find the next page and do it all over again
                if (paging == 1){
                    counter++;
                    int nextPid = pid; // Might not actually be different but it could be so yea...
                    int nextPage = (location/pageSize)+1;
                    if (nextPage >= pageTables.at(nextPid).size()){
                        nextPid++;
                        nextPage = 0;
                        if (nextPid >= pageTables.size())
                            nextPid = 0;
                    }
                    while (pageTables.at(nextPid).at(nextPage)[1]){
                        nextPage++;
                        if (nextPage >= pageTables.at(nextPid).size()){
                            nextPid++;
                            nextPage = 0;
                            if (nextPid >= pageTables.size())
                                nextPid = 0;
                        }
                    }
                    if (mainMemory[mainPages-1] == -1){
                        for (int i = 0; i < mainPages; i++){
                            if (mainMemory[i] == -1){
                                pageTables.at(nextPid).at(nextPage)[1] = 1;
                                mainMemory[i] = pageTables.at(nextPid).at(nextPage)[0];
                                Rbit[i] = 1;
                                break;
                            }
                        }
                    } else if (algorithm == "FIFO"){
                        for (int i = 0; i < totalPrograms; i++){
                            if (pageTables.at(i).at(0)[0] <= mainMemory[firstIn] && mainMemory[firstIn] <= pageTables.at(i).at(pageTables.at(i).size() - 1)[0]){
                                pageTables.at(i).at(mainMemory[firstIn] - pageTables.at(i).at(0)[0])[1] = 0;
                                break;
                            }
                        }
                        mainMemory[firstIn] = pageTables.at(nextPid).at(nextPage)[0];
                        pageTables.at(nextPid).at(nextPage)[1] = 1;
                        firstIn = (mainPages-1) ? 0 : firstIn + 1;
                    } else if (algorithm == "LRU"){
                        unsigned long lowest = 0;
                        int nextPidpage[3]; // nextPid, Program Page, Main Page
                        for (int i = 0; i < mainPages; i++){
                            for (int j = 0; j < totalPrograms; j++){
                                if (pageTables.at(j).at(0)[0] <= mainMemory[i] && mainMemory[i] <= pageTables.at(j).at(pageTables.at(j).size() - 1)[0]){
                                    if (i == 0 || pageTables.at(j).at(mainMemory[i] - pageTables.at(j).at(0)[0])[2] < lowest){
                                        lowest = pageTables.at(j).at(mainMemory[i] - pageTables.at(j).at(0)[0])[2];
                                        nextPidpage[0] = j;
                                        nextPidpage[1] = mainMemory[i] - pageTables.at(j).at(0)[0];
                                        nextPidpage[2] = i;
                                    }
                                    break;
                                }
                            }
                        }
                        pageTables.at(nextPidpage[0]).at(nextPidpage[1])[1] = 0;
                        mainMemory[nextPidpage[2]] = pageTables.at(nextPid).at(nextPage)[0];
                        pageTables.at(nextPid).at(nextPage)[1] = 1;
                    } else if (algorithm == "Clock"){
                        while (Rbit[firstIn] != 0){
                            Rbit[firstIn] = 0;
                            if (firstIn >= mainPages - 1){
                                firstIn = 0;
                            } else {
                                firstIn++;
                            }
                        }
                        Rbit[firstIn] = 1;
                        for (int i = 0; i < totalPrograms; i++){
                            if (pageTables.at(i).at(0)[0] <= mainMemory[firstIn] && mainMemory[firstIn] <= pageTables.at(i).at(pageTables.at(i).size() - 1)[0]){
                                pageTables.at(i).at(mainMemory[firstIn] - pageTables.at(i).at(0)[0])[1] = 0;
                                break;
                            }
                        }
                        mainMemory[firstIn] = pageTables.at(nextPid).at(nextPage)[0];
                        pageTables.at(nextPid).at(nextPage)[1] = 1;
                        if (firstIn >= mainPages - 1){
                            firstIn = 0;
                        } else {
                            firstIn++;
                        }
                    }
                    pageTables.at(nextPid).at(nextPage)[2] = counter;
                }
            } else {
                // No page fault, do the normal stuff and move on
                pageTables.at(pid).at(location/pageSize)[2] = counter;
                for (int i = 0; i < mainPages; i++){
                    if (mainMemory[i] == pageTables.at(pid).at(location/pageSize)[0])
                        Rbit[i] = 1;
                }
            }
        }
        ptrace.close();
    }
    cout << pagefaults << endl;
}