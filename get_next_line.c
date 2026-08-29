/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vimauric <vimauric@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 19:53:32 by vimauric          #+#    #+#             */
/*   Updated: 2026/08/29 09:28:54 by vimauric         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

char	*fill_stash(int fd, char *stash);

char	*extract_line(char *stash);

char	*advance_stash(char *stash);

char	*get_next_line(int fd)
{
	static char	*stash;
}