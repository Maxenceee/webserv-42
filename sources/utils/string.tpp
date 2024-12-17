/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string.tpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 13:18:00 by mgama             #+#    #+#             */
/*   Updated: 2024/12/17 15:31:11 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

template <typename T>
std::string	toString(T val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}