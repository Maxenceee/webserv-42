/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readsocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/21 21:46:34 by mgama             #+#    #+#             */
/*   Updated: 2023/12/31 18:32:51 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "readsocket.hpp"

unsigned char	*readsocket(int fd, unsigned char *file, uint32_t *rsize)
{
	unsigned char	*buff;
	uint32_t		read_bytes;
	uint32_t		size;

	size = 0;
	buff = (unsigned char *)malloc((BUFFER_SIZE + 1) * sizeof(unsigned char));
	if (!buff)
		return (NULL);
	read_bytes = 1;
	while (read_bytes != 0)
	{
		read_bytes = read(fd, buff, BUFFER_SIZE);
		if (read_bytes == -1)
			return (free(buff), CNULL);
		buff[read_bytes] = '\0';
		file = ft_memjoin(file, buff, size, read_bytes);
		if (!file)
			return (free(buff), CNULL);
		size += read_bytes;
		if (rsize)
			*rsize += size;
	}
	free(buff);
	return (file);
}
