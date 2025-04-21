#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h
Command::Command(const char *cmd_line) : cmd_line(cmd_line) {}

Command::~Command() {}


std::string Command::getCommand() {
    return cmd_line;
}





void ShowPidCommand::execute() {
 std::cout <<"smash pid is " << SmallShell::getInstance().getPid() << std::endl; /// todo add the pid to here  << std::endl;
}

void GetCurrDirCommand::execute() {
    std::cout <<SmallShell::getInstance().getPwd() << std::endl;
}


void ChpromtCommand::execute() {
    std::string newPromtText = "smash";
   if (commands_parts.size() > 1) {
       newPromtText = commands_parts[1];
       // char temp[COMMAND_MAX_LENGTH];
       // strncpy(temp, newPromtText.c_str(), COMMAND_MAX_LENGTH - 1);
       // temp[COMMAND_MAX_LENGTH - 1] = '\0';
       //
       // _removeBackgroundSign(temp); // This is OK now

       // newPromtText = std::string(temp); // Update the string after cleaning
       if (newPromtText.empty()) {
           newPromtText = "smash";

       }
   }
    SmallShell::getInstance().setprompt(newPromtText);
}

void ChangeDirCommand::execute() {
    if (commands_parts.size() > 2) {
        if (commands_parts[commands_parts.size() - 1 ].compare("&") == 0) {

            commands_parts.pop_back();

        }
    }

        if (commands_parts.size() == 1) {

            SmallShell::getInstance().changePwd(SmallShell::getInstance().getPwd());
            return;
        }
        if (commands_parts.size() == 2) {
            std::string path = commands_parts[1];
            if (path.compare("-") == 0 ) { // todo : now we do in case we get -
                if (SmallShell::getInstance().getLastPwd().compare("") == 0) { /// in case of the lastpwd is none
                    cerr << "smash error: cd: OLDPWD not set" << endl;
                    return;
                }
                if (chdir(SmallShell::getInstance().getLastPwd().c_str()) == 0) {
                    SmallShell::getInstance().changePwd(SmallShell::getInstance().getLastPwd());

                }else {
                    perror("smash error: chdir failed");
                }


            }else { /// todo if we dont get - in the end
                if (chdir(path.c_str()) == 0) {
                    SmallShell::getInstance().changePwd(path);
                }else {
                    perror("smash error: chdir failed");
                }
            }
        } else {
            std::cerr <<"smash error: cd: too many arguments" << std::endl;

        }


    }


void JobsList::addJob(Command *cmd, bool isStopped ) {
    removeFinishedJobs();

    int jopid = 1;

    if (!jobs_list.empty()) {
        jopid = jobs_list.back()->jopId + 1;   //// if we have jop in our list as the jop we add id is the last one + 1

    }
    std::string command = cmd->getCommand();



}

void JobsList::printJobsList() {
    removeFinishedJobs();
    for(JobsList::JobEntry* jobEntry:jobs_list) {
        std::cout << "[" << jobEntry->jopId << "] " << jobEntry->command << std::endl;
    }

}

void JobsList::killAllJobs() {

}

void JobsList::removeFinishedJobs() {
    for (auto it = jobs_list.begin(); it != jobs_list.end(); ) {
        if (waitpid((*it)->pid, nullptr, WNOHANG) != 0) {
            delete *it;
            it = jobs.erase(it);
        } else {
            ++it;
        }
    }
}


JobsList::JobEntry *JobsList::getJobById(int jobId) {

}

void JobsList::removeJobById(int jobId) {

}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {

}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {

}



void JobsCommand::execute() { //// todo : command jop number 6 in hw pdf
    jobsList->printJobsList();
}








void ForegroundCommand::execute() {
int jopid = 0;
if (commands_parts.size() ==  1) { // todo : we need the max jop id the last in the list
    if(jobs->jobs_list.empty()) {
        std::cerr << "smash error: fg: jobs list is empty" << std::endl;
        return;
    } else

}
}






















SmallShell::SmallShell() : prompt("smash"),pid(getPid()),pwd(getPwd()),lastpwd("") {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

int SmallShell::getPid() const {
    return pid;
}

std::string SmallShell::getPwd()  {
    char temp[COMMAND_MAX_LENGTH];
    getcwd(temp, COMMAND_MAX_LENGTH);
    return std::string(temp);
}

void SmallShell::setprompt(const std::string &new_prompt) {
    prompt = new_prompt;
}
const std::string &SmallShell::getprompt() const {
    return prompt;
}
char **SmallShell::getLastPwdPtr() {
    return lastpwd.c_str();
}
void SmallShell::changePwd(std::string new_pwd) {
    std::string temp = pwd;
    pwd = new_pwd;
    lastpwd = temp;
}
std::string SmallShell::getLastPwd() {
    return lastpwd;
}








/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line) {
    // For example:
    /*
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("pwd") == 0) {
      return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
      return new ShowPidCommand(cmd_line);
    }
    else if ...
    .....
    else {
      return new ExternalCommand(cmd_line);
    }
    */

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("chpromt") == 0) {
        return new ChpromtCommand(cmd_line);
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line,SmallShell::getInstance().getLastPwdPtr());
    }else {
        return nullptr;
    }


}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}