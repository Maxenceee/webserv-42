/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:18:32 by mgama             #+#    #+#             */
/*   Updated: 2025/01/05 14:49:44 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Parser.hpp"

Parser::Parser(Cluster &c): cluster(c)
{
	this->new_server = NULL;
}

Parser::~Parser(void)
{
	Logger::debug("Parser destroyed");
	for (std::vector<ServerConfig *>::iterator it = this->configs.begin(); it != this->configs.end(); it++) {
		if ((*it)->used == false)
			delete *it;
	}
}

int		Parser::open_and_read_file(const std::string &file_name)
{
	std::string	line;
	
	this->file.open(file_name.c_str(), std::ios::in);
	if (!this->file) {
		Logger::error("parser error: Could not open file " + std::string(file_name));
		return (EXIT_FAILURE);
	}

	while (getline(this->file, line))
	{
		// Toutes les tabulations sont remplacées par des espaces pour éviter les erreurs d'espacement
		this->buffer.push_back(replaceAll(line, '\t', ' '));
	}
	this->file.close();
	return (EXIT_SUCCESS);
}

void	Parser::parse(const std::string &configPath)
{
	if (!isFile(configPath))
	{
		Logger::error("parser error: could not open file " + configPath);
		throw std::invalid_argument("parser error: invalid file path");
	}
	Logger::debug("Parsing: start reading from " + configPath, B_GREEN);
	this->open_and_read_file(configPath);
	Logger::debug("Parsing: extracting data", B_GREEN);
	this->extract();
	if (!this->new_server)
	{
		throw std::invalid_argument("No server found in the configuration file");
	}
	this->cluster.initConfigs(this->configs);
	Logger::debug("Parsing: done", B_GREEN);
}

void	Parser::extract(void)
{
	std::vector<std::string> context;
	std::string chunkedLine = "";
	int braceCount = 0;
	std::string lastProcessedLine;

	for (std::vector<std::string>::iterator lineIt = this->buffer.begin(); lineIt != this->buffer.end(); ++lineIt) {
		std::string lineRaw = *lineIt;
		trim(lineRaw);

		// Si le ligne est vide ou si elle commence par un commentaire, on l'ignore
		if (lineRaw.empty() || lineRaw[0] == '#') continue;

		// Accumuler les lignes tant qu'on ne rencontre pas un '{', '}' ou ';'
		chunkedLine += (chunkedLine.empty() ? "" : " ") + lineRaw;

		// Si la ligne contient une accolade ouvrante, fermante ou un point-virgule
		if (lineRaw.find('{') != std::string::npos || lineRaw.find('}') != std::string::npos || lineRaw.find(';') != std::string::npos) {
			std::vector<std::string> tokens;

			try
			{
				tokens = tokenize(chunkedLine);
			}
			catch(...)
			{
				this->throwError(chunkedLine, "expected '\"'", chunkedLine.length() - 1);
			}

			// Vérification et mise à jour du compteur d'accolades
			for (size_t i = 0, j = 0; i < tokens.size(); i++) {
				if (tokens[i] == "{") {
					braceCount++;  // Incrémenter lors d'une accolade ouvrante
				} else if (tokens[i] == "}") {
					braceCount--;  // Décrémenter lors d'une accolade fermante
				}

				if (braceCount < 0) {
					this->throwError(join(tokens, " "), "unexpected '}'", j);
				}

				j += tokens[i].length() + 1;
			}

			if (!tokens.empty()) {
				lastProcessedLine = lineRaw; // Stock la dernière ligne traitée pour les erreurs

				// Vérifier si le compteur d'accolades est cohérent
				if (braceCount < 0 && context.size() == 0) {
					this->throwError(lastProcessedLine, "unexpected '}'", lastProcessedLine.length() - 1);
				}

				// Pass the tokens to be processed by processInnerLines
				this->processInnerLines(tokens, context);
			}

			// Réinitialiser la ligne accumulée une fois traitée
			chunkedLine.clear();
		}
	}

	if (!chunkedLine.empty()) {
		this->throwError(chunkedLine, "unterminated directive, missing ';'", chunkedLine.length() - 1);
	}

	if (braceCount != 0) {
		this->throwError(lastProcessedLine, "expected '}'", lastProcessedLine.length() - 1);
	}
}

void	Parser::processInnerLines(std::vector<std::string> &tokens, std::vector<std::string> &context)
{
	std::string chunkedLine;
	size_t lineLength = 0;
	std::string joined = join(tokens, " ");

	for (size_t i = 0; i < tokens.size(); ++i) {
		std::string token = tokens[i];
		lineLength += token.length();

		// Ouverture de bloc
		if (token == "{") {
			std::string key = readDirectiveKey(chunkedLine);
			if (key.empty()) {
				this->throwError(joined, "invalid directive found", lineLength + i - 1);
			}
			std::string l_context = last(context);
			Logger::debug(RED + key + RESET + " " + GREEN + chunkedLine + RESET + " " + l_context);

			this->addContextualRule(key, chunkedLine, l_context, lineLength + i - 1, joined);
			context.push_back(key);

			chunkedLine.clear(); // Si la ligne est traitée, on la réinitialise
		}
		// Fermeture de bloc
		else if (token == "}") {
			pop(context);
			if (this->tmp_router)
				this->tmp_router = this->tmp_router->getParent();
			if (!chunkedLine.empty()) {
				this->throwError(joined, "unterminated directive, missing ';'", lineLength + i - 1);
			}
			chunkedLine.clear(); // Si la ligne est traitée, on la réinitialise
		}
		// Fin de ligne
		else if (token == ";") {
			std::string key = readDirectiveKey(chunkedLine);
			if (key.empty()) {
				this->throwError(joined, "invalid directive found", lineLength + i - 1);
			}
			std::string l_context = last(context);
			Logger::debug(RED + key + RESET + " " + GREEN + chunkedLine + RESET + " " + l_context);

			std::vector<std::string> keyValTokens = parseQuotedAndSplit(chunkedLine);

			this->addNonContextualRule(key, keyValTokens, chunkedLine.length(), l_context, lineLength + i - 1, joined);
			chunkedLine.clear(); // Si la ligne est traitée, on la réinitialise
		}
		// On accumule si la ligne ne contient pas de token de fin
		else {
			if (!chunkedLine.empty()) {
				chunkedLine += " ";
			}
			chunkedLine += token;
		}
	}
}

void	Parser::throwError(const std::string &raw_line, const char *message, const int pos)
{
	std::string	tmp("parser error: ");
	tmp += message;
	tmp += RESET;
	tmp += "\n\t" + raw_line;
	tmp += "\n";
	tmp += B_GREEN;
	tmp += "\t"+std::string(pos, ' ')+"^\n";
	tmp += RESET;
	tmp += "1 error generated.";
	tmp += RESET;
	throw std::invalid_argument(tmp.c_str());
}

void	Parser::throwError(const std::string &raw_line, const std::string &message, const int pos)
{
	this->throwError(raw_line, message.c_str(), pos);
}

void	Parser::minmaxArgs(const std::string &raw_line, const size_t key_length, const size_t vallength, const std::vector<std::string> &valtokens, const size_t min, const size_t max)
{
	size_t pos = 0;

	if (valtokens.size() < min) {
		pos = key_length + vallength;

		this->throwError(raw_line, "too few arguments", pos);
	} else if (max != 0 && valtokens.size() > max) {
		pos = key_length + vallength - valtokens.back().length();

		this->throwError(raw_line, "too many arguments", pos);
	}
}

bool	Parser::onoffArgs(const std::string &raw_line, const size_t key_length, const size_t vallength, const std::vector<std::string> &valtokens)
{
	(void)vallength;

	if (valtokens[0] == "on")
		return (true);
	else if (valtokens[0] != "off")
		this->throwError(raw_line, "unknown option", key_length);

	return (false);
}

void	Parser::createNewRouter(const std::string &key, const std::string &val, const std::string &context, const size_t processed, const std::string &raw_line)
{
	struct wbs_router_location	location;

	if (context != "server" && context != "location")
		this->throwError(raw_line, "location directive must be inside server or location block", processed - key.length() - 1);

	std::vector<std::string> tokens = split(val, ' ');
	if (tokens.size() < 1)
		this->throwError(raw_line, "missing location path", processed);
	else if (tokens.size() > 2)
		this->throwError(raw_line, "too many entries", processed + val.length() - tokens.back().length());

	trim(tokens[0]);
	if (tokens.size() == 2 && this->isValidModifier(tokens[0])) {
		location.modifier = tokens[0];
		if (location.modifier == "=")
			location.strict = true;
	}
	else if (tokens.size() == 2) {
		this->throwError(raw_line, "invalid modifier", processed);
	}
	location.path = trim(tokens.back());

	Router *tmp = this->tmp_router;
	this->tmp_router = new Router(tmp, location, tmp->level + 1);

	/**
	 * Si le context est `server`, on utilise le routeur par défaut du serveur.
	 * Sinon, on imbrique le routeur enfant dans le routeur parent.
	 */
	if (context == "server")
		this->new_server->use(this->tmp_router);
	else
		tmp->use(this->tmp_router);

	const struct wbs_router_location &parent_l = tmp->getLocation();
	const struct wbs_router_location &child_l = this->tmp_router->getLocation();

	/**
	 * Cette horreur permet de vérifier si le bloc `location` enfant peut être imbriqué dans le bloc `location` parent.
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#location)
	 */
	if ((!parent_l.modifier.empty() && parent_l.modifier != "^~" && (child_l.modifier.empty() || child_l.modifier == "^~"))
		|| (((parent_l.modifier.empty() || parent_l.modifier == "^~") && (child_l.modifier.empty() || child_l.modifier == "^~"))
			&& (child_l.path.size() < parent_l.path.size() || child_l.path.substr(0, parent_l.path.size()) != parent_l.path))) {
		this->throwError(raw_line, "location \""+child_l.path+"\" is outside location \""+parent_l.path+"\"", processed);
	}
}

void	Parser::addContextualRule(const std::string &key, const std::string &val, const std::string &context, const size_t processed, const std::string &raw_line)
{
	const size_t key_length = processed - val.length() - (val.length() > 0);
	const size_t key_pos = key_length - key.length() - 1;

	if (!this->new_server && key != "server")
		this->throwError(raw_line, "directive out of server block", key_pos);

	/**
	 * Directive server
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#server)
	 */
	if (key == "server") {
		// On empeche l'imbrication de blocs server
		// S'il le context est vide c'est qu'on est dans le bloc principal
		if (!context.empty())
			this->throwError(raw_line, "server block cannot be nested", key_pos);
		if (!val.empty())
			this->throwError(raw_line, "server directive cannot have value", key_length);

		this->new_server = new ServerConfig();
		this->configs.push_back(this->new_server);
		this->tmp_router = this->new_server->getDefaultHandler();
		return;
	}

	/**
	 * Directive location
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#location)
	 */
	if (key == "location") {
		this->createNewRouter(key, val, context, key_length, raw_line);

		return;
	}

	/**
	 * Default
	 */
	this->throwError(raw_line, "unknown directive", key_pos);
}

void	Parser::addNonContextualRule(const std::string &key, const std::vector<std::string> &valtokens, const size_t vallength, const std::string &context, const size_t processed, const std::string &raw_line)
{
	// Taille du nom de la directive (+1 pour l'espace après la clé)
	const size_t key_length = processed - vallength - (vallength > 0);
	const size_t key_pos = key_length - key.length() - 1;

	if (context.empty())
		this->throwError(raw_line, "directive outside of context", key_pos);

	if (context == "server")
		this->tmp_router = this->new_server->getDefaultHandler();

	if (vallength == 0)
		this->throwError(raw_line, "missing directive value", processed);

	/**
	 * Directive Listen
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#listen)
	 */
	if (key == "listen") {
		if (context != "server")
			this->throwError(raw_line, "listen directive must be inside server block", key_pos);

		std::vector<std::string> tokens = split(valtokens[0], ':');

		bool canUsePort = false;
		if (isDigit(tokens[0]))
		{
			this->new_server->setPort(std::atoi(tokens[0].c_str()));
		}
		else if (isIPAddress(tokens[0]))
		{
			this->new_server->setAddress(tokens[0]);
			canUsePort = true;
		}
		else if (tokens[0] == "*")
		{
			if (tokens.size() == 2)
				this->new_server->setAddress(INADDR_ANY);
			else
				this->throwError(raw_line, "invalid address", key_length);
			canUsePort = true;
		}
		else
		{
			struct hostent *hostent = gethostbyname(tokens[0].c_str());
			if (hostent == NULL) {
				this->throwError(raw_line, hstrerror(h_errno), key_length);
			}
			if (hostent->h_addrtype != AF_INET)
				this->throwError(raw_line, "address family not suported", key_length);

			uint32_t ip_address = *(uint32_t *)hostent->h_addr;
			// L'adresse retournée par gethostbyname est en network byte order, on la convertit en host byte order
			this->new_server->setAddress(ntohl(ip_address));
			canUsePort = true;
		}

		if (tokens.size() == 2)
		{
			if (!canUsePort)
				this->throwError(raw_line, "port not allowed here", key_length + tokens[0].length() + 1);
			if (!isDigit(tokens[1]))
				this->throwError(raw_line, "invalid port", key_length + tokens[0].length() + 1);
			this->new_server->setPort(std::atoi(tokens[1].c_str()));
		}

		if (valtokens.size() > 1)
		{
			if (valtokens[1] == "ssl")
				this->new_server->setSSL(true);
			else if (valtokens[1] == "default_server")
				this->new_server->setDefault();
			else
				this->throwError(raw_line, "unsupported behaviour: each server block can only listen on a single address:port pair at a time. Please consider using one server block per address:port pair.", key_length + valtokens[0].length() + 1);
		}

		return ;
	}

	/**
	 * Directive server_name
	 * 
	 * (https://nginx.org/en/docs/http/server_names.html)
	 */
	if (key == "server_name") {
		if (context != "server")
			this->throwError(raw_line, "server_name directive must be inside server block", key_pos);

		size_t l = 0;
		for (size_t i = 0; i < valtokens.size(); i++)
		{
			size_t pos = valtokens[i].find(':');
			if (pos != std::string::npos)
				this->throwError(raw_line, "invalid server name", key_length + l + pos);

			l += valtokens[i].length() + 1;
		}
		
		this->new_server->addNames(valtokens);
		return ;
	}

	/**
	 * Directive root/alias
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#root)
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#alias)
	 */
	if (key == "root") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		if (!isDirectory(valtokens[0])) {
			this->throwError(raw_line, "not a directory", key_length);
		}
		this->tmp_router->setRoot(valtokens[0]);
		return ;
	}
	else if (key == "alias") {
		if (context == "server")
			this->throwError(raw_line, "alias directive cannot be used in server block", key_pos);

		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		if (!isDirectory(valtokens[0])) {
			this->throwError(raw_line, "not a directory", key_length);
		}
		this->tmp_router->setAlias(valtokens[0]);
		return ;
	}

	/**
	 * Directive index
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_index_module.html#index)
	 */
	if (key == "index") {
		this->tmp_router->setIndex(valtokens);
		return ;
	}

	/**
	 * Directive autoindex
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_autoindex_module.html#autoindex)
	 */
	if (key == "autoindex") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		bool onf = this->onoffArgs(raw_line, key_length, vallength, valtokens);
		this->tmp_router->setAutoIndex(onf);
		return ;
	}

	/**
	 * Directive return
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_rewrite_module.html#return)
	 */
	if (key == "return") {
		int status = 302;
		std::string loc = valtokens[0];

		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 2);

		if (valtokens.size() == 1 && isDigit(loc))
		{
			status = std::atoi(loc.c_str());
			loc = "";
		}
		else if (valtokens.size() == 2)
		{
			if (!isDigit(valtokens[0]))
				this->throwError(raw_line, "invalid status code", key_length);
			status = std::atoi(valtokens[0].c_str());
			loc = valtokens[1];
		}
		else
			this->throwError(raw_line, "invalid status code", key_length);

		if (!Response::isValidStatus(status)) {
			// Logger::warning("parser warning: invalid status code, this may cause unexpected behavior.");
			this->throwError(raw_line, "invalid status code", key_length);
		}
		this->tmp_router->setRedirection(loc, status);
		return ;
	}

	/**
	 * Directive allow_methods
	 * 
	 */
	if (key == "allow_methods") {
		size_t l = 0;
		for (size_t i = 0; i < valtokens.size(); i++) {
			if (!Server::isValidMethod(valtokens[i]))
				this->throwError(raw_line, "unknown method", key_length + l);
			l += valtokens[i].length() + 1;
			this->tmp_router->allowMethod(valtokens[i]);
		}
		return ;
	}

	/**
	 * Directive error_page
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#error_page)
	 */
	if (key == "error_page") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 2, 0);

		size_t l = 0;
		for (size_t i = 0; i < valtokens.size() - 1; i++) {
			if (!isDigit(valtokens[i])) {
				this->throwError(raw_line, "invalid status code", key_length + l);
			}
			int code = std::atoi(valtokens[i].c_str());
			if (code < 300 || code > 599) {
				this->throwError(raw_line, "error_page cannot be used with the status code", key_length + l);
			}
			l += valtokens[i].length() + 1;
			this->tmp_router->setErrorPage(code, valtokens.back());
		}
		return ;
	}

	/**
	 * Directive client_max_body_size
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#client_max_body_size)
	 */
	if (key == "client_max_body_size") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		size_t ts = parseSize(valtokens[0]);
		if (ts == (size_t)-1) {
			this->throwError(raw_line, "invalid size", key_length);
		}
		this->tmp_router->setClientMaxBodySize(ts);
		return ;
	}

	/**
	 * Directive fastcgi_pass
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_fastcgi_module.html#fastcgi_pass)
	 */
	if (key == "fastcgi_pass") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		if (!isDirectory(valtokens[0])) {
			this->throwError(raw_line, "not a directory", key_length);
		}
		this->tmp_router->setCGI(valtokens[0]);
		return ;
	}

	/**
	 * Directive fastcgi_param
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_fastcgi_module.html#fastcgi_param)
	 */
	if (key == "fastcgi_param") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 2, 2);		

		this->tmp_router->addCGIParam(valtokens[0], valtokens[1]);
		return ;
	}

	/**
	 * Directive add_header
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_headers_module.html#add_header)
	 */
	if (key == "add_header") {
		bool always = false;
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 2, 3);

		if (valtokens.size() == 3) {
			if (valtokens[2] != "always")
				this->throwError(raw_line, "unknown option", key_length + vallength - valtokens[2].length());
			always = true;
		}
		this->tmp_router->setHeader(valtokens[0], valtokens[1], always);
		return ;
	}

	/**
	 * Directive client_header_timeout
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#client_header_timeout)
	 */
	if (key == "client_header_timeout") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		time_t	t = parseTime(valtokens[0]);
		if (t < 0) {
			this->throwError(raw_line, "invalid time", key_length);
		}
		this->tmp_router->setTimeout(t, "header");
		return ;
	}

	/**
	 * Directive client_body_timeout
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_core_module.html#client_body_timeout)
	 */
	if (key == "client_body_timeout") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		time_t	t = parseTime(valtokens[0]);
		if (t < 0) {
			this->throwError(raw_line, "invalid time", key_length);
		}
		this->tmp_router->setTimeout(t, "body");
		return ;
	}

	/**
	 * Directive proxy_method
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_proxy_module.html#proxy_method)
	 */
	if (key == "proxy_method") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		if (!Server::isValidMethod(valtokens[0]))
			this->throwError(raw_line, "unknown method", key_length);
		this->tmp_router->setProxyMethod(valtokens[0]);
		return ;
	}

	/**
	 * Directive proxy_pass
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_proxy_module.html#proxy_pass)
	 */
	if (key == "proxy_pass") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		if (context != "location")
			this->throwError(raw_line, "proxy_pass directive must be inside location block", key_pos);

		std::string url = valtokens[0];

		wbs_url tu;
		try
		{
			tu = newURL(url);
		}
		catch(...)
		{
			this->throwError(raw_line, "invalid url", key_length);
		}

		if (tu.protocol != "http" && tu.protocol != "https")
			this->throwError(raw_line, "unsupported protocol (only http: and https: are supported)", key_length);

		if (tu.path != "/")
			this->throwError(raw_line, "using path in proxy_pass directive in not supported", key_length + tu.protocol.length() + 3 + tu.host.length() + (tu.port_s.length() > 0) + tu.port_s.length());

		this->tmp_router->setProxy(tu);
		return ;
	}

	/**
	 * Directive proxy_set_body
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_proxy_module.html#proxy_set_body)
	 */
	if (key == "proxy_set_body") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		this->tmp_router->setProxyBody(valtokens[0]);
		return ;
	}

	/**
	 * Directive proxy_set_header
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_proxy_module.html#proxy_set_header)
	 */
	if (key == "proxy_set_header") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 2, 2);

		this->tmp_router->addProxyHeader(valtokens[0], valtokens[1]);
		return ;
	}

	/**
	 * Directive proxy_pass_header
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_proxy_module.html#proxy_pass_header)
	 */
	if (key == "proxy_pass_header") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		this->tmp_router->enableProxyHeader(valtokens[0]);
		return ;
	}

	/**
	 * Directive proxy_pass_request_body
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_proxy_module.html#proxy_pass_request_body)
	 */
	if (key == "proxy_pass_request_body") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		bool onf = this->onoffArgs(raw_line, key_length, vallength, valtokens);
		this->tmp_router->setProxyForwardBody(onf);
		return ;
	}

	/**
	 * Directive proxy_pass_request_headers
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_proxy_module.html#proxy_pass_request_headers)
	 */
	if (key == "proxy_pass_request_headers") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		bool onf = this->onoffArgs(raw_line, key_length, vallength, valtokens);
		this->tmp_router->setProxyForwardHeaders(onf);
		return ;
	}

	/**
	 * Directive proxy_hide_header
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_proxy_module.html#proxy_hide_header)
	 */
	if (key == "proxy_hide_header") {
		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		this->tmp_router->hideProxyHeader(valtokens[0]);
		return ;
	}

	/**
	 * Directive ssl_certificate
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_ssl_module.html#ssl_certificate)
	 */
	if (key == "ssl_certificate") {
		if (context != "server")
			this->throwError(raw_line, "ssl_certificate directive must be inside server block", key_pos);

		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		if (!isFile(valtokens[0]))
			this->throwError(raw_line, "no such file", key_length);

		this->new_server->setSSLCertFile(valtokens[0]);
		return ;
	}

	/**
	 * Directive ssl_certificate_key
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_ssl_module.html#ssl_certificate_key)
	 */
	if (key == "ssl_certificate_key") {
		if (context != "server")
			this->throwError(raw_line, "ssl_certificate_key directive must be inside server block", key_pos);

		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		if (!isFile(valtokens[0]))
			this->throwError(raw_line, "no such file", key_length);

		this->new_server->setSSLKeyFile(valtokens[0]);
		return ;
	}

	/**
	 * Directive ssl_ciphers
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_ssl_module.html#ssl_ciphers)
	 */
	if (key == "ssl_ciphers") {
		if (context != "server")
			this->throwError(raw_line, "ssl_ciphers directive must be inside server block", key_pos);

		this->minmaxArgs(raw_line, key_length, vallength, valtokens, 1, 1);

		this->new_server->setSSLCiphers(valtokens[0]);
		return ;
	}

	/**
	 * Default
	 */
	this->throwError(raw_line, "unknown directive", key_pos);
}

bool	Parser::isValidModifier(const std::string &modifier) const
{
	return (modifier == "=" || modifier == "~" || modifier == "~*" || modifier == "^~");
}
