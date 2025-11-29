#include <fstream> 
#include <iostream>

#include "utils.h"
#include "move.h"

using namespace std; 

int main(int argc, char* argv[])
{
	// parse arguments and make sure both filenames are given
	auto [in_file_name, out_file_name] = parse_args(argc, argv);
	
	if(in_file_name == "")
	{
		cerr << "Input file not provided. Pass it with -H" << endl;
		return 1;
	}
	if(out_file_name == "")
	{
		cerr << "Output file not provided. Pass it with -m" << endl;
		return 1;
	}
	
	ifstream inFile(in_file_name);

	if(!inFile.is_open()) 
	{
		cerr << "Error: Unable to open input file." << endl;
		return 1;
	}

	// read input data

	inFile.close();
	


	// run fancy algorithm
	string code;
    cin>>code;

	move_t move(code);
	cout<<"from_file = "<<move.from_file<<'\n';
	cout<<"to_file = "<<move.to_file<<'\n';
	cout<<"promotion = " << move.promotion<<'\n';



	ofstream outFile(out_file_name);
	
	outFile << move.to_code() << endl;

	if(!outFile.is_open())
	{
		cerr << "Error: Unable to open output file." << endl;
		return 1;
	}

	outFile.close();

    return 0;
}
