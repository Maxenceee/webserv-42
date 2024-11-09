#include <queue>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

class ThreadPool {
public:
	ThreadPool(size_t numThreads);
	~ThreadPool();

	void	enqueueTask(void (*function)(int, int), int client_fd, int backend_fd);

	void	kill();

private:
	std::vector<pthread_t> workers;
	std::queue<void (*)(int, int)> tasks;
	std::queue<std::pair<int, int> > taskArgs;

	pthread_mutex_t queueMutex;
	pthread_cond_t condition;
	bool stop;

	static void* workerThread(void* arg);
	void run();
};

ThreadPool::ThreadPool(size_t numThreads): stop(false)
{
	std::cout << "\n\n" << "New thread pool" << std::endl;
	if (pthread_mutex_init(&queueMutex, NULL)) {
		perror("mutex init");
		throw "could not init mutex";
	}
	if (pthread_cond_init(&condition, NULL)) {
		perror("cond init");
		throw "could not init cond";
	}

	for (size_t i = 0; i < numThreads; ++i) {
		pthread_t worker;
		pthread_create(&worker, NULL, workerThread, this);
		workers.push_back(worker);
	}
}

ThreadPool::~ThreadPool()
{
	std::cout << "thread pool desctructor" << std::endl; 
	if (!stop) {
		this->kill();
	}	
}

void	ThreadPool::kill()
{
	std::cout << "request kill on poll" << std::endl;
	std::cout << "trying to lock" << std::endl;
	pthread_mutex_lock(&queueMutex);
	stop = true;
	std::cout << "stop changed to: " << stop << std::endl;
	pthread_cond_broadcast(&condition);
	std::cout << "broadcast sent" << std::endl;
	pthread_mutex_unlock(&queueMutex);

	for (size_t i = 0; i < workers.size(); ++i) {
		std::cout << "canceling thread " << i << std::endl;
		pthread_cancel(workers[i]);
	}

	for (size_t i = 0; i < workers.size(); ++i) {
		std::cout << "joining thread" << std::endl;
		pthread_join(workers[i], NULL);
	}

	pthread_mutex_destroy(&queueMutex);
	pthread_cond_destroy(&condition);
}

void	ThreadPool::enqueueTask(void (*function)(int, int), int client_fd, int backend_fd)
{
	pthread_mutex_lock(&queueMutex);
	tasks.push(function);
	taskArgs.push(std::make_pair(client_fd, backend_fd));
	std::cout << "task pushed" << std::endl;
	pthread_cond_signal(&condition);
	pthread_mutex_unlock(&queueMutex);
}

void	*ThreadPool::workerThread(void* arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	ThreadPool* pool = (ThreadPool*)arg;
	pool->run();
	return NULL;
}

void	ThreadPool::run()
{
#ifdef __linux__
	pid_t tid = gettid() % 10;
#else
	pid_t tid = 0;
#endif

	while (true) {
		std::cout << tid << ": " << "thread lock" << std::endl;
		pthread_mutex_lock(&queueMutex);
		std::cout << tid << ": " << "thread aquired" << std::endl;

		std::cout << tid << ": " << "thread stop: " << stop << " " << std::endl;
		while (!stop && tasks.empty()) {
			std::cout << tid << ": " << "thread wait" << std::endl;
			pthread_cond_wait(&condition, &queueMutex);
			std::cout << tid << ": " << "thread woke up" << std::endl;
		}

		std::cout << tid << ": " << "after wait thread stop: " << stop << std::endl;
		if (stop) {
			std::cout << tid << ": " << "stop thread" << std::endl;
			pthread_mutex_unlock(&queueMutex);
			return;
		}

		void (*task)(int, int) = tasks.front();
		std::pair<int, int> args = taskArgs.front();
		tasks.pop();
		taskArgs.pop();

		std::cout << tid << ": " << "thread unlock" << std::endl;
		pthread_mutex_unlock(&queueMutex);
		
		std::cout << tid << ": " << "test cancel before task" << std::endl;
		pthread_testcancel();

		std::cout << tid << ": " << "thread task" << std::endl;
		task(args.first, args.second);

		std::cout << tid << ": " << "thread cancel after task" << std::endl;
		pthread_testcancel();
	}
}

bool running = true;

static void interruptHandler(int sig_int) {
	(void)sig_int;
	running = false;
}

void task(int a, int b)
{
	size_t i = a + b;
	size_t r = 0;

	for (; i < 1000000; i++)
		r = a + i;
	sleep(1);
	std::cout << "task done: " << r << std::endl;
}

int main(void)
{
	ThreadPool *pool = new ThreadPool(4);

	signal(SIGINT, interruptHandler);

	while (running)
		;

	pool->kill();
}