// Ver: 10-4-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <sstream>
#include <algorithm>
#include <list>
#include <map>
#include <regex>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
class JobsList;
class Command {
// TODO: Add your data members
protected:
    const  char* cmd_line ;
    int processid;
    std::vector<std::string> commands_parts; /// todo : if we have command like yazan yaman as because of the space betwen them we save in vector
    bool isbackground ;
    bool has_alias;
    std::string alias;



public:
    Command(const char *cmd_line);






    virtual ~Command();

    virtual void execute() = 0;

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed

    int getProcessID();

    bool isValidAlias(const char *cmd_line);
    std::string getCommand();
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~BuiltInCommand() {
    }
};

class ExternalCommand : public Command {
public:
const char* m_line ;
    ExternalCommand(const char *cmd_line ):Command(cmd_line),m_line(cmd_line){}

    virtual ~ExternalCommand() {
    }

    void execute() override;
};



class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    std::vector<std::string> actual_command;  // The command without the redirection part
    std::string output_file;     // The target file to redirect output to
    bool append_mode;

    explicit RedirectionCommand(const char *cmd_line) : Command(cmd_line) {
        append_mode = true;
        bool find_redirection = false;


        for (size_t i = 0; i < commands_parts.size(); ++i) {
            const std::string& token = commands_parts[i];

            if (token == ">") {
                append_mode = false;
                find_redirection = true;
                if (i + 1 < commands_parts.size()) {
                    output_file = commands_parts[i + 1];
                }
                ++i; // skip the filename in the next iteration
            } else if (token == ">>") {
                append_mode = true;
                find_redirection = true;
                if (i + 1 < commands_parts.size()) {
                    output_file = commands_parts[i + 1];
                }
                ++i;
            } else if (!find_redirection) {
                actual_command.push_back(token);
            }
        }
    }


    virtual ~RedirectionCommand() {
    }

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members

    std::vector<std::string> leftCommand;  //the command before the pipe
    std::vector<std::string> rightCommand;  //the command after the pipe
    bool errorPipe; // is the pipe |&?
public:



    PipeCommand(const char *cmd_line):Command(cmd_line) {
        errorPipe = true;
        bool find_pipe = false;

        for (size_t i = 0; i < commands_parts.size(); ++i) {
            const std::string& token = commands_parts[i];
            if (token == "|") {
                find_pipe = true;
                errorPipe = false;
            }else if (token == "|&") {
                errorPipe = true;
                find_pipe = true;
            }else if (!find_pipe) {
                leftCommand.push_back(token);
            }else{
                rightCommand.push_back(token);
            }
        }

    }

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class DiskUsageCommand : public Command {
public:
    DiskUsageCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~DiskUsageCommand() {
    }

    void execute() override;
};

class WhoAmICommand : public Command {
public:
    WhoAmICommand(const char *cmd_line):Command(cmd_line){}

    virtual ~WhoAmICommand() {
    }

    void execute() override;
};

class NetInfo : public Command {
    // TODO: Add your data members **BONUS: 10 Points**
    // std::string interface;

public:
    NetInfo(const char *cmd_line):Command(cmd_line) {
        // if (commands_parts.size() > 1) {
        //     interface = commands_parts[1];
        // }else {
        //     interface = "";
        // }
    }

    virtual ~NetInfo() {
    }

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
    public:
    // TODO: Add your data members public:
    char **plastPwd ;

    ChangeDirCommand(const char *cmd_line, char **plastPwd):BuiltInCommand(cmd_line) , plastPwd(plastPwd) {}

    virtual ~ChangeDirCommand() {
    }

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~GetCurrDirCommand() {
    }

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ShowPidCommand() {
    }

    void execute() override;
};

class ChpromtCommand : public BuiltInCommand {
public:
    ChpromtCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}
    virtual ~ChpromtCommand() {}
    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
public:
    // TODO: Add your data members public:
    JobsList *jobs ;
    QuitCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~QuitCommand() {
    }

    void execute() override;
};




class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
        public:
        int jopId ;
        int pid;
        std::string command;
        bool isStoped;
        Command* cmd;



        JobEntry(int jopid ,int pid, std::string command, Command* cmd = nullptr,bool isStoped = false ) : jopId(jopid),
        pid(pid),command(std::move(command)),isStoped(isStoped),cmd(cmd){}

        ~JobEntry() = default;

         std::string getDetails() {
            std::ostringstream details;
            details << "[" << jopId << "] " << command;
            if (isStoped) {
                details << " (stopped)";
            }
            return details.str();
        }


    };

    // TODO: Add your data members

    std::list<JobEntry*> jobs_list;


public:
    JobsList() = default;

    ~JobsList() {
        for (auto jop : jobs_list) {
            delete jop;
        }
        jobs_list.clear();
    }

    // void addJob(Command *cmd, bool isStopped = false);

    void addJob(pid_t pid ,  std::string cmd_str, bool isStopped );
    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();



    JobEntry *getJobBypid(int pid);

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId); ///// todo : ther are a very good trick ask chat to explain it to you




    JobEntry *getLastStoppedJob(int *jobId);

    // TODO: Add extra methods or modify exisitng ones as needed

    void printjopsListpid();
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobsList;
public:
    JobsCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line), jobsList(jobs) {}

    virtual ~JobsCommand() {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsList* jobs;
    KillCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~KillCommand() {
    }

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList *jobs;
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line) , jobs(jobs) {}

    virtual ~ForegroundCommand() {
    }

    void execute() override;
};

class AliasCommand : public BuiltInCommand {
public:
    AliasCommand(const char *cmd_line):BuiltInCommand(cmd_line){}

    virtual ~AliasCommand() {
    }

    void execute() override;
};

class UnAliasCommand : public BuiltInCommand {
public:
    UnAliasCommand(const char *cmd_line):BuiltInCommand(cmd_line){}

    virtual ~UnAliasCommand() {
    }

    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand {
public:
    UnSetEnvCommand(const char *cmd_line): BuiltInCommand(cmd_line){}

    virtual ~UnSetEnvCommand() {
    }

    void execute() override;
};

class WatchProcCommand : public BuiltInCommand {
public:
    WatchProcCommand(const char *cmd_line):BuiltInCommand(cmd_line){}

    virtual ~WatchProcCommand() {
    }

    void execute() override;
};

class SmallShell {
private:
    // TODO: Add your data members
      std::string prompt;
    int pid ;

    std::vector<std::string> commands_vector ;
    std::string pwd;
    std::string lastpwd;
    JobsList* shell_jops;
    int current_jop_pid_front;
    std::map<std::string, std::string> aliases;
    std::vector<std::string> aliases_inorder;
    std::map<int,JobsList::JobEntry*> pid_map ; ////  its a map that save the jops that the key of the map is the pid
    std::vector<std::string> builtInCommands_vector ;


    SmallShell();

public:
    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    void executeCommand(const char *cmd_line);


    int getPid() const;
    std::string getPwd() ;
    void setprompt(const std::string &new_prompt);
    const std::string &getprompt() const;
    char** getLastPwdPtr();
    std::string getLastPwd();
    void changePwd( std::string new_pwd);
    JobsList *getJobsList();
    void bringToForeground(int pid);
    void creatCommand_vector();
    std::vector<std::string> getcommand_vector();
    void change_current_jop_pid_front(int pid);
    int get_current_jop_pid_front();

    std::vector<std::string> getBuiltInCommandsVector();



    void addAlias(const std::string& alias, const std::string& command);
    void removeAlias(const std::string& alias);
    std::vector<std::string> getAliases_ord();
    std::map<std::string, std::string> getAliases();

    std::map<int,JobsList::JobEntry*> getPidMap();




    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
