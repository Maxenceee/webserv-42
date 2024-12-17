/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ThreadPool.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/16 13:31:50 by mgama             #+#    #+#             */
/*   Updated: 2024/12/17 15:50:37 by mgama            ###   ########.fr       */
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

	void	stop(void);
	void	kill(void);

private:
	std::vector<bool> 			_joined;
	std::vector<pthread_t>		_workers;
	std::queue<ProxyWorker *>	_tasks;

	pthread_mutex_t	_queueMutex;
	pthread_cond_t	_condition;
	bool			_stop;
	bool 			_available;

	static void* workerThread(void* arg);
	void run();

};

#endif /* THREADPOOL_HPP */
