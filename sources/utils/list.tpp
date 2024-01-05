/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   list.tpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 12:06:50 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 12:32:58 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

template <typename T>
bool	contains(const T &container, const std::string &test)
{
	return std::find(container.begin(), container.end(), test) != container.end();
}
