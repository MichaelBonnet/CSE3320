/*

  Name: Michael Bonnet
  ID:   1001753296

*/

// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017, 2020 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE  255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 5      // Mav shell only supports five arguments
#define MAX_COMMAND_HIST  15     // Mav shell only records last 15 commands


/* ================================== */
/* === LINKED LIST IMPLEMENTATION === */
/* ================================== */

/*
	This obviously isn't a function, but whatever.
	This is the prototype for the struct hist,
	an entry in the linked list that will store the PIDs
	and text of commands previously entered into the shell.
	int PID stores the PID of the command given,
	char* command stores the text of the command given,
	and struct hist* next is a pointer to the next item in the list.
*/

struct hist
{
	int PID;            // Records the PID of a command
	char* command;      // Records the text of a command
	struct hist *next;  // Pointer to next item in linked list
};

/*
	This obviously isn't a function, but whatever.
	This is a declaration of the NULL-ed out blank first entry in the linked list
	that will store the PIDs and command text of commands previously entered into the shell.
	It's NULL-ed out so that create_node can recognize that and replace it with the first command.
*/

struct hist *start = NULL;

/*
	Name                : create_hist
	Parameters Expected : int PID, the PID of the command to be stored
					      character array command, the text of the command to be stored
	Return Values       : None.
	What it Does        : Creates,
						  allocates memory for,
						  and links to the last list a new list in a linked list 
						  storing the PID and command text of a previous command.
*/

void create_hist(int PID, char* command)
{
	struct hist *temp;      // This will hold all the passed-in data
							// and then be copied into storage where needed
	struct hist *current;   // This will be the "current record"
							// as thumbed through by create_hist
	temp = (struct hist*)malloc(sizeof(struct hist)); // malloc'ing it makes sure
													  // it can be accessed in multiple scopes

	temp->PID = PID;                 // setting the PID of the temp record to that passed PID
	temp->command = strdup(command); // copy with strdup to ensure we have a malloc'd string
	temp->next = NULL;               // since we dont have the next record yet, we set it to NULL
									 // so that it can be caught as not having a next record
									 // and avoid segfaulting.

	if(start == NULL) // checks if no records have been made yet
	{
		start=temp;   // in which case, start becomes a copy of temp
	}
	else              // otherwise, if there are existing records
	{
		// current set equal to the starting record to act as the "current record"
		// as the following while loop goes through the whole linked list
		current=start;

		while(current->next != NULL) // ensures we don't venture into 
									 //unknown quadrants past the end of the linked list
		{
			current=current->next;   // if we aren't in uncharted space,  to the next record
		}
		current->next=temp;          // once we've found the last record
									 // (aka the while loop stops because current->next == NULL),
						             // we have that last record's next field point to the temp 
									 // record, which has been malloc'd.
	}
}


/*
	Name 				: delete_first
	Parameters Expected : None.
	Return Values 		: None.
	What it Does 		: Deletes the first item in a linked list.
						  In this specific case, the first item is the oldest item,
						  and so when we reach the maximum history, deleting the
						  first/oldest item "makes room" for the newest item to be
						  tagged onto the end of the links.
*/

void delete_first()
{
	
	struct hist *temp = start; // Temporarily store the old start in a pointer
	start = start->next;       // Reassign the second record as the first
	free(temp);                // Free the memory taken up by the old first record.
}

/*
	Name 				  : display_pids
	Parameters Expected   : int history_size, aka the number 
							of PIDs in the history, up to 15.
							Used to get the 15 most recent commands' PIDS
	Return Values		  : None.
	What it Does 		  : Goes through every list in the linked list
						    that stores a PID and command string in every entry,
							printing the indexes and PIDs of each entry.
							i, limited by history_size, notes the index to be printed
							with the process's commands.
							checking for NULL at start ensures we don't go into unmapped territory,
							and then checking for it as current becomes the next list
							ensures the same thing.
*/

void display_pids(int history_size)
{
	int i = 0;         // iterator variable
	struct hist *current = start;  // This will be the "current record" as thumbed through by
								   // display_pids. initially set to the global "start" record

	while(current != NULL) // as long as we haven't ventured into uncharted space, continue
	{
		printf("%d: %d\n", i, current->PID);  // print the iteration and the record's PID
		current = current->next;              // go to the next record for the next  loop pass

		if (i < history_size)  // if we haven't iterated more times than the size of the list,
		{
			i++;               // we iterate again
		}
	}
}

/*
	Name 				: display_commands
	Parameters Expected : int history_size, aka the number 
                          of commands in the history, up to 15.
                          Used to get the 15 most recent commands
	Return Values 		: None.
	What it Does 		: Goes through every list in the linked list
                		  that stores a PID and command string in every entry,
                		  printing the indexes and command strings of each entry.
                		  i, limited by history_size, notes the index to be printed
                		  with the process's commands.
                		  checking for NULL at start ensures we don't go into 
                		  unmapped territory, and then checking for it as current becomes
                		  the next list ensures the same thing.
*/

void display_commands(int history_size)
{
	int i = 0;         // iterator variable
	struct hist *current = start;  // This will be the "current record" as thumbed through by
								   // display_commands, initially set to the global "start" record

	while(current != NULL) // as long as we haven't ventured into uncharted space, continue
	{
		printf("%d: %s", i, current->command);  // print the iteration and the records command text
		current = current->next;                // go to the next record for the next loop pass

		if (i < history_size)  // if we haven't iterated more times than the size of the list,
		{
			i++;               // we iterate again
		}
	}
}

/*
	Name 				  : at_idx
	Parameters Expected   : int history_index, aka the index in the linked list
							that contains the information desired, in this case a string.
	Return Values		  : a string that is the command text of the desired historic command.
	What it Does 		  : Goes through every list in the linked list
							that stores a PID and command string in every entry,
							checks to see if the record is the one needed.
							then if so, copies the record's text and returns it,
							then if not, checks the next record,
							or if we've checked too many, return an error string.
*/

char* at_idx(int history_index)
{
	int i = 0;      // Iterator
	char* command;  // Holds string to be returned if record not found
	char* default_resp = "Index not found \n";  // Default error message if record not found
	struct hist *current = start;  // This will be the "current record" as thumbed through by
								   // at_idx, initially set to the global "start" record

	while(current != NULL) // ensuring we don't venture into unknown quadrants of the galaxy
	{
		if (i == history_index)
		{
			return strdup(current->command); // if i == index of the record required,
											 // return a copy the command text of that record
		}
		else if (i > history_index)
		{
			// if i has exceeded the index needed,
			// copy the error message into the string that will be returned.
			command = strncpy(command, default_resp, strlen(default_resp));
		}
		else
		{
			i++;                    // if not found at index i and i !> index needed,
			current=current->next;  // go to the next record.
		}
	}
	return command; // Return the error message if necessary.
					// this had to be here because the compiler 
					// yelled at me if I didn't have return at the very end.

}


/* ======================= */
/* === OTHER FUNCTIONS === */
/* ======================= */


// This was a failed experiment. 
// I wanted to abstract away the record creation 
// at each point but figuring out pointers and references
// was too hard for too little gain.

// void historicize(char* cmd_str, int history_size)
// {
// 	// Record commands and PID in a linked list.
// 	pid_t pid = getpid();
// 	if (history_size < 14)
// 	{
// 		create_hist(pid, cmd_str);
// 		history_size++;
// 	}
// 	if (history_size == 14)
// 	{
// 		delete_first(); // Deletes the first list in the linked
//						// list to "make room" for the new entry.
// 		create_hist(pid, cmd_str);
// 	}
// }

/* ========================= */
/* ===== MAIN FUNCTION ===== */
/* === (does main stuff) === */
/* ========================= */

int main(void)
{
	char* cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
	char* history_commands[15];
	int   history_size = 0;
	int   ret;

	int i;
	for(i = 0; i < MAX_COMMAND_HIST; i++)
	{
		history_commands[i] = (char*) malloc(255);
	}

	while( 1 )
	{
		// Print out the msh prompt
		printf ("msh> ");

		// Read the command from the commandline.  The
		// maximum command that will be read is MAX_COMMAND_SIZE
		// This while command will wait here until the user
		// inputs something since fgets returns NULL when there
		// is no input
		while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

		// Jesus tap dancing heck why does this block need to be here of all places,
		// why can't I have this AFTER string parsing Like, I get it. 
		// You have to test the whole  string for this, so you must do it before parsing.
		// But it's still just ... So ugly. I want this to be with the rest of my if-elses.
		// Requirement 6: if user types a blank line, shell will  with no other output,
		// print another prompt, and accept a new line of input
		if (strcmp(cmd_str, "\n") == 0)
		{
			continue; // Could this possibly be break? UPDATE: no it cannot
		}

		// PROGRAMCONTROL:

		strncpy(history_commands[history_size], cmd_str, strlen(cmd_str));

		/* Parse input */
		char *token[MAX_NUM_ARGUMENTS];

		int   token_count = 0;                          

		// Pointer to point to the token
		// parsed by strsep
		char *argument_ptr;

		char *working_str  = strdup( cmd_str );     


		// save history here he says - oops, using linked lists
		// unless ... ?

		// replace the ! command here he says

		// oh my god this didnt work for like an hour because I used "!" instead of '!'
		// Requirement 12: Handling !n to re-run command n in history
		if (cmd_str[0] == '!') // if first character is !, we have a !n case
		{
			// I would have really appreciated it had there been a note in the assignment pdf
			// detailing if the commands run or if !n itself was wanted to be recorded
			// in the command history.

			char *histnum = strtok(cmd_str, "!");  // Tokenizes based on the ! to get the cmd number
												   // tokenizing the whole string was much easier
												   // than tokenizing the first token much later on.
			int  hist_cmd = atoi(histnum);         // Converts that token to an int

			if (hist_cmd > history_size || hist_cmd < 0) // Weird case where if my n in !n is 1 more
														 // than what the new command's index will be,
														 // it just goes to a new entry.
			{
				printf("Command not in history.\n"); // Inform the user their chosen n does not exist.
			}
			else
			{
				working_str = strdup(at_idx(hist_cmd));
			}
		}

		// we are going to move the working_str pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		char *working_root = working_str;

		// Tokenize the input stringswith whitespace used as the delimiter
		while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
			  (token_count<MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
			if( strlen( token[token_count] ) == 0 )
			{
				token[token_count] = NULL;
			}
			token_count++;
		}

		/* 
		   The following massive if-else statement essentially determines
		   if the user entered certain special commands or not.
		   If so, it executes them according to a special method.
		   If not, it goes to execvp to be taken care of.
		   I figured this was the best way to both control AND show the
		   flow of logic.
		*/

		// Requirement 5: Exit with status zero if command is "quit" or "exit"
		if (strcmp(token[0], "quit")==0 || strcmp(token[0], "exit") == 0)
		{
			exit(0);
		}
		// Requirement 10: Handling "cd" command on its own to change directories
		else if (strcmp(token[0], "cd") == 0)
		{
			// the following block will be repeated a lot.
			// I tried to function-ize it as seen before main(),
			// but whatever.

			// Record commands and PID in a linked list.
			pid_t pid = getpid();
			if (history_size < 14)
			{
				create_hist(pid, cmd_str);
				history_size++;
			}
			if (history_size == 14)
			{
				delete_first(); // Deletes the first list to "make room" for the new entry.
				create_hist(pid, cmd_str);
			}

			// Change directory to the second item in the entered line
			chdir(token[1]);
		}
		// Requirement 11: Handling "showpids" to display the PIDS
		// of the last 15 processes spawned by msh
		else if (strcmp(token[0], "showpids") == 0)
		{
			// Record commands and PID in a linked list.
			pid_t pid = getpid();
			if (history_size < 14)
			{
				create_hist(pid, cmd_str);
				history_size++;
			}
			if (history_size == 14)
			{
				delete_first(); // Deletes the first list to "make room" for the new entry.
				create_hist(pid, cmd_str);
			}

			// Call function that displays the PIDs of most recent up to 15 processes.
			display_pids(history_size);
		}
		// Requirement 12: Handling "history" to display
		// the last 15 commands entered by the user
		else if (strcmp(token[0], "history") == 0)
		{
			// Record commands and PID in a linked list.
			pid_t pid = getpid();
			if (history_size < 14)
			{
				create_hist(pid, cmd_str);
				history_size++;
			}
			if (history_size == 14)
			{
				delete_first(); // Deletes the first list to "make room" for the new entry.
				create_hist(pid, cmd_str);
			}

			// Call function that displays the contents of the last 15 commands entered.
			display_commands(history_size);
		}
		// Non-special-case command handling
		else
		{
			pid_t pid = fork();
			if (pid == 0)
			{
				ret = execvp(token[0], &token[0]);
			}

			// Checks to see if the command is invalid
			if (ret == -1)
			{
				// I used to record the invalid command here;
				// figured that probably wasn't gucci?

				// Record commands and PID in a linked list.
				// if (history_size < 14)
				// {
				// 	create_hist(pid, cmd_str);
				// 	history_size++;
				// }
				// if (history_size==14)
				// {
				// 	delete_first(); // Deletes the first list to "make room" for the new entry.
				// 	create_hist(pid, cmd_str);
				// }
				// Informs user their command is invalid.
				printf("%s: Command not found.\n", token[0]);
				exit(1);
			}
			// If commands were valid, record the PIDs of the child processes
			else
			{
				// Record commands and PID in a linked list.
				if (history_size < 14)
				{
					create_hist(pid, cmd_str);
					history_size++;
				}
				if (history_size == 14)
				{
					delete_first(); // Deletes the first list "make room" for the new entry.
					create_hist(pid, cmd_str);
				}

				int status;
				wait(&status);
			}
		}

		free( working_root );

	}
	return 0;
}
