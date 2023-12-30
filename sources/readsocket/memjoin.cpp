/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   memjoin.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/22 16:47:15 by mgama             #+#    #+#             */
/*   Updated: 2023/12/30 18:13:29 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "readsocket.hpp"

static void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	size_t	i;
	void	*lst_dst;

	if (n == 0 || dst == src)
		return (dst);
	i = 0;
	lst_dst = dst;
	while (i < n)
	{
		((char *)dst)[i] = ((char *)src)[i];
		i++;
	}
	return (lst_dst);
}

unsigned char	*ft_memjoin(unsigned char *s1, unsigned char *s2,
	uint32_t size, uint32_t length)
{
	unsigned char	*str;

	if (!s1)
		s1 = (unsigned char *)malloc(sizeof(unsigned char));
	if (!s1)
		return (CNULL);
	if (!s2)
		return (free(s1), CNULL);
	str = (unsigned char *)malloc(sizeof(unsigned char) * (size + length));
	if (!str)
		return (free(s1), CNULL);
	ft_memcpy(str, s1, size);
	ft_memcpy(str + size, s2, length);
	free(s1);
	return (str);
}
