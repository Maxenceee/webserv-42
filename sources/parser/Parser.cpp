/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:18:32 by mgama             #+#    #+#             */
/*   Updated: 2024/03/21 13:56:33 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Parser.hpp"

#define PARSER_ERR		"parser error: invalid file path"

Parser::Parser(Cluster &c): cluster(c)
{
	this->new_server = NULL;
}

Parser::~Parser(void)
{
}

int		Parser::open_and_read_file(const char *file_name)
{
	std::string		line;
	std::vector<std::string>	tmp;
	
	this->file.open(file_name, std::ios::in);
	if (!this->file) {
		Logger::error("parser error: Could not open file " + std::string(file_name), RESET);
		return (EXIT_FAILURE);
	}

	while (getline(this->file, line))
	{
		tmp.push_back(line);
	}
	this->buffer = join(tmp, "\n");
	return (EXIT_SUCCESS);
}

void	Parser::parse(const char *configPath)
{
	if (!isFile(configPath))
		throw std::invalid_argument(PARSER_ERR);
	this->open_and_read_file(configPath);
	this->extract(this->buffer);
	this->cluster.initConfigs(this->configs);
}

void	Parser::processInnerLines(std::string &lineRaw, std::string &chunkedLine, std::string &parent)
{
	std::vector<std::string> innerLines = split(lineRaw, '\n');

	for (std::vector<std::string>::iterator innerIt = innerLines.begin(); innerIt != innerLines.end(); ++innerIt) {
		std::string line = *innerIt;
		trim(line);
		line = chunkedLine.empty() ? line : chunkedLine + " " + line;

		// Object opening line
		if (line[line.length() - 1] == '{') {
			chunkedLine = "";
			std::string key = line.substr(0, line.length() - 1);
			trim(key);
			std::vector<std::string> tokens = split(key, ' ');
			key = tokens[0];
			trim(key);
			shift(tokens);
			std::string val = join(tokens, " ");
			if (!parent.empty()) {
				parent += '.' + key;
			} else {
				parent = key;
			}
			Logger::debug(YELLOW + key + RESET + "\t" + val + "\t" + GREEN + parent + RESET);
			this->switchConfigDirectives(key, val, parent, line);
		}
		// Standard property line
		else if (line[line.length() - 1] == ';') {
			chunkedLine = "";
			std::istringstream iss(line);
			std::vector<std::string> tokens;
			std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(tokens));
			std::string key = tokens[0];
			tokens.erase(tokens.begin());
			std::string val = join(tokens, " ");
			trim(key);
			trim(val);
			if (key[key.length() - 1] == ';') {
				pop(key);
			}
			pop(val);
			Logger::debug(RED + key + RESET + "\t" + val + "\t" + GREEN + parent + RESET);
			this->switchConfigDirectives(key, val, parent, line);
		}
		// Object closing line
		else if (line[line.length() - 1] == '}') {
			chunkedLine = "";
			std::vector<std::string> parentTokens = split(parent, '.');
			parentTokens.pop_back();
			parent = join(parentTokens, ".");
			this->tmp_router = this->tmp_router->getParent();
		}
		// Line may not contain '{' ';' '}' symbols at the end
		else {
			chunkedLine = line;
		}
	}
}

void	Parser::extract(const std::string &conf)
{
	std::vector<std::string> lines = split(conf, '\n');
	std::string parent = "";
	std::string chunkedLine = "";

	for (std::vector<std::string>::iterator lineIt = lines.begin(); lineIt != lines.end(); ++lineIt) {
		std::string lineRaw = *lineIt;
		trim(lineRaw);

		// If line is blank line or is comment, do not process it
		if (lineRaw.empty() || lineRaw[0] == '#') continue;

		// Line can contain comments, we need to remove them
		lineRaw = split(lineRaw, '#')[0];
		trim(lineRaw);
		replace(lineRaw, "\t", " ");
		replaceAll(lineRaw, ' ', ' ');
		replace(lineRaw, "{", "{\n");
		// replace(lineRaw, "}", "\n}");
		this->processInnerLines(lineRaw, chunkedLine, parent);
	}
}

void	Parser::throwError(const std::string key, const std::string val, const std::string raw_line)
{
	std::string	tmp("parser error: invalid directive found");
	if (raw_line.size() > 0) {
		tmp += RESET;
		tmp += "\n\t" + raw_line;
		tmp += "\n"B_GREEN"\t^\n"RESET;
		tmp += "1 error generated.";
	}
	else
		tmp += ": " + key + " " + val;
	tmp += RESET;
	throw std::invalid_argument(tmp.c_str());
}

void	Parser::switchConfigDirectives(std::string key, std::string val, const std::string parent, const std::string raw_line)
{
	/**
	 * Directive server
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#server)
	 */
	if (!this->new_server && key != "server")
		this->throwError(key, val, raw_line);
	else if (key == "server") {
		// On empeche l'imbrication de blocs server
		if (parent != "server")
			this->throwError(key, val, raw_line);
		this->new_server = new ServerConfig();
		this->configs.push_back(this->new_server);
		this->tmp_router = this->new_server->getDefaultHandler();
	}
	/**
	 * Directive location
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#location)
	 */
	else if (key == "location") {
		this->createNewRouter(key, val, parent, raw_line);
	}
	else
	{
		if (parent == "server")
			this->tmp_router = this->new_server->getDefaultHandler();
		if (val.empty())
			this->throwError(key, val, raw_line);
		this->addRule(key, val, parent, raw_line);
	}
}

void	Parser::createNewRouter(std::string key, std::string val, const std::string parent, const std::string raw_line)
{
	struct s_Router_Location	location;

	std::vector<std::string> tokens = split(val, ' ');
	if (tokens.size() > 2 || tokens.size() < 1)
		this->throwError(key, val, raw_line);
	trim(tokens[0]);
	if (tokens.size() == 2 && this->isValidModifier(tokens[0])) {
		location.modifier = tokens[0];
		if (location.modifier == "=")
			location.strict = true;
	}
	else if (tokens.size() == 2)
		this->throwError(key, val, raw_line);
	location.path = trim(tokens[tokens.size() - 1]);
	Router *tmp = this->tmp_router;
	this->tmp_router = new Router(tmp, location);
	if (parent == "server.location")
		this->new_server->use(this->tmp_router);
	else
		tmp->use(this->tmp_router);
	const struct s_Router_Location &parent_l = tmp->getLocation();
	const struct s_Router_Location &child_l = this->tmp_router->getLocation();
	if ((!parent_l.modifier.empty() && parent_l.modifier != "^~" && (child_l.modifier.empty() || child_l.modifier == "^~"))
		|| (((parent_l.modifier.empty() || parent_l.modifier == "^~") && (child_l.modifier.empty() || child_l.modifier == "^~"))
			&& (child_l.path.size() < parent_l.path.size() || child_l.path.substr(0, parent_l.path.size()) != parent_l.path))) {
		Logger::error("parser error: location \""+child_l.path+"\" is outside location \""+parent_l.path+"\"");
		this->throwError(key, val, raw_line);
	}
}

void	Parser::addRule(const std::string key, const std::string val, const std::string parent, const std::string raw_line)
{
	/**`
	 * Directive Listen
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#listen)
	 */
	if (key == "listen" && parent != "server")
		this->throwError(key, val);
	else if (key == "listen") {
		// check if there is a column in the value
		if (val.find(':') == std::string::npos) {
			if (!isDigit(val))
				this->throwError(key, val);
			this->new_server->setPort(std::atoi(val.c_str()));
		} else {
			std::vector<std::string> tokens = split(val, ':');
			if (tokens.size() == 2) {
				if (!isIPAddress(tokens[0]) || !isDigit(tokens[1]))
					this->throwError(key, val);
				this->new_server->setAddress(tokens[0]);
				this->new_server->setPort(std::atoi(tokens[1].c_str()));
			}
			else
				this->throwError(key, val);
		}
		return ;
	}

	/**
	 * Directive server_name
	 * (https://nginx.org/en/docs/http/server_names.html)
	 */
	if (key == "server_name" && parent != "server")
		this->throwError(key, val);
	else if (key == "server_name") {
		std::vector<std::string> names = split(val, ' ');
		this->new_server->addNames(names);
		return ;
	}

	/**
	 * Directive root/alias
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#root)
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#alias)
	 */
	if (key == "root") {
		this->tmp_router->setRoot(val);
		return ;
	}
	else if (key == "alias" && parent != "server") {
		this->tmp_router->setAlias(val);
		return ;
	} else if (key == "alias") {
		this->throwError(key, val);
		return ;
	}

	/**
	 * Directive index
	 * (https://nginx.org/en/docs/http/ngx_http_index_module.html#index)
	 */
	if (key == "index") {
		/**
		 */
		std::vector<std::string> index = split(val, ' ');
		this->tmp_router->setIndex(index);
		return ;
	}

	/**
	 * Directive autoindex
	 * (https://nginx.org/en/docs/http/ngx_http_autoindex_module.html#autoindex)
	 */
	if (key == "autoindex") {
		if (val == "on")
			this->tmp_router->setAutoIndex(true);
		else if (val != "off")
			this->throwError(key, val);
		return ;
	}

	/**
	 * Directive return
	 * (https://nginx.org/en/docs/http/ngx_http_rewrite_module.html#return)
	 */
	if (key == "return") {
		int status = 302;
		std::string loc = val;
		if (val.find(' ') != std::string::npos) {
			std::vector<std::string> tokens = split(val, ' ');
			if (tokens.size() == 2) {
				if (!isDigit(tokens[0]))
					this->throwError(key, val);
				status = std::atoi(tokens[0].c_str());
				loc = tokens[1];
			}
			else if (tokens.size() > 2)
				this->throwError(key, val);
		} else if (isDigit(val)) {
			status = std::atoi(val.c_str());
			loc = "";
		}
		if (!Response::isValidStatus(status)) {
			Logger::warning("parser warning: invalid status code, this may cause unexpected behavior.");
		}
		this->tmp_router->setRedirection(loc, status);
		return ;
	}

	/**
	 * Directive allow_methods
	 */
	if (key == "allow_methods") {
		std::vector<std::string> tokens = split(val, ' ');
		for (size_t i = 0; i < tokens.size(); i++) {
			if (!Server::isValidMethod(tokens[i]))
				this->throwError(key, val);
			this->tmp_router->allowMethod(tokens[i]);
		}
		return ;
	}

	/**
	 * Directive error_page
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#error_page)
	 */
	if (key == "error_page") {
		/**
		 * TODO:
		 * regarder commment fonctionne le paramettre [=[response]]
		 */
		std::vector<std::string> tokens = split(val, ' ');
		if (tokens.size() < 2)
			this->throwError(key, val);
		for (size_t i = 0; i < tokens.size() - 1; i++) {
			if (!isDigit(tokens[i])) {
				this->throwError(key, val);
			}
			this->tmp_router->setErrorPage(std::atoi(tokens[i].c_str()), tokens[tokens.size() - 1]);
		}
		return ;
	}

	/**
	 * Directive client_max_body_size
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#client_max_body_size)
	 */
	if (key == "client_max_body_size") {
		this->tmp_router->setClientMaxBodySize(val);
		return ;
	}

	/**
	 * Directive cgi_extension
	 * see fastcgi_pass
	 */
	// if (key == "cgi") {
	// 	if (val == "on")
	// 		this->tmp_router->enableCGI();
	// 	return ;
	// }

	/**
	 * Directive fastcgi_pass
	 * (https://nginx.org/en/docs/http/ngx_http_fastcgi_module.html#fastcgi_pass)
	 */
	if (key == "fastcgi_pass") {
		this->tmp_router->setCGI(val);
		return ;
	}

	/**
	 * Directive fastcgi_param
	 * (https://nginx.org/en/docs/http/ngx_http_fastcgi_module.html#fastcgi_param)
	 */
	if (key == "fastcgi_param") {
		/**
		 * TODO:
		 * tester avec des sockets unix
		 */
		std::vector<std::string> tokens = split(val, ' ');
		if (tokens.size() < 2)
			this->throwError(key, val);
		this->tmp_router->addCGIParam(tokens[0], tokens[1]);
		return ;
	}

	/**
	 * Directive add_header
	 * (https://nginx.org/en/docs/http/ngx_http_headers_module.html#add_header)
	 */
	if (key == "add_header") {
		std::vector<std::string> tokens = split(val, ' ');
		bool always = false;
		if (tokens.size() < 2 || tokens.size() > 3)
			this->throwError(key, val);
		if (tokens.size() == 3) {
			if (tokens[2] != "always")
				this->throwError(key, val);
			always = true;
		}
		this->tmp_router->addHeader(tokens[0], tokens[1], always);
		return ;
	
	}

	/**
	 * Default
	 */
	this->throwError(key, val, raw_line);
}

bool	Parser::isValidModifier(const std::string &modifier) const
{
	return (modifier == "=" || modifier == "~" || modifier == "~*" || modifier == "^~");
}
