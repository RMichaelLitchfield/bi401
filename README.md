bi401
=====
Bi401 (Fall 2013) project under Dr. Rodger Voelker (rvoelker@uoregon.edu) by R. Michael Litchfield (rml@uoregon.edu) to 
create a program to sample a fastq genetics sequencer file (which tend to be very large) for quality analysis and 
shotgun evaluation of sequencing efforts.

Commands for controlling sampler
Allowed options:
  -h [ --help ]               usage help
  -s [ --samples ] arg (=300) number of samples (default 300)
  -i [ --input-file ] arg     input file (if filename has '.1.' in name it will
                              be treated as paired end and a second '.2.' file 
                              will be expected)
  -o [ --output-file ] arg    output file (default 'sample-<input file>')
  -n [ --noreplace ]          samples will be unique (noreplace) (default 
                              false)
