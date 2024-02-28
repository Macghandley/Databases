/* This is a skeleton code for External Memory Sorting, you can make modifications as long as you meet 
   all question requirements*/  

#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

//defines how many blocks are available in the Main Memory 
#define buffer_size 22
#define INT_SIZE = 4
#define DOUBLE_SIZE = 8
#define STRING_SIZE = 40

class Records{
    public:
    
    struct EmpRecord {
        int eid;
        string ename;
        int age;
        double salary;

        bool operator < (const Records& emp) const
        {
            return (eid < emp.emp_record.eid);
        }
    }emp_record;

    void print(){
        cout << emp_record.eid << ","
             << emp_record.ename << ","
             << emp_record.age << ","
             << emp_record.salary << "\n";
    }

    /*** You can add more variables if you want below ***/

    int no_values = 0; //You can use this to check if you've don't have any more tuples
    int number_of_emp_records = 0; // Tracks number of emp_records you have on the buffer
};

// Grab a single block from the Emp.csv file and put it inside the EmpRecord structure of the Records Class
Records Grab_Emp_Record(fstream &empin) {
    string line, word;
    Records emp;
    // grab entire line
    if (getline(empin, line, '\n')) {
        // turn line into a stream
        stringstream s(line);
        // gets everything in stream up to comma
        getline(s, word,',');
        emp.emp_record.eid = stoi(word);
        getline(s, word, ',');
        emp.emp_record.ename = word;
        getline(s, word, ',');
        emp.emp_record.age = stoi(word);
        getline(s, word, ',');
        emp.emp_record.salary = stod(word);

        return emp;
    } else {
        emp.no_values = -1;
        return emp;
    }
}

vector<Records> buffers; //use this class object of size 22 as your main memory

/***You can change return type and arguments as you want.***/

//PASS 1
//Sorting the buffers in main_memory
void Sort_Buffer(){
    //Remember: You can use only [AT MOST] 22 blocks for sorting the records / tuples and create the runs
    // Sort backwards so we can pop the smallest from the back
    vector<Records> sorted_buffers;

    while(buffers.size() != 0)
    {
        int biggestId = 0;
        int biggestIdIndex = 0;

        // Find the "biggest" id
        for(int i = 0; i < buffers.size(); i++)
        {
            if(buffers[i].emp_record.eid > biggestId)
            {
                biggestId = buffers[i].emp_record.eid;
                biggestIdIndex = i;
            }
        }

        // Store the "biggest" id Records in the sorted buffer and remove from other buffer 
        sorted_buffers.push_back(buffers[biggestIdIndex]);
        buffers.erase(buffers.begin()+biggestIdIndex);
    }

    buffers = sorted_buffers;
}

void PrintSorted(Records tempRecord, fstream &outputFile){
    //Store in EmpSorted.csv
    outputFile << to_string(tempRecord.emp_record.eid) << ","
               << tempRecord.emp_record.ename << ","
               << to_string(tempRecord.emp_record.age) << ","
               << to_string(tempRecord.emp_record.salary) << "\n";
    return;
}

//PASS 2
//Use main memory to Merge the Runs 
//which are already sorted in 'runs' of the relation Emp.csv 
void Merge_Runs(){

    //Creating the EmpSorted.csv file where we will store our sorted results
    fstream SortOut;
    SortOut.open("EmpSorted.csv", ios::out | ios::app);

    // Clear buffers to make sure we only have at max 22 pages in main mem
    buffers.clear();
    // Store the line of the next Record to read in each temp file 
    int lineCounter[buffer_size] = {0}; 

    // Temp vars
    fstream tempFile;
    Records tempRecord;
    std::string filename;

    // Used to find the smallest id in buffers
    int smallestId = 0;
    int smallestIdIndex = 0;

    // Start by reading the first line from each file.
    for(int i = 0; i < buffer_size; i++)
    {
        filename = "tempFile_" + std::to_string(i);
        tempFile.open(filename, ios::in);

        // Exit when we are out of temp_files
        if (!tempFile)
        {
            //std::cerr << "Error opening file: " << filename << std::endl;
            break;
        }

        // Grab the first record from each file and put it in buffers
        tempRecord = Grab_Emp_Record(tempFile);
        buffers.push_back(tempRecord);
        buffers[i].print();
        //Inc lineCounter to make sure we don't read first line again
        lineCounter[i] += 1; 
        tempFile.close();
    }

    while(true){
        // Resest helper vars for next loop
        smallestId = 999999;
        smallestIdIndex = 0;

        // Find the smallest eid
        for(int i = 0; i < buffers.size(); i++)
        {
            // Find the smallest eid but also consider the fact that we may have run out of Records in a tempFile
            // (If we've looked at all Records in a file, Grab_Emp_Record will store a Records var where no_values = -1.
            // Essentially we want to ignore considering Records from buffers[i] if tempFile_i has been fully read)
            if(buffers[i].emp_record.eid < smallestId && buffers[i].no_values != -1)
            {
                smallestId = buffers[i].emp_record.eid;
                smallestIdIndex = i;
            }
        }
        // If we get through this loop and smallestIdIndex == 999999, then we have looked at every record from every file
        if(smallestIdIndex == 999999)
        {
            break;
        }

        cout << "Smallest eid in buffers: " << buffers[smallestIdIndex].emp_record.eid << endl;

        // output the "smallest" eid into the EmpSorted.csv file
        tempRecord = buffers[smallestIdIndex];
        PrintSorted(tempRecord, SortOut);

        // Open up the file that has the "smallest" eid and take the next record
        filename = "tempFile_" + std::to_string(smallestIdIndex);
        tempFile.open(filename, ios::in);

        // Get next Record from the file we just took from and store in its place
        for(int i = 0; i <= lineCounter[smallestIdIndex]; i++)
        {
            tempRecord = Grab_Emp_Record(tempFile);
        }
        // Update buffer to hold next record in temp_file. Inc line counter of the file we just grabbed from
        buffers[smallestIdIndex] = tempRecord;
        lineCounter[smallestIdIndex] += 1;

        cout << "Replacement eid in buffers[" << smallestIdIndex << "]:" << buffers[smallestIdIndex].emp_record.eid << endl;
    }
    // Continue to find the smallest from each file until there are no more lines left to read
    return;
}

int main() {

    //Open file streams to read and write
    //Opening out the Emp.csv relation that we want to Sort
    fstream empin;
    empin.open("Emp.csv", ios::in);

    fstream tempFiles[buffer_size]; // Store pointers to all the temp files we make

    //1. Create runs for Emp which are sorted using Sort_Buffer()
    
    int runCounter = 0; // To keep track of how many temp files we have
    int number_of_emp_records = 0;
    Records empRecord;
    empRecord.no_values = 0;

    // Read in 22 lines at a times from Emp.csv, create 22 Records, create a temp file and store those 22 Records
    while(true)
    {
        // Attempt to read a line and create a Records to store in buffers
        empRecord = Grab_Emp_Record(empin);
        //empRecord.print();

        // Exit if we are at the end of Emp.csv
        if(empRecord.no_values == -1)
        {
            cout << "All records have been read into " << runCounter << " temp files" << endl;
            break;
        }

        // Store the empRecord in buffers and inc the counter
        buffers.push_back(empRecord);
        number_of_emp_records++;

        // Do not exceed 22 Records in main memory. Make temp_files store 20 records so we can sort them with an output buffer later
        if(number_of_emp_records == 20)
        {
            // Sort buffers before storing in file
            Sort_Buffer();

            // Open a new temp file, call it tempFile_{runCounter}
            std::string filename = "tempFile_" + std::to_string(runCounter);
            tempFiles[runCounter].open(filename, ios::out);

            // Check if the file is successfully opened
            if (!tempFiles[runCounter]) {
                std::cerr << "Error opening file: " << filename << std::endl;
                return 1;
            }

            // Output buffers to tempFile_{runCounter}
            while(buffers.size() != 0)
            {
                Records tempRecord = buffers.back();
                buffers.pop_back();
                tempFiles[runCounter] << to_string(tempRecord.emp_record.eid) << ","
                                      << tempRecord.emp_record.ename << ","
                                      << to_string(tempRecord.emp_record.age) << ","
                                      << to_string(tempRecord.emp_record.salary) << "\n";
            }
             
            // Reset buffers and number_of_emp_records for next temp file. Inc runCounter
            buffers.clear();
            number_of_emp_records = 0;
            tempFiles[runCounter].close();
            runCounter++;
        }
    }
    
    //2. Use Merge_Runs() to Sort the runs of Emp relations 
    cout << "Merging temp_files and outputting EmpSorted.csv" << endl;
    Merge_Runs();
   
    //Please delete the temporary files (runs) after you've sorted the Emp.csv
    for (int i = 0; i < buffer_size; ++i) {
        tempFiles[i].close();
    }
    
    return 0;
}
