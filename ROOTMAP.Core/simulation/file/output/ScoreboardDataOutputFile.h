#ifndef ScoreboardDataOutputFile_H
#define ScoreboardDataOutputFile_H
#include <CDataFile.h>
#include "JDataFile.h"
#include "JDataOutputFile.h"

namespace rootmap
{

class Process;

class ScoreboardDataOutputFile : public DataOutputFile
{
    TCL_DECLARE_CLASS;

protected:
// Data members
    Process * m_process;
    long m_characteristicnumber;

    // why not:
    Scoreboard * m_scoreboard;
    CharacteristicIndex m_characteristicIndex;

    bool m_succeeded;

public:
// Member functions
    // construction & destruction
    ScoreboardDataOutputFile();

    virtual void DoProvokeOutput(ProcessActionDescriptor* action);
    virtual void DoOutput(ProcessActionDescriptor* action);
};

} // namespace rootmap

#endif // #ifndef ScoreboardDataOutputFile_H
