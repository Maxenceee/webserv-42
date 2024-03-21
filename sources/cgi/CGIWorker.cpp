/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWorker.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 15:35:51 by mgama             #+#    #+#             */
/*   Updated: 2024/03/21 14:58:15 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIWorker.hpp"

std::string	formatHeaderKey(const std::string &key)
{
	std::string res = "HTTP_";
	res += to_upper(key);
	return (replaceAll(res, '-', '_'));
}

t_mapss		CGIWorker::init(const Request &req, const std::string &scriptpath, t_mapss &params, const std::string &body)
{
	t_mapss headers = req.getHeaders();
	t_mapss env;

	env["REDIRECT_STATUS"] = "200";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_SOFTWARE"] = W_SERVER_NAME;
	env["REQUEST_METHOD"] = req.getMethod();
	env["REQUEST_URI"] = req.getRawPath();
	env["SCRIPT_NAME"] = req.getPath();
	env["SCRIPT_FILENAME"] = scriptpath;
	env["PATH_TRANSLATED"] = req.getPath();
	env["PATH_INFO"] = req.getPath();
	env["QUERY_STRING"] = req.getQueryString();
	env["CONTENT_LENGTH"] = toString<int>(body.length());
	if (headers.count("Content-Type") && headers["Content-Type"].size() > 0)
		env["CONTENT_TYPE"] = headers["Content-Type"];
	env["COOKIE"] = req.getHeader("Cookie");
	env["SERVER_PORT"] = toString<int>(req.getPort());
	env["REMOTE_ADDR"] = req.getIP();
	env["REMOTE_PORT"] = toString<int>(req.getPort());
	if (headers.count("Hostname"))
		env["SERVER_NAME"] = headers["Hostname"];
	else
		env["SERVER_NAME"] = env["REMOTE_ADDR"];
	env["SERVER_HOST"] = req.getHost();
	/**
	 * Ajout des en-tête HTTP au CGI
	 */
	for (t_mapss::iterator it = headers.begin(); it != headers.end(); it++)
	{
		if (it->second.size() > 0)
			env[formatHeaderKey(it->first)] = it->second;
	}
	/**
	 * Ajout des paramètres de la requête au CGI
	 */
	for (t_mapss::iterator it = params.begin(); it != params.end(); it++)
	{
		env[it->first] = it->second;
	}
	return env;
}

char	**CGIWorker::getEnv(const t_mapss &_env)
{
	char **env = new char*[_env.size() + 1];
	int i = 0;

	for (t_mapss::const_iterator it = _env.begin(); it != _env.end(); it++)
	{
		std::string tmp = it->first + "=" + it->second;
		std::cout << tmp << std::endl;
		env[i] = strdup(tmp.c_str());
		i++;
	}
	env[i] = NULL;
	return env;
}

std::string		CGIWorker::run(const Request &req, const std::string &scriptpath, t_mapss &params, const std::string &scriptpname, const std::string &body)
{
	int		sstdin = dup(STDIN_FILENO);
	int		sstdout = dup(STDOUT_FILENO);
	char	**env;

	try
	{
		env = CGIWorker::getEnv(CGIWorker::init(req, scriptpath, params, body));
	}
	catch(const std::exception& e)
	{
		Logger::error("CGIWorker error: " + std::string(e.what()));
		return ("Status: 500\r\n\r\n");
	}

	Logger::debug("CGIWorker: running: " + scriptpname);
	Logger::debug("CGIWorker: with body: " + body);
	
	FILE *tmpin = tmpfile();
	FILE *tmpout = tmpfile();
	int fdin = fileno(tmpin);
	int fdout = fileno(tmpout);
	std::string result;

	write(fdin, body.c_str(), body.size());
	lseek(fdin, 0, SEEK_SET);
	
	pid_t pid = fork();
	if (pid == -1)
	{
		Logger::error("CGIWorker error: fork failed");
		return ("Status: 500\r\n\r\n");
	}
	if (pid == 0)
	{
		dup2(fdin, STDIN_FILENO);
		dup2(fdout, STDOUT_FILENO);
		execve(scriptpname.c_str(), (char * const *)NULL, env);
		Logger::error("CGIWorker error: execve failed");
		write(STDOUT_FILENO, "Status: 500\r\n\r\n", 15);
	}
	else
	{
		char buffer[CGIWORKER_BUFFER_SIZE] = {0};

		waitpid(pid, NULL, 0);
		lseek(fdout, 0, SEEK_SET);
		
		int r = 1;
		while (r > 0)
		{
			memset(buffer, 0, CGIWORKER_BUFFER_SIZE);
			r = read(fdout, buffer, CGIWORKER_BUFFER_SIZE);
			if (r > 0)
				result += buffer;
		}
	}

	dup2(sstdin, STDIN_FILENO);
	dup2(sstdout, STDOUT_FILENO);
	fclose(tmpin);
	fclose(tmpout);
	close(fdin);
	close(fdout);
	close(sstdin);
	close(sstdout);

	for (size_t i = 0; env[i]; i++)
		delete[] env[i];
	delete[] env;

	if (!pid)
		exit(0);

	return (result);
}
