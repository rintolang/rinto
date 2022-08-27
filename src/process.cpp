#include "process.h"

void process_command(const string &command)
{
	vector<string> delimitedCommand = delimitSplit(command, ' ');
	bool valid = (delimitedCommand[0] == "rinto");
	bool cmdIsFound = false;
	bool targIsFound = false;
	if(valid){
		for(int i = 0; i < sizeof(flagArr)/sizeof(flagArr[0]); i++){
			if(flagArr[i].flag[0] == delimitedCommand[1] ||
			  (flagArr[i].flag[1] != "" &&
			   flagArr[i].flag[1] == delimitedCommand[1])){
			   	cmdIsFound = true;
			   	if(flagArr[i].takesValue){
			   		for(int i = 0 ; i < 5; i++){
			   			if(targetArr[i] == delimitedCommand[2]){
			   				cout<<delimitedCommand[2]<<endl;
			   				targIsFound = true;
			   				break;
			   			}
			   		}

			   		if(!targIsFound)
			   			cout<<"Invalid target for command"<<endl;
			   	}
				cout<<flagArr[i].help<<endl;
				break;
			}
		}
		if(!cmdIsFound)
			cout<<"Please type in a proper command"<<endl;
	} else {
		cout<<"Invalid command"<<endl;	
	}
}

vector<string> delimitSplit(const string& str, const char& ch)
{
	string next;
	
	vector<string> result;
	
	for (string::const_iterator it = str.begin(); it != str.end(); it++) {
		if (*it == ch || *it == '='){
			if (!next.empty()){
                		result.push_back(next);
                		next.clear();
            		}
        	} else {
            		next += *it;
        	}
    	}

	if (!next.empty())
		result.push_back(next);

	return result;
}