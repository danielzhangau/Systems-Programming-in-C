#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "secret.h"
#include "phases.h"

void submitGuess(const char* s);

int main(int argc, char** argv)
{
    if (argc!=2)
    {
        fprintf(stderr, "Usage: %s status|phase\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1],"status")==0)
    {
      getStatus();
    }
    else
    {
      strfn phase=getPhase(argv[1]);
      if (!phase)
      {
	  fprintf(stderr, "I don't recognise that phase. Exiting.\n");  
	  return 2;
      }
      bombSetup(argv[1]);
      char line[80];
      printf("Enter text to disarm %s: ", argv[1]);
      fflush(stdout);
      if (fgets(line,79,stdin) && line[0]!='\0' && line[0]!='\n')
      {
	  if (line[strlen(line)-1]=='\n')
	  {
	      line[strlen(line)-1]='\0';
	  }
          phase(line);
          submitGuess(line);
      }
      else
      {
	  printf("No text read. Exiting\n");
      }
    }  
    return 0;
}


/* Some notes and tips:

The original idea of this assignment (and the previous version) come from 
an assignment run by CMU.

No guess counts until you answer 'Y' to the "Are sure[Y/N]?" prompt.
If the program exits early or some other response is given there is
no penalty.

If you answer 'Y' to the "Are sure[Y/N]?" prompt, the phase function 
specified on the command line will be run again in a different context 
(where you can't see it). Any attempt to interfere with this second 
run will not be tolerated. [That isn't something you could do by
accident.]

The print command is your friend. 

There is no need to complete all the phases in one session. If you 
rush you are more likely to make mistakes which cost you marks.

Each run of the bomb program executes a single attempt against a single 
phase. If you wish to try again or to attempt another phase you must 
run the bomb again.

*/

/*
What does the bomb man know?
*/
