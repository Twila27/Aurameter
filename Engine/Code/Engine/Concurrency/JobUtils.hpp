#pragma once


#include "Engine/Memory/ObjectPool.hpp"
#include "Engine/Memory/CBuffer.hpp"
#include "Engine/Concurrency/ThreadSafeQueue.hpp"
struct Job;
class Thread;
typedef ThreadSafeQueue<Job*> JobQueue;
typedef void( JobCallback )( Job* job );


//--------------------------------------------------------------------------------------------------------------
/* Tips on Job Use
	--> DO NOT LET JOBS STALL/SLEEP, it prevents the job thread from consuming. In those cases it's best to spin up a dedicated thread ( e.g. Logger thread ).
		FileRead-style I/O tasks fall under this umbrella.
	--> No critical sections in a job.
	--> NEVER have a while true loop in these worker job functions.
*/


//--------------------------------------------------------------------------------------------------------------
enum JobCategory //"Where" the job should run, e.g. don't want rendering GENERIC_SLOW jobs run on the main thread.
{
	JOB_CATEGORY_GENERIC = 0,
	JOB_CATEGORY_GENERIC_SLOW, //May take multiple frames to complete.
//	JOB_CATEGORY_IO,
//	JOB_CATEGORY_RENDERING,
	NUM_JOB_CATEGORIES
};


//--------------------------------------------------------------------------------------------------------------
struct Job //The benefit of a job system over just spawning a thread per job: CREATING AND DELETING THREADS IS EXPENSIVE.
	//However, # jobs != # threads, i.e. worker threads eat up an arbitrary # job requests over time from 1+ thread-safe queue(s).
{
	//Important: jobs need to remain the same size for the JobSystem::m_jobPool object pool allocator.
	JobCategory jobType;
	int refCount; //Start at 2. Releases one from the thread that completes its work, and the other either immediately from DetachJob or on completion in WaitOnJob.

	JobCallback* jobCallback; //Note: best to send jobs for anything that can be thought of as an array of elements updated independently, e.g. particle list.
	CBuffer jobData;
	static const size_t JOB_DATA_BUFFER_SIZE = 128;

	template < typename T > T Write( const T& data ) 
	{ 
		T* out = jobData.WriteToBuffer<T>(); 
		*out = data; 
		return *out; 
	}
	template < typename T > T Read() { return *jobData.ReadFromBuffer<T>(); }
};


//--------------------------------------------------------------------------------------------------------------
class JobSystem
{
public:
	static JobSystem* Instance();

	void Startup( int numWorkerThreads ); //e.g. -2 workers for "as many as possible, minus two".
		//This is a polling job system, as opposed to a workload-balancing job system which would likely be slower.
	bool IsRunning() const { return m_isRunning; }
	void Shutdown() { m_isRunning = false; } //Stops all threads, letting remaining jobs empty out like for Logger.

	Job* CreateJob( JobCategory jobType, JobCallback* jobFunc );
	void DispatchJob( Job* job ); //AKA "QueueJobToBeRunByJobConsumer". After this, call either DetachJob or WaitOnJob(s).

	void DetachJob( Job* job ); //Alternative to the Wait route--means no dependency on its work exists. Won't return a handle because it doesn't expect you to need it.
	void WaitOnJobForCompletion( Job* job ); //AKA the "JoinJob" in our analogy to thread terminology, vis-a-vis detach above.
	void WaitOnJobsForCompletion( const std::vector<Job*>& jobs ); //Only checks the pointers we have in jobs[], not the queue of messages, and not all jobs.

	JobQueue* GetJobQueueForCategory( JobCategory category ) { return m_categoryQueues[ category ]; }

	void ReleaseJob( Job* job ); //Else JobConsumer can't get at it.

private:
	void AcquireJob( Job* job );

	bool m_isRunning;
	static JobSystem* s_theJobSystem;
	JobQueue* m_categoryQueues[ NUM_JOB_CATEGORIES ];
	std::vector< Thread* > m_threads; //Does it need to be thread-safe?
	ObjectPool<Job> m_jobPool;

	static const int MAX_NUM_JOBS							= 100;
	static const int JOB_DATA_BUFFER_SIZE					= 128; //In bytes.
	static const int JOB_REFCOUNT_UNREFERENCED				= 0;
	static const int JOB_REFCOUNT_CREATED					= 1;
	static const int JOB_REFCOUNT_DISPATCHED				= 1;
	static const int JOB_REFCOUNT_CREATED_AND_DISPATCHED	= 2;
};


//--------------------------------------------------------------------------------------------------------------
class JobConsumer //These are NOT shared, make one per thread. Jobs are created/detached by game code apart through JobSystem, game shouldn't see JobConsumer underneath.
	//If the category queues are checkout lanes, these are the staff manning them. ONLY JobConsumers can pull jobs off queues, a thread makes a local one in JobSystem::Startup().
{
public:
	static void CreateAndRunUntilShutdown( JobCategory orderedFilterCategories[], size_t numCategories ); //Prefer this unless you need special exit handling (see WaitForJob).
	static JobConsumer* Create( JobCategory orderedFilterCategories[], size_t numCategories );

	//These run-prefixed functions differ from try-prefixed because they check JobSystem::IsRunning.
	static void RunJobsUntilShutdown( JobConsumer* consumer );
	static void RunJobsForMilliseconds( JobConsumer* consumer, float ms );
	static void RunOneJob( JobConsumer* consumer );


private:
	void ProcessJob( Job* job ); //Called by consume methods below.
	void TryConsumingAllJobs() { while ( TryConsumingOneJob() ); } //Spins until Consume() returns false, then ThreadYield() is hit in Create().
	bool TryConsumingOneJob();

	std::vector< JobQueue* > m_queues; //ONLY the ones sent in by the ctor, and the order these are checked == its consumer order in ctor.
		//These consumers use the SAME queue, and it is thread-safe, so multiple consumers can just grab off the top.
};
