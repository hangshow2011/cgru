#include "afcontainer.h"

#include <stdio.h>

#include <QtCore/QString>
#include <QtCore/QRegExp>

#include "../libafanasy/msgclasses/mcafnodes.h"

#include "aflist.h"
#include "afcontainerit.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfContainer::AfContainer( int maximumsize):
   count( 0),
   size( maximumsize),
   first_ptr( NULL),
   last_ptr( NULL),
   initialized( false)
{
   if( pthread_rwlock_init( &rwlock, NULL) != 0)
   {
      AFERRPE("AfContainer::AfContainer:");
      return;
   }
   nodesTable = new af::Node*[size];
   if( nodesTable == NULL)
   {
      AFERRAR("AfContainer::AfContainer: cant't allocate memory for %d nodes.\n", size);
      return;
   }
   AFINFA("AfContainer::AfContainer: %d bytes allocated for table at %p\n", size*sizeof(af::Node*), nodesTable);
   for( int i = 0; i < size; i++ ) nodesTable[i] = NULL;

   initialized = true;
}

AfContainer::~AfContainer()
{
AFINFO("AfContainer::~AfContainer:\n");
//   pthread_rwlock_trywrlock( &rwlock);
   size = 0;
   while( first_ptr != NULL)
   {
      last_ptr = first_ptr;
      first_ptr = first_ptr->next_ptr;
      delete last_ptr;
   }
   if( nodesTable != NULL) delete [] nodesTable;
//   pthread_rwlock_unlock( &rwlock);
}

int AfContainer::add( af::Node *node)
{
   if( node == NULL )
   {
      AFERROR("AfContainer::add: node == NULL.\n");
      return 0;
   }
   if( count >= size-1)
   {
      AFERRAR("AfContainer::add: maximum number of nodes = %d reached.\n", count);
      return 0;
   }

   int newId = node->id;
   bool founded = false;

   if( newId != 0)
   {
      if( nodesTable[newId] != NULL )
      {
         AFERRAR("AfContainer::add: node->id = %d already exists.\n", newId);
      }
      else
      {
         founded = true;
      }
   }
   else
   {
      for( newId = 1; newId < size; newId++)
      {
         if( nodesTable[newId] == NULL )
         {
            founded = true;
            break;
         }
      }
   }

   if( !founded )
   {
      newId = 0;
   }
   else
   {
      node->id = newId;

//
// get an unique name
      {
      std::string origname = node->name;
      int number = 1;
      for(;;)
      {
         bool unique = true;
         af::Node * another = first_ptr;
         while( another != NULL)
         {
            if((!another->zombie) && (another->name == node->name))
            {
               node->name = origname + '-' + af::itos( number++);
               unique = false;
               break;
            }
            another = another->next_ptr;
         }
         if( unique ) break;
      }
      }

//
// find a *before node with greater or equal priority and a node *after it
      af::Node *before = first_ptr;
      af::Node *after  = NULL;
      while( before != NULL )
      {
         if( *before < *node )
         {
            after = before;
            before = before->prev_ptr;
            break;
         }
         after = before->next_ptr;
         if( after == NULL ) break;
         before = after;
      }

      if( before == NULL)
      {
         first_ptr = node;
//         printf("before == NULL\n");
      }
      else
      {
//         printf("before:"); before->stdOut();
         before->next_ptr = node;
         node->prev_ptr = before;
      }
      if( after  == NULL)
      {
         last_ptr = node;
//         printf("after == NULL\n");
      }
      else
      {
//         printf("after:"); after->stdOut();
         after->prev_ptr = node;
         node->next_ptr = after;
      }

      nodesTable[node->id] = node;
      count++;
   }

   if( !founded )
      AFERROR("AfContainer::add: nodes table full.\n");
   AFINFA("AfContainer::add: new id = %u, count = %u \n", node->id, count);
   return newId;
}

void AfContainer::refresh( AfContainer * pointer, MonitorContainer * monitoring)
{
   time_t currnetTime = time( NULL);
   for( af::Node * node = first_ptr; node != NULL; node = node->next_ptr)
   {
      if( node->zombie ) continue;
      node->refresh( currnetTime, pointer, monitoring);
   }
}

MsgAf* AfContainer::generateList( int type)
{
   af::MCAfNodes mcNodes;
   for( af::Node * node = first_ptr; node != NULL; node = node->next_ptr)
   {
      if( node->zombie ) continue;
      mcNodes.addNode( node);
   }
   return new MsgAf( type, &mcNodes);
}

MsgAf* AfContainer::generateList( int type, const af::MCGeneral & mcgeneral)
{
   af::MCAfNodes mcNodes;
   int getcount = mcgeneral.getCount();

   for( int i = 0; i < getcount; i++)
   {
      int pos = mcgeneral.getId(i);
      if( pos >= size)
      {
         AFERRAR("AfContainer::generateList: position >= size (%d>=%d)\n", pos, size);
         continue;
      }
      af::Node * node = nodesTable[ pos];
      if( node == NULL   ) continue;
      if( node -> zombie ) continue;
      mcNodes.addNode( node);
   }
   if(( getcount == 0) && (false == mcgeneral.getName().empty()))
   {
      QRegExp rx( QString::fromUtf8( mcgeneral.getName().c_str()));
      if( false == rx.isValid())
      {
         AFERRAR("Name pattern \"%s\" is invalid:\n", mcgeneral.getName().c_str());
         printf("%s\n", rx.errorString().toUtf8().data());
      }
      else
      {
         bool namefounded = false;
         for( af::Node *node = first_ptr; node != NULL; node = node->next_ptr )
         {
            if( node == NULL   ) continue;
            if( node -> zombie ) continue;
            if( rx.exactMatch( QString::fromUtf8( node->name.c_str())))
            {
               mcNodes.addNode( node);
               if( false == namefounded) namefounded = true;
            }
         }
         if( namefounded == false )
         AFERRAR("AfContainer::generateList: No node matches \"%s\" founded.\n", mcgeneral.getName().c_str());
      }
   }
   return new MsgAf( type, &mcNodes);
}

bool AfContainer::setZombie( int id)
{
   if( id < 1 )
   {
      AFERRAR("AfContainer::setZombie: invalid id = %d\n", id);
      return false;
   }
   if( id >= size )
   {
      AFERRAR("AfContainer::setZombie: Too big id = %d < %d = maximum.\n", id, size);
      return false;
   }
   af::Node * node = nodesTable[ id];
   if( node == NULL )
   {
      AFERRAR("AfContainer::setZombie: No node with id=%d.\n", id);
      return false;
   }
   if( node->isZombie())
   {
      AFERRAR("AfContainer::setZombie: Node with id=%d already a zombie.\n", id);
      return false;
   }
   node->setZombie();
   return true;
}

void AfContainer::freeZombies()
{
   af::Node *node = first_ptr;
   last_ptr = NULL;
   while( node != NULL)
   {
      if((node->zombie) && node->unLocked())
      {
         af::Node* z_node = node;
         node = z_node->next_ptr;
         if( last_ptr != NULL)
         {
            last_ptr->next_ptr = node;
            if( node != NULL ) node->prev_ptr = last_ptr;
         }
         else
         {
            first_ptr = node;
            if( node != NULL ) first_ptr->prev_ptr = NULL;
         }
         nodesTable[ z_node->id] = NULL;

         AfListsList::iterator it = z_node->lists.begin();
         AfListsList::iterator end_it = z_node->lists.end();
         while( it != end_it) (*it++)->remove( z_node);

         delete z_node;
         count--;
      }
      else
      {
         last_ptr = node;
         node = node->next_ptr;
      }
   }
}

void AfContainer::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   bool namefounded = false;
   std::string name = mcgeneral.getName();
   int getcount = mcgeneral.getCount();

   if( getcount < 1 )
   {
      QRegExp rx( QString::fromUtf8( name.c_str()));
      if( false == rx.isValid())
      {
         AFERRAR("Name pattern \"%s\" is invalid:\n", name.c_str())
         printf("%s\n", rx.errorString().toUtf8().data());
         return;
      }
      for( af::Node *node = first_ptr; node != NULL; node = node->next_ptr )
      {
         if( rx.exactMatch( QString::fromUtf8( node->name.c_str())))
         {
            action( node, mcgeneral, type, pointer, monitoring);
            if( false == namefounded) namefounded = true;
         }
      }
   }
   else
   {
      for( int i = 0; i < getcount; i++)
      {
         int pos = mcgeneral.getId( i);
         if( pos >= size)
         {
            AFERRAR("AfContainer::action: position >= size (%d>=%d)\n", pos, size);
            continue;
         }
         af::Node *node = nodesTable[ pos];
         if( node == NULL ) continue;
         action( node, mcgeneral, type, pointer, monitoring);
      }
   }

   if(( size == 0) && ( namefounded == false))
      AFERRAR("No node matches \"%s\" founded.\n", name.c_str());
}

void AfContainer::action( af::Node * node, const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   if( node->isLocked()) return;
   if( node->action( mcgeneral, type, pointer, monitoring) == false )
   {
      AFERRAR("AfContainer::action: Error: [%s]:\n", af::Msg::TNAMES[type]);
      mcgeneral.stdOut( true );
      return;
   }
   switch( type)
   {
      case af::Msg::TRenderPriority:
      case af::Msg::TJobPriority:
      case af::Msg::TUserPriority:
         sortPriority( node);
   }
}

void AfContainer::sortPriority( af::Node * node)
{
   if( count < 2 ) return;

// extract node from list by connecting pointer of previous and next nodes
   af::Node * before  = node->prev_ptr;
   af::Node * after   = node->next_ptr;
   if(    before != NULL ) before->next_ptr = node->next_ptr;
   else first_ptr = node->next_ptr;
   if(    after  != NULL )  after->prev_ptr = node->prev_ptr;
   else last_ptr  = node->prev_ptr;

   bool lessPriorityFounded = false;
// insetring node after last node with a greater or same priority
   for( before = first_ptr; before != NULL; before = before->next_ptr )
   {
      if( *before >= *node ) continue;

      after  = before;
      before = before->prev_ptr;
      node->prev_ptr = before;
      node->next_ptr = after;
      after->prev_ptr = node;
      if( before != NULL ) before->next_ptr = node;
      else first_ptr = node;

      lessPriorityFounded = true;
      break;
   }

   if( lessPriorityFounded == false )
   {
   // push node into the end of list
      last_ptr->next_ptr = node;
      node->prev_ptr = last_ptr;
      node->next_ptr = NULL;
      last_ptr = node;
   }

   AfListsList::iterator it = node->lists.begin();
   AfListsList::iterator end_it = node->lists.end();
   while( it != end_it) (*it++)->sortPriority( node);
}
