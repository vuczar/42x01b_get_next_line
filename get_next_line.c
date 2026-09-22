/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vimauric <vimauric@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 19:53:32 by vimauric          #+#    #+#             */
/*   Updated: 2026/09/22 18:06:38 by vimauric         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

char	*fill_stash(int fd, char *stash, char *buf)
{
	ssize_t	bytes;

	bytes = 1;
	while (bytes > 0 && !ft_strchr(stash, '\n'))
	{
		bytes = read(fd, buf, BUFFER_SIZE);
		if (bytes < 0)
		{
			free(stash);
			return (NULL);
		}
		buf[bytes] = '\0';
		stash = ft_strjoin_free(stash, buf);
		if (!stash)
			return (NULL);
	}
	return (stash);
}

char	*extract_line(char *stash)
{
	char	*line;
	size_t	i;

	i = 0;
	while (stash[i] && stash[i] != '\n')
		i++;
	if (stash[i] == '\n')
		i++;
	line = malloc(i + 1);
	if (!line)
		return (NULL);
	line[i] = '\0';
	while (i > 0)
	{
		i--;
		line[i] = stash[i];
	}
	return (line);
}

char	*advance_stash(char *stash)
{
	char	*rest;
	char	*newline;
	size_t	i;

	newline = ft_strchr(stash, '\n');
	if (!newline || !newline[1])
	{
		free(stash);
		return (NULL);
	}
	rest = malloc(ft_strlen(newline + 1) + 1);
	if (!rest)
	{
		free(stash);
		return (NULL);
	}
	i = 0;
	while (newline[i + 1])
	{
		rest[i] = newline[i + 1];
		i++;
	}
	rest[i] = '\0';
	free(stash);
	return (rest);
}

char	*get_next_line(int fd)
{
	static char	*stash;
	char		*buf;
	char		*line;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	buf = malloc(BUFFER_SIZE + 1);
	if (!buf)
		return (NULL);
	stash = fill_stash(fd, stash, buf);
	free(buf);
	if (!stash || !stash[0])
	{
		free(stash);
		stash = NULL;
		return (NULL);
	}
	line = extract_line(stash);
	stash = advance_stash(stash);
	return (line);
}
