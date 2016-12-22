#include "Engine/Concurrency/JobUtils.hpp"
#include "Engine/Concurrency/Thread.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Concurrency/ConcurrencyUtils.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Time/Stopwatch.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC JobSystem* JobSystem::s_theJobSystem = nullptr;


//--------------------------------------------------------------------------------------------------------------
STATIC JobSystem* JobSystem::Instance()
{
	if ( s_theJobSystem == nullptr )
		s_theJobSystem = new JobSystem();

	return s_theJobSystem; //Needs to be initialized before worker threads get created.
		//Right now works here because you have to call Instance() to get to Startup().
}


//--------------------------------------------------------------------------------------------------------------
static void GenericJobWorkerThreadEntry( void* )
{
	JobCategory categories[ 2 ] = { JOB_CATEGORY_GENERIC, JOB_CATEGORY_GENERIC_SLOW };
	JobConsumer::CreateAndRunUntilShutdown( categories, 2 ); //Cleanup handled internally.	
}


//--------------------------------------------------------------------------------------------------------------
STATIC void JobSystem::Startup( int numWorkerThreads )
{
	m_isRunning = true;

	//# queues created == # job categories (e.g. IO, RENDERING, GENERIC_SLOW). Default to 1 (GENERIC).
	for ( int categoryIndex = 0; categoryIndex < NUM_JOB_CATEGORIES; categoryIndex++ )
		m_categoryQueues[ categoryIndex ] = new JobQueue();

	//Spin up threads.
	int actualNumWorkerThreads = abs( numWorkerThreads );
	if ( numWorkerThreads < 0 ) //Negative parameter means " # cores minus however many I specified ".
		actualNumWorkerThreads = SystemGetCoreCount() - actualNumWorkerThreads;
	if ( actualNumWorkerThreads <= 0 )
		actualNumWorkerThreads = 1; //Always at least one created.

	for ( int threadIndex = 0; threadIndex < actualNumWorkerThreads; threadIndex++ )
		m_threads.push_back( new Thread( GenericJobWorkerThreadEntry ) );

	//Initialize job pool.
	m_jobPool.Init( MAX_NUM_JOBS );
}


//--------------------------------------------------------------------------------------------------------------
Job* JobSystem::CreateJob( JobCategory jobType, JobCallback* jobFunc )
{
	Job* newJob = m_jobPool.Allocate();
	newJob->refCount = 0;
	newJob->jobType = jobType;
	newJob->jobCallback = jobFunc;
	newJob->jobData.Initialize( malloc( JOB_DATA_BUFFER_SIZE ), JOB_DATA_BUFFER_SIZE );

	AcquireJob( newJob );

	return newJob;
}


//--------------------------------------------------------------------------------------------------------------
void JobSystem::DetachJob( Job* job )
{
	ReleaseJob( job );
}


//--------------------------------------------------------------------------------------------------------------
void JobSystem::DispatchJob( Job* job )
{
	AcquireJob( job );
	m_categoryQueues[ job->jobType ]->Enqueue( job );
}


//--------------------------------------------------------------------------------------------------------------
void JobSystem::WaitOnJobsForCompletion( const std::vector<Job*>& jobs )
{
	for each ( Job* job in jobs )
		WaitOnJobForCompletion( job );
}

//--------------------------------------------------------------------------------------------------------------
void JobSystem::WaitOnJobForCompletion( Job* job )
{
	JobConsumer* interimConsumer;
	JobCategory interimConsumerCategories[] = { JOB_CATEGORY_GENERIC };
	interimConsumer = JobConsumer::Create( interimConsumerCategories, 1 );

	while ( job->refCount >= JOB_REFCOUNT_CREATED_AND_DISPATCHED ) //Until complete (refCount of 1), run interim jobs.
		JobConsumer::RunOneJob( interimConsumer );

	delete interimConsumer;

	ReleaseJob( job );
}


//--------------------------------------------------------------------------------------------------------------
void JobSystem::AcquireJob( Job* job )
{
	++job->refCount;
}


//--------------------------------------------------------------------------------------------------------------
void JobSystem::ReleaseJob( Job* job )
{
	--job->refCount; 
	if ( job->refCount == JOB_REFCOUNT_UNREFERENCED )
		m_jobPool.Delete( job );
}


//--------------------------------------------------------------------------------------------------------------
STATIC void JobConsumer::CreateAndRunUntilShutdown( JobCategory orderedFilterCategories[], size_t numCategories )
{
	JobConsumer* consumer = JobConsumer::Create( orderedFilterCategories, numCategories );
	JobConsumer::RunJobsUntilShutdown( consumer ); //Cleanup handled internally.
}


//--------------------------------------------------------------------------------------------------------------
STATIC JobConsumer* JobConsumer::Create( JobCategory orderedFilterCategories[], size_t numCategories )
{
	JobConsumer* consumer = new JobConsumer();

	for ( size_t index = 0; index < numCategories; index++ )
		consumer->m_queues.push_back( JobSystem::Instance()->GetJobQueueForCategory( orderedFilterCategories[ index ] ) );

	return consumer;
}


//--------------------------------------------------------------------------------------------------------------
bool JobConsumer::TryConsumingOneJob()
{
	Job* job;
	for each ( JobQueue* jobQueue in m_queues ) //Enforces an alternating order of job category access.
	{
		if ( jobQueue->Dequeue( &job ) )
		{
			ProcessJob( job ); //Runs the job--i.e. its callback--and release job when done--calling whatever callback is set for when the job has finished.
			return true;
		}
	}
	return false; //When nothing can be dequeued.
}


//--------------------------------------------------------------------------------------------------------------
STATIC void JobConsumer::RunJobsUntilShutdown( JobConsumer* consumer )
{
	while ( JobSystem::Instance()->IsRunning() )
	{
		consumer->TryConsumingAllJobs();
		Thread::ThreadSleep( std::chrono::milliseconds( 100 ) );
	}
	consumer->TryConsumingAllJobs(); //Re-runs the above loop one last time, in case we were told to stop while messages are still queued.
}


//--------------------------------------------------------------------------------------------------------------
STATIC void JobConsumer::RunJobsForMilliseconds( JobConsumer* consumer, float endTimeMilliseconds )
{
	TODO( "Test to see if it runs about the expected time amount." );
	float endTimeSeconds = endTimeMilliseconds / 1000.f; //May or may not be a speed-critical division depending on if usage is per-frame.
	double currentTimeSeconds; FIXME( "Should be a child of a system clock, not an independent counter!" );
	Stopwatch timer = Stopwatch( endTimeSeconds ); 
		//Hesitant to end via an event--this way it can just check it all internally without touching the event system other threads could also be touching.

	double previousTimeSeconds = GetCurrentTimeSeconds();
	timer.Start();
	while ( JobSystem::Instance()->IsRunning() && !timer.IsPaused() ) //Timer will pause when done. Misnomer since usually it'd be done by event trigger.
		//i.e. Do not use this in all cases to check if completed, only works here because we never intend to pause the timer between its start and end.
	{
		if ( !consumer->TryConsumingOneJob() )
			return;

		currentTimeSeconds = GetCurrentTimeSeconds();
		timer.Update( static_cast<float>( currentTimeSeconds - previousTimeSeconds ) );
		previousTimeSeconds = currentTimeSeconds;
	}
	//Going to let the other worker threads handle cleanup, not this special case.
}


//--------------------------------------------------------------------------------------------------------------
STATIC void JobConsumer::RunOneJob( JobConsumer* consumer )
{
	if ( JobSystem::Instance()->IsRunning() )
	{
		if ( !consumer->TryConsumingOneJob() )
			Thread::ThreadSleep( std::chrono::milliseconds( 100 ) );
	}
}


//--------------------------------------------------------------------------------------------------------------
void JobConsumer::ProcessJob( Job* job )
{
	job->jobCallback( job ); 
	JobSystem::Instance()->ReleaseJob( job );

	//May want to have a second callback set for when the job has finished.
}
