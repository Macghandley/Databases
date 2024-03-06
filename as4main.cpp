/* 
    CS 440 Assignment 4

    McKellam Handley - handleym@oregonstate.edu - 933654458
    Dylan Varga - vargad@oregonstate.edu - 933831567
*/  

#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

//defines how many blocks are available in the Main Memory 
#define buffer_size 22

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

vector<Records> buffers(22); //use this class object of size 22 as your main memory

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
    //Store in EmpSorted.
    int intSalary = static_cast<int>(tempRecord.emp_record.salary);
    outputFile << to_string(tempRecord.emp_record.eid) << ","
               << tempRecord.emp_record.ename << ","
               << to_string(tempRecord.emp_record.age) << ","
               << to_string(intSalary) << "\n";
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

        // Exit when we are out of temp_files to read from. In other words, store at max 22 pages in main mem
        if (!tempFile)
        {
            break;
        }

        // Grab the first record from each file and put it in buffers
        tempRecord = Grab_Emp_Record(tempFile);
        buffers.push_back(tempRecord);

        //Inc lineCounter to make sure we don't read first line again
        lineCounter[i] += 1; 
        tempFile.close();
    }

    // Continue to find the smallest eidn from each file until there are no more lines left to read
    while(true){
        // Resest helper vars for next loop
        smallestId = 999999;
        smallestIdIndex = 0;

        // Find the smallest eid
        for(int i = 0; i < buffers.size(); i++)
        {
            // Find the smallest eid but also consider the fact that we may have run out of Records in a tempFile
            // (If we've looked at all Records in a file, Grab_Emp_Record will store a Records var where no_values = -1.
            // Ignore Records from buffers[i] if tempFile_i has been fully read)
            if(buffers[i].emp_record.eid < smallestId && buffers[i].no_values != -1)
            {
                smallestId = buffers[i].emp_record.eid;
                smallestIdIndex = i;
            }
        }
        // If we get through this loop and smallestIdIndex == 999999, then we have looked at every record from every temp file
        if(smallestId == 999999)
        {
            break;
        }

        // output the Record with  "smallest" eid into the EmpSorted.csv file
        tempRecord = buffers[smallestIdIndex];
        PrintSorted(tempRecord, SortOut); 

        // Open up the file that has the "smallest" eid to  take the next record
        filename = "tempFile_" + std::to_string(smallestIdIndex);
        tempFile.open(filename, ios::in);

        // Get next Record from the temp_file we just took from to print to EmpSorted.csv
        for(int i = 0; i <= lineCounter[smallestIdIndex]; i++)
        {
            tempRecord = Grab_Emp_Record(tempFile);
        }
        tempFile.close();

        // Update buffer to hold next record in temp_file. Inc line counter of the file we just grabbed from
        buffers[smallestIdIndex] = tempRecord;
        lineCounter[smallestIdIndex] += 1;
    }
    return;
}

int main() {

    //Open file streams to read and write
    //Opening out the Emp.csv relation that we want to Sort
    fstream empin;
    empin.open("Emp.csv", ios::in);

    fstream tempFiles[buffer_size]; // Store pointers to all the temp files we make

    //1. Create tempFiles using Emp.csv. Temp files are sorted using Sort_Buffer()
    
    int runCounter = 0; // To keep track of how many temp files we have
    int number_of_emp_records = 0;
    Records empRecord;
    empRecord.no_values = 0;
    

    // Read in 20 lines at a time from Emp.csv, create 20 Records, then create a temp file and store those 20 Records
    while(true)
    {
        // Attempt to read a line and create a Records to store in buffers
        empRecord = Grab_Emp_Record(empin);
        //empRecord.print();

        // If we are at the end of Emp.csv, create last temp file with whatever is in buffers and exit
        if(empRecord.no_values == -1)
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

            cout << "All records have been read into " << runCounter << " sorted temp files" << endl;
            break;
        }

        // Store the empRecord in buffers and inc the counter
        buffers.push_back(empRecord);
        number_of_emp_records++;

        // Do not exceed 22 Records in main memory. Make tempFiles store 20 records so we can sort merge later
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
            // buffers gets popped to make sure we don't exceed 22 pages in memory
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
            number_of_emp_records = 0;
            tempFiles[runCounter++].close();
        }
    }

    //2. Use Merge_Runs() to Sort the runs of Emp relations 
    cout << "Merging temp_files and outputting EmpSorted.csv" << endl;
    Merge_Runs();
   
    // Delete the temporary files (runs) after you've sorted the Emp.csv
    for (int i = 0; i < buffer_size; ++i) {
        tempFiles[i].close();
        std::string filename = "tempFile_" + std::to_string(i);
        const char* filename_cstr = filename.c_str();
        remove(filename_cstr);
    }
    
    return 0;
}
