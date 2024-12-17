/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ThreadPool.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/16 13:31:28 by mgama             #+#    #+#             */
/*   Updated: 2024/12/17 12:05:40 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ThreadPool.hpp"
#include "cluster/Cluster.hpp"

ThreadPool::ThreadPool(size_t numThreads): _stop(false), _available(true)
{
	if (numThreads == 0) {
		_available = false;
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
	if (_available && !_stop) {
		this->stop();
	}
}

void	ThreadPool::kill(void)
{
	if (!_stop) return;

	for (size_t i = 0; i < workers.size(); ++i) {
		pthread_cancel(workers[i]);
	}
}

void	ThreadPool::stop()
{
	if (_available == false || _stop == true) {
		return;
	}

	pthread_mutex_lock(&queueMutex);
	_stop = true;
	pthread_cond_broadcast(&condition);
	pthread_mutex_unlock(&queueMutex);

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += 10; 

	bool needForce = false;
	std::vector<bool> joined(workers.size(), false);

	/**
	 * Dans un premier temps, on essaie de joindre les threads qui ont terminé leur travail.
	 * On leur laisse 10s pour terminer.
	 */
	for (size_t i = 0; i < workers.size(); ++i) {
		int ret = pthread_timedjoin_np(workers[i], NULL, &timeout);
		if (ret == 0) {
			Logger::debug("Thread joined successfully: " + toString<int>(i));
			joined[i] = true;
		} else if (ret == ETIMEDOUT) {
			Logger::debug("Thread timed out: " + toString<int>(i));
			needForce = true;
		}
	}

	/**
	 * Si certains threads n'ont pas pu être joints, on les annule.
	 */
	if (needForce) {
		Logger::print("Forcing remaining workers to stop", B_GREEN);
        for (size_t i = 0; i < workers.size(); ++i) {
            if (!joined[i]){
                pthread_cancel(workers[i]);
            }
        }
	}

	/**
	 * Si certains threads n'ont pas pu être joints, une fois annulés, on les joint.
	 */
	for (size_t i = 0; i < workers.size(); ++i) {
		if (!joined[i]) {
			pthread_join(workers[i], NULL);
		}
	}

	for (size_t i = 0; i < tasks.size(); ++i) {
		delete tasks.front();
		tasks.pop();
	}

	pthread_mutex_destroy(&queueMutex);
	pthread_cond_destroy(&condition);
	Logger::debug("ThreadPool stopped");
	_available = false;
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

	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	pool->run();
	return NULL;
}

static void cleanup(void* arg)
{
	delete static_cast<ProxyWorker*>(arg);
}

void	ThreadPool::run()
{
	while (true) {
		pthread_mutex_lock(&queueMutex);

		while (!_stop && tasks.empty()) {
			pthread_cond_wait(&condition, &queueMutex);
		}

		if (_stop) {
			pthread_mutex_unlock(&queueMutex);
			return;
		}

		ProxyWorker *worker = tasks.front();
		tasks.pop();

		pthread_mutex_unlock(&queueMutex);

		/**
		 * Ajout d'un callback pour nettoyer les ressources du worker en cas d'annulation du thread.
		 */
		pthread_cleanup_push(cleanup, worker);

		worker->work();

		/**
		 * Nettoyage des ressources du worker.
		 */
		pthread_cleanup_pop(1);
	}
}
