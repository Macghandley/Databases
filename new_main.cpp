/*
 * CS 440 - HW 2
 * Page and Record Storage
 * By Dylan Varga and McKellam Handley
 * vargad@oregonstate.edu; handleym@oregonstate.edu
 * 933831567; 933654458
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>

using namespace std;

// Struct to hold records
struct Record {
    int id;
    string name;
    string bio;
    int managerId;
};

// Struct to hold "slot" tuples for page layout (rid's)
struct Slot {
     int startOffset;
     int recordLength;
};

// Parse through a newline from csv file and return it as a record
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

// Write a single record
void writeRecord(ofstream& outfile, const Record& record) {
    // Calculate sizes of data elements
    int idSize = sizeof(record.id);
    int nameSize = record.name.size();
    int bioSize = record.bio.size();
    int managerIdSize = sizeof(record.managerId);

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

    // Open files
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
    
    // Create 3 page buffer for memory optimization
    vector<string> buffer;
    string line;
    int bytesRead = 0;

    int currentPageSize = 0; // Current page size
    vector<Slot> principalSlotDir; // Overarching slot directory
    vector<Slot> slotDir; // Slot directory for the current page
    int freeSpacePointer = 0; // Pointer to the start of free space on the current page
    int pages = 0; // Track # of previous "pages"
    int directorySize = 0;  // Track size of directory
    int currentPosition = 0; // Track position at start of line read to go back to if overflowing buffer 
    
    // Loop through file system repeatedly in "increments" of 3 pages (in hindsight could just say "while true" but don't want to test)
    while (bytesRead <= (pageSize * 3)) {

        // Save current position before getting a newline
        currentPosition = infile.tellg();
        if(!getline(infile, line)){
            break;  // End file system write if reached end of file
        }

        // If new line would exceed buffer size, process current buffer, and start new one
        if(line.length() + bytesRead > (pageSize * 3)){
            bytesRead = 0;  // For starting new buffer
            infile.seekg(currentPosition);  // Go to saved position so overflow line not skipped in next buffer

            // Process buffer before moving on
            for(int s = 0; s < buffer.size(); s++){

                // For each record in buffer, parse and process
                Record record;
                parseCSV(buffer[s], record);  
                
                // Calculate the size of the record
                int idSize = sizeof(record.id);
                int nameSize = record.name.size();
                int bioSize = record.bio.size();
                int managerIdSize = sizeof(record.managerId);
                int recordSize = idSize + nameSize + bioSize + managerIdSize + sizeof(int) * 4; // Include sizes of data elements and offsets

                // Fill in slot directory
                Slot slot;
                slot.startOffset = freeSpacePointer;
                slot.recordLength = recordSize;

                // Put slot in page slotDir and principalSlotDir to iterate over later
                slotDir.push_back(slot);
                principalSlotDir.push_back(slot);
                directorySize = slotDir.size();

                // Check if adding the record will exceed the page size
                if (currentPageSize + recordSize + directorySize> pageSize) {

                    // Find where to write the directory
                    outfile.seekp(pageSize * (pages + 1) - (directorySize));   // Write slot directory to the file
                    for (const auto& slot : slotDir) {
                        outfile << slot.startOffset << slot.recordLength;
                    //cout << slot.startOffset << ", " << slot.recordLength << endl;
                    }
                    // outfile << directorySize; // Write the size of the slot directory
                    // outfile << freeSpacePointer; // Write the free space pointer

                    // Reset slot directory and free space pointer for the new page
                    slotDir.clear();
                    pages += 1;
                    freeSpacePointer = pages * pageSize;
                    currentPageSize = 0;
                }
                
                // Write the record to the file
                outfile.seekp(freeSpacePointer);
                writeRecord(outfile, record);

                // // Update slot directory and free space pointer
                // slotDirectory.push_back(recordSize);
                
                // Update free space pointer
                freeSpacePointer += recordSize;
                // Update current page size
                currentPageSize += recordSize;
            }

        // If buffer not full, simply read the line and add it to the buffer, tracking its size    
        }else{
            bytesRead += line.length();
            buffer.push_back(line);
        }
    }

    // File system complete
    infile.close();
    outfile.close();

    // Search for ID loop
    const string binaryFileName = "EmployeeRelation.dat";
    ifstream binaryFile(binaryFileName, ifstream::binary);

    // Check if file opens
    if (!binaryFile) {
        cerr << "Error opening files." << endl;
        return 1;
    }
    
    // Search for employee by ID
    int employeeId = 0;
    while (employeeId != -1) {
        cout << "Enter Employee ID to search (or -1 to quit): ";
        cin >> employeeId;

        if (employeeId != -1) {
            for (int i = 0; i < principalSlotDir.size(); i++)
            {
                // Get the offset and length of i'th record
                int recordOffset = principalSlotDir[i].startOffset;
                int recordLength = principalSlotDir[i].recordLength;

                // Create buffer to read record
                std::vector<char> buffer(recordLength);

                // go to every offset and check the ID
                binaryFile.seekg(recordOffset, std::ios::beg);
                binaryFile.read(buffer.data(), recordLength);

                // Check if the read operation was successful
                if (!binaryFile) {
                        std::cerr << "Error reading from binary file at." << recordOffset << std::endl;
                        return 1;
                }

                // Compare the first 8 post-offset chars (ID) to user input
                string employeeIdString = to_string(employeeId);
                int idOffset = 17;  // Fixed distance from start of record to id so simplifies things
                std::string idFromBuffer(buffer.data(), idOffset);
                if (idFromBuffer.substr(9,17) == employeeIdString)
                {       
                        string employeeData(buffer.data(), recordLength);
                        cout << employeeData.substr(9, recordLength) << endl;
                }
            }
        }
    }

    binaryFile.close();

    return 0;
}
