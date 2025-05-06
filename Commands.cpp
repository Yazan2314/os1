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

        if (commands_parts.size() == 1) {

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
            std::cout << "[" << jobEntry->jopId << "] " << jobEntry->command << " " << jobEntry->pid <<std::endl;
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


void AliasCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    std::vector<std::string> alias_vector = smash.getAliases_ord();
    std::map<std::string, std::string> temp = smash.getAliases();
    std::vector<std::string> command_vector = smash.getcommand_vector();



    // std::vector<std::string> alias_vector = SmallShell::getInstance().getAliases_ord();
    // std::map<std::string, std::string> temp = SmallShell::getInstance().getAliases();
    // std::vector<std::string> command_vector = SmallShell::getInstance().getcommand_vector();
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
        for (size_t i = 1 ; i < commands_parts.size() ; ++i){
            if(!temp.count(commands_parts[i])) {
            std::cerr << "smash error: unalias: "<<commands_parts[i] <<" alias does not exist" << std::endl;
                return;
            }
            for (size_t j = 1 ; j < commands_parts.size() ; ++j) { /// todo: we need to check back
                SmallShell::getInstance().removeAlias(commands_parts[j]);
            }


        }
    }
}


//
// void UnSetEnvCommand::execute() {
//     if (commands_parts.size() < 2) {
//         std::cerr << "smash error: unsetenv: not enough arguments " << std::endl;
//         return;
//     }
//     for (int i = 1; i < commands_parts.size() ; ++i ) {
//         const char* var = commands_parts[i].c_str();
//         if (getenv(var) == nullptr) {
//             std::cerr << "smash error: unsetenv: " << var << " does not exist" << std::endl;
//             return;
//         }
//         if (unsetenv(var) != 0) {
//             perror("smash error: unsetenv");
//             return;
//         }
//     }
//
// }




// extern char **environ;

// void UnSetEnvCommand::execute() {
//     if (commands_parts.size() < 2) {
//         std::cerr << "smash error: unsetenv: not enough arguments" << std::endl;
//         return;
//     }
//     std::string path = "/proc/self/environ";
//     int fd = open(path.c_str(),O_RDONLY);
//     if (fd == -1) {
//         perror("smash error: open");
//         return;
//     }


//     // Use fstat to get the size of the file
//     struct stat st;
//     if (fstat(fd, &st) < 0) {
//         perror("smash error: fstat");
//         close(fd);
//         return;
//     }

//     size_t size = st.st_size;
//     char* buffer = (char*)malloc(size + 1);
//     if (!buffer) {
//         perror("smash error: malloc");
//         close(fd);
//         return;
//     }

//     ssize_t bytes_read = read(fd, buffer, size);
//     close(fd);

//     if (bytes_read < 0) {
//         perror("smash error: read");
//         free(buffer);
//         return;
//     }

//     buffer[bytes_read] = '\0';  // Ensure null-terminated

//     // For each variable given
//     for (size_t i = 1; i < commands_parts.size(); ++i) {
//         std::string var_name = commands_parts[i];
//         std::string prefix = var_name + "=";
//         bool found = false;

//         // Search in the environ buffer for the variable
//         char* p = buffer;
//         while (p < buffer + bytes_read) {
//             size_t len = strlen(p);
//             if (strncmp(p, prefix.c_str(), prefix.length()) == 0) {
//                 found = true;
//                 break;
//             }
//             p += len + 1;
//         }

//         if (!found) {
//             std::cerr << "smash error: unsetenv: " << var_name << " does not exist" << std::endl;
//             free(buffer);
//             return;
//         }

//         // Now manually remove it from the actual environ array
//         for (int j = 0; environ[j] != nullptr; ++j) {
//             if (strncmp(environ[j], prefix.c_str(), prefix.length()) == 0) {
//                 // Shift all remaining pointers left by one
//                 for (int k = j; environ[k] != nullptr; ++k) {
//                     environ[k] = environ[k + 1];
//                 }
//                 break;
//             }
//         }
//     }

//     free(buffer);
// }





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
//     buff1[n1] = '\0';
//     buff2[n2] = '\0';
//     close(fd1);
//     close(fd2);
//
//     // stat parsing (utime, stime, starttime)
//     std::istringstream s1(buff1);
//     std::vector<std::string> f(std::istream_iterator<std::string>{s1}, {});
//     long utime = stol(f[13]), stime = stol(f[14]), start = stol(f[21]);
//
// // uptime
//     double uptime = 0;
//     { int fd = open("/proc/uptime", O_RDONLY);
//         if(fd < 0) {
//             perror("smash error: open failed");
//             return;
//         }
//         char u[128]; ssize_t n = read(fd, u, 127);
//         if(n < 0){
//             perror("smash error: read failed");
//             close(fd);
//         }
//         u[n] = 0; close(fd); std::istringstream(u) >> uptime; }
//
// // CPU usage
//     long ticks = sysconf(_SC_CLK_TCK);
//     double total = (utime + stime) / (double)ticks;
//     double seconds = uptime - (start / (double)ticks);
//     double cpu = 100.0 * (total / seconds);
//
// // mem parsing (VmRSS)
//     double memMB = 0;
//     for (char* p = strtok(buff2, "\n"); p; p = strtok(nullptr, "\n"))
//         if (strncmp(p, "VmRSS:", 6) == 0){
//             memMB = atoi(p + 6) / 1024.0;
//         }
//
// // printing the result
// cout << "PID: "<< pid << " | CPU Usage: "<<  std::setprecision(1) << cpu
// <<"% | Memory Usage: "<<  std::setprecision(1) << memMB <<" MB" << endl;
//
// }
// }

















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
            execlp(bash_cmd.c_str(), bash_cmd.c_str(), c_flage.c_str(), cmd_line, nullptr);
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

                SmallShell::getInstance().getJobsList()->addJob(pid, m_line, false);




        }
    }
    SmallShell::getInstance().change_current_jop_pid_front(-1);
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

struct linux_dirent64 {
    uint64_t        d_ino;
    int64_t         d_off;
    unsigned short  d_reclen;
    unsigned char   d_type;
    char            d_name[];
};

bool traverse(const std::string& path, size_t& total_kb) {
    int fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        std::cerr << "smash error: du: directory " << path << " does not exist" << std::endl;
        return 0;
    }

    char buf[4096];
    while (true) {
        int nread = syscall(SYS_getdents64, fd, buf, sizeof(buf));
        if (nread == -1) {
            perror("smash error: getdents64 failed");
            close(fd);
            return 0;
        }
        if (nread == 0) break;

        for (int bpos = 0; bpos < nread;) {
            linux_dirent64* d = (linux_dirent64*)(buf + bpos);
            std::string name(d->d_name);
            if (name == "." || name == "..") {
                bpos += d->d_reclen;
                continue;
            }

            std::string fullpath = path + "/" + name;

            struct stat st;
            if (lstat(fullpath.c_str(), &st) == -1) {
                perror(("smash error: lstat failed on " + fullpath).c_str());
                bpos += d->d_reclen;
                continue;
            }

            if (S_ISLNK(st.st_mode)) {

            } else if (S_ISDIR(st.st_mode)) {
                traverse(fullpath, total_kb);
            }

            // Add size in kilobytes (512-byte blocks -> KB)
            total_kb += (st.st_blocks + 1) / 2;

            bpos += d->d_reclen;
        }
    }

    close(fd);
    return 1;
}


void DiskUsageCommand::execute(){
    if (commands_parts.size() > 2) {
        std::cerr << "smash error: du: too many arguments" << std::endl;
        return;
    }
    std::string path = commands_parts.size() == 1 ? SmallShell::getInstance().getPwd() : commands_parts[1] ;
    size_t total = 0;
if(traverse(path , total)){
std::cout << "Total disk usage: " << total << " KB" << std::endl;
}
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
void NetInfo::execute() {
    if (commands_parts.size() < 2) {
        std::cerr << "smash error: netinfo: interface not specified" << std::endl;
        return;
    }

    const std::string& iface = commands_parts[1];
    char buffer[4096];

    // -------- 1. Default Gateway and Subnet Mask from /proc/net/route --------
    std::string gateway = "N/A";
    std::string subnet = "N/A";
    int fd = open("/proc/net/route", O_RDONLY);
    if (fd == -1) {
        perror("smash error: open failed");
        return;
    }

    ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        perror("smash error: read failed");
        close(fd);
        return;
    }
    buffer[bytesRead] = '\0';
    close(fd);

    std::istringstream iss_route(buffer);
    std::string line;
    std::getline(iss_route, line);  // Skip header
    bool iface_found = false;

    while (std::getline(iss_route, line)) {
        std::istringstream iss(line);
        std::string iface_name, destination, gateway_hex, flags, refcnt, use, metric, mask;
        iss >> iface_name >> destination >> gateway_hex >> flags >> refcnt >> use >> metric >> mask;

        if (iface_name == iface) {
            iface_found = true;
            if (destination == "00000000") {
                // Default gateway
                unsigned int gw;
                sscanf(gateway_hex.c_str(), "%x", &gw);
                struct in_addr addr;
                addr.s_addr = gw;
                gateway = inet_ntoa(addr);

                unsigned int mask_ip;
                sscanf(mask.c_str(), "%x", &mask_ip);
                addr.s_addr = mask_ip;
                subnet = inet_ntoa(addr);
            }
        }
    }

    if (!iface_found) {
        std::cerr << "smash error: netinfo: interface " << iface << " does not exist" << std::endl;
        return;
    }

    // -------- 2. IP Address from /proc/net/fib_trie --------
    fd = open("/proc/net/fib_trie", O_RDONLY);
    if (fd == -1) {
        perror("smash error: open failed");
        return;
    }

    bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        perror("smash error: read failed");
        close(fd);
        return;
    }
    buffer[bytesRead] = '\0';
    close(fd);

    std::string ip = "N/A";
    std::istringstream iss_fib(buffer);
    bool found = false;

    while (std::getline(iss_fib, line)) {
        if (line.find("32 host LOCAL") != std::string::npos) {
            std::getline(iss_fib, line);  // Go to next line
            std::getline(iss_fib, line);  // IP address line
            std::istringstream ip_line(line);
            std::string token;
            while (ip_line >> token) {
                if (token.find('.') != std::string::npos) {
                    ip = token;
                    found = true;
                    break;
                }
            }
        }
        if (found) break;
    }

    // -------- 3. DNS from /etc/resolv.conf --------
    fd = open("/etc/resolv.conf", O_RDONLY);
    if (fd == -1) {
        perror("smash error: open failed");
        return;
    }

    bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        perror("smash error: read failed");
        close(fd);
        return;
    }
    buffer[bytesRead] = '\0';
    close(fd);

    std::istringstream iss_dns(buffer);
    std::string dns_line;
    std::vector<std::string> dns_servers;

    while (std::getline(iss_dns, dns_line)) {
        if (dns_line.find("nameserver") == 0) {
            std::istringstream dns_stream(dns_line);
            std::string label, address;
            dns_stream >> label >> address;
            dns_servers.push_back(address);
        }
    }

    // -------- Output --------
    std::cout << "IP Address: " << ip << std::endl;
    std::cout << "Subnet Mask: " << subnet << std::endl;
    std::cout << "Default Gateway: " << gateway << std::endl;
    std::cout << "DNS Servers: ";
    for (size_t i = 0; i < dns_servers.size(); ++i) {
        std::cout << dns_servers[i];
        if (i < dns_servers.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
}
//
// void NetInfo::execute() {
// if (interface.empty()) {
//     std::cerr << "smash error: netinfo: interface not specified" << std::endl;
// }
//
//     std::string ip_path =  "/sys/class/net/" + interface +"/address";
//     int fd = open(ip_path.c_str(),O_RDONLY);
//     if (fd == -1) {
//         std::cerr << "smash error: netinfo: interface " << interface << " does not exist" << std::endl;
//         return;
//     }
//     if (close(fd) < 0) {
//         perror("smash error: close failed");
//         return;
//     }
//
//
//     int dns_fd = open("/etc/resolv.conf", O_RDONLY);
//     if (dns_fd < 0) {
//         perror("smash error: open failed");
//         return;
//     }
//     char buffer[2048];
//     ssize_t bytes_read = read(dns_fd, buffer, sizeof(buffer) - 1);
//     if (bytes_read < 0) {
//         perror("smash error: read failed");
//         close(dns_fd);
//         return;
//     }
//     buffer[bytes_read] = '\0';
//
//     if (close(dns_fd) < 0) {
//         perror("smash error: close failed");
//         return;
//     }
//     std::string dns_servers = "";
//     std::istringstream iss(buffer);
//     std::string line;
//     while (std::getline(iss, line)) {
//         if (line.find("nameserver") == 0) {
//             if (!dns_servers.empty()) dns_servers += ", ";
//             dns_servers += line.substr(11); // skip "nameserver "
//         }
//     }
//
//     std::cout << "IP Address: 192.168.1.100" << std::endl;        // Placeholder
//     std::cout << "Subnet Mask: 255.255.255.0" << std::endl;       // Placeholder
//     std::cout << "Default Gateway: 192.168.1.1" << std::endl;     // Placeholder
//     std::cout << "DNS Servers: " << (dns_servers.empty() ? "Unavailable" : dns_servers) << std::endl;
// }



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
    commands_vector = {"ls", "cp", "mv", "rm", "mkdir", "rmdir", "touch", "chmod", "chown",
        "echo", "cat", "grep", "find", "head", "tail", "cut", "sort", "uniq",
        "diff", "wc", "sleep", "ps", "top", "kill", "ping", "whoami", "du",
        "df", "man", "date", "uptime", "hostname", "clear", "env", "which",
        "ssh", "scp", "git", "curl", "wget", "tar", "zip", "unzip", "nano",
        "vim" , "chprompt" , "pwd" , "showpid" , "cd" ,"jobs" , "fg" , "quit" ,
         "alias" , "unalias" , "unsetenv" , "watchproc" , "netinfo"};
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

    std::string cmd_str(cmd_line);
    std::istringstream iss(cmd_str);
    std::string first_word;
    iss >> first_word;


    /// look for alias
    auto alias_it = aliases.find(first_word);

    if (alias_it != aliases.end()) {
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
