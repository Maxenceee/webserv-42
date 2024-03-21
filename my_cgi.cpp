#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
	for (int i = 0; envp[i]; i++)
	{
		std::cerr << "my_cgi: " << envp[i] << std::endl;
	}
	std::string content;
	std::string line;
	while (std::getline(std::cin, line))
	{
		content += line + "\n";
	}
	std::cout << "Status: 200\r\n";
	std::cerr << "my_cgi: " << content << std::endl;
	std::cout << content << std::endl;
	return 0;
}