/**
 *  @name           memorycheck.cc
 *
 *  @description    This program will display memory size of a file in three
 *                  different ways to make sure everything works as expected.
 *
 *  @author         R. Michael Litchfield
 *  @email          RML@uoregon.edu
 */
 
 #include <iostream>
 #include <fstream>
 #include <iomanip>
 #include <ios>
 #include <string>
/* included to use fstat */
 #include <unistd.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 using namespace std; 

 int main(int argc, char *argv[])
 {
    if ( argc != 2 ) { // need two arguments (argc) to work
        // argv[0] is the program name
        cout<<"usage: "<< argv[0] <<" <filename>\n";
        return -1;
    } else {
        // argv[1] is a filename to open
        string offered_filename;
        offered_filename = argv[1];
        long size = 0;//file size (end)
        long start = 0;//file beginning
        //Test filesize with fstream    
        ifstream myfile;
        myfile.open (offered_filename);
        if (myfile.is_open()) {
            start = myfile.tellg();
            myfile.seekg (0,myfile.end);
            size = myfile.tellg();
            cout << "File end of " << offered_filename << " is " <<size<<" according to fstream.\n";
            cout << "End-Start is " << size-start << "\n";
            myfile.close();
        } else {
            std::cout << "\nCould not open that file with fstream.\n";
            return -1;
        }
        //  get filesize with fstat 
        size=0;
        struct stat buffer;
        int status;
        FILE * file;
        if((file=fopen(offered_filename.c_str(),"r")) < 0){
             std::cout << "\nCould not open that file with fstat.\n";
            return -1;
        } else {
            status = fstat(fileno(file),&buffer);
            size=buffer.st_size;
            cout << "File end of " << offered_filename << " is " <<size<<" according to fstat.\n";
            size=0;
            start=ftell(file);
            fseek(file,0,SEEK_END);
            size=ftell(file);
            cout << "File end of " << offered_filename << " is " <<size<<" according to fseek.\n";
            cout << "End-Start is " << size-start << "\n";
            fclose(file);
        }
    }
    return 0;
}