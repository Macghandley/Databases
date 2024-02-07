#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>

using namespace std;

struct Record {
    int id;
    string name;
    string bio;
    int managerId;
};

void parseCSV(const string& line, Record& record) {
    stringstream ss(line);
    string token;

    getline(ss, token, ',');
    record.id = stoi(token);

    getline(ss, record.name, ',');
    getline(ss, record.bio, ',');

    getline(ss, token, ',');
    record.managerId = stoi(token);
}

void writeRecord(ofstream& outfile, const Record& record) {
    // Calculate sizes of data elements
    int idSize = sizeof(record.id);
    /*cout << "record.id = " << record.id << endl;
    cout << "idSize = " << idSize << endl;*/
    int nameSize = record.name.size();
/*     cout << "record.name = " << record.name << endl;
    cout << "nameSize = " << nameSize << endl; */
    int bioSize = record.bio.size();
/*     cout << "record.bio = " << record.bio << endl;
    cout << "bioSize = " << bioSize << endl; */
    int managerIdSize = sizeof(record.managerId);
 /*    cout << "record.managerId = " << record.managerId << endl;
    cout << "managerIdSize = " << managerIdSize << endl; */


    // Calculate offsets
    int idOffset = sizeof(int) * 4; // 4 integers before the id
    int nameOffset = idOffset + idSize;
    int bioOffset = nameOffset + nameSize;
    int managerIdOffset = bioOffset + bioSize;

    // Write offsets as strings
    outfile << idOffset;
    outfile << nameOffset;
    outfile << bioOffset;
    outfile << managerIdOffset;

    // Write data as strings
    outfile << record.id;
    outfile << record.name;
    outfile << record.bio;
    outfile << record.managerId;
}

int main() {
    const string fileName = "EmployeeRelation.dat";
    const string inputFile = "Employee.csv";
    const int pageSize = 4096; // Page size

    // Create the EmployeeRelation file from Employee.csv
    ofstream outfile(fileName, ios::binary);
    ifstream infile(inputFile);

    if (!infile || !outfile) {
        cerr << "Error opening files." << endl;
        return 1;
    }
    
    // Create buffer for memory optimization
    vector<string> buffer;
    string line;
    int bytesRead = 0;

    int currentPageSize = sizeof(int) * 2; // Current page size
    vector<int> slotDirectory; // Slot directory for the current page
    int freeSpacePointer = 0; // Pointer to the start of free space on the current page
    int pages = 0; // Track # of previous "pages"
    int currentPosition = 0; // Track position at start of line read to go back to if overflowing buffer 
    
    // For initial read/write, loop through 
    while (bytesRead <= (pageSize * 3)) {

        currentPosition = infile.tellg();
        if(!getline(infile, line)){
            break;
        }

        if(line.length() + bytesRead > (pageSize * 3)){
            bytesRead = 0;
            infile.seekg(currentPosition);
            for(int s = 0; s < buffer.size(); s++){
                Record record;
                parseCSV(buffer[s], record);
                
                // Calculate the size of the record
                int idSize = sizeof(record.id);
                int nameSize = record.name.size();
                int bioSize = record.bio.size();
                int managerIdSize = sizeof(record.managerId);
                int recordSize = idSize + nameSize + bioSize + managerIdSize + sizeof(int) * 4; // Include sizes of data elements and offsets

                // Check if adding the record will exceed the page size
                if (currentPageSize + recordSize > pageSize) {
                    int directorySize = slotDirectory.size();
                    // Find where to write the directory
                    outfile.seekp(4096 * (pages + 1) - (directorySize*sizeof(int) + 2*sizeof(int)));   // Write slot directory to the file
                    for (int& size : slotDirectory) {
                        outfile << size;
                        
                    }
                    outfile << directorySize; // Write the size of the slot directory
                    outfile << freeSpacePointer; // Write the free space pointer

                    // Reset slot directory and free space pointer for the new page
                    slotDirectory.clear();
                    pages += 1;
                    freeSpacePointer = pages * 4096;
                    currentPageSize = sizeof(int) * 2;
                }
                
                // Write the record to the file
                outfile.seekp(freeSpacePointer);
                writeRecord(outfile, record);

                
                // Update slot directory and free space pointer
                slotDirectory.push_back(recordSize);
                
                freeSpacePointer += recordSize;
                // Update current page size
                currentPageSize += recordSize + sizeof(int);
            }
        }else{
            bytesRead += line.length();
            buffer.push_back(line);
        }
    }

    infile.close();
    outfile.close();

    return 0;
}
