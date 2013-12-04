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
 * 
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
 #include <boost/program_options.hpp>
 #include <boost/algorithm/string.hpp>
 /*
 #include <boost/program_options/options_description.hpp>
 #include <boost/program_options/parsers.hpp>
 #include <boost/program_options/variables_map.hpp>
 */
using namespace std;
namespace po = boost::program_options;

unsigned int goodseed();
int main(int argc, char *argv[] )
{
//    cout << "Boost version: " << BOOST_LIB_VERSION<<endl;
    string offered_filename;
    int max_samples;
    string out_filename;
    bool noreplace;
    vector<int> samplelist;
    int record_position;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "usage help")
        ("samples,s", po::value<int>(&max_samples)->default_value(300), "number of samples (default 300)")
        ("input-file,i", po::value<string>(&offered_filename), "input file")
        ("output-file,o", "output file (default 'sample-<input file>')")
        ("noreplace,n", "will samples be unique (noreplace) (default false)")
        ;
    po::positional_options_description positionalOptions; 
    positionalOptions.add("input-file", 1); 
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc) 
            .positional(positionalOptions).run(), 
          vm); 
    //po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout<<"Commands for controlling sampler"<<endl<<desc<<endl;
    }
    if (!vm.count("input-file")) {
        cout<<"No input file offered\n";
        return -1;
    }
    /* getting weird errors with this
    if ( !boost::algorithm::endswith(vm.count("input-file"),"fastq") {
        cout<<"This is not a fastq file\n";
        return-1
    }
    */
    if (!vm.count("output-file")) {
        out_filename = "sample-" + offered_filename;
    }
    if (vm.count("noreplace")) {
        noreplace = true;
    } else {
        noreplace = false;
    }
    po::notify(vm);
    cout<<"Offered file: "<<offered_filename<<".\n";
    bool paired = true;
    if ( offered_filename.find(".1.") == string::npos) { //if filename doesn't have '.1.'
        paired = false;
        cout<<"This is not a paired end file\n";
    }
/*  relic from pre-boost  commandline tools
    if ((argc<2) || argc>5) { // need two to four arguments (argc) to work
        // argv[0] is the program name
        cout<<"usage: "<< argv[0] <<" <filename> <number of samples> <outputfilename> <-n no replacement flag<\n";
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
        (argv[4] == "-n") ? noreplace = true:noreplace=false;
    }*/
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
    int samples = 0;
    //boost::mt19937_64 gen; // boost > 1.46
    boost::rand48 gen(goodseed()); //boost < 1.47
    boost::uniform_int<> dist(1, size);
    //boost::variate_generator<boost::mt19937_64&, boost::uniform_int<> > position(gen, dist);
    boost::variate_generator<boost::rand48&, boost::uniform_int<> > position(gen, dist);
    while (samples < max_samples) {
        cout<<"Getting sample "<<samples<<" of "<<max_samples<<endl;
        int random_position = position();
        cout<<"random position: "<<random_position<<"\n";
        myfile.seekg(random_position,myfile.beg);
        string buffer;
        getline(myfile,buffer);
        cout<<"line: "<<buffer<<".\n";
        record_position = myfile.tellg();
        cout<<"1record position: "<<record_position<<"\n";
        if (noreplace) {
            cout<<"Operating under noreplace\n";
            //while item is not in list
            if (find(samplelist.begin(),samplelist.end(),record_position) != samplelist.end()) {
                cout<<"Skipping record at position "<<record_position<<endl;
                getline(myfile,buffer);
            }
        }
        while (!myfile.eof()) {
            while (identifier != buffer.substr(0,buffer.find(':'))) {
                getline(myfile,buffer);
            }
            samplelist.push_back(record_position);
            for (int i=0;i<4;++i) {
                outfile << buffer <<endl;
                getline(myfile,buffer);
            }
            ++samples;
        }
        if (myfile.eof()) {
            cout << "EOF reached" << endl;
            myfile.clear();
        }
    }
    //  call report class report;
    cout << max_samples << " samples taken and put in " << out_filename << ".\n";
    cout << "Got record at positions:";
    for( vector<int>::const_iterator i = samplelist.begin(); i != samplelist.end(); ++i){
        std::cout << *i << ' ';
    }
    cout <<endl;
    myfile.close();
    outfile.close();
    return 0;
}
unsigned int goodseed() {
    unsigned int random_seed, random_seed_a, random_seed_b; 
    std::ifstream file ("/dev/urandom", std::ios::binary);
    if (file.is_open()) {
        char * memblock;
        int size = sizeof(int);
        memblock = new char [size];
        file.read (memblock, size);
        file.close();
        random_seed_a = *reinterpret_cast<int*>(memblock);
        delete[] memblock;
    } else {
        random_seed_a = 0;
    }
    random_seed_b = std::time(0);
    random_seed = random_seed_a xor random_seed_b;
    /* Troubleshooting
    std::cout << "random_seed_a = " << random_seed_a << std::endl;
    std::cout << "random_seed_b = " << random_seed_b << std::endl;
    std::cout << " random_seed =  " << random_seed << std::endl;
    */
    return random_seed;
} // end goodseed()