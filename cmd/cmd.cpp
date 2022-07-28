#include "cmd.h"
std::map<std::string, Flag> flags;
Flag arr[3];

void initFlags(){
  Flag assembly,help,version;

  assembly.flag[0] = "-S";
  assembly.flag[1] = "";
  assembly.help = "Compiles the *.rin into an assembly file.";
  assembly.takesValue = false;
  arr[0] = assembly;

  help.flag[0] = "-h";
  help.flag[1] = "--help";
  help.help = "Prints out a list of commands.";
  help.takesValue = false;
  arr[1] = help;

  version.flag[0] = "-v";
  version.flag[1] = "--version";
  version.help = "Prints out the current version of rinto.";
  version.takesValue = false;
  arr[2] = version;

  for(int a = 0; a < sizeof(arr)/sizeof(arr[0]); a++) {
    flags[arr[a].flag[0]] = arr[a];
    if(arr[a].flag[1] != ""){
      flags[arr[a].flag[1]] = arr[a];
    }
  }
}

int main(){
  initFlags();
  printHelp();
}

void printHelp(){
  std::cout<<"Usage:\n\n\trinto [Parameters=[value], flag] [file.rin]+"<<std::endl;
  std::cout<<"\nThe commands are: \n" << std::endl;

  for(int a = 0; a < sizeof(arr)/sizeof(arr[0]); a++){
    std::cout << "\t" << arr[a].flag[0] << " " << arr[a].flag[1];
    for (int i = 25 - arr[a].flag[1].length(); i>0; i--)
      std::cout << " ";

    std::cout << "\t" << arr[a].help << std::endl;
  }

}
