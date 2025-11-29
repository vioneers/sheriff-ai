#pragma once

#include <string>
#include <vector>
#include <tuple>

#include "move.h"

using namespace std;

tuple<string, string> parse_args(int argc, char* argv[])
{
	string in_file = "";
	string out_file = "";

	for(int i = 1; i < argc; i++)
	{
		string arg = argv[i];

		if(arg == "-H" && i+1 < argc)
		{
			in_file = argv[i+1];
			++i;
		}
		else if(arg == "-m" && i+1 < argc)
		{
			out_file = argv[i+1];
			++i;
		}
	}

	return {in_file, out_file};
}

vector<move_t> get_move_history(string in_file_name)
{
	ifstream inFile(in_file_name);

	if(!inFile.is_open()) 
		exit(1);

	vector<move_t> history;
	string code;
	while(getline(inFile, code, '\n'))
	{
		if(code.empty())
			break;
			
		//cout << code << '\n';
		move_t move{code};
		history.push_back(move);
	}

	inFile.close();

	return history;
}

void write_move(move_t move, string out_file_name)
{
	ofstream outFile(out_file_name);
	
	outFile << move.to_code() << endl;

	if(!outFile.is_open())
		exit(1);

	outFile.close();
}
