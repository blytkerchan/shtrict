/* Vlinder shtrict: very restricted shell
 * Copyright (C) 2011   Ronald Landheer-Cieslak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3, as 
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.*/
#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>

#define SHTRICT_PROMPT 			"$ "
#define SHTRICT_MAX_COMMANDS		16
#define SHTRICT_MAX_COMMAND_SIZE	40
#define SHTRICT_MAX_COMMAND_PATH	80
#define SHTRICT_CMDS_FILENAME_DEFAULT	"/etc/shtrict/commands"
#define SHTRICT_OPENLOG_IDENT		"shtrict"
#define SHTRICT_OPENLOG_FLAGS		LOG_CONS | LOG_NDELAY | LOG_PID
#define SHTRICT_OPENLOG_FACILITY	LOG_USER

#define SHTRICT_HELPTEXT		""				\
	"Usage: %s [-h] [-v] [-i <config-file>]\n"			\
	"\t-h:\t\t\tdisplay this help and exit\n"			\
	"\t-v:\t\t\tdisplay version info and exit\n"			\
	"\t-i <config-file>:\tread configuration file\n"
#define SHTRICT_VERSIONTEXT						\
	"shtrict 1.0.00\n"						\
	"\n"								\
	"Vlinder shtrict  Copyright (C) 2011  Ronald Landheer-Cieslak\n"\
	"This program comes with ABSOLUTELY NO WARRANTY; see the GNU\n"	\
	"General Public License, version 3, for details.\n"		\
	"This is free software, and you are welcome to redistribute\n"	\
	"it under the conditions of the GNU General Public License,\n"	\
	"version 3.\n"

struct Command_struct {
	char command_[SHTRICT_MAX_COMMAND_SIZE + 1];
	char path_[SHTRICT_MAX_COMMAND_PATH + 1];
} commands__[SHTRICT_MAX_COMMANDS];

void run(const char *cmd)
{
	system(cmd);
}

void chomp(char *s)
{
	static const char *spaces__ = "\t\n\r\v ";
	size_t l = strlen(s);
	char *c = s + l;
	if (c != s) --c;
	while (c != s)
	{
		if (strchr(spaces__, *c))
		{
			*c-- = 0;
		}
		else
		{
			break;
		}
	}
	l = strlen(s);
	c = s;
	while ((c != (s + l)) && strchr(spaces__, *c))
	{
		++c;
	}
	if (c != s)
	{
		memmove(s, c, l - (c - s));
		s[l - (c - s)] = 0;
	}
}

int readCommands(FILE *file)
{
	char *line = NULL, *c;
	unsigned int i = 0;
	size_t s;
	enum {
		ok__,
		comment__,
		command_too_long__,
		path_too_long__,
		no_space_for_command__,
	} state;

	while (getline(&line, &s, file) > 0)
	{
		state = ok__;
		chomp(line);
		if (line
		&& (line[0] == '#' || line[0] == ';' || line[0] == 0))
		{
			/* skip this line - it's a comment */
			state = comment__;
		}
		else
		{ /* don't know yet */ }

		if (state == ok__)
		{
			c = strchr(line, '\t');
			if (c == NULL)
			{
				c = line + strlen(line);
			}
			*c++ = 0;
			if (c - line > SHTRICT_MAX_COMMAND_SIZE)
			{
				state = command_too_long__;
			}
			else
			{ /* command looks ok */ }
		}
		else
		{ /* we already know this is msesed up */ }

		if (state == ok__)
		{
			while (*c == '\t')
			{
				*c++ = 0;
			}
			if (strlen(c) > SHTRICT_MAX_COMMAND_PATH)
			{
				state = path_too_long__;
			}
			else
			{ /* all is well so far */ }
		}
		else
		{ /* already know something is wrong */ }

		if ((state == ok__) && (i < SHTRICT_MAX_COMMANDS))
		{
			strcpy(commands__[i].command_, line);
			strcpy(commands__[i].path_, c);
			++i;
			if (i < SHTRICT_MAX_COMMANDS)
			{
				commands__[i].command_[0] = 0;
			}
			else
			{ /* this will have been the last command */ }
		}
		else if (state == ok__)
		{
			state = no_space_for_command__;
		}

		switch (state)
		{
		case ok__ :
			/* fall through */
		case comment__ :
			break;
		case command_too_long__ :
			fprintf(stderr,
					"Warning: command \"%s\" too long - "
					"skipping\n", line);
			syslog(LOG_WARNING, "Command \"%s\" too long - "
					"skipping\n", line);
			break;
		case path_too_long__ :
			fprintf(stderr,
					"Warning: command for \"%s\" "
					"too long - skipping\n", line);
			syslog(LOG_WARNING,
					"Command for \"%s\" too long - "
					"skipping\n", line);
			break;
		case no_space_for_command__ :
			fprintf(stderr, "Warning: no space for \"%s\" - "
					"skipping\n", line);
			syslog(LOG_WARNING, "No space for \"%s\" - skipping\n"
					, line);
			break;
		}
		free(line);
		line = NULL;
	}

	return 0;
}

int listCommands(int count, int key)
{
	int i;
	if (rl_line_buffer[0])
	{	/* step 1: gather candidates */
		int candidates[SHTRICT_MAX_COMMANDS];
		int current_candidate_count = 0;
		size_t strlen_rl_line_buffer = strlen(rl_line_buffer);
		for (i = 0; (i < SHTRICT_MAX_COMMANDS) && commands__[i].command_[0]; ++i)
		{
			if ((strlen_rl_line_buffer <= strlen(commands__[i].command_)) && (memcmp(rl_line_buffer, commands__[i].command_, strlen_rl_line_buffer) == 0))
			{
				candidates[current_candidate_count++] = i;
			}
		}
		if (current_candidate_count == 1)
		{
			rl_insert_text(commands__[candidates[0]].command_ + strlen_rl_line_buffer);
		}
		else
		{
			printf("\nAvailable commands: \n");
			for (i = 0; i < current_candidate_count; ++i)
			{
				printf("\t%s\n", commands__[candidates[i]].command_);
			}
			rl_on_new_line();
		}
	}
	else
	{
		printf("\nAvailable commands: \n");
		for (i = 0; (i < SHTRICT_MAX_COMMANDS) && commands__[i].command_[0]; ++i)
		{
			printf("\t%s\n", commands__[i].command_);
		}
		rl_on_new_line();
	}

	return 0;
}

void setupReadline()
{
	rl_bind_key('\t', listCommands);
}

int main(int argc, char * argv[])
{
	int opt, i;
	char *in = NULL;
	const char *cmds_filename = SHTRICT_CMDS_FILENAME_DEFAULT;
	FILE *cmds_file = NULL;

	openlog(SHTRICT_OPENLOG_IDENT, SHTRICT_OPENLOG_FLAGS, SHTRICT_OPENLOG_FACILITY);

	while ((opt = getopt(argc, argv, "hvi:")) != -1)
	{
		switch (opt)
		{
		case 'h' :
			printf(SHTRICT_HELPTEXT, argv[0]);
			exit(EXIT_SUCCESS);
		case 'v' :
			printf(SHTRICT_VERSIONTEXT);
			exit(EXIT_SUCCESS);
		case 'i' :
			cmds_filename = optarg;
			break;
		default: /* '?' */
			fprintf(stderr, "Usage: %s [-i cmds_filename]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	syslog(LOG_NOTICE, "Started with euid %u\n", geteuid());
	syslog(LOG_NOTICE, "Configuration file: %s\n", cmds_filename);
	cmds_file = fopen(cmds_filename, "r");
	if (cmds_file != NULL)
	{
		readCommands(cmds_file);
	}
	else
	{
		fprintf(stderr, "Failed to open %s: %d", cmds_filename, errno);
		syslog(LOG_ERR, "Failed to open %s: %d", cmds_filename, errno);
		exit(EXIT_FAILURE);
	}
	setupReadline();
	while ((in = readline(SHTRICT_PROMPT)) != NULL)
	{
		chomp(in);
		if (in[0]) for (i = 0; i < SHTRICT_MAX_COMMANDS; ++i)
		{
			if (commands__[i].command_[0] == 0)
			{
				syslog(LOG_INFO, "Attempt to run \"%s\" failed: not found", in);
				fprintf(stderr, "Command not found\n");
				break;
			}
			else if (strcmp(commands__[i].command_, in) == 0)
			{
				syslog(LOG_INFO, "Running \"%s\"", in);
				run(commands__[i].path_);
				break;
			}
			else
			{ /* not this one */ }
		}
		free(in);
	}

	exit(EXIT_SUCCESS);
}

