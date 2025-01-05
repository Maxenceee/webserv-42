/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   list.tpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 12:06:50 by mgama             #+#    #+#             */
/*   Updated: 2025/01/05 13:44:17 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

template <typename T>
bool	contains(const T &container, const std::string &test)
{
	return std::find(container.begin(), container.end(), test) != container.end();
}

template <typename T>
T	&pop(T &container)
{
	if (!container.empty())
		container.pop_back();
	return (container);
}

template <typename T>
T	&shift(T &container)
{
	if (!container.empty())
	{
		typename T::iterator it = container.begin();
		container.erase(it);
	}
	return (container);
}

template <typename T>
std::string	last(T &container)
{
	if (!container.empty())
	{
		return container.back();
	}
	return ("");
}

template <typename T>
std::string	toStringl(T &container, std::string separator)
{
	std::string str;

	for (typename T::const_iterator it = container.begin(); it != container.end(); it++)
	{
		str += *it;
		if (it + 1 != container.end())
			str += separator;
	}
	return (str);
}

template <class InputIterator, class UnaryPredicate>
bool all_of(InputIterator first, InputIterator last, UnaryPredicate pred)
{
	while (first != last) {
		if (!pred(*first)) return false;
		++first;
	}
	return true;
}