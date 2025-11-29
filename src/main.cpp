#include <iostream> 
#include "move.h"
using namespace std; 

int main(){
	string code;
    cin>>code;

	move_t move(code);
	cout<<"from_rank = "<<move.from_rank<<'\n';
	cout<<"to_rank = "<<move.to_rank<<'\n';
	cout<<"promotion = " << move.promotion<<'\n';

	cout<<move.to_code();

    return 0;
}
