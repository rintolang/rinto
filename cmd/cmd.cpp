#include "cmd.h"

std::map<std::string, Flag> flags;
std::map<std::string, Target> targets;

Flag arr[3];
Target targ[3];

void initTarget(){
  Target x86, x64, ARM;

  x86.type = "x86";
  x86.intel = true;
  targ[0] = x86;

  x64.type = "x64";
  x64.intel = true;
  targ[1] = x64;

  ARM.type = "ARM";
  ARM.intel = false;
  targ[2] = ARM;

  for(int a = 0; a < sizeof(targ)/sizeof(targ[0]); a++) {
    targets[targ[a].type] = targ[a];
  }
}

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

void printHelp(){
  std::cout<<"Usage:\n\n\trinto [Parameters=[value], flag] [file.rin]+"<<std::endl;
  std::cout<<"\nThe commands are: \n" << std::endl;
  for(int a = 0; a < sizeof(arr)/sizeof(arr[0]); a++){
    std::cout << "\t" << arr[a].flag[0] << " " << arr[a].flag[1];
    for (int i = 25 - arr[a].flag[1].length(); i>0; i--)
      std::cout << " ";

    std::cout << "\t" << arr[a].help << std::endl;
  }

  std::cout<<"\nThe parameters are: \n" << std::endl;
  for(int a = 0; a < sizeof(targ)/sizeof(targ[0]); a++)
    std::cout << "\t" << targ[a].type << std::endl;
}

int main(){
  initTarget();
  initFlags();
  printHelp();
}


