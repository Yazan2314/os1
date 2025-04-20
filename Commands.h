// Ver: 10-4-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <sstream>
#include <algorithm>
#include <list>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
    // TODO: Add your data members
protected:
    const char* cmd_line ;
    int processid ;
    std::vector<std::string> commands_parts; /// todo : if we have command like yazan yaman as because of the space betwen them we save in vector


public:
    Command(const char *cmd_line);

    virtual ~Command();

    virtual void execute() = 0;

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~BuiltInCommand() {
    }
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line);

    virtual ~ExternalCommand() {
    }

    void execute() override;
};


class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {
    }

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class DiskUsageCommand : public Command {
public:
    DiskUsageCommand(const char *cmd_line);

    virtual ~DiskUsageCommand() {
    }

    void execute() override;
};

class WhoAmICommand : public Command {
public:
    WhoAmICommand(const char *cmd_line);

    virtual ~WhoAmICommand() {
    }

    void execute() override;
};

class NetInfo : public Command {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    NetInfo(const char *cmd_line);

    virtual ~NetInfo() {
    }

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
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
    ChpromtCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}
    virtual ~ChpromtCommand() {}
    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
    // TODO: Add your data members public:
    QuitCommand(const char *cmd_line, JobsList *jobs);

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



        JobEntry(int jopid ,int pid, std::string command, bool isStoped = false ,Command* cmd = nullptr) : jopId(jopid),
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
    int nextJobId = 1 ;
public:
    JobsList() = default;

    ~JobsList() {
        for (auto jop : jobs_list) {
            delete jop;
        }
        jobs_list.clear();
    }

    void addJob(Command *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);

    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand() {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

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
    AliasCommand(const char *cmd_line);

    virtual ~AliasCommand() {
    }

    void execute() override;
};

class UnAliasCommand : public BuiltInCommand {
public:
    UnAliasCommand(const char *cmd_line);

    virtual ~UnAliasCommand() {
    }

    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand {
public:
    UnSetEnvCommand(const char *cmd_line);

    virtual ~UnSetEnvCommand() {
    }

    void execute() override;
};

class WatchProcCommand : public BuiltInCommand {
public:
    WatchProcCommand(const char *cmd_line);

    virtual ~WatchProcCommand() {
    }

    void execute() override;
};

class SmallShell {
private:
    // TODO: Add your data members
    int pid ;
    std::string prompt;
    std::vector<std::string> commands_vector ;
    std::string pwd;
    std::string lastpwd;



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



    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
