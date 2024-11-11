/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ThreadPool.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/16 13:31:28 by mgama             #+#    #+#             */
/*   Updated: 2024/11/11 13:33:40 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ThreadPool.hpp"
#include "cluster/Cluster.hpp"

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

	// for (size_t i = 0; i < workers.size(); ++i) {
	// 	std::cout << "canceling thread " << i << std::endl;
	// 	pthread_cancel(workers[i]);
	// }

	for (size_t i = 0; i < workers.size(); ++i) {
		std::cout << "joining thread" << std::endl;
		pthread_join(workers[i], NULL);
	}

	pthread_mutex_destroy(&queueMutex);
	pthread_cond_destroy(&condition);
	Logger::debug("ThreadPool stopped");
}

void	ThreadPool::enqueueTask(void (*function)(int, int), int client_fd, int backend_fd)
{
	pthread_mutex_lock(&queueMutex);
	tasks.push(function);
	taskArgs.push(std::make_pair(client_fd, backend_fd));
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
	while (true) {
		std::cout << "thread lock" << std::endl;
		pthread_mutex_lock(&queueMutex);
		std::cout << "thread aquired" << std::endl;

		std::cout << "thread stop: " << stop << " " << Cluster::exit << std::endl;
		while (!stop && tasks.empty()) {
			std::cout << "thread wait" << std::endl;
			pthread_cond_wait(&condition, &queueMutex);
			std::cout << "thread woke up" << std::endl;
		}

		std::cout << "after wait thread stop: " << stop << " " << Cluster::exit << std::endl;
		if (stop) {
			std::cout << "stop thread" << std::endl;
			pthread_mutex_unlock(&queueMutex);
			return;
		}

		void (*task)(int, int) = tasks.front();
		std::pair<int, int> args = taskArgs.front();
		tasks.pop();
		taskArgs.pop();

		std::cout << "thread unlock" << std::endl;
		pthread_mutex_unlock(&queueMutex);
		
		// std::cout << "test cancel before task" << std::endl;
		// pthread_testcancel();

		std::cout << "thread task" << std::endl;
		task(args.first, args.second);

		// std::cout << "thread cancel after task" << std::endl;
		// pthread_testcancel();
	}
}
