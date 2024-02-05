/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWorker.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 15:35:51 by mgama             #+#    #+#             */
/*   Updated: 2024/02/05 16:20:33 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIWorker.hpp"

CGIWorker::CGIWorker(const std::string path): _path(path)
{
	this->_body = "";
}

CGIWorker::~CGIWorker()
{
}

void	CGIWorker::run(void)
{
	FILE *tmpin = tmpfile();
	FILE *tmpout = tmpfile();
	int fdin = fileno(tmpin);
	int fdout = fileno(tmpout);

	write(fdin, this->_body.c_str(), this->_body.length());
	lseek(fdin, 0, SEEK_SET);
	
	pid_t pid = fork();
	if (pid == 0)
	{
		dup2(fdin, STDIN_FILENO);
		dup2(fdout, STDOUT_FILENO);
		close(fdin);
		close(fdout);
		execl(this->_path.c_str(), this->_path.c_str(), NULL);
		exit(0);
	}
	else
	{
		char buffer[CGIWORKER_BUFFER_SIZE] = {0};

		waitpid(pid, NULL, 0);
		lseek(fdout, 0, SEEK_SET);
		int len = read(fdout, buffer, CGIWORKER_BUFFER_SIZE);
		buffer[len] = 0;
		std::cout << buffer;
	}
}
