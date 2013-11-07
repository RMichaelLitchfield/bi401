/**
 *  @name           qualitycheck.cc
 *
 *  @description    This program will prompt that user for a fastq file, open 
 *                  it, parse the first four lines to determine which fastq
 *                  format is being used, go to the last four lines, confirm it
 *                  is the same format, randomly sample the file,return quality
 *                  averages.
 *
 *  @author         R. Michael Litchfield
 *  @email          RML@uoregon.edu
 */
 
 #include <iostream>
 #include <fstream>
 #include <iomanip>
 #include <ios>
 #include <string>
 #include <boost/random/mersenne_twister.hpp>
 #include <boost/version.hpp>
 //#include <boost/random/uniform_int_distribution.hpp> //boost > 1.46
 #include <boost/random/uniform_int.hpp> //boost < 1.47
 #include <boost/random/variate_generator.hpp>
 using namespace std;

int main(int argc, char *argv[] )
{
    cout << "Boost version: " << BOOST_LIB_VERSION<<endl;
    string offered_filename;
    int max_samples;
    string out_filename;
    if ((argc<2) || argc>4) { // need two to four arguments (argc) to work
        // argv[0] is the program name
        cout<<"usage: "<< argv[0] <<" <filename> <number of samples> <outputfilename> \n";
        return -1;
    } else {
        //argv[1] is the file to be samples
        offered_filename = argv[1];
        if (argv[2] != NULL) {//argv[2] is the number of samples
            max_samples = atoi(argv[2]);
            out_filename = argv[3];//argv[3] is the output filename
        } else {
            max_samples = 300;
            out_filename = "sample" + offered_filename;
        }
    }
    //    getfile
    ifstream myfile;
    ofstream outfile;
    myfile.open (offered_filename);
    outfile.open (out_filename);
    //read first fastq record to use as a recognition template
    string record[4];
    if (myfile.is_open()) {
        for (int i=0; i<4;++i) {
            getline(myfile,record[i]);
        }
    } else {
        std::cout << "\nThere was a problem opening that file.\n";
        return -1;
    }
    string identifier = record[0].substr(0,record[0].find(':'));
    //  get filesize
    long size;
    myfile.seekg (0,myfile.end);
    size = myfile.tellg();
    // do real stuff with size
    cout << "File " << offered_filename << " is " <<size<<" bytes long.\n";
    int samples = 0;
//    boost::mt19937_64 gen; // boost > 1.46
    boost::rand48 gen; //boost < 1.47
    while (samples < max_samples) {
        boost::uniform_int<> dist(1, size);
//      boost::variate_generator<boost::mt19937_64&, boost::uniform_int<> > position(gen, dist);
        boost::variate_generator<boost::rand48&, boost::uniform_int<> > position(gen, dist);
        int random_position = position();
        myfile.seekg(random_position,myfile.beg);
        string buffer;
        getline(myfile,buffer);
        while (identifier != buffer.substr(0,buffer.find(':'))) {
            getline(myfile,buffer);
        }
        for (int i=0;i<4;++i) {
            outfile << buffer <<endl;
            getline(myfile,buffer);
        }
        ++samples;
    }
    //  call report class report;
    cout << max_samples << " samples taken and put in " << out_filename << ".\n";
    myfile.close();
    outfile.close();
    return 0;
}