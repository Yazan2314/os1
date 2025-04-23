#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <string>

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
Command::Command(const char *cmd_line) : cmd_line(cmd_line),processid(getpid()) {
    isbackground = false;
    std::istringstream iss(cmd_line);
    std::string word;
    while (iss >> word) {
        commands_parts.push_back(word);
    }
    if (commands_parts.size() > 0) {
        if (commands_parts[commands_parts.size() - 1].compare("&") == 0) {
            isbackground = true;
            commands_parts.pop_back();
        }
    }





}

Command::~Command() {}


std::string Command::getCommand() {
    return cmd_line;
}
bool Command::isValidAlias(const char* cmd_line) {
    std::regex pattern("^alias [a-zA-Z0-9_]+='[^']*'$");
    return std::regex_match(cmd_line, pattern);
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
    for (auto job : jobs_list) {
        if (kill(job->pid,SIGKILL) == -1) {
            perror("smash error: kill failed");
        }
        delete job;
    }

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


    for (auto job : jobs_list) {
        if (job->jopId == jobId) {
         return job;
        }
    }
        return nullptr;
}

void JobsList::removeJobById(int jobId) {

}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {

    if (jobs_list.empty() ) {
        return nullptr;
    }
  JobEntry * last_jop;
        int maxid =  -1;
    for (auto jop : jobs_list) {
        if (jop->jopId > maxid) {
            maxid = jop->jopId;
            last_jop = jop;
        }
    }
    if (last_jop != nullptr) {
        *lastJobId = maxid;
    }

    return last_jop;
}



JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {

}
void JobsList::printjopsListpid() {
    removeFinishedJobs();
    for(JobsList::JobEntry* jobEntry:jobs_list) {
        std::cout << jobEntry->pid << ": " << jobEntry->command << std::endl;
    }

}



void JobsCommand::execute() { //// todo : command jop number 6 in hw pdf
    jobsList->printJobsList();
}








void ForegroundCommand::execute() {
    int jopid = 0;
    JobsList::JobEntry *job = nullptr;

if (commands_parts.size() ==  1) { // todo : we need the max jop id the last in the list
    if(jobs->jobs_list.empty()) {
        std::cerr << "smash error: fg: jobs list is empty" << std::endl;
        return;
    } else {
         job = jobs->getLastJob(&jopid);  /// todo : i use the treck i get the id and the jop in single function
        if (!job) {
            std::cerr << "smash error: fg: jobs list is empty" << std::endl;
            return;
        }
    }

}else if (commands_parts.size() == 2) {
    try {
        jopid = stoi(commands_parts[1]);


    }catch (...) {
        std::cerr << "smash error: fg: invalid arguments" << std::endl;
        return;
    }
    if(jobs->jobs_list.empty()) {
        std::cerr << "smash error: fg: jobs list is empty" << std::endl;
        return;
    }
    job = jobs->getJobById(jopid);
    if (job == nullptr) {
        std::cerr << "smash error: fg: job-id " << jopid <<  " <job-id> does not exist " << std::endl;
        return;
    }
    std::cout << job->command << " " << job->pid << std::endl;
    int pid = job->pid;


    jobs->removeJobById(jopid);

    SmallShell::getInstance().bringToForeground(pid);


    if (waitpid(pid, nullptr, WUNTRACED) == -1) {
        perror("smash error: waitpid failed");
    }

    // بعد ما تخلص نحط -1 لأنو ما في عملية بالـ foreground
    SmallShell::getInstance().current_jop_pid_front = -1;
}

}
}





void QuitCommand::execute() {
    if (commands_parts.size() == 1) {
        exit(0);

    }
    if (commands_parts.size() > 1) {
        if (commands_parts[1].compare("kill") != 0) {
            exit(0);
        }else {
            std::cout << "smash: sending SIGKILL signal to " << jobs->jobs_list.size() <<" jobs:" << std::endl;
            jobs->printjopsListpid();
            jobs->killAllJobs();
        }
    }
    exit(0);  /// todo: may have a proplem here
}



void KillCommand::execute() {
    if (commands_parts.size() != 3) {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }else {
        int signum;
        int jopid;

        try {
            signum = stoi(commands_parts[1]);
            jopid = stoi(commands_parts[2]);
        }catch (...) {
            cerr << "smash error: kill: invalid arguments" << std::endl;
            return;
        }

        JobsList::JobEntry *job = jobs->getJobById(jopid);
        if (job != nullptr) {
            if (kill(job->pid,signum) == -1) {
                perror(("smash error: kill failed"));
            }else {
                std::cout << "signal number: "  << signum * -1 <<" was sent to pid " << job->pid << std::endl;
            }
        } else {
            cerr << "smash error: kill: job-id "<<  jopid  <<" does not exist " << std::endl;
        }


    }
}
bool parseAlias(const std::string& input, std::string& name, std::string& command) {
    size_t equal_pos = input.find('=');
    if (equal_pos == std::string::npos || equal_pos == 0) {
        return false;
    }

    name = input.substr(0, equal_pos);

    // Check command is wrapped in single quotes
    if (equal_pos + 2 >= input.size() || input[equal_pos + 1] != '\'' || input.back() != '\'') {
        return false;
    }

    command = input.substr(equal_pos + 2, input.size() - equal_pos - 3);
    return true;
}

void AliasCommand::execute() {
    std::vector<std::string> alias_vector = SmallShell::getInstance().getAliases_ord();
    std::map<std::string, std::string> temp = SmallShell::getInstance().getAliases();
    std::vector<std::string> command_vector = SmallShell::getInstance().getcommand_vector();
    if (commands_parts.size() == 1) {


        for (auto ali : alias_vector) {
            std::cout << ali << "='" << temp[ali] << "'" << std::endl;
        }


    }else {
        if (!isValidAlias(cmd_line)) {
            cerr << "smash error: alias: invalid alias format" << std::endl;
            return;
        }else if (commands_parts.size() == 2) {

            std::string  name;
            std::string command;
            bool reserved = false;
            bool result  = parseAlias(commands_parts[1],name,command);
            for (auto cam : command_vector) {
                if (cam == name){
                    reserved = true;
                }
            }
                if (temp.count(name) || reserved) {

                    cerr << "smash error: alias: " << name << " already exists or is a reserved command" << endl;

                }


                SmallShell::getInstance().addAlias(name , command );


        }
    }
}



void::UnAliasCommand::execute() {
    std::vector<std::string> alias_vector = SmallShell::getInstance().getAliases_ord();
    std::map<std::string, std::string> temp = SmallShell::getInstance().getAliases();
    std::vector<std::string> command_vector = SmallShell::getInstance().getcommand_vector();
    if (commands_parts.size() <= 1){

        std::cerr << "smash error: unalias: not enough arguments" << std::endl;
    }else {
        for (int i = 1 ; i < commands_parts.size() ; ++i){
            if(!temp.count(commands_parts[i])) {
            std::cerr << "smash error: unalias: "<<commands_parts[i] <<" alias does not exist" << std::endl;
                return;
            }
            for (int i = 1 ; i < commands_parts.size() ; ++i) { /// todo: we need to check back
                SmallShell::getInstance().removeAlias(commands_parts[i]);
            }


        }
    }
}

















SmallShell::SmallShell() : prompt("smash"),pid(getPid()),pwd(getPwd()),lastpwd("") {
// TODO: add your implementation
    creatCommand_vector();
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


JobsList *SmallShell::getJobsList() {
    return shell_jops;
}


void SmallShell::bringToForeground(int pid) {
    current_jop_pid_front = pid;
}

void SmallShell::addAlias(const std::string &alias, const std::string &command) {
    aliases[alias] = command;
    aliases_inorder.push_back(alias);
}

void SmallShell::removeAlias(const std::string &alias) {
    aliases.erase(alias);
    for (auto it = aliases_inorder.begin(); it != aliases_inorder.end(); ++it) {
        if (*it == alias) {
            aliases_inorder.erase(it);
            break;  // Exit the loop after removing the alias
        }
    }
}

std::vector<std::string> SmallShell::getAliases_ord() {
    return aliases_inorder;
}

std::map<std::string, std::string> SmallShell::getAliases() {
    return aliases;
}


void SmallShell::creatCommand_vector(){
    commands_vector.push_back("chprompt");
    commands_vector.push_back("pwd");
    commands_vector.push_back("showpid");
    commands_vector.push_back("cd");
    commands_vector.push_back("jobs");
    commands_vector.push_back("fg");
    commands_vector.push_back("quit");
    commands_vector.push_back("kill");
    commands_vector.push_back("alias");
    commands_vector.push_back("unalias");
    commands_vector.push_back("unsetenv");
    commands_vector.push_back("watchproc");
    commands_vector.push_back("du");
    commands_vector.push_back("whoami");
    commands_vector.push_back("netinfo");
}
std::vector<std::string> SmallShell::getcommand_vector() {
    return commands_vector;
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
    }else if (firstWord.compare("jops") == 0) {
        return new JobsCommand(cmd_line,SmallShell::getInstance().getJobsList());
    } else if (firstWord.compare("fg") == 0) {
    }else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line,SmallShell::getInstance().getJobsList());
    }else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line,SmallShell::getInstance().getJobsList());
    }else if (firstWord.compare("alias") == 0) {

    }


}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}