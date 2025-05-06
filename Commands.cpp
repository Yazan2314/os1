#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <string>
#include <fcntl.h>
#include <sys/types.h>  // for basic system types
#include <sys/stat.h>
#include <syscall.h>
#include <netinet/in.h>   // for struct in_addr
#include <arpa/inet.h>
#include <fstream>



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
#define  SYS_FAIL  -1
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
    has_alias = false;
    his_alias = "";


    char cmd_copy[1024];
    strncpy(cmd_copy, cmd_line, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0'; // null terminate

    if (_isBackgroundComamnd(cmd_line)) {
        isbackground = true;
        _removeBackgroundSign(cmd_copy);



    }


    std::istringstream iss(cmd_copy);
    std::string word;
    while (iss >> word) {
        commands_parts.push_back(word);
    }






}

Command::~Command() {}


int Command::getProcessID() {
    return processid;
}



std::string Command::getCommand() {
    return cmd_line;
}
bool Command::isValidAlias(const char* cmd_line) {
    std::regex pattern("^alias [a-zA-Z0-9_]+='[^']*'$");
    return std::regex_match(cmd_line, pattern);
}





void ShowPidCommand::execute() {
 std::cout <<"smash pid is " << getpid() << std::endl; /// todo add the pid to here  << std::endl;
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
       std::cerr << "smash error: cd: too many arguments" << std::endl;
       return;
    }

        if (commands_parts.size() == 1 ) {

            SmallShell::getInstance().changePwd(SmallShell::getInstance().getPwd());
            return;
        }
    char cwd[COMMAND_MAX_LENGTH];
        if (commands_parts.size() == 2) {
            std::string path = commands_parts[1];
            if (path.compare("-") == 0 ) {
                // todo : now we do in case we get -
                if (SmallShell::getInstance().getLastPwd().compare("") == 0) { /// in case of the lastpwd is none
                    cerr << "smash error: cd: OLDPWD not set" << endl;
                    return;
                }
                if (chdir(SmallShell::getInstance().getLastPwd().c_str()) == 0) {
                    SmallShell::getInstance().changePwd(SmallShell::getInstance().getLastPwd());

                }else {
                    perror("smash error: chdir failed");
                }
            }else if (path.compare(".") == 0) {
                SmallShell::getInstance().changePwd(SmallShell::getInstance().getPwd());
                return;
            } else if (path.compare("..") == 0) {
                if (chdir("..") == 0) {
                    if (getcwd(cwd,COMMAND_MAX_LENGTH) != nullptr){
                        SmallShell::getInstance().changePwd(cwd);

                    }else {
                        perror("smash error: getcwd failed");
                    }
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




 void JobsList::addJob(pid_t pid ,  std::string cmd_str   , bool isStopped   )  {
        removeFinishedJobs();

        int jopid = 1;



        int maxid =  -1;

        // if (!jobs_list.empty()) {
        //     jopid = jobs_list.back()->jopId + 1;   //// if we have jop in our list as the jop we add id is the last one + 1
        //
        // }
        for (const auto &job : jobs_list) {
            if (job->jopId > maxid) {
                maxid = job->jopId;
            }
        }
        if (maxid != -1) {
            jopid = maxid + 1;
        }
        JobEntry *new_job = new JobEntry(jopid,pid,cmd_str,nullptr);

        // std::string command = cmd->getCommand();
        //
        // int pid  = cmd->getProcessID();



        jobs_list.push_back(new_job);




    }

    void JobsList::printJobsList() {
    removeFinishedJobs();
    for(JobsList::JobEntry* jobEntry:jobs_list) {

        std::cout << "[" << jobEntry->jopId << "] " << jobEntry->command <<  std::endl;


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
        int status;
        // Check if the process has finished using waitpid with WNOHANG
        if (*it == nullptr || waitpid((*it)->pid, &status, WNOHANG) != 0) {
            delete *it;                    // Free memory
            it = jobs_list.erase(it);     // Remove from list and update iterator
        } else {
            ++it; // Move to next job
        }
    }
}

JobsList::JobEntry *JobsList::getJobBypid(int pid) {
        for (auto jop : jobs_list) {
            if (jop->pid == pid) {
                return jop;
            }
        }
    return nullptr;
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
    for (auto it = jobs_list.begin(); it != jobs_list.end(); ++it) {
        if ((*it)->jopId == jobId) {
            delete *it;             // Free memory
            jobs_list.erase(it);    // Remove from list
            return;                 // Exit after removing
        }
    }
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
    // if (jobs_list.empty() ) {
    //     return nullptr;
    // }
    // JobEntry * last_jop;
    // int maxid =  -1;
    // for (auto jop : jobs_list) {
    //     if (jop->jopId > maxid && jop->isStopped) {
    //         maxid = jop->jopId;
    //         last_jop = jop;
    //     }
    // }
    return nullptr;

}
void JobsList::printjopsListpid() {
    removeFinishedJobs();
    for(JobsList::JobEntry* jobEntry:jobs_list) {
        std::cout << jobEntry->pid << ": " << jobEntry->command << std::endl;
    }

}



void JobsCommand::execute() { //// todo : command jop number 6 in hw pdf
    if(jobsList == nullptr){
        return;
    }
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
        perror("smash error: waitpid failed"); /// todo : it dont want to fail but we keep it for test
    }

    // بعد ما تخلص نحط -1 لأنو ما في عملية بالـ foreground
    SmallShell::getInstance().change_current_jop_pid_front(-1);
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
            jobs->removeFinishedJobs(); /// todo : this line was missing
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
        std::string signal_str = commands_parts[1];
        // Check if signal starts with '-'
        if (signal_str.empty() || signal_str[0] != '-') {
            std::cerr << "smash error: kill: invalid arguments" << std::endl;
            return;
        }

        try {
            // signum = stoi(commands_parts[1]);
            signum = std::stoi(signal_str.substr(1));


            jopid = stoi(commands_parts[2]);
        }catch (...) {
            cerr << "smash error: kill: invalid arguments" << std::endl;
            return;
        }
        if (signum < 1 || signum > 31) {
            perror("smash error: kill failed");
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

// void AliasCommand::execute() {
//     // SmallShell& smash = SmallShell::getInstance();
//     std::vector<std::string> alias_vector = SmallShell::getInstance().getAliases_ord();
//     std::map<std::string, std::string> temp = SmallShell::getInstance().getAliases();
//     std::vector<std::string> command_vector = SmallShell::getInstance().getcommand_vector();
//
//
//     if (commands_parts.size() == 1) {
//
//
//         for (auto ali : alias_vector) {
//             std::cout << ali << "='" << temp[ali] << "'" << std::endl;
//         }
//         return;
//
//
//     }
//     std::string str_full ="";
//     for (auto str : commands_parts) {
//             str_full += str + " ";
//         }
//         if (!str_full.empty()) {
//
//             str_full = str_full.substr(0, str_full.size() - 1);
//
//         }
//
//     int first = -1;
//     int last = -1;
//
//     std::regex expression("^alias [a-zA-Z0-9_]+='[^']*'$");
//     if(!std::regex_match(str_full,expression))
//     {
//         std::cerr << "smash error: alias: invalid alias format"  << std::endl;
//         return;
//     }
//     std::string temp2 = str_full.substr(commands_parts[0].size()+1,str_full.size()-commands_parts[0].size()-1);
//     int size = temp2.size();
//     for(int i=0;i<size;i++)
//     {
//         if(temp2[i]=='\'')
//         {
//             if(first==-1)
//             {
//                 first=i;
//             }
//             else
//             {
//                 if(last==-1)
//                 {
//                     last=i;
//                 }
//                 else//more than two, not good
//                 {
//                     std::cerr << "smash error: alias: invalid alias format"  << std::endl;
//                     return;
//                 }
//             }
//         }
//     }
//     std::string alias=temp2.substr(0,first-1);
//     std::string command=temp2.substr(first+1,last-first-1);
//     if(temp.count(alias) > 0){
//         std::cerr <<"smash error: alias: "<< alias <<" already exists or is a reserved command"  << std::endl;
//         return;
//     }
//     // if(SmallShell::getInstance().validCommand(alias))
//     // {
//     //     std::cerr <<"smash error: alias: "<< alias <<" already exists or is a reserved command"  << std::endl;
//     //     return;
//     // }
//     SmallShell::getInstance().addAlias(alias,command);
// }

//
// void AliasCommand::execute() {
//     SmallShell& smash = SmallShell::getInstance();
//     std::vector<std::string> alias_vector = smash.getAliases_ord();
//     std::map<std::string, std::string> temp = smash.getAliases();
//     std::vector<std::string> command_vector = smash.getcommand_vector();
//
//
//
//     if (commands_parts.size() == 1) {
//
//
//         for (auto ali : alias_vector) {
//             std::cout << ali << "='" << temp[ali] << "'" << std::endl;
//         }
//
//
//     }else {
//         if (!isValidAlias(cmd_line)) {
//             cerr << "smash error: alias: invalid alias format" << std::endl;
//             return;
//         }else if (commands_parts.size() == 2) {
//
//             std::string  name;
//             std::string command;
//             bool reserved = false;
//             std::string cmd_str(cmd_line);
//
//
//             size_t first_quote = cmd_str.find('\'');
//             size_t second_quote = cmd_str.find('\'', first_quote + 1);
//             command = cmd_str.substr(first_quote + 1, second_quote - first_quote - 1);
//             size_t  equal_pos = cmd_str.find('=');
//             name = cmd_str.substr(6, equal_pos - 6);
//             for (auto cam : command_vector) {
//                 if (cam == name) {
//                     reserved = true;
//                 }
//             }
//
//
//             if (temp.count(name) || reserved) {
//
//                 cerr << "smash error: alias: " << name << " already exists or is a reserved command" << endl;
//
//             }
//
//
//             SmallShell::getInstance().addAlias(name , command );
//
//
//         }
//     }
// }
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
        }else {

            std::string  name;
            std::string command;
            bool reserved = false;
            std::string cmd_str(cmd_line);  // Convert to std::string

            size_t first_quote = cmd_str.find('\'');
            size_t second_quote = cmd_str.find('\'', first_quote + 1);
            command = cmd_str.substr(first_quote + 1, second_quote - first_quote - 1);
            size_t equal_pos = cmd_str.find('=');
            name = cmd_str.substr(6, equal_pos - 6);
            for (auto cam : command_vector) {
                if (cam == name){
                    reserved = true;
                }
            }
            if (temp.count(name) || reserved) {

                cerr << "smash error: alias: " << name << " already exists or is a reserved command" << endl;
                return;

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
        for (size_t i = 1 ; i < commands_parts.size() ; ++i){
            if(!temp.count(commands_parts[i])) {
            std::cerr << "smash error: unalias: "<<commands_parts[i] <<" alias does not exist" << std::endl;
                return;
            }
            // for (size_t j = 1 ; j < commands_parts.size() ; ++j) { /// todo: we need to check back
                SmallShell::getInstance().removeAlias(commands_parts[i]);



        }
    }
}




extern char **environ;
bool env_var_exists(const std::string& varname) {
    int fd = open("/proc/self/environ", O_RDONLY);
    if (fd == -1) {
        perror("smash error: open /proc/self/environ failed");
        return false;
    }

    char buffer[8192]; // Buffer to hold the environment variables
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        perror("smash error: read /proc/self/environ failed");
        close(fd);
        return false;
    }
    close(fd);

    buffer[bytes_read] = '\0'; // Null-terminate the buffer

    std::string target = varname + "="; // Variable name followed by "="
    size_t pos = 0;
    while (pos < (size_t)bytes_read) {
        std::string entry = std::string(&buffer[pos]);
        if (entry.compare(0, target.size(), target) == 0) {
            return true; // Found the variable
        }
        pos += entry.size() + 1; // Move to the next variable
    }

    return false; // Variable does not exist
}

bool remove_env_var(const std::string& varname) {
    size_t len = varname.length();
    int i = 0;

    while (environ[i]) {
        if (strncmp(environ[i], varname.c_str(), len) == 0 && environ[i][len] == '=') {
            // Shift left: remove environ[i] by replacing it with environ[i+1], etc.
            for (int j = i; environ[j]; ++j) {
                environ[j] = environ[j + 1];
            }
            return true; // Variable removed
        }
        ++i;
    }

    return false; // Variable not found
}

void UnSetEnvCommand::execute() {
    if (commands_parts.size() <= 1) {
        std::cerr << "smash error: unsetenv: not enough arguments" << std::endl;
        return;
    }

    for (size_t i = 1; i < commands_parts.size(); ++i) {
        const std::string& var = commands_parts[i];

        // Check if the environment variable exists using /proc/self/environ
        if (!env_var_exists(var)) {
            std::cerr << "smash error: unsetenv: " << var << " does not exist" << std::endl;
            return;  // If the variable doesn't exist, exit the function
        }

        // Remove the variable from environ
        bool removed = remove_env_var(var);
        if (!removed) {
            std::cerr << "smash error: unsetenv: " << var << " does not exist" << std::endl;
        }
    }
}



bool pidExists(int pid) {
    std::string path = "/proc/" + std::to_string(pid) + "/stat";
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) return false;
    close(fd);
    return true;
}

void WatchProcCommand::execute() {
    if(commands_parts.size() != 2){
        cerr << "smash error: watchproc: invalid arguments" << endl;
        return;
    }else{
        int pid;
        try {
            pid = stoi(commands_parts[1]);
        }catch(...){
            cerr << "smash error: watchproc: invalid arguments" << endl;
            return;
        }
        if (!pidExists(pid)) {
            std::cerr << "smash error: watchproc: pid "<< pid <<" does not exist" << std::endl;
            return;
        }
        std::string stat_path = "/proc/" + to_string(pid) + "/stat";
        std::string status_path = "/proc/" + to_string(pid) + "/status";
        int fd1 = open(stat_path.c_str() , O_RDONLY);
        int fd2 = open(status_path.c_str() , O_RDONLY);
        char buff1[4096];
        char buff2[4096];
        if(fd1 < 0 || fd2 < 0){
            perror("smash error: open failed");
            return;
        }
        ssize_t n1 = read(fd1, buff1, sizeof(buff1) - 1);
        ssize_t n2 = read(fd2, buff2, sizeof(buff2) - 1);
        if(n1 < 0 || n2 < 0){
            perror("smash error: read failed");
            close(fd1);
            close(fd2);
            return;
        }
        long utime, stime, starttime;
        std::istringstream stat_stream(buff1);
        std::vector<std::string> stat_fields;
        std::string temp;
        while (stat_stream >> temp) {
            stat_fields.push_back(temp);
        }

        utime = std::stol(stat_fields[13]);
        stime = std::stol(stat_fields[14]);
        starttime = std::stol(stat_fields[21]);

        // Read uptime
        double uptime = 0.0;
        int fd = open("/proc/uptime", O_RDONLY);
        if (fd < 0) {
            perror("smash error: open failed");
            return;
        }
        char u[128];
        ssize_t n = read(fd, u, 127);
        if (n < 0) {
            perror("smash error: read failed");
            close(fd);
            return;
        }
        u[n] = 0;
        close(fd);
        std::istringstream(u) >> uptime;

        // Calculate CPU usage (delta method)
        long ticks = sysconf(_SC_CLK_TCK);
        double total_process_time = (utime + stime) / (double)ticks;
        double seconds = uptime - (starttime / (double)ticks);
        double cpu_usage = (100.0 * (total_process_time / seconds));

        // Parse VmRSS for memory usage
        double memMB = 0;
        for (char* p = strtok(buff2, "\n"); p; p = strtok(nullptr, "\n")) {
            if (strncmp(p, "VmRSS:", 6) == 0) {
                memMB = std::atoi(p + 6) / 1024.0; // Convert to MB
            }
        }

        // Output the result
        std::cout << "PID: " << pid << " | CPU Usage: " << std::fixed << std::setprecision(1) << cpu_usage
                  << "% | Memory Usage: " << std::fixed << std::setprecision(1) << memMB << " MB" << std::endl;
    }
}



bool complexCommand(const char* cmd_line)
{
    std::string temp=string(cmd_line);
    if(temp.find('*')!=std::string::npos || temp.find('?')!=std::string::npos)
    {
        return true;
    }
    return false;
}








void ExternalCommand::execute() {




    pid_t pid = fork();

    if (pid < 0) {
        perror("smash error: fork faild");
        return;
    }

    if (pid == 0) {
        setpgrp();


        if (complexCommand(cmd_line)) {
            /// todo : its a complex command we need to run it in bash
            // If redirection is needed
            std::string bash_cmd = "/bin/bash";
            std::string c_flage = "-c";
            // execlp(bash_cmd.c_str(), bash_cmd.c_str(), c_flage.c_str(), cmd_line, nullptr);
            execl(bash_cmd.c_str(), bash_cmd.c_str(), c_flage.c_str(), cmd_line, nullptr);

        }else  {
            /// todo :: simple command

            char* args[COMMAND_MAX_ARGS +1];
            int i = 0;
            for (const auto &part : commands_parts) {
                args[i++] = const_cast<char*>(part.c_str());

            }
            args[i] = nullptr;

            execvp(args[0], args);

        }

        perror("smash error: execvp failed");
        exit(1);


    }else { /// todo : the parent
        if (!isbackground) { /// todo he not a baground jop so wee need to wait for him to finish
            SmallShell::getInstance().change_current_jop_pid_front(pid);
            pid_t result = waitpid(pid,NULL,0);
             if (result == -1) {
                 perror("smash error: waitpid failed");
                 return;
             }
            SmallShell::getInstance().change_current_jop_pid_front(-1); /// no jop in the front
            }else {//// we need to add the jop to bacround
                std::string cmd_without_bg;
                for (size_t i = 0; i < commands_parts.size(); ++i) {
                    cmd_without_bg += commands_parts[i];
                    if (i < commands_parts.size() - 1) {
                        cmd_without_bg += " ";
                    }
                }


                SmallShell::getInstance().getJobsList()->addJob(pid, getHisAlias(), false);



        }
    }

}


void RedirectionCommand::execute() {
    if (actual_command.empty() || output_file.empty()) {
        std::cerr << "redirection error: command not found" << std::endl;
        return;
    }
    int stdout_copy = dup(STDOUT_FILENO);
    if (stdout_copy == SYS_FAIL) {
        perror("smash error: dup failed");
        return;
    }
    int fd  = open(output_file.c_str(),O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC),0644);

    if (fd == SYS_FAIL) {
        perror("smash error: open failed");
        return;
    }
    std::vector<std::string> vec = SmallShell::getInstance().getBuiltInCommandsVector();
    if (std::find(vec.begin() , vec.end() , actual_command[0]) != vec.end()) {
        if (dup2(fd, STDOUT_FILENO) == SYS_FAIL) {
            perror("smash error: dup2 failed");
            close(fd);
            close(stdout_copy);
            return;
        }
        if (close(fd) == SYS_FAIL) {
            perror("smash error: close failed");
            exit(1);
        }

        // Execute in current shell
        std::string combined;
        for (const auto& word : actual_command) {
            combined += word + " ";
        }
        combined.pop_back();
        char* Args = const_cast<char*>(combined.c_str());
        SmallShell::getInstance().executeCommand(Args);


        if (dup2(stdout_copy, STDOUT_FILENO) == SYS_FAIL) {
            perror("smash error: dup2 failed");
            return;
        }
        if (close(stdout_copy) == SYS_FAIL) {
            perror("smash error: close failed");
            return;
        }
    return;
    }
    pid_t pid = fork();
    if (pid == SYS_FAIL) {
        perror("smash error: fork failed");
        if (close(fd) == SYS_FAIL) {
            perror("smash error: close failed");
        }
        return;
    }
    if (pid == 0) { /// todo : child

        if (setpgrp() == SYS_FAIL) {
            perror("smash error: setpgrp failed");
            if (close(fd) == SYS_FAIL) {
                perror("smash error: close failed");
            }
            exit(1);
        }

        if (dup2(fd, STDOUT_FILENO) == SYS_FAIL) {
            perror("smash error: dup2 failed");
            if (close(fd) == SYS_FAIL) {
                perror("smash error: close failed");
            }
            exit(1);
        }
        if (close(fd) == SYS_FAIL) {
            perror("smash error: close failed");
            exit(1);
        }

        std::string combined;
        for (const auto& word : actual_command) {
            combined += word + " ";
        }
        combined.pop_back(); // remove trailing space
        char* Args = const_cast<char*>(combined.c_str());
        SmallShell::getInstance().executeCommand(Args);
        exit(0);
    } else { /// todo : parent

        if (close(fd) == SYS_FAIL) {
            perror("smash error: close failed");
            return;
        }
        if (waitpid(pid,nullptr,0) == SYS_FAIL) {
            perror("smash error: waitpid failed");
            return;
        }

    }
}

void PipeCommand::execute() {
    if (leftCommand.empty() || rightCommand.empty()) {
        std::cerr << "pipe error: command not found" << std::endl;
        return;
    }

    int fd[2];
    pipe(fd);
    pid_t pid1 = fork(), pid2;
    if (pid1 == SYS_FAIL) {
        perror("smash error: fork failed");
        if (close(fd[0]) == SYS_FAIL) {
            perror("smash error: close failed");
        }
        if (close(fd[1]) == SYS_FAIL) {
            perror("smash error: close failed");
        }
        return;
    }
    if (pid1 == 0) { //first son
        if (setpgrp() == SYS_FAIL) {
            perror("smash error: setpgrp failed");
            if (close(fd[0]) == SYS_FAIL) {
                perror("smash error: close failed");
            }
            if (close(fd[1]) == SYS_FAIL) {
                perror("smash error: close failed");
            }
            return;
        }
        if (!errorPipe) {
            if (dup2(fd[1], STDOUT_FILENO) == SYS_FAIL) {
                perror("smash error: dup2 failed");
                if (close(fd[0]) == SYS_FAIL) {
                    perror("smash error: close failed");
                }
                if (close(fd[1]) == SYS_FAIL) {
                    perror("smash error: close failed");
                }
                return;
            }
        } else {
            if (dup2(fd[1], STDERR_FILENO) == SYS_FAIL) {
                perror("smash error: dup2 failed");
                if (close(fd[0]) == SYS_FAIL) {
                    perror("smash error: close failed");
                }
                if (close(fd[1]) == SYS_FAIL) {
                    perror("smash error: close failed");
                }
                return;
            }
        }
        if (close(fd[0]) == SYS_FAIL) {
            perror("smash error: close failed");
        }
        if (close(fd[1]) == SYS_FAIL) {
            perror("smash error: close failed");
        }
        std::string combined;
        // now we execute the command that will give output to the pipe
        for (const auto& word : leftCommand) {
            combined += word + " ";
        }
        combined.pop_back(); // remove trailing space
        char* leftArgs = const_cast<char*>(combined.c_str());
        SmallShell::getInstance().executeCommand(leftArgs);
        exit(0);
    }
    pid2 = fork();
    if (pid2 == SYS_FAIL) {
        perror("smash error: fork failed");
        if (close(fd[0]) == SYS_FAIL) {
            perror("smash error: close failed");
        }
        if (close(fd[1]) == SYS_FAIL) {
            perror("smash error: close failed");
        }
        return;
    }
    if (pid2 == 0) { //second son
        if (setpgrp() == SYS_FAIL) {
            perror("smash error: setpgrp failed");
            if (close(fd[0]) == SYS_FAIL) {
                perror("smash error: close failed");
            }
            if (close(fd[1]) == SYS_FAIL) {
                perror("smash error: close failed");
            }
            return;
        }
        if (dup2(fd[0], 0) == SYS_FAIL) {
            perror("smash error: dup2 failed");
            if (close(fd[0]) == SYS_FAIL) {
                perror("smash error: close failed");
            }
            if (close(fd[1]) == SYS_FAIL) {
                perror("smash error: close failed");
            }
            return;
        }
        if (close(fd[0]) == SYS_FAIL) {
            perror("smash error: close failed");
        }
        if (close(fd[1]) == SYS_FAIL) {
            perror("smash error: close failed");
        }
        std::string combined;
        // now we execute the command that will get input from the pipe
        for (const auto& word : rightCommand) {
            combined += word + " ";
        }
        combined.pop_back(); // remove trailing space
        char* rightArgs = const_cast<char*>(combined.c_str());
        SmallShell::getInstance().executeCommand(rightArgs);
        exit(0);
    }
    if (close(fd[0]) == SYS_FAIL) {
        perror("smash error: close failed");
    }
    if (close(fd[1]) == SYS_FAIL) {
        perror("smash error: close failed");
    }
    if (waitpid(pid1,nullptr, WUNTRACED) == SYS_FAIL) {
        perror("smash error: waitpid failed");
        return;
    }
    if (waitpid(pid2,nullptr, WUNTRACED) == SYS_FAIL) {
        perror("smash error: waitpid failed");
        return;
    }
}

// struct linux_dirent64 {
//     uint64_t        d_ino;
//     int64_t         d_off;
//     unsigned short  d_reclen;
//     unsigned char   d_type;
//     char            d_name[];
// };
//
// bool traverse(const std::string& path, size_t& total_kb) {
//     int fd = open(path.c_str(), O_RDONLY
struct linux_dirent64 {
    ino64_t        d_ino;
    off64_t        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

// Round up st_blocks (512-byte blocks) to KB (1024 bytes)
// inline size_t blocksToKB(blksize_t blocks) {
//     // Standard block size is 512 bytes for st_blocks
//     return (blocks * 512 + 1023) / 1024; // Round up to nearest KB
// }
//
// bool duHelper(const std::string& path, size_t& total_kb) {
//     int fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
//     if (fd == -1) {
//         std::cerr << "smash error: du: directory " << path << " does not exist" << std::endl;
//         return false;
//     }
//
//     char buf[4096];
//     while (true) {
//         int nread = syscall(SYS_getdents64, fd, buf, sizeof(buf));
//         if (nread == -1) {
//             perror("smash error: getdents64 failed");
//             close(fd);
//             return false;
//         }
//         if (nread == 0) break;
//
//         for (int bpos = 0; bpos < nread;) {
//             struct linux_dirent64* d = (struct linux_dirent64*)(buf + bpos);
//             std::string name(d->d_name);
//             if (name == "." || name == "..") {
//                 bpos += d->d_reclen;
//                 continue;
//             }
//
//             std::string fullpath = path + "/" + name;
//             struct stat st;
//             if (lstat(fullpath.c_str(), &st) == -1) {
//                 perror(("smash error: lstat failed on " + fullpath).c_str());
//                 bpos += d->d_reclen;
//                 continue;
//             }
//
//             // Skip symlinks
//             if (S_ISLNK(st.st_mode)) {
//                 bpos += d->d_reclen;
//                 continue;
//             }
//
//             // Recurse into subdirectories
//             if (S_ISDIR(st.st_mode)) {
//                 duHelper(fullpath, total_kb);
//             }
//
//             total_kb += blocksToKB(st.st_blocks);
//             bpos += d->d_reclen;
//         }
//     }
//
//     close(fd);
//     return true;
// }
//
// void DiskUsageCommand::execute() {
//     if (commands_parts.size() > 2) {
//         std::cerr << "smash error: du: too many arguments" << std::endl;
//         return;
//     }
//
//     std::string path = commands_parts.size() == 1 ? SmallShell::getInstance().getPwd() : commands_parts[1];
//
//     struct stat st;
//     if (lstat(path.c_str(), &st) == -1 || !S_ISDIR(st.st_mode)) {
//         std::cerr << "smash error: du: directory " << path << " does not exist" << std::endl;
//         return;
//     }
//
//     size_t total_kb = 0;
//     if (duHelper(path, total_kb)) {
//         std::cout << "Total disk usage: " << total_kb << " KB" << std::endl;
//     }
// }
inline size_t blocksToKB(blksize_t blocks) {
    return (blocks * 512 + 1023) / 1024;
}

bool duHelper(const std::string& path, size_t& total_kb) {
    int fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        struct stat st;
        if (lstat(path.c_str(), &st) == -1) {
            return false;
        }
        total_kb += blocksToKB(st.st_blocks);
        return true;
    }

    struct stat dir_stat;
    if (lstat(path.c_str(), &dir_stat) == -1) {
        close(fd);
        return false;
    }
    total_kb += blocksToKB(dir_stat.st_blocks);

    char buf[8192];
    int nread;

    while ((nread = syscall(SYS_getdents64, fd, buf, sizeof(buf))) > 0) {
        for (long pos = 0; pos < nread;) {
            struct linux_dirent64* d = (struct linux_dirent64*)(buf + pos);
            std::string name = d->d_name;

            if (name != "." && name != "..") {
                std::string full_path = path;
                if (path.back() != '/') {
                    full_path += '/';
                }
                full_path += name;

                struct stat st;
                if (lstat(full_path.c_str(), &st) == -1) {
                    close(fd);
                    return false;
                }

                if (S_ISDIR(st.st_mode)) {
                    if (!duHelper(full_path, total_kb)) {
                        close(fd);
                        return false;
                    }
                } else {
                    total_kb += blocksToKB(st.st_blocks);
                }
            }
            pos += d->d_reclen;
        }
    }

    close(fd);
    return nread >= 0;
}

// void DiskUsageCommand::execute() {
//     std::string path = ".";
//
//     char* args[COMMAND_MAX_ARGS];
//     int num_args = _parseCommandLine(cmd_line, args);
//     if (num_args > 1) {
//         path = args[1];
//     }
//
//     size_t total_kb = 0;
//     if (duHelper(path, total_kb)) {
//         std::cout << "Total disk usage: " << total_kb << " KB" << std::endl;
//     } else {
//         std::cerr << "Error calculating disk usage" << std::endl;
//     }
//
//     for (int i = 0; i < num_args; i++) {
//         free(args[i]);
//     }
// }
void DiskUsageCommand::execute() {
    // Check for too many arguments
    if (commands_parts.size() > 2) {
        std::cerr << "smash error: du: too many arguments" << std::endl;
        return;
    }

    // Get the path (default is "." if no argument provided)
    std::string path = (commands_parts.size() > 1) ? commands_parts[1] : ".";

    // Check if directory exists
    struct stat st;
    if (stat(path.c_str(), &st) == -1) {
        std::cerr << "smash error: du: directory " << path << " does not exist" << std::endl;
        return;
    }

    // Calculate disk usage
    size_t total_kb = 0;
    if (duHelper(path, total_kb)) {
        std::cout << "Total disk usage: " << total_kb << " KB" << std::endl;
    }
    // No else branch - we don't print any error message for calculation failures
}

void WhoAmICommand::execute(){
    uid_t uid = getuid();  // current user ID

    int fd = open("/etc/passwd", O_RDONLY);
    if (fd == SYS_FAIL) {
        perror("smash error: open failed");
        return;
    }

    char buffer[4096];
    std::string leftover;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, 4096)) > 0) {
        leftover += std::string(buffer, bytes_read);

        size_t pos;
        while ((pos = leftover.find('\n')) != std::string::npos) {
            std::string line = leftover.substr(0, pos);
            leftover = leftover.substr(pos + 1);

            // Split by ':'
            std::vector<std::string> fields;
            std::stringstream ss(line);
            std::string token;
            while (std::getline(ss, token, ':')) {
                fields.push_back(token);
            }

            if (fields.size() < 6) continue;

            // Compare UID
            try {
                if (std::stoi(fields[2]) == static_cast<int>(uid)) {
                    std::cout << fields[0] << " " << fields[5] << std::endl;
                    if(close(fd) == SYS_FAIL){
                        perror("smash error: close failed");
                    }
                    return;
                }
            } catch (...) {
                continue;
            }
        }
    }

    if (bytes_read == SYS_FAIL) {
        perror("smash error: read failed");
    }

    if(close(fd) == SYS_FAIL){
        perror("smash error: close failed");
    }
}
bool interfaceExists(const std::string& iface) {
    int fd = open("/proc/net/dev", O_RDONLY);
    if (fd < 0) return false;

    char buf[4096] = {0};
    ssize_t bytes = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (bytes <= 0) return false;

    std::istringstream iss(buf);
    std::string line;
    int line_count = 0;
    while (std::getline(iss, line)) {
        if (++line_count <= 2) continue; // skip headers
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string name = line.substr(0, colon);
        name.erase(0, name.find_first_not_of(" \t")); // strip leading spaces
        if (name == iface) return true;
    }
    return false;
}

std::string hexToIp(const std::string& hexStr) {
    unsigned int ip;
    sscanf(hexStr.c_str(), "%x", &ip);
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    std::ostringstream oss;
    oss << (int)bytes[0] << "." << (int)bytes[1] << "."
        << (int)bytes[2] << "." << (int)bytes[3];
    return oss.str();
}

bool ipInSubnet(const std::string& ip, const std::string& subnet) {
    // Compare first 3 octets (assumes /24)
    size_t a = subnet.find('.');
    size_t b = subnet.find('.', a + 1);
    size_t c = subnet.find('.', b + 1);
    std::string subnetPrefix = subnet.substr(0, c);
    return ip.substr(0, c) == subnetPrefix;
}


std::string getIPAddress(const std::string& iface) {
    std::string iface_subnet_hex;
    std::string iface_mask_hex;

    // Step 1: Read /proc/net/route
    int fd_route = open("/proc/net/route", O_RDONLY);
    if (fd_route < 0) return "";
    char route_buf[4096] = {0};
    ssize_t route_bytes = read(fd_route, route_buf, sizeof(route_buf) - 1);
    close(fd_route);
    if (route_bytes <= 0) return "";

    std::istringstream route_iss(route_buf);
    std::string line;
    std::getline(route_iss, line); // skip header

    while (std::getline(route_iss, line)) {
        std::istringstream ls(line);
        std::string dev, dest, gateway, flags, refcnt, use, metric, mask;
        ls >> dev >> dest >> gateway >> flags >> refcnt >> use >> metric >> mask;

        if (dev == iface && dest != "00000000") {
            iface_subnet_hex = dest;
            iface_mask_hex = mask;
            break;
        }
    }

    std::string iface_subnet = hexToIp(iface_subnet_hex);

    // Step 2: Read /proc/net/fib_trie
    int fd = open("/proc/net/fib_trie", O_RDONLY);
    if (fd < 0) return "";

    char buf[8192] = {0};
    ssize_t bytes = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (bytes <= 0) return "";

    std::istringstream iss(buf);
    std::string lastIp;

    while (std::getline(iss, line)) {
        std::string trimmed = line.substr(line.find_first_not_of(" \t|+-"));

        // Detect IP lines
        int dots = std::count(trimmed.begin(), trimmed.end(), '.');
        if (dots == 3 && trimmed.find('/') == std::string::npos) {
            lastIp = trimmed;
        }

        if (trimmed.find("/32 host LOCAL") != std::string::npos) {
            if (iface == "lo" && lastIp.find("127.") == 0) {
                return lastIp;
            }
            if (!iface_subnet.empty() && ipInSubnet(lastIp, iface_subnet)) {
                return lastIp;
            }
        }
    }

    return "";
}





std::pair<std::string, std::string> getMaskAndGateway(const std::string& iface) {
    int fd = open("/proc/net/route", O_RDONLY);
    if (fd < 0) return {"", ""};

    char buf[8192] = {0};
    ssize_t bytes = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (bytes <= 0) return {"", ""};

    std::istringstream iss(buf);
    std::string line;
    std::getline(iss, line); // skip header

    std::string mask, gateway;
    while (std::getline(iss, line)) {
        std::istringstream line_stream(line);
        std::string ifaceName, destination, gw, flags, refCnt, use, metric, maskHex;
        line_stream >> ifaceName >> destination >> gw >> flags >> refCnt >> use >> metric >> maskHex;

        if (ifaceName != iface) continue;

        if (destination == "00000000" && gateway.empty()) {
            gateway = hexToIp(gw);
        } else if (mask.empty()) {
            mask = hexToIp(maskHex);
        }
    }
    return {mask, gateway};
}


std::string getDNSServers() {
    int fd = open("/etc/resolv.conf", O_RDONLY);
    if (fd < 0) return "";

    char buf[2048] = {0};
    ssize_t bytes = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (bytes <= 0) return "";

    std::istringstream iss(buf);
    std::string line;
    std::vector<std::string> servers;
    while (std::getline(iss, line)) {
        if (line.find("nameserver") == 0) {
            std::istringstream linestream(line);
            std::string token;
            linestream >> token >> token;
            servers.push_back(token);
        }
    }
    std::ostringstream result;
    for (size_t i = 0; i < servers.size(); ++i) {
        if (i > 0) result << ", ";
        result << servers[i];
    }
    return result.str();
}


void NetInfo::execute(){
if(commands_parts.size() < 2){
    cerr << "smash error: netinfo: interface not specified" << endl;
    return;
}
if (!interfaceExists(commands_parts[1])) {
    std::cerr << "smash error: netinfo: interface " << commands_parts[1] << " does not exist" << std::endl;
    return;
}


std::string ip = getIPAddress(commands_parts[1]);
auto [subnetMask, gateway] = getMaskAndGateway(commands_parts[1]);
std::string dns = getDNSServers();

cout << "IP Address: "<< ip << endl
<< "Subnet Mask: " << subnetMask << endl
<< "Default Gateway: " << gateway << endl
<< "DNS Servers: " << dns << endl;

}


SmallShell::SmallShell() : prompt("smash"),pid(getPid()),pwd(getPwd()),lastpwd(""),commands_vector(),
aliases(),aliases_inorder(),current_jop_pid_front(-1) {
// TODO: add your implementation
    creatCommand_vector();
    shell_jops = new JobsList();
   builtInCommands_vector =  {"chprompt" , "pwd" , "showpid" , "cd" ,"jobs" , "fg" , "quit" ,
         "alias" , "unalias" , "unsetenv" , "watchproc" ,"du" ,  "whoami","netinfo"};
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
char** SmallShell::getLastPwdPtr() {
    static char* pwd_buffer = nullptr;
    // Clean up old allocation if exists
    delete[] pwd_buffer;
    // Allocate new buffer and copy the string
    pwd_buffer = new char[lastpwd.length() + 1];
    strcpy(pwd_buffer, lastpwd.c_str());
    // Return address of the static pointer
    return &pwd_buffer;
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
    // if (std::find(aliases_inorder.begin(), aliases_inorder.end(), alias) == aliases_inorder.end()) {
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

std::map<int, JobsList::JobEntry *> SmallShell::getPidMap() {
    return pid_map;
}




void SmallShell::creatCommand_vector(){
    commands_vector = { "chprompt" , "pwd" , "showpid" , "cd" ,"jobs" , "fg" , "quit" ,
         "alias" , "unalias" , "unsetenv" , "watchproc" ,"du","whoami", "netinfo"};
}

std::vector<std::string> SmallShell::getcommand_vector() {
    return commands_vector;
}

void SmallShell::change_current_jop_pid_front(int pid) {
    current_jop_pid_front = pid;
}

int SmallShell::get_current_jop_pid_front() {
    return current_jop_pid_front;
}


std::vector<std::string> SmallShell::getBuiltInCommandsVector() {
    return builtInCommands_vector;
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

    SmallShell &smash = SmallShell::getInstance();


    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if (cmd_s.find(">") != std::string::npos) {
        return new RedirectionCommand(cmd_line);
    }else if (cmd_s.find("|") != std::string::npos) {
        return new PipeCommand(cmd_line);
    }else if (firstWord.compare("chprompt") == 0) {
        return new ChpromtCommand(cmd_line);
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line,SmallShell::getInstance().getLastPwdPtr());
    }else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line,SmallShell::getInstance().getJobsList());
    } else if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(cmd_line,smash.getJobsList());
    }else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line,SmallShell::getInstance().getJobsList());
    }else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line,SmallShell::getInstance().getJobsList());
    }else if (firstWord.compare("alias") == 0) {
        return new AliasCommand(cmd_line);
    }else if (firstWord.compare("unalias") == 0) {
        return new UnAliasCommand(cmd_line);
    }else if (firstWord.compare("unsetenv") == 0) {
        return new UnSetEnvCommand(cmd_line);

    }else if (firstWord.compare("watchproc") == 0) {
        return new WatchProcCommand(cmd_line);





    }else if (firstWord.compare("du") == 0) {
        return new DiskUsageCommand(cmd_line);

    }else if (firstWord.compare("whoami") == 0) {
        return new WhoAmICommand(cmd_line);
    }else if (firstWord.compare("netinfo") == 0){
        return new  NetInfo(cmd_line);



    }else {

        return new ExternalCommand(cmd_line);
    }


    return nullptr;
}


void SmallShell::executeCommand(const char *cmd_line) {
    if (cmd_line == nullptr || strlen(cmd_line) == 0) {
        return;
    }
    std::string orig = cmd_line;
    std::string cmd_str(cmd_line);
    std::istringstream iss(cmd_str);
    std::string first_word;
    iss >> first_word;


    /// look for alias
    auto alias_it = aliases.find(first_word);
    bool is_alias = false;
    std::string original_alias = "";

    if (alias_it != aliases.end()) {
        // Save the original alias command before modifying anything
        std::string original_alias = first_word;
        bool is_alias = true;





        // Find position after first word including any following spaces
        size_t cmd_start = cmd_str.find_first_not_of(" ", first_word.length());
        std::string rest_of_cmd;

        if (cmd_start != std::string::npos) {
            // If there are arguments, get them
            rest_of_cmd = cmd_str.substr(cmd_start);
        }

        // Combine alias command with original arguments
        std::string new_cmd = alias_it->second + " " + rest_of_cmd;
        cmd_str = new_cmd;
        cmd_line = cmd_str.c_str();


    }


    Command* cmd = CreateCommand(cmd_line);
    if (cmd != nullptr) {
        cmd->setHisAlias(orig);

        try {
            cmd->execute();
        }catch (...) {
            perror("smash error: execute failed");

        }
        delete cmd;

    }



    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}
