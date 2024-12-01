/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ThreadPool.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/16 13:31:50 by mgama             #+#    #+#             */
/*   Updated: 2024/12/01 21:11:05 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef THREADPOOL_HPP
# define THREADPOOL_HPP

#include "webserv.hpp"

class ThreadPool
{
public:
	ThreadPool(size_t numThreads);
	~ThreadPool();

	void	enqueueTask(void (*function)(int, int), int client_fd, int backend_fd);

	void	kill(bool force = false);

private:
	std::vector<pthread_t> workers;
	std::queue<void (*)(int, int)> tasks;
	std::queue<std::pair<int, int> > taskArgs;

	pthread_mutex_t	queueMutex;
	pthread_cond_t	condition;
	bool			stop;
	bool 			available;

	static void* workerThread(void* arg);
	void run();
};

#endif /* THREADPOOL_HPP */
