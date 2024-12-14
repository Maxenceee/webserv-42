/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ThreadPool.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/16 13:31:50 by mgama             #+#    #+#             */
/*   Updated: 2024/12/14 20:15:54 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef THREADPOOL_HPP
# define THREADPOOL_HPP

#include "webserv.hpp"

typedef void* wbs_threadpool_client;
typedef int wbs_threadpool_backend;

class ThreadPool
{
public:
	ThreadPool(size_t numThreads);
	~ThreadPool();

	void	enqueueTask(void (*function)(wbs_threadpool_client, wbs_threadpool_backend), wbs_threadpool_client client, wbs_threadpool_backend backend_fd);

	void	kill(bool force = false);

private:
	std::vector<pthread_t> workers;
	std::queue<void (*)(wbs_threadpool_client, wbs_threadpool_backend)> tasks;
	std::queue<std::pair<wbs_threadpool_client, wbs_threadpool_backend> > taskArgs;

	pthread_mutex_t	queueMutex;
	pthread_cond_t	condition;
	bool			stop;
	bool 			available;

	static void* workerThread(void* arg);
	void run();
};

#endif /* THREADPOOL_HPP */
