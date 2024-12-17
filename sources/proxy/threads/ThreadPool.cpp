/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ThreadPool.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/16 13:31:28 by mgama             #+#    #+#             */
/*   Updated: 2024/12/17 19:58:22 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ThreadPool.hpp"
#include "cluster/Cluster.hpp"

ThreadPool::ThreadPool(size_t numThreads): _stop(false), _available(true)
{
	if (numThreads == 0) {
		this->_available = false;
		return;
	}
	pthread_mutex_init(&this->_queueMutex, NULL);
	pthread_cond_init(&this->_condition, NULL);

	for (size_t i = 0; i < numThreads; ++i) {
		pthread_t worker;
		pthread_create(&worker, NULL, workerThread, this);
		this->_workers.push_back(worker);
	}
	/**
	 * Initialisation du vecteur de threads joints, on doit lui réserver la taille
	 * nécessaire à l'avance, car une fois un signal reçu, on ne pouura plus modifier
	 * sa taille, car cela necessite une allocation mémoire.
	 * C'est un mesure préventive pour éviter les problèmes.
	 */
	this->_joined.resize(this->_workers.size(), false);
	Logger::debug("ThreadPool started with " + toString(numThreads) + " threads");
}

ThreadPool::~ThreadPool()
{
	if (this->_available && !this->_stop) {
		this->stop();
	}
}

void	ThreadPool::kill(void)
{
	if (!this->_stop) {
		Logger::error("ThreadPool is not stopped, please stop pool before.");
		return;
	}

	Logger::print("Forcing remaining workers to stop", B_GREEN);
	for (size_t i = 0; i < this->_workers.size(); ++i) {
		if (!this->_joined[i]){
			pthread_cancel(this->_workers[i]);
		}
	}
}

void	ThreadPool::stop(void)
{
	if (this->_available == false || this->_stop == true) {
		return;
	}

	pthread_mutex_lock(&this->_queueMutex);
	this->_stop = true;
	pthread_cond_broadcast(&this->_condition);
	pthread_mutex_unlock(&this->_queueMutex);

	bool needForce = false;

#ifdef __APPLE__

	/**
	 * Sur MacOS, pthread_timedjoin_np n'est pas disponible, en attendant une solution, on force l'arrêt
	 * des threads.
	 */
	this->kill();

#else

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += 10; 

	/**
	 * Dans un premier temps, on essaie de joindre les threads.On leur laisse 10s pour terminer.
	 */
	for (size_t i = 0; i < this->_workers.size(); ++i) {
		int ret = pthread_timedjoin_np(this->_workers[i], NULL, &timeout);
		if (ret == 0) {
			Logger::debug("Thread joined successfully: " + toString(i));
			this->_joined[i] = true;
		} else if (ret == ETIMEDOUT) {
			Logger::debug("Thread timed out: " + toString(i));
			needForce = true;
		}
	}

#endif /* __APPLE__ */

	/**
	 * Si certains threads n'ont pas pu être joints, on les annule pour forcer
	 * leur arrêt.
	 */
	if (needForce) {
		Logger::print("Webserve did not stop within 10s", B_GREEN);
		this->kill();
	}

	/**
	 * Si certains threads n'ont pas pu être joints, une fois annulés, on les joint.
	 */
	for (size_t i = 0; i < this->_workers.size(); ++i) {
		if (!this->_joined[i]) {
			pthread_join(this->_workers[i], NULL);
		}
	}

	for (size_t i = 0; i < this->_tasks.size(); ++i) {
		delete this->_tasks.front();
		this->_tasks.pop();
	}

	pthread_mutex_destroy(&this->_queueMutex);
	pthread_cond_destroy(&this->_condition);
	_available = false;
	Logger::debug("ThreadPool stopped");
}

void	ThreadPool::enqueueWorker(ProxyWorker *worker)
{
	pthread_mutex_lock(&this->_queueMutex);
	this->_tasks.push(worker);
	pthread_cond_signal(&this->_condition);
	pthread_mutex_unlock(&this->_queueMutex);
}

void	*ThreadPool::workerThread(void *arg)
{
	/**
	 * Configuration du thread pour qu'il puisse être annulé.
	 */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	pool->run();
	return NULL;
}

static void cleanup(void* arg)
{
	/**
	 * Fonction de nettoyage pour les ressources du worker, appelée lors de l'annulation du thread.
	 */
	delete static_cast<ProxyWorker*>(arg);
}

void	ThreadPool::run()
{
	while (true) {
		pthread_mutex_lock(&this->_queueMutex);

		while (!this->_stop && this->_tasks.empty()) {
			/**
			 * Pour éviter les problèmes de concurrence, on utilise une this->_condition pour
			 * bloquer les threads tant qu'aucune tâche n'est disponible.
			 */
			pthread_cond_wait(&this->_condition, &this->_queueMutex);
		}

		if (this->_stop) {
			pthread_mutex_unlock(&this->_queueMutex);
			return;
		}

		ProxyWorker *worker = this->_tasks.front();
		this->_tasks.pop();

		pthread_mutex_unlock(&this->_queueMutex);

		/**
		 * Ajout d'un callback pour nettoyer les ressources du worker en cas d'annulation du thread.
		 */
		pthread_cleanup_push(cleanup, worker);

		/**
		 * Exécution de la routine de travail du worker.
		 */
		worker->work();

		/**
		 * Nettoyage des ressources du worker.
		 */
		pthread_cleanup_pop(1);
	}
}
