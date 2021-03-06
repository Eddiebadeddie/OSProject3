#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "cube.h"
#include "wizard.h"

#define DEFAULT_CUBE_SIZE 4
#define DEFAULT_TEAM_SIZE 5
#define DEFAULT_SEED 1
#define HOWBUSY 100000000
#define TRUE 1
#define FALSE 0

void 
command_line_usage()
{
  fprintf(stderr, "-size <size of cube> -teamA <size of team> -teamB <size of team> -seed <seed value>\n"); 
}

void 
kill_wizards(struct wizard *w)
{
  /* Fill in */
	w->cube = NULL;
	sem_destroy(&w->frozen);
	free(w);

  return;
}

int 
check_winner(struct cube* cube)
{
  /* Fill in */
	int  i;
	int tA = 0;
	int tB = 0;

	for(i=0; i < cube->teamA_size; ++i){
		if(cube->teamA_wizards[i]->status){
			++tA;
		}
	}

	if(tA == cube->teamA_size){
		printf("Team B wins\n");
		//team B wins
		return 1;
	}
	
	for(i = 0; i < cube->teamB_size; ++i){
		if(cube->teamB_wizards[i]->status){
			++tB;
		}
	}

	if(tB == cube->teamB_size){
		printf("Team A wins\n");
		//team A wins
		//return
		return 2;
	}

	
  return 0;
}

void 
print_cube(struct cube *cube)
{
  int i;
  int j;
  int k;

  assert(cube);

  for (i = 0; i < cube->size; i++)
    {
      printf("+");
      for (j = 0; j < cube->size; j++)
	{
	  printf("--+");
	}
      printf("\n|");
      
      for (j = 0; j < cube->size; j++)
	{
	  /* Print the status of wizards in this room here */
	  for (k = 0; k < 2; k++)
	    {
	      if (cube->rooms[j][i]->wizards[k] != NULL)
		{
		  if (cube->rooms[j][i]->wizards[k]->status)
		    {
		      printf("%c", tolower(cube->rooms[j][i]->wizards[k]->team));
		    }
		  else
		    {
		      printf("%c", toupper(cube->rooms[j][i]->wizards[k]->team));
		    }
		}
	      else 
		{
		  printf(" ");
		}
	    }
	  printf("|");
	}
      printf("\n");
      
    }
  printf("+");
  for (j = 0; j < cube->size; j++)
    {
      printf("--+");
    }
  printf("\n");
  return;
}

struct wizard *init_wizard(struct cube* cube, char team, int id)
{
  int x, newx;
  int y, newy;
  int initflag;
  struct wizard *w;

  w = (struct wizard *)malloc(sizeof(struct wizard));
  assert(w);

  initflag = FALSE;
  
  w->team = team;
  w->id = id; 
  w->status = 0;
  w->cube = cube;
  
  x = rand() % cube->size;
  y = rand() % cube->size;
  
  if (cube->rooms[x][y]->wizards[0] == NULL)
    {
      cube->rooms[x][y]->wizards[0] = w;
      ++cube->rooms[x][y]->wizCount;
      w->x = x;
      w->y = y;
    }
  else if (cube->rooms[x][y]->wizards[1] == NULL)
    {
      cube->rooms[x][y]->wizards[1] = w;
      ++cube->rooms[x][y]->wizCount;
      w->x = x;
      w->y = y;
    }
  else
    {
      newx = (x + 1) % cube->size;
      if (newx == 0) newy = (y + 1) % cube->size;
      else newy = y;

      while((newx != x || newy != y) && !initflag)
	{

	  if (cube->rooms[newx][newy]->wizards[0] == NULL)
	    {
	      cube->rooms[newx][newy]->wizards[0] = w;
	      ++cube->rooms[x][y]->wizCount;
	      w->x = newx;
	      w->y = newy;
	      initflag = TRUE;
	    }
	  else if (cube->rooms[newx][newy]->wizards[1] == NULL)
	    {
	      cube->rooms[newx][newy]->wizards[1] = w;
	      ++cube->rooms[x][y]->wizCount;
	      w->x = newx;
	      w->y = newy;
	      initflag = TRUE;
	    }
	  else
	    {
	      newx = (newx + 1) % cube->size;
	      
	      if (newx == 0)
		{
		  newy = (newy + 1) % cube->size;
		}
	    }
	  
	}
      if (!initflag){
	free(w);
	return NULL;
      }
    }

  /* Fill in */
	sem_init(&w->frozen, 0, 1);

  return w;
}

int 
interface(void *cube_ref)
{
  struct cube* cube;
  char *line;
  char *command;
  int i;

  char choice;

  int continuous = FALSE;

  cube = (struct cube *)cube_ref;
  assert(cube);

  using_history();
  sem_init(&cube->go, 0, 1);
  while (1)
    {
	sem_wait(&cube->go);
	int winner = check_winner(cube);
	//1 - tB
	//2 - tA
	
	if(winner == 1){
		printf("Team B wins!\n");
		break;
	}
	else if(winner == 2){
		printf("Team A wins!\n");
		break;
	}

	if(continuous){
		sem_post(&cube->go);
		continue;
	}	
	else{
      		line = readline("cube> ");
      		if (line == NULL) continue;
      		if (strlen(line) == 0) continue;
      
      		add_history(line);

      		i = 0;
      		while (isspace(line[i])) i++;
      
      		command = &line[i];
		if(!strcmp(command, "c")){
			printf("this equals c\n");
			choice = 'c';
		}
		else{
			choice = '\0';
		}
	}

      if (!strcmp(command, "exit"))
	{
	  return 0;
	}
      else if (!strcmp(command, "show"))
	{
	  print_cube(cube);
	}
      else if (!strcmp(command, "start"))
	{
	  if (cube->game_status == 1)
	    {
	      fprintf(stderr, "Game is over. Cannot be started again\n");
	    }
	  else if (cube->game_status == 0)
	    {
	      fprintf(stderr, "Game is in progress. Cannot be started again\n");
	    }
	  else
	    {
	      cube->game_status = 0;
	      
	      /* Start the game */

	      /* Fill in */
		
		
  		int num = cube->teamA_size + cube->teamB_size;
		printf("\nMain:: num = %d\n\n", num);
  		pthread_t wizardThreadsA[cube->teamA_size];
		pthread_t wizardThreadsB[cube->teamB_size];
		
		
  		for(i = 0; i < cube->teamA_size; ++i){
			pthread_create(&wizardThreadsA[i], NULL, wizard_func, cube->teamA_wizards[i]);
			printf("wizardThreadsA[%d] has been created\n", i);
  		}

		for(i = 0; i < cube->teamB_size; ++i){
			pthread_create(&wizardThreadsB[i], NULL, wizard_func, cube->teamB_wizards[i]);
			printf("wizardThreadsB[%d] has been created\n", i);
		}

	    }
	}
      else if (!strcmp(command, "stop"))
	{
	  /* Stop the game */
	  return 1;
	}
      else
	{
		if(!strcmp(command, "c") && cube->game_status == 0){
			printf("Set to Continuous\n");
			continuous = TRUE;
		}
		else if(!strcmp(command, "s") && cube->game_status == 0){
			printf("Step\n");
		}
		else if(cube->game_status == -1){
			printf("Need to START a game\n");
		}	
		else{
	  		fprintf(stderr, "unknown command %s\n", command);
		}	
	}
	printf("free's line\n");
	sem_post(&cube->go);
      free(line);
    }
  return 0;
}

int 
main(int argc, char** argv)
{
  int cube_size = DEFAULT_CUBE_SIZE;
  int teamA_size = DEFAULT_TEAM_SIZE;
  int teamB_size = DEFAULT_TEAM_SIZE;
  unsigned seed = DEFAULT_SEED;
  struct cube *cube;
  struct room *room;
  struct room **room_col;
  int res;
  struct wizard *wizard_descr;
  int i, j;


  /* Parse command line and fill:
     teamA_size, timeBsize, cube_size, and seed */

  i = 1;
  while(i < argc) 
    {
      if (!strcmp(argv[i], "-size")) 
        {
	  i++;
	  if (argv[i] == NULL) 
            {
              fprintf(stderr, "Missing cube size\n");
              command_line_usage();
              exit(-1);
            }
          cube_size = atoi(argv[i]);
	  if (cube_size == 0)
	    {
              fprintf(stderr, "Illegal cube size\n");
              exit(-1);
	    }
        }
      else if (!strcmp(argv[i], "-teamA")) 
        {
          i++;
          if (argv[i] == NULL) 
            {
              fprintf(stderr, "Missing team size\n");
              command_line_usage();
              exit(-1);
            }
          teamA_size = atoi(argv[i]);
	  if (teamA_size == 0)
	    {
              fprintf(stderr, "Illegal team size\n");
              exit(-1);
	    }
        }
      else if (!strcmp(argv[i], "-teamB")) 
        {
          i++;
          if (argv[i] == NULL) 
            {
              fprintf(stderr, "Missing team size\n");
              command_line_usage();
              exit(-1);
            }
          teamB_size = atoi(argv[i]);
	  if (teamB_size == 0)
	    {
              fprintf(stderr, "Illegal team size\n");
              exit(-1);
	    }
        }
      else if (!strcmp(argv[i], "-seed")) 
        {
          i++;
          if (argv[i] == NULL) 
            {
              fprintf(stderr, "Missing seed value\n");
              command_line_usage();
              exit(-1);
            }
          seed = atoi(argv[i]);
	  if (seed == 0)
	    {
              fprintf(stderr, "Illegal seed value\n");
              exit(-1);
	    }
        }
      else 
        {
          fprintf(stderr, "Unknown command line parameter %s\n", argv[i]);
          command_line_usage();
          exit(-1);
        }
      i++;
    }

	printf("Main:: finished initializing\n");
  /* Sets the random seed */
  srand(seed);

  /* Checks that the number of wizards does not violate
     the "max occupancy" constraint */
  if ((teamA_size + teamB_size) > ((cube_size * cube_size) * 2))
    {
      fprintf(stderr, "Sorry but there are too many wizards!\n");
      exit(1);
    }


  /* Creates the cube */
  cube = (struct cube *)malloc(sizeof(struct cube));
  assert(cube);
  cube->size = cube_size;
  cube->game_status = -1;

  /* Creates the rooms */
  cube->rooms = malloc(sizeof(struct room **) * cube_size);
  assert(cube->rooms);

  for (i = 0; i < cube_size; i++)
    {
      /* Creates a room column */
      room_col = malloc(sizeof(struct room *) * cube_size);
      assert(room_col);

      for (j = 0; j < cube_size; j++)
	{
	  /* Creates a room */
	  room = (struct room *)malloc(sizeof(struct room));
	  assert(room);
	  room->x = i;
	  room->y = j;
	  room->wizards[0] = NULL;
	  room->wizards[1] = NULL;
	  room_col[j] = room;

	  /* Fill in */
	  sem_init(&room->door, 0, 1); 
	}
      
      cube->rooms[i] = room_col;
    }

	printf("Main:: cube is initialized\n");

  /* Creates the wizards and positions them in the cube */
  cube->teamA_size = teamA_size;
  cube->teamA_wizards = (struct wizard **)malloc(sizeof(struct wizard *) * teamA_size);
  assert(cube->teamA_wizards);

	printf("Main:: team A set for initialization\n");

  cube->teamB_size = teamB_size;
  cube->teamB_wizards = (struct wizard **)malloc(sizeof(struct wizard *) * teamB_size);

  assert(cube->teamB_wizards);

	printf("Main:: team B set for initialization\n");

  /* Team A */
  for (i = 0; i < teamA_size; i++)
    {
      if ((wizard_descr = init_wizard(cube, 'A', i)) == NULL)
	{
	  fprintf(stderr, "Wizard initialization failed (Team A number %d)\n", i);
	  exit(1);
	}
      cube->teamA_wizards[i] = wizard_descr;
	printf("Main:: teamA_wizard[%d] initialized\n", i);
    }

	printf("Main:: teamA is intialized\n");

  /* Team B */

  for (i = 0; i < teamB_size; i++)
    {
      if ((wizard_descr = init_wizard(cube, 'B', i)) == NULL)
	{
	  fprintf(stderr, "Wizard initialization failed (Team B number %d)\n", i);
	  exit(1);
	}
      cube->teamB_wizards[i] = wizard_descr;
	printf("Main:: teamB_wizard[%d] initialized\n", i);
    }

	printf("Main:: team B is initalized\n");

  /* Fill in */
  /* Goes in the interface loop */
  res = interface(cube);

	
  exit(res);

  	for(i = 0; i < cube->teamA_size; ++i){
		kill_wizards(cube->teamA_wizards[i]);
	}
	
	for(i = 0; i < cube->teamB_size; ++i){
		kill_wizards(cube->teamB_wizards[i]);
	}
	
	free(cube->teamA_wizards);
	free(cube->teamB_wizards);

	sem_destroy(&cube->go);

	int a, b;

	for(a = 0; a < cube->size; ++a){
		for(b = 0; b < cube->size; ++b){
			sem_destroy(&cube->rooms[a][b]->door);
			free(cube->rooms[a][b]->wizards);
			free(cube->rooms[a][b]);
		}
		free(cube->rooms[a]);
	}
	free(cube);
}

void dostuff()
{
  int i;
  int wait;
  
  wait = rand() % HOWBUSY;

  for (i = 0; i < wait; i++) {}

  return;
}

struct room * 
choose_room(struct wizard* w)
{
  int newx = 0; 
  int newy = 0;

  /* The two values cannot be both 0 - no move - or 1 - diagonal move */ 
  while (newx == newy)
    {
      newx = rand() % 2;
      newy = rand() % 2;
    }
  if ((rand() % 2) == 1) {
    newx = 0 - newx;
    newy = 0 - newy;
  }

  return w->cube->rooms[(w->x + w->cube->size + newx) % w->cube->size][(w->y + w->cube->size + newy) % w->cube->size];

}

int 
try_room(struct wizard *w, struct room *oldroom, struct room* newroom)
{
	print_cube(w->cube);
	printf("\ntry_room::Entered\n");
  /* Fill in */
  sem_wait( &newroom->door);
  if(newroom->wizCount == 2){
	printf("\troom is full\n");
  	sem_post(&newroom->door);
  	return 1;
  }
  else{
	printf("\troom has occupancy\n");
	sem_post(&newroom->door);
	return 0;
  }
}

struct wizard *
find_opponent(struct wizard* self, struct room *room)
{
  struct wizard *other = NULL;

  /* Updates room wizards and determines opponent */
  if ((room->wizards[0] == self))
    {
      other = room->wizards[1];
    }
  else if (room->wizards[1] == self)
    {
      other = room->wizards[0];
    }
  
  return other;
}

void 
switch_rooms(struct wizard *w, struct room *oldroom, struct room* newroom)
{
	printf("\nswitch_rooms::Entered\n");
  struct wizard *other;

  /* Removes self from old room */
  if (oldroom->wizards[0] == w)
    {
      oldroom->wizards[0] = NULL;
    }
  else if (oldroom->wizards[1] == w)
    {
      oldroom->wizards[1] = NULL;
    }
  else /* This should never happen */
    {
      printf("Wizard %c%d in room (%d,%d) can't find self!\n",
	     w->team, w->id, oldroom->x, oldroom->y);
      print_cube(w->cube);
      exit(1);
    }

  /* Fill in */
  oldroom->wizCount -= 1;
  sem_post(&oldroom->door);

  sem_wait(&newroom->door);
  /* Updates room wizards and determines opponent */
  if (newroom->wizards[0] == NULL)
    {
      newroom->wizards[0] = w;
      other = newroom->wizards[1];
    }
  else if (newroom->wizards[1] == NULL)
    {
      newroom->wizards[1] = w;
      other = newroom->wizards[0];
    }
  else /* This should never happen */
    {
      printf("Wizard %c%d in room (%d,%d) gets in a room already filled with people!\n",
	     w->team, w->id, newroom->x, newroom->y);
      print_cube(w->cube);
      exit(1);
    }
  
  /* Sets self's location to current room */
  w->x = newroom->x;
  w->y = newroom->y;
  newroom->wizCount += 1;
}

int 
fight_wizard(struct wizard *self, struct wizard *other, struct room *room)
{
	printf("\nfight_wizard::Entered\n");
  int res;
  sem_t mutex;
  sem_init(&mutex, 0, 1);

  /* Computes the result of the fight */
  res = rand() % 2;
		  
  /* The opponent becomes frozen */
  if (res == 0)
    {
      printf("Wizard %c%d in room (%d,%d) freezes enemy %c%d\n",
	     self->team, self->id, room->x, room->y,
	     other->team, other->id);

      /* Fill in */
      sem_wait(&mutex);
	other->status = 1;
	sem_wait(&other->frozen);
	sem_post(&mutex);

    }

  /* Self freezes and release the lock */
  else
    {

      printf("Wizard %c%d in room (%d,%d) gets frozen by enemy %c%d\n",
	     self->team, self->id, room->x, room->y,
	     other->team, other->id);

      /* Fill in */
	sem_wait(&mutex);
	self->status= 1;
	sem_wait(&self->frozen);
	sem_post(&mutex);
      return 1;
    }
  return 0;
}

int 
free_wizard(struct wizard *self, struct wizard *other, struct room* room)
{
  int res;
	sem_t mutex;
	sem_init(&mutex, 0, 1);

  /* Computes the results of the unfreeze spell */
  res = rand() % 2;

  /* The friend is unfrozen */
  if (res == 0)
    {
      printf("Wizard %c%d in room (%d,%d) unfreezes friend %c%d\n",
	     self->team, self->id, room->x, room->y,
	     other->team, other->id);

      /* Fill in */
	sem_wait(&mutex);
	other->status = 0;
	sem_post(&other->frozen);
	sem_post(&mutex);
      
    }

  /* The spell failed */
  printf("Wizard %c%d in room (%d,%d) fails to unfreeze friend %c%d\n",
	 self->team, self->id, room->x, room->y,
	 other->team, other->id);

  return 0;
}
