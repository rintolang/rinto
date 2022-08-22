#include "cmd.h"

map<string, Flag> flagMap;
map<string, Target> targetMap;

Target targetArr[5];
Flag flagArr[4];

void initTarget()
{
        Target x86, x64, ARM, ARM64, RISCV;

        x86.type = "x86-32";
        x86.intel = true;
        targetArr[0] = x86;

        x64.type = "x86-64";
        x64.intel = true;
        targetArr[1] = x64;

        ARM.type = "ARM";
        ARM.intel = false;
        targetArr[2] = ARM;

        ARM64.type = "ARM-64";
        ARM64.intel = false;
        targetArr[3] = ARM64;

        RISCV.type = "RISC-V";
        RISCV.intel = false;
        targetArr[4] = RISCV;

        for(int i = 0; i < sizeof(targetArr)/sizeof(targetArr[0]); i++) 
                targetMap[targetArr[i].type] = targetArr[i];
}

void initFlags()
{
        Flag assembly,help,version,target;

        assembly.flag[0] = "-S";
        assembly.flag[1] = "";
        assembly.help = "Compiles the *.rin into an assembly file.";
        assembly.takesValue = false;
        flagArr[0] = assembly;

        help.flag[0] = "-h";
        help.flag[1] = "--help";
        help.help = "Prints out i list of commands.";
        help.takesValue = false;
        flagArr[1] = help;

        version.flag[0] = "-v";
        version.flag[1] = "--version";
        version.help = "Prints out the current version of rinto.";
        version.takesValue = false;
        flagArr[2] = version;

        target.flag[0] = "-t";
        target.flag[1] = "--target";
        target.help = "Target specifies the file's compilation target architecture.";
        target.takesValue = true;
        flagArr[3] = target;

        for(int i = 0; i < sizeof(flagArr)/sizeof(flagArr[0]); i++) {

                flagMap[flagArr[i].flag[0]] = flagArr[i];

                if(flagArr[i].flag[1] != "")
                        flagMap[flagArr[i].flag[1]] = flagArr[i];
        }
}

void printHelp()
{
        cout<<"Usage:\n\n\trinto [Parameters=[value], flag] [file.rin]+"<<endl;
        cout<<"\nThe commands are: \n" << endl;

        for(int i = 0; i < sizeof(flagArr)/sizeof(flagArr[0]); i++){
                cout << "\t" << flagArr[i].flag[0] << " " << flagArr[i].flag[1];

                for (int i = 25 - flagArr[i].flag[1].length(); i>0; i--)
                        cout << " ";

                cout << "\t" << flagArr[i].help << endl;
        }
        cout<<"\ntargetMap: \n" << endl;
        
        for(int i = 0; i < sizeof(targetArr)/sizeof(targetArr[0]); i++)
                cout << "\t" <<targetArr[i].type;

        cout<<endl;
}
