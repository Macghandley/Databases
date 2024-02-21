/*
 * CS 440 Assignment 3
 * Dylan Varga, vargad, 933831567
 * McKellam Handley, handleym, 933654458
 * 02/20/2024
*/

#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <bitset>


using namespace std;

class Record {
public:
    int id;
    string name;
    string bio;
    int managerID;

    Record(vector<string> fields) {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        managerID = stoi(fields[3]);
    }

    void print() {
        cout << "--------------------------------" << endl;
        cout << "ID: " << id << endl;
        cout << "Name: " << name << endl;
        cout << "Bio: " << bio << endl;
        cout << "Manager ID: " << managerID << endl;
        cout << "--------------------------------" << endl;
    }
};


class LinearHashIndex {

private:
    const int BLOCK_SIZE = 4096;

    vector<int> blockDirectory; // Map the least-significant-bits of h(id) to a bucket location in EmployeeIndex (e.g., the jth bucket)
                                // can scan to correct bucket using j*BLOCK_SIZE as offset (using seek function)
								// can initialize to a size of 256 (assume that we will never have more than 256 regular (i.e., non-overflow) buckets)
    int n;  // The number of indexes in blockDirectory currently being used
    int i;	// The number of least-significant-bits of h(id) to check. Will need to increase i once n > 2^i
    int j;  // For searching block directory
    int numRecords;    // Records currently in index. Used to test whether to increase n
    int nextFreeBlock; // Next place to write a bucket. Should increment it by BLOCK_SIZE whenever a bucket is written to EmployeeIndex
    int freeSpacePointer; // Offset of next open space
    string fName;      // Name of index file
    int hashedInput;    // Holds hash coded code
    string pageBuffer;  // Holds entire page
    int cursorOffset;  // Marks cursor position
    string stringBuffer; // Declare once, holds strings for data read
    string inputBuffer; // Declare once, holds inputs as strings
    stringstream stringStreamX; // Declare once, generic stringstream

    void readPageData(string stringBuffer, int cursorOffset, stringstream& stringStreamX){
        ifstream inFile;
        inFile.open(fName, ios::binary);
        inFile.seekg(cursorOffset);

        // Read from filestream using c-style string
        char* pageBuffer = new char[BLOCK_SIZE + 1];
        inFile.read(pageBuffer, BLOCK_SIZE);
        pageBuffer[BLOCK_SIZE] = '\0';

        inFile.close();

        // Convert to a "new" string and use stringstream to parse
        stringBuffer = pageBuffer;
        delete[] pageBuffer;

        stringStreamX.str(stringBuffer);
        stringBuffer.clear();
    }

    int idToBinary(int recordID){
        // Hashify
        int hashedInput = recordID % 216;
        int divResult = hashedInput;
        int binaryOutput = 0;
        int mod2Output = 0;

        // Fake binary
        for(int k = 0; k < i; k++){
            mod2Output = divResult%2;
            divResult = divResult/2;
            binaryOutput += pow(10,k) * mod2Output;
        }

        // Bit flip
        if(binaryOutput > blockDirectory.at(n-1)){
            int negator = pow(10, i-1);
            binaryOutput -= negator;
        }

        return binaryOutput;
    }

    void sendRecordToFile(Record record){
        
        cursorOffset = j * BLOCK_SIZE;

        // Calculate size of record plus pointers
        int recordLength = 8 + record.bio.length() + record.name.length() + 8 + 4 + 12;

        while(true)
        {
            readPageData(stringBuffer, cursorOffset, stringStreamX);

            // Track size of page slots
            int currOffset = 0;
            int currLength = 0;
            int slotStart = BLOCK_SIZE - 12;
            int slotSize = 0;

            // Add offsets in slot directory
            if (getline(stringStreamX, inputBuffer, '<')){

                if(stringStreamX.tellg() != -1){    // If getline succeeds
                    slotStart = stringStreamX.tellg();
                    slotStart -= 1;
                    slotSize = BLOCK_SIZE - slotStart;

                    // Get position data from slots
                    getline(stringStreamX, inputBuffer, '%');
                    currOffset = stoi(inputBuffer);
                    getline(stringStreamX, inputBuffer, ',');
                    getline(stringStreamX, inputBuffer, '%');
                    currLength = stoi(inputBuffer);
                    
                // Reset stringstream if getline fails    
                }else{
                    stringStreamX.clear();
                    readPageData(stringBuffer, cursorOffset, stringStreamX);
                }
            }

            // Add to page if space exists
            if(BLOCK_SIZE >= (currOffset + currLength + slotSize + recordLength)){

                stringBuffer = stringStreamX.str();
                stringStreamX.clear();
                char* pageBuffer = new char[BLOCK_SIZE + 1];
                pageBuffer[BLOCK_SIZE] = '\0';
                strcpy(pageBuffer, (stringBuffer).c_str());
                stringBuffer.clear();

                // Create record
                string newRecord = to_string(record.id) + ';' + record.name + ';' + record.bio + ';' + to_string(record.managerID) + ";";
                string RID = "<%%%%%,%%%%>";
                RID.replace(1, to_string(currOffset + currLength).length(), to_string(currOffset + currLength));
                RID.replace(7, to_string(newRecord.length()).length(), to_string(newRecord.length()));

                // Enter record, slot into stream
                for(int i = 0; i < newRecord.length(); i++){
                    pageBuffer[i + (currOffset + currLength)] = newRecord[i];
                }

                for(int i = 0; i < RID.length(); i++){
                    pageBuffer[i + (slotStart - 12)] = RID[i];
                }

                // Write stream to file
                ofstream outFile;
                outFile.open(fName, ios::in | ios::ate | ios::binary);
                outFile.seekp(cursorOffset);
                outFile.write(pageBuffer, 4096);
                outFile.close();

                delete[] pageBuffer;

                break;

            // If page is full
            }else{
                stringStreamX.seekg(BLOCK_SIZE-11);     // Check for file pointer
                getline(stringStreamX, inputBuffer, ']');

                int newPageOffset = stoi(inputBuffer);

                // If none:
                if(newPageOffset == 0)
                {
                    // Create overflow page
                    stringBuffer = stringStreamX.str();
                    stringStreamX.clear();
                    
                    char* pageBuffer = new char[BLOCK_SIZE + 1];
                    pageBuffer[BLOCK_SIZE] = '\0';
                    strcpy(pageBuffer, (stringBuffer).c_str());
                    stringBuffer.clear();

                    // Fix current free space pointer and write
                    string newPointer = to_string(freeSpacePointer);
                    freeSpacePointer += BLOCK_SIZE;

                    int placementLength = (BLOCK_SIZE-(newPointer.length()+1));
                    for(int i = 0; i < newPointer.length(); i++){
                        pageBuffer[i + placementLength] = newPointer[i];
                    }

                    // Write in correct position
                    ofstream outFile;
                    outFile.open(fName, ios::in | ios::ate | ios::binary);
                    outFile.seekp(cursorOffset);
                    outFile.write(pageBuffer, BLOCK_SIZE);

                    string offsetArray = "[0000000000]";
                    memset(pageBuffer, ' ', BLOCK_SIZE);
                    for(int i = 0; i < offsetArray.length(); i++){
                        pageBuffer[i + 4084] = offsetArray[i];
                    }                    

                    outFile.seekp(stoi(newPointer));
                    outFile.write(pageBuffer, BLOCK_SIZE);
                    outFile.close();
                    cursorOffset = stoi(newPointer);

                    delete[] pageBuffer;

                // Continue forward to new page
                }else{
                    cursorOffset = newPageOffset; 
                }
            }
            
        }

    }

    // Insert new record into index
    void insertRecord(Record record) {

        // No records written to index yet
        if (numRecords == 0) {

            // Initialize index with first blocks (start with 4)
            nextFreeBlock = BLOCK_SIZE * 4;
            freeSpacePointer = 4096*256;

            blockDirectory.push_back(00);
            blockDirectory.push_back(01);
            blockDirectory.push_back(10);
            blockDirectory.push_back(11);

        }

        numRecords++;

        int binaryOutput = idToBinary(record.id);

        // Find which page to go to
        for(int k = 0; k < n; k++){
            if(blockDirectory.at(k) == binaryOutput){
                j = k;
            }
        }

        // Add to correct page
        sendRecordToFile(record);
                
        // Check if capacity is reached
        if((numRecords * 730) > (.7 * n * 4096)){
            n++;
            nextFreeBlock += BLOCK_SIZE;

            // Check if i sufficient
            if(pow(2,i) < n){
                i++;
            }
            // Update block directory
            int divResult = n-1;
            binaryOutput = 0;
            int k = 0;

            // Fake binary
            while(divResult != 0){
                int mod2Output = divResult%2;
                divResult = divResult/2;
                binaryOutput += pow(10,k) * mod2Output;
                k++;
            }

            blockDirectory.push_back(binaryOutput);
            for(int l = 0; l < n-1; l++)
            {
                int numSlots = 0;
                int numOverCap = 0;

                while(true){
                    ifstream inFile;
                    inFile.open(fName, ios::binary);

                    if(numOverCap == 0){
                        inFile.seekg(BLOCK_SIZE * l);
                    }else{
                        inFile.seekg(numOverCap);
                    }

                    // C string to stringstream conversion again
                    char* pageBuffer = new char[BLOCK_SIZE + 1];
                    inFile.read(pageBuffer, BLOCK_SIZE);
                    pageBuffer[BLOCK_SIZE] = '\0';
                    string stringBuffer = pageBuffer;
                    delete[] pageBuffer;

                    stringStreamX.str(stringBuffer);
                    stringBuffer.clear();

                    inFile.close();

                    if(numSlots == 0){
                        if (getline(stringStreamX, inputBuffer, '<')){

                            if(stringStreamX.tellg() != -1){
                                numSlots = stringStreamX.tellg();
                                int slotOffset = stringStreamX.tellg();
                                slotOffset -= 1;

                                // Get record offset
                                getline(stringStreamX, inputBuffer, '%');
                                int recordOffset = stoi(inputBuffer);
                                stringStreamX.seekg(stoi(inputBuffer));

                                // Get ID, name, bio, managerID
                                getline(stringStreamX, inputBuffer, ';');
                                string id = inputBuffer;
                                getline(stringStreamX, inputBuffer, ';');
                                string name = inputBuffer;
                                getline(stringStreamX, inputBuffer, ';');
                                string bio = inputBuffer;
                                getline(stringStreamX, inputBuffer, ';');
                                string manid = inputBuffer;

                                // Create record
                                vector<string> newVector{id, name, bio, manid};
                                Record recordToSend(newVector);

                                // Get binary
                                int binaryOutput = idToBinary(recordToSend.id);

                                if(binaryOutput != blockDirectory.at(j)){
                                    // Find proper page
                                    for(int k = 0; k < n; k++){
                                        if(blockDirectory.at(k) == binaryOutput){
                                            j = k;
                                        }
                                    }

                                    stringBuffer = stringStreamX.str();
                                    stringStreamX.clear();
                                    char* pageBuffer = new char[BLOCK_SIZE + 1];
                                    pageBuffer[BLOCK_SIZE] = '\0';
                                    strcpy(pageBuffer, (stringBuffer).c_str());
                                    stringBuffer.clear();

                                    // Create empty record/slot and enter
                                    int blankRecordLength = 8 + 8 + 4 + recordToSend.name.length() + recordToSend.bio.length();
                                    string blankRecord;
                                    blankRecord.append(blankRecordLength, ' ');
                                    string blankSlot = "            ";
                                    
                                    for(int i = 0; i < blankRecord.length(); i++){
                                        pageBuffer[i + recordOffset] = blankRecord[i];
                                    }
                                    for(int i = 0; i < blankSlot.length(); i++){
                                        pageBuffer[i + slotOffset] = blankSlot[i];
                                    }

                                    // Write to file
                                    ofstream outFile;
                                    outFile.open(fName, ios::in | ios::ate | ios::binary);
                                    if(numOverCap == 0){
                                        outFile.seekp(BLOCK_SIZE*l);
                                    }else{
                                        outFile.seekp(numOverCap);
                                    }
                                    outFile.write(pageBuffer, 4096);
                                    outFile.close();

                                    delete[] pageBuffer;

                                    // Send record to proper place
                                    sendRecordToFile(recordToSend);
                                }
                                
                            // If no slots available
                            }else{
                                // Reset ss
                                stringStreamX.clear();
                                if(numOverCap == 0){
                                    cursorOffset = BLOCK_SIZE * j;
                                }else{
                                    cursorOffset = numOverCap;
                                }

                                readPageData(stringBuffer, cursorOffset, stringStreamX);

                                // Get page offset
                                stringStreamX.seekg(BLOCK_SIZE-11);
                                getline(stringStreamX, inputBuffer, ']');
                                int newPageOffset = stoi(inputBuffer);

                                if(newPageOffset == 0){
                                    stringStreamX.clear();
                                    break;
                                }else{
                                    numOverCap = newPageOffset;
                                    numSlots = 0;
                                }

                            }
                        }
                    
                    // Already have slot on page
                    }else{
                        numSlots += 12;
                        // 4084 = end of page
                        if(numSlots == 4085){
                            stringStreamX.seekg(BLOCK_SIZE-11);
                            getline(stringStreamX, inputBuffer, ']');
                            int newPageOffset = stoi(inputBuffer);

                            if(newPageOffset == 0){
                                stringStreamX.clear();
                                break;
                            }else{
                                numOverCap = newPageOffset;
                                numSlots = 0;
                            }
                        }else{

                            stringStreamX.seekg(numSlots);

                            // If not spaces then use record
                            if((stringStreamX.str())[numSlots] != ' '){
                                int slotOffset = stringStreamX.tellg();
                                slotOffset -= 1;
                                
                                // Get record offset
                                getline(stringStreamX, inputBuffer, '%');
                                stringStreamX.seekg(stoi(inputBuffer));
                                int recordOffset = stoi(inputBuffer);
                                
                                // Get ID, name, bio, managerID
                                getline(stringStreamX, inputBuffer, ';');
                                string id = inputBuffer;
                                getline(stringStreamX, inputBuffer, ';');
                                string name = inputBuffer;
                                getline(stringStreamX, inputBuffer, ';');
                                string bio = inputBuffer;
                                getline(stringStreamX, inputBuffer, ';');
                                string manid = inputBuffer;

                                // Create and send new record
                                vector<string> newVector{id, name, bio, manid};
                                Record recordToSend(newVector);

                                int binaryOutput = idToBinary(recordToSend.id);

                                // Find potential move location
                                if(binaryOutput != blockDirectory.at(j)){
                                    for(int k = 0; k < n; k++){
                                        if(blockDirectory.at(k) == binaryOutput){
                                            j = k;
                                        }
                                    }

                                    stringBuffer = stringStreamX.str();
                                    stringStreamX.clear();
                                    char* pageBuffer = new char[BLOCK_SIZE + 1];
                                    pageBuffer[BLOCK_SIZE] = '\0';
                                    strcpy(pageBuffer, (stringBuffer).c_str());
                                    stringBuffer.clear();

                                    // Create blank record/slot
                                    int blankRecordLength = 8 + 8 + 4 + recordToSend.name.length() + recordToSend.bio.length();
                                    string blankRecord;
                                    blankRecord.append(blankRecordLength, ' ');
                                    string blankSlot = "            ";

                                    for(int i = 0; i < blankRecord.length(); i++){
                                        pageBuffer[i + recordOffset] = blankRecord[i];
                                    }
                                    for(int i = 0; i < blankSlot.length(); i++){
                                        pageBuffer[i + slotOffset] = blankSlot[i];
                                    }

                                    // Write to file
                                    ofstream outFile;
                                    outFile.open(fName, ios::in | ios::ate | ios::binary);
                                    if(numOverCap == 0){
                                        outFile.seekp(BLOCK_SIZE*l);
                                    }else{
                                        outFile.seekp(numOverCap);
                                    }
                                    outFile.write(pageBuffer, 4096);
                                    outFile.close();

                                    delete[] pageBuffer;

                                    // Send record
                                    sendRecordToFile(recordToSend);
                                }
                                
                            }

                        }

                    }

                }

            }
            
        }
        
    }

public:
    LinearHashIndex(string indexFileName) {
        n = 4; // Start with 4 buckets in index
        i = 2; // Need 2 bits to address 4 buckets
        numRecords = 0;
        nextFreeBlock = 0;
        fName = indexFileName;
        // Create your EmployeeIndex file and write out the initial 4 buckets
        // make sure to account for the created buckets by incrementing nextFreeBlock appropriately

        ofstream outFile;
        outFile.open(indexFileName, ios::binary);

        // Add empty pointer to end of page
        char* newPage = new char[4096];
        string offsetArray = "[0000000000]";
        memset(newPage, ' ', 4096);                                  
        for(int i = 0; i < offsetArray.length(); i++){
            newPage[i + 4084] = offsetArray[i];
        }

        // Create buffer
        for(int i = 0; i < 256; i++){
            outFile.write(newPage, 4096);
        }

        delete[] newPage;
        outFile.close();
      
    }

    // Read csv file and add records to the index
    void createFromFile(string csvFName) {

        ifstream inputFile;
        inputFile.open(csvFName);

        // Read from file
        string currentLine;
        string id;
        string manid;
        string name;
        string bio;

        
        // Send records to be inserted
        while(true){
            
            if(getline(inputFile, currentLine, ',')){
                id = currentLine;
                getline(inputFile, currentLine, ',');
                name = currentLine;
                getline(inputFile, currentLine, ',');
                bio = currentLine;
                getline(inputFile, currentLine);
                manid = currentLine;
                
                vector<string> newVector{id, name, bio, manid};
                Record recordFromFile(newVector);
                insertRecord(recordFromFile);
            
            // If getline invalid
            }else{
                break;
            }

        }

        inputFile.close();
    }

    // Given an ID, find the relevant record and print it
    void findRecordById(int searchId) {
        
        // Convert to binary
        int binaryOutput = idToBinary(searchId);

        // Find block to seek to
        for(int k = 0; k < n; k++){
            if(blockDirectory.at(k) == binaryOutput){
                j = k;
            }
        }

        int numSlots = 0;
        int numOverCap = 0;

        while(true){
            // Take page from file
            if(numOverCap == 0){
                cursorOffset = BLOCK_SIZE * j;
            }else{
                cursorOffset = numOverCap;
            }

            readPageData(stringBuffer, cursorOffset, stringStreamX);

            // Slot  handling
            if(numSlots == 0){
                if (getline(stringStreamX, inputBuffer, '<')){
                    if(stringStreamX.tellg() != -1){

                        // Catch position
                        numSlots = stringStreamX.tellg();
                        int slotOffset = stringStreamX.tellg();
                        slotOffset -= 1;

                        // Get record
                        getline(stringStreamX, inputBuffer, '%');
                        int recordOffset = stoi(inputBuffer);
                        stringStreamX.seekg(stoi(inputBuffer));
                        // Get ID, name, bio, managerID
                        getline(stringStreamX, inputBuffer, ';');
                        string id = inputBuffer;
                        getline(stringStreamX, inputBuffer, ';');
                        string name = inputBuffer;
                        getline(stringStreamX, inputBuffer, ';');
                        string bio = inputBuffer;
                        getline(stringStreamX, inputBuffer, ';');
                        string manid = inputBuffer;

                        // Print record
                        if(stoi(id) == searchId){
                            vector<string> newVector{id, name, bio, manid};
                            Record recordFromFile(newVector);
                            recordFromFile.print();
                        }

                    // If no slots
                    }else{
                        stringStreamX.clear();
                        if(numOverCap == 0){
                            cursorOffset = BLOCK_SIZE * j;
                        }else{
                            cursorOffset = numOverCap;
                        }

                        readPageData(stringBuffer, cursorOffset, stringStreamX);

                        stringStreamX.seekg(BLOCK_SIZE-11);
                        getline(stringStreamX, inputBuffer, ']');
                        int newPageOffset = stoi(inputBuffer);

                        // Check if new page needed
                        if(newPageOffset == 0){
                            stringStreamX.clear();
                            break;
                        }else{
                            numOverCap = newPageOffset;
                            numSlots = 0;
                        }

                    }
                }
            }else{

                numSlots += 12;
                if(numSlots == 4085){
                    stringStreamX.seekg(BLOCK_SIZE-11);
                    getline(stringStreamX, inputBuffer, ']');
                    int newPageOffset = stoi(inputBuffer);

                    if(newPageOffset == 0){
                        stringStreamX.clear();
                        break;
                    }else{
                        numOverCap = newPageOffset;
                        numSlots = 0;
                    }
                }else{

                    stringStreamX.seekg(numSlots);

                    if((stringStreamX.str())[numSlots] != ' '){
                        // Get position
                        int slotOffset = stringStreamX.tellg();
                        slotOffset -= 1;
  
                        // Get record
                        getline(stringStreamX, inputBuffer, '%');
                        int recordOffset = stoi(inputBuffer);
                        stringStreamX.seekg(stoi(inputBuffer));
                        // Get ID, name, bio, managerID
                        getline(stringStreamX, inputBuffer, ';');
                        string id = inputBuffer;
                        getline(stringStreamX, inputBuffer, ';');
                        string name = inputBuffer;
                        getline(stringStreamX, inputBuffer, ';');
                        string bio = inputBuffer;
                        getline(stringStreamX, inputBuffer, ';');
                        string manid = inputBuffer;

                        if(stoi(id) == searchId){
                            // Print record
                            vector<string> newVector{id, name, bio, manid};
                            Record recordFromFile(newVector);
                            recordFromFile.print();

                        }

                    }

                }

            }

        }

    }
};

int main(int argc, char* const argv[]) {

    // Create index
    LinearHashIndex employees("EmployeeIndex.dat");
    employees.createFromFile("Employee.csv");

    // Take user inputs and call lookup
    int userInput = 0;
    while(userInput != -1)
    {
        cout << "Enter an employee ID to search, or -1 to quit: ";
        cin >> userInput;
        if(userInput == -1){
            cout << "Quitting database..." << endl;
            break;
        }else{
            employees.findRecordById(userInput);
        }
    }
    
    return 0;
}