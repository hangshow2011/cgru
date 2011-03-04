#pragma once

#include "af.h"
#include "../include/afjob.h"

#include "afnode.h"
#include "blockdata.h"

#include <QtCore/QString>

namespace af
{
/// Job class. Main structure Afanasy was written for.
class Job : public Node
{
public:
   Job( int Id);

/// Construct data from message buffer, corresponding to message type.
/** Used when user create a new job and send it to Afanasy.
/// And when user watching job progress ( for example from Watch).
**/
   Job( Msg * msg);

   virtual ~Job();

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

//   inline uint32_t getFlags()                const { return flags;                 }
   inline uint32_t getState()             const { return state;            }
   inline int      getBlocksNum()         const { return blocksnum;        }
   inline uint32_t getTimeCreation()      const { return time_creation;    }
   inline uint32_t getTimeStarted()       const { return time_started;     }
   inline uint32_t getTimeWait()          const { return time_wait;        }
   inline uint32_t getTimeDone()          const { return time_done;        }
   inline int      getLifeTime()          const { return lifetime;         }
   inline int      getUserListOrder()     const { return userlistorder;    }
   inline int      getMaxRunningTasks()   const { return maxrunningtasks;  }

   inline const std::string & getUserName()     const { return username;    }
   inline const std::string & getHostName()     const { return hostname;    }
   inline const std::string & getCmdPre()       const { return cmd_pre;     }
   inline const std::string & getCmdPost()      const { return cmd_post;    }
   inline const std::string & getDescription()  const { return description; }
   inline const std::string & getAnnontation()  const { return annotation;  }

   inline bool isStarted() const {return time_started != 0 ; }                ///< Whether job is started.
   inline bool isReady()   const {return state & AFJOB::STATE_READY_MASK;   }///< Whether job is ready.
   inline bool isRunning() const {return state & AFJOB::STATE_RUNNING_MASK; }///< Whether job is running.
   inline bool isDone()    const {return state & AFJOB::STATE_DONE_MASK;    }///< Whether job is done.

   inline bool setHostsMask(         const std::string & str  )
      { return setRegExp( hostsmask, str, "job hosts mask");}
   inline bool setHostsMaskExclude(  const std::string & str  )
      { return setRegExp( hostsmask_exclude, str, "job exclude hosts mask");}
   inline bool setDependMask(        const std::string & str  )
      { return setRegExp( dependmask, str, "job depend mask");}
   inline bool setDependMaskGlobal(  const std::string & str  )
      { return setRegExp( dependmask_global, str, "job global depend mask");}
   inline bool setNeedOS(            const std::string & str  )
      { return setRegExp( need_os, str, "job need os mask");}
   inline bool setNeedProperties(    const std::string & str  )
      { return setRegExp( need_properties, str, "job need properties mask");}

   inline const std::string getHostsMask()          const { return hostsmask.pattern().toUtf8().data();          }
   inline const std::string getHostsMaskExclude()   const { return hostsmask_exclude.pattern().toUtf8().data();  }
   inline const std::string getDependMask()         const { return dependmask.pattern().toUtf8().data();         }
   inline const std::string getDependMaskGlobal()   const { return dependmask_global.pattern().toUtf8().data();  }
   inline const std::string getNeedOS()             const { return need_os.pattern().toUtf8().data();            }
   inline const std::string getNeedProperties()     const { return need_properties.pattern().toUtf8().data();    }

   inline bool hasHostsMask()          const { return false == hostsmask.isEmpty();          }
   inline bool hasHostsMaskExclude()   const { return false == hostsmask_exclude.isEmpty();  }
   inline bool hasDependMask()         const { return false == dependmask.isEmpty();         }
   inline bool hasDependMaskGlobal()   const { return false == dependmask_global.isEmpty();  }
   inline bool hasNeedOS()             const { return false == need_os.isEmpty();            }
   inline bool hasNeedProperties()     const { return false == need_properties.isEmpty();    }

   inline bool checkHostsMask(         const std::string & str  ) const
      { if( hostsmask.isEmpty()        ) return true;   return hostsmask.exactMatch( QString::fromUtf8( str.c_str())); }
   inline bool checkHostsMaskExclude(  const std::string & str  ) const
      { if( hostsmask_exclude.isEmpty()) return false;  return hostsmask_exclude.exactMatch( QString::fromUtf8( str.c_str())); }
   inline bool checkDependMask(        const std::string & str  ) const
      { if( dependmask.isEmpty()       ) return false;  return dependmask.exactMatch( QString::fromUtf8( str.c_str())); }
   inline bool checkDependMaskGlobal(  const std::string & str  ) const
      { if( dependmask_global.isEmpty()) return false;  return dependmask_global.exactMatch( QString::fromUtf8( str.c_str())); }
   inline bool checkNeedOS(            const std::string & str  ) const
      { if( need_os.isEmpty()          ) return true;   return  QString::fromUtf8( str.c_str()).contains( need_os); }
   inline bool checkNeedProperties(    const std::string & str  ) const
      { if( need_properties.isEmpty()  ) return true;   return  QString::fromUtf8( str.c_str()).contains( need_properties); }

   inline int getRunningTasksNumber() const /// Get job running tasks.
      {int n=0;for(int b=0;b<blocksnum;b++)n+=blocksdata[b]->getRunningTasksNumber();return n;}

   const std::string & getTasksOutputDir() const { return tasksoutputdir; }

/// Get block constant pointer.
   inline BlockData* getBlock( int n) const { if(n<(blocksnum))return blocksdata[n];else return NULL;}

   virtual int calcWeight() const;                   ///< Calculate and return memory size.

protected:
   BlockData  ** blocksdata;    ///< Blocks pointer.
   int32_t blocksnum;   ///< Number of blocks in job.

   uint32_t flags;      ///< Job flags.

   int32_t userlistorder;   ///< Job order in user jobs list.

   uint32_t state;      ///< Job state.

   std::string description; ///< Job description for statistics purposes only.
   std::string annotation;

   std::string username;    ///< Job user ( creator ) name.
   std::string hostname;    ///< Computer name, where job was created.

/// Job pre command (executed by server on job registration)
   std::string cmd_pre;

/// Job post command (executed by server on job deletion)
   std::string cmd_post;

/// Maximum number of running tasks
   int32_t maxrunningtasks;

/// Life time, after this time job will be automatically deleted.
   int32_t lifetime;

/// set in this constructor
   uint32_t time_creation;

/// time to wait to start
   uint32_t time_wait;

/// set in JobAf::refresh(): if job is running or done, but was not started, we set job header time_started
   uint32_t time_started;

/// set in JobAf::refresh(): if job was not done, but now is done we set job header time_done
   uint32_t time_done;

/// Job hosts mask ( huntgroup ).
   QRegExp hostsmask;
/// Job hosts exclude mask ( huntgroup ).
   QRegExp hostsmask_exclude;
/// Jobs names mask current job depends on ( wait until they will be done).
   QRegExp dependmask;
/// Jobs names mask current job depends on ( wait until they will be done).
   QRegExp dependmask_global;
   QRegExp need_os;
   QRegExp need_properties;

   std::string tasksoutputdir;       ///< Tasks output directory.

private:
   void initDefaultValues();
   void readwrite( Msg * msg); ///< Read or write data in buffer.
   void rw_blocks( Msg * msg); ///< Read or write blocks.
   virtual BlockData * newBlockData( Msg * msg);
};
}
