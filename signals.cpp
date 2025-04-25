#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    // TODO: Add your implementation
    std::cout << "smash: got ctrl-C" << std::endl;
    SmallShell& smash = SmallShell::getInstance();
    int fg_pid = smash.get_current_jop_pid_front();


    if (fg_pid != -1) {
        if (kill(fg_pid,SIGKILL) == 0) {
            std::cout << "smash: process " << fg_pid <<" was killed" << std::endl;

        }else {
            perror("smash error: kill failed");
        }
    }
smash.change_current_jop_pid_front(-1);

}
