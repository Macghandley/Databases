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

struct Slot {
     int startOffset;
     int recordLength;
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
    int nameSize = record.name.size();
    int bioSize = record.bio.size();
    int managerIdSize = sizeof(record.managerId);

    // Calculate offsets
    int idOffset = 0;
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
    
    // Create 3 page buffer for memory optimization
    vector<string> buffer;
    string line;
    int bytesRead = 0;

    int currentPageSize = 0; // Current page size
    vector<Slot> principalSlotDir;
    vector<Slot> slotDir; // Slot directory for the current page
    int freeSpacePointer = 0; // Pointer to the start of free space on the current page
    int pages = 0; // Track # of previous "pages"
    int directorySize = 0;
    int currentPosition = 0;
    
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
                int idSize = 8;
                int nameSize = record.name.size();
                int bioSize = record.bio.size();
                int managerIdSize = 8;
                int recordSize = idSize + nameSize + bioSize + managerIdSize;

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
                
                freeSpacePointer += recordSize;
                // Update current page size
                currentPageSize += recordSize;
            }
        }else{
            bytesRead += line.length();
            buffer.push_back(line);
        }
    }

    infile.close();
    outfile.close();

    const string binaryFileName = "EmployeeRelation";
     ifstream binaryFile(binaryFileName, ifstream::binary);

     if (!binaryFile) {
          cerr << "Error opening files." << endl;
          return 1;
     }

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
                    cout << buffer.data() << endl;

                    // Check if the read operation was successful
                    if (!binaryFile) {
                         std::cerr << "Error reading from binary file at." << recordOffset << std::endl;
                         return 1;
                    }

                    // Compare the first 8 chars (ID) to user input
                    string employeeIdString = to_string(employeeId);
                    std::string idFromBuffer(buffer.data(), 8);

                    if (idFromBuffer == employeeIdString)
                    {
                         cout << buffer.data() << endl;
                    }
               }
          }
     }

     binaryFile.close();

    return 0;
}
