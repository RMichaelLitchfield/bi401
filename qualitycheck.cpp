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
 #include <boost/random/uniform_int_distribution.hpp> //boost > 1.46
 #include <boost/random/variate_generator.hpp>
 #include <boost/program_options.hpp>
 #include <boost/algorithm/string.hpp>
 #include <boost/algorithm/string/predicate.hpp>

using namespace std;
namespace po = boost::program_options;

unsigned int goodseed();
int main(int argc, char *argv[] )
{
//    cout << "Boost version: " << BOOST_LIB_VERSION<<endl;
    string offered_filename;
    int max_samples;
    string out_filename;
    bool noreplace = false;
    vector<int> samplelist;
    int record_position;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "usage help")
        ("samples,s ", po::value<int>(&max_samples)->default_value(300), "number of samples (default 300)")
        ("input-file,i ", po::value<string>(&offered_filename), "input file (if filename has '.1.' in name "
        		"it will be treated as paired end and a second '.2.' file will be expected)")
        ("output-file,o ", po::value<string>(&out_filename),"output file (default 'sample-<input file>')")
        ("noreplace,n ", "samples will be unique (noreplace) (default false)")
        ;
    po::positional_options_description positionalOptions;
    positionalOptions.add("input-file", 1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc)
            .positional(positionalOptions).run(), 
          vm);
    //po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        cout<<"Commands for controlling sampler"<<endl<<desc<<endl;
        return-1;
    }
    if (!vm.count("input-file")) {
        cout<<"No input file offered\n";
        return -1;
    }
    if ( !boost::iends_with(offered_filename,".fastq") ) {
        cout<<offered_filename<<" does not end in fastq\n";
        return -1;
    }
    if (!vm.count("output-file")) {
        out_filename = "sample-" + offered_filename;
        cout<<"Writing to " << out_filename << " as no output file was"
        	" specified.\n";
    }
    if (vm.count("noreplace")) {
        noreplace = true;
        cout<<"Noreplace (no duplicate samples) is active\n";
    }
    bool paired = true;
    if ( offered_filename.find(".1.") == string::npos) { //if filename doesn't have '.1.'
        paired = false;
        //cout<<"This is not a paired end file\n";
    }
    //    get from offered_filename and output to out_filename
    ifstream myfile;
    ofstream outfile;
    myfile.open(offered_filename);
    outfile.open(out_filename);
    //if it is a paired end filename get from ".2" version of offered filename
	ifstream myotherfile;
	ofstream outfile2;
    if (paired) {
    	string infile2 = offered_filename;
    	string out2_filename = out_filename;
    	boost::replace_first(infile2, ".1.",".2.");
    	boost::replace_first(out2_filename, ".1.",".2.");
    	cout<<"Paired file so will draw from "<<infile2<<" and write those samples to "<<out2_filename<<".\n";
        myotherfile.open (infile2);
        outfile2.open (out2_filename);
    }
    //read first fastq record to use as a recognition template
    string record[4];
    int sample_length;
    if (myfile.is_open()) {
        for (int i=0; i<4;++i) {
            getline(myfile,record[i]);
        }
        sample_length = myfile.tellg();
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
    boost::random::mt19937_64 gen(goodseed()); // boost > 1.46
    boost::random::uniform_int_distribution<> dist(1, size);
    boost::variate_generator<boost::mt19937_64&, boost::random::uniform_int_distribution<> > position(gen, dist);
	int loopdetect = 0;
    while (samples < max_samples) {
        //cout<<"Getting sample "<< samples+1 <<" of "<<max_samples<<endl;
    	//Pick a random location in the source file, move back one sample length (or to 0 in first sample)
        int random_position = position();
		if (random_position > (sample_length) ){
			random_position -= (sample_length);
		} else random_position = 0;
        myfile.seekg(random_position,myfile.beg);
        if (paired){
        	myotherfile.seekg(random_position,myotherfile.beg);
        }
        string buffer;
        string buffer2;
        bool goodrecord = true;
        if (!myfile.eof()) {
        	getline(myfile,buffer);
        	if (paired){
        		getline(myotherfile,buffer2);
        	}
        }
        //cout<<"line: "<<buffer<<".\n";
        if (!myfile.eof()) {
        	//use illumina machine name as identifier to determine start of sample
            while (identifier != buffer.substr(0,buffer.find(':')) && (!myfile.eof())){
                //cout<< "this is not a start record:\n"<<buffer<<".\n";
                getline(myfile,buffer);
            	if (paired){
            		getline(myotherfile,buffer2);
            	}
            }
            record_position = myfile.tellg();
            //cout<<"1record position: "<<record_position<<"\n";
            if (noreplace) {
                //cout<<"Operating under noreplace\n";
                //if item is in list of used sample, don't use it again
                if (find(samplelist.begin(),samplelist.end(),record_position) != samplelist.end()) {
                    //cout<<"Skipping record at position "<<record_position<<endl;
                    goodrecord = false;
                }
            }
            if (!myfile.eof() && (goodrecord)) {
					samplelist.push_back(record_position);
					//cout<<record_position<<endl;
					//cout << "Writing record at "<<record_position<<".\n";
					for (int i=0;i<4;++i) {
						//send 4 lines of sample to outfile
						outfile << buffer <<endl;
						getline(myfile,buffer);
			        	if (paired){
			        		outfile2<<buffer2<<endl;
			        		getline(myotherfile,buffer2);
			        	}
					}
					++samples;
					loopdetect = 0;
				}
        }
        if (myfile.eof()) {
            //cout << "EOF reached" << endl;
            myfile.clear();
        	if (paired){
        		myotherfile.clear();
        	}
        }
    ++loopdetect;
    if (loopdetect == 100) {
    	cout<<"Unable to find "<<max_samples<<" creating a file with "<<samples<<" samples.\n";
    	samples = max_samples;
    }
    }
    myfile.close();
    outfile.close();
	if (paired){
		myotherfile.close();
		outfile2.close();
	}
    return 0;
}
/* goodseed() routine was obtained from:
 * http://stackoverflow.com/questions/2640717/c-generate-a-good-random-seed-for-psudo-random-number-generators
 */
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
