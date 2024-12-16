/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ThreadPool.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/16 13:31:28 by mgama             #+#    #+#             */
/*   Updated: 2024/12/16 14:16:19 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ThreadPool.hpp"
#include "cluster/Cluster.hpp"

ThreadPool::ThreadPool(size_t numThreads): stop(false), available(true)
{
	if (numThreads == 0) {
		available = false;
		return;
	}
	pthread_mutex_init(&queueMutex, NULL);
	pthread_cond_init(&condition, NULL);

	for (size_t i = 0; i < numThreads; ++i) {
		pthread_t worker;
		pthread_create(&worker, NULL, workerThread, this);
		workers.push_back(worker);
	}
	Logger::debug("ThreadPool started with " + toString<int>(numThreads) + " threads");
}

ThreadPool::~ThreadPool()
{
	if (available && !stop) {
		this->kill();
	}
}

void	ThreadPool::kill(bool force)
{
	if (available == false || stop == true) {
		return;
	}

	pthread_mutex_lock(&queueMutex);
	stop = true;
	pthread_cond_broadcast(&condition);
	pthread_mutex_unlock(&queueMutex);

#ifdef __APPLE__
	if (force) {
		Logger::print("Forcing workers to finish", B_GREEN);
		for (size_t i = 0; i < workers.size(); ++i) {
			pthread_cancel(workers[i]);
		}
	} else {
		Logger::print("Waiting for workers to finish", B_GREEN);
	}
#else
	(void)force;
	Logger::print("Waiting for workers to finish", B_GREEN);
# endif /* __APPLE__ */

	for (size_t i = 0; i < workers.size(); ++i) {
		pthread_join(workers[i], NULL);
	}

	for (size_t i = 0; i < tasks.size(); ++i) {
		delete tasks.front();
		tasks.pop();
	}

	pthread_mutex_destroy(&queueMutex);
	pthread_cond_destroy(&condition);
	Logger::debug("ThreadPool stopped");
	available = false;
}

void	ThreadPool::enqueueWorker(ProxyWorker *worker)
{
	pthread_mutex_lock(&queueMutex);
	tasks.push(worker);
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
		pthread_mutex_lock(&queueMutex);

		while (!stop && tasks.empty()) {
			pthread_cond_wait(&condition, &queueMutex);
		}

		if (stop) {
			pthread_mutex_unlock(&queueMutex);
			return;
		}

		ProxyWorker *worker = tasks.front();
		tasks.pop();

		pthread_mutex_unlock(&queueMutex);

		worker->work();
		delete worker;
	}
}
