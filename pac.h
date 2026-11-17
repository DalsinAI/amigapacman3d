/***********************************/
/*
         P A C M A N   3 D

   v1.3

	Copyright (c) 2000-2003 Andrew Pike

                                   */
/***********************************/


/*****/// Key Presses

#define KEYPRESS_OK    0
#define KEYPRESS_PRESS 1
#define KEYPRESS_DOWN  2

/*****/// Math Stuff

float hyp(float a);
float antihyp(float hypot);

/*****/ // Functions
/*****/

int load_level(char lmap[][102], int levno);
int run_intro(int intro_no);
void end_game_feature(int howitended);
int menu(int mcode);
void do_scoreboard(void);
void check_ini_file(void);
void getcmdline(int vArgc, char **vArgv);
void blot_out_bumcracks(void);

typedef struct {
                  int onflag;
                  char name[30];
                  int score;
                  int lives;
                  int level;
               } LOADGAME;


