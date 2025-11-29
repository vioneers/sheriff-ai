#pragma once

#include <string>
#include <tuple>

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
			cout << argv[i+1] << '\n';
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
