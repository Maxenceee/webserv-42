/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ThreadPool.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/16 13:31:50 by mgama             #+#    #+#             */
/*   Updated: 2024/12/17 11:53:00 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef THREADPOOL_HPP
# define THREADPOOL_HPP

#include "webserv.hpp"
#include "proxy/ProxyWorker.hpp"

class ProxyWorker;

class ThreadPool
{
public:
	ThreadPool(size_t numThreads);
	~ThreadPool();

	void	enqueueWorker(ProxyWorker *worker);

	void	stop();

private:
	std::vector<pthread_t> workers;
	std::queue<ProxyWorker *> tasks;

	pthread_mutex_t	queueMutex;
	pthread_cond_t	condition;
	bool			_stop;
	bool 			_available;

	static void* workerThread(void* arg);
	void run();

	void	kill(void);
};

#endif /* THREADPOOL_HPP */
