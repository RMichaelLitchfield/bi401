all: qualitycheck

%: %.cc
	g++ -std=c++11 -lboost_program_options $< -o $@

%: %.c
	gcc $< -o $@

