/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ThreadPool.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/16 13:31:28 by mgama             #+#    #+#             */
/*   Updated: 2024/06/18 00:01:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ThreadPool.hpp"
#include "cluster/Cluster.hpp"

ThreadPool::ThreadPool(size_t numThreads) : stop(false)
{
	pthread_mutex_init(&queueMutex, NULL);
	pthread_cond_init(&condition, NULL);

	for (size_t i = 0; i < numThreads; ++i) {
		pthread_t worker;
		pthread_create(&worker, NULL, workerThread, this);
		workers.push_back(worker);
	}
}

ThreadPool::~ThreadPool()
{
	if (!stop) {
		this->kill();
	}	
}

void    ThreadPool::kill()
{
	{
		pthread_mutex_lock(&queueMutex);
		stop = true;
		pthread_cond_broadcast(&condition);
		pthread_mutex_unlock(&queueMutex);
	}

	for (size_t i = 0; i < workers.size(); ++i) {
        pthread_cancel(workers[i]);
    }

	for (size_t i = 0; i < workers.size(); ++i) {
		pthread_join(workers[i], NULL);
	}

	pthread_mutex_destroy(&queueMutex);
	pthread_cond_destroy(&condition);
	Logger::debug("ThreadPool stopped");
}

void    ThreadPool::enqueueTask(void (*function)(int, int), int client_fd, int backend_fd)
{
	pthread_mutex_lock(&queueMutex);
	tasks.push(function);
	taskArgs.push(std::make_pair(client_fd, backend_fd));
	pthread_cond_signal(&condition);
	pthread_mutex_unlock(&queueMutex);
}

void    *ThreadPool::workerThread(void* arg)
{
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	pthread_sigmask(SIG_BLOCK, &mask, NULL);

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	ThreadPool* pool = (ThreadPool*)arg;
	pool->run();
	return NULL;
}

void    ThreadPool::run()
{
	while (true) {
		pthread_mutex_lock(&queueMutex);

		std::cout << "thread stop: " << stop << " " << Cluster::exit << std::endl;
		while (!stop && tasks.empty()) {
			pthread_cond_wait(&condition, &queueMutex);
		}

		std::cout << "thread stop: " << stop << " " << Cluster::exit << std::endl;
		if (stop || Cluster::exit) {
			pthread_mutex_unlock(&queueMutex);
			return;
		}

		void (*task)(int, int) = tasks.front();
		std::pair<int, int> args = taskArgs.front();
		tasks.pop();
		taskArgs.pop();

		pthread_mutex_unlock(&queueMutex);

		pthread_testcancel();

		task(args.first, args.second);

		pthread_testcancel();
	}
}
