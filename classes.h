#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
using namespace std;

class Record {
public:
    int id, manager_id;
    std::string bio, name;

    Record(vector<std::string> fields) {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }

    void print() {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }
};


class StorageBufferManager {

private:
    
    const int BLOCK_SIZE = 4096; // initialize the block size allowed in main memory according to the question 
    const int MAX_RECORD_SIZE = 716; // each record = (8 + 200 + 500 + 8 bytes)

    // You may declare variables based on your need 
    int numRecords;
    std::string csvFileName;
    std::vector<int> offsets; // array of offsets to store start position of variable-length records in a page

    // Insert new record 
    void insertRecord(Record record) {

        // Calc recordSize to see if it fits on page
        int recordSize = calcRecordSize(record);

        // No records written yet
        if (numRecords == 0) {
            // Initialize first block

        }
        // Check that there is space to add to block 

        // Add record to the block
        numRecords++;

        // Take neccessary steps if capacity is reached (you've utilized all the blocks in main memory)


    }

    // Helper function to calculate the size of the record
    int calcRecordSize(Record record) {
        // id + name + bio + manager_id
        return sizeof(record.id) + record.name.length() + record.bio.length() + sizeof(record.manager_id);
    }

public:
    StorageBufferManager(string NewFileName) {
        
        //initialize your variables
        numRecords = 0;

        // Create your EmployeeRelation file 
        ofstream outputFile(NewFileName, ios::binary | ios::out);
        if (!outputFile.is_open()) {
            cerr << "Error: Unable to open file for writing.\n";
            exit(EXIT_FAILURE);
        }
        outputFile.close();
        
    }

    // Read csv file (Employee.csv) and add records to the (EmployeeRelation)
    void createFromFile(string csvFName) {

        // Open input file
        ifstream inputFile(csvFName);
        if (!inputFile.is_open()) {
            cerr << "Error: Unable to open input CSV file." << endl;
            exit(EXIT_FAILURE);
        }

        // Insert Records read from inputCSV into our new file
        string line;
        vector<string> fields;
        int idx = 0;
        while (getline(inputFile, line)) {
            // Parse CSV
            stringstream ss(line);
            string field;
            
            while (getline(ss, field, ',')) {
                fields.push_back(field);
            }
            cout << fields[idx++] << endl;

            // Create a record from the fields and insert it into the data file
            insertRecord(Record(fields));
        }
        cout << fields.size() << endl;

        inputFile.close();
        
    }

    // Given an ID, find the relevant record and print it
    Record findRecordById(int id) {
        cout << "Not Implemented Yet";
    }
};
