#include "cmd/cmd.cpp"
#include "process/process.cpp"

int main(){
	initTarget();
        initFlags();
        printHelp();

        process_command("rinto -S");

        return 0;
}