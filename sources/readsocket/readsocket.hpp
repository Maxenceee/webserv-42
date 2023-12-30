/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readsocket.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 17:54:13 by mgama             #+#    #+#             */
/*   Updated: 2023/12/30 18:17:41 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 1024
# endif

#define CNULL ((unsigned char *)0)

unsigned char	*readsocket(int fd, unsigned char *file, uint32_t *rsize);
unsigned char	*ft_memjoin(unsigned char *s1, unsigned char *s2,
	uint32_t size, uint32_t length);