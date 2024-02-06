/*
Skeleton code for storage and buffer management
*/

#include <string>
#include <ios>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cstring>
//#include "classes.h"

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
     // outfile << idOffset;
     // outfile << nameOffset;
     // outfile << bioOffset;
     // outfile << managerIdOffset;

     // Write data as strings
     outfile << record.id;
     outfile << record.name;
     outfile << record.bio;
     outfile << record.managerId << endl;
}

// Given an ID, find the relevant record and print it
Record findRecordById(int id) {
     cout << "Not Implemented Yet";
}

int main(int argc, char* const argv[]) {

     const string fileName = "EmployeeRelation.txt";
     const string inputFile = "Employee.csv";
     const int pageSize = 4096; // Page size

     // Create the EmployeeRelation file from Employee.csv
     ofstream outfile(fileName, ios::binary);
     ifstream infile(inputFile);

     if (!infile || !outfile) {
          cerr << "Error opening files." << endl;
          return 1;
     }

     int currentPageSize = sizeof(int) * 2; // Current page size
     vector<Slot> slotDir;
     int freeSpacePointer = 0; // Pointer to the start of free space on the current page
     int pages = 0; // Track # of previous "pages"
     int directorySize = 0;

     string line;
     while (getline(infile, line)) {
          Record record;
          parseCSV(line, record);

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
          slotDir.push_back(slot);
          directorySize = slotDir.size() * sizeof(Slot);

          // Check if adding the record will exceed the page size
          if (currentPageSize + recordSize + directorySize > pageSize) {

               // Find where to write the directory
               outfile.seekp(4096 * (pages + 1) - (directorySize * sizeof(int) + 2 * sizeof(int)));   // Write slot directory to the file
               for (const auto& slot : slotDir) {
                    outfile << slot.startOffset << slot.recordLength;
               }

               // outfile << directorySize; // Write the size of the slot directory
               // outfile << freeSpacePointer; // Write the free space pointer

               // Reset slot directory and free space pointer for the new page
               slotDir.clear();
               pages += 1;
               freeSpacePointer = pages * 4096;
               currentPageSize = sizeof(int) * 2;
          }

          // Write the record to the file
          outfile.seekp(freeSpacePointer);
          writeRecord(outfile, record);

          // Move freeSpacePointer to end of the record we just output
          freeSpacePointer += recordSize;

          // Update current page size
          currentPageSize += recordSize + sizeof(int);
     }

     infile.close();

     int employeeId = 0;
     while (employeeId != -1) {
          cout << "Enter Employee ID to search (or -1 to quit): ";
          cin >> employeeId;

          if (employeeId != -1) {
               //Record result; //= manager.findRecordById(employeeId);
               //cout << result;
          }

     }

     outfile.close();

     return 0;
}