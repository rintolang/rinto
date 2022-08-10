#include "process.h"

void process_command(const std::string &command)
{

	std::string keyword = "rinto";
	std::string flag = "";
	bool valid = true;


	//Could use a while loop for this?
	for(int i = 0 ; i < 5 ; i++){
		if(command[i] != keyword[i]){
			valid = false;
			break;
		}
	}

	/*	If the keyword rinto is used in the command,
	*	then "process" the command after the first
	*	5 charachters. The "processing" right now is
	*	just printing each charachter in a new line.
	*	
	*	This needs to be updated to sort out which
	*	parameter is being passed through the cmd
	*	and use the appropriate structures to be used
	*	to expect further input (such as targets) to
	*	be required.
	*/

	if(valid) {
		for (std::string::size_type i = 5; i < command.size(); i++)
			std::cout<<command[i]<<std::endl;
	} else {
		std::cout<<"Invalid command"<<std::endl;
	}
}