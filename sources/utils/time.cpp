/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:25:34 by mgama             #+#    #+#             */
/*   Updated: 2024/02/26 13:45:58 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

#include <sys/time.h>

double	getTimestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}
