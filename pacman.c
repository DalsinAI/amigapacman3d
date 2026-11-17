/***********************************/
/*
         P A C M A N   3 D

   one spot = 10 points
   eat thing = 50 points
   eating pacs = 500, 1000, 1500, 2000 etc
   free life at 10,000 points

   v1.3

	Copyright (c) 2000-2003 Andrew Pike

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to 
	deal in the Software without restriction, including without limitation the 
	rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
	sell copies of the Software, and to permit persons to whom the Software is 
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in 
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
	IN THE SOFTWARE.

	www.fifsoft.com
                                   */
/***********************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <string.h>
#include <allegro.h>

#ifdef ALLEGRO_WINDOWS
#include <conio.h>
#endif

#include "pac.h"
#include "data/pacdata.h"
#include "data/enem.h"

#define DEBUG         0

#ifdef SHOWDISPLAY
#undef SHOWDISPLAY
#endif

#ifdef ALLEGRO_DOS
#define SHOWDISPLAY
#endif

#ifdef ALLEGRO_LINUX
#if DEBUG
#define SHOWDISPLAY
#endif
#endif

#define BIGSIZE      22
#define NO_ENEMIES    4
#define MAX_OBJECTS 700
#define MAX_SCORES   50

#define GO_UP    1
#define GO_DOWN  2
#define GO_LEFT  3
#define GO_RIGHT 4

#define CALCMETHOD 1

#define PLAYSMP(x) if (opt_soundon) play_sample(x, PACMAN_SOUND_VOL, 128, 1000, FALSE)

/*****/// Timers

volatile int speed_counter = 0;

void increment_speed_counter()
{
   speed_counter++;
}

END_OF_FUNCTION(increment_speed_counter);

/*****/

int PACMAN_VID_MODE = GFX_AUTODETECT;
//int PACMAN_VID_MODE = GFX_AUTODETECT_WINDOWED;
int PACMAN_SOUND_VOL = 200; // 0 - 255
int PACMAN_VID_WIDTH = 320;
int PACMAN_VID_HEIGHT = 200;

int tempy_var=0;
int no_of_objects;
int no_of_scores;

struct enem {
               int enx;
               int eny;
               int enxoff;
               int enyoff;
               int en_dir;
               int en_go;
               int en_inv;
            } enemies[NO_ENEMIES];

struct obje {
               int obx;
               int oby;
               int obon;
               int obtype;
            } objects[MAX_OBJECTS];

struct scor {
               int fsx;
               int fsy;
               int fson;
               int fsscore;
            } scores[MAX_SCORES];     // Go the flyin scoreboard!!

/*****/ // Functions
/*****/

void calculations(void);
void draw_stuff(void);
void enemy_movements(void);
void calc_offsets(float *offx, float *offy, float xoffset, float yoffset);
void new_score(int amt, int nsx, int nsy);
void score_control(void);
int game_sequence(void);
int reset_all(void);
void p3d_puts(char *str);
int run_game(int levno);

int save_game(int level_no, int savefrom);

#ifndef ALLEGRO_WINDOWS
#ifndef ALLEGRO_DOS
int get_exec_dir(char *str);
#endif
#endif

#ifdef ALLEGRO_WINDOWS
void win_error_message(char *msg);
#endif

int now_in_vid_mode=0;

/*****/ // Variables
/*****/

	int screenimage_left = 0;
	int screenimage_top = 0;

   int beginx[16]={3,4,5,6,7,8,9,10,11,12,12,11,10,9,8,7};
   int endx[16]={-2,-3,-4,-5,-6,-7,-8,-7,-6,-5,-4,-3,-2,-1,0,1};

   char pacmap[52][102];
   DATAFILE *mwf;

   DATAFILE *bg;
   DATAFILE *fl;
   DATAFILE *pacdata;
   DATAFILE *enempix;
   DATAFILE *objectpix;

   BITMAP *bk;
   BITMAP *minimap;
   BITMAP *tempscore;
   BITMAP *tempscore2;

   int c1, c2, c3, x=2, y=2, i, nextx=0, nexty=0;
   int xoff=0, yoff=0, frames=0, tempi, mapx, mapy;
   int h_dir, next_dir;
   float newxoff, newyoff;
   int go=0;
   int sqoffset_y[20]={ 0, 10, 22, 10, 10, 10, 20, 30, 0, 0, 30, 10, 10, 10, 10, 10, 20, 46, 10, 10 };
   int sqoffset_x[20]={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 10, 0 };
   int curr_lev;

   long pacscore=0, actscore=0;           // Pacman Stuff
   int paclives=3, pacinv;
   int pacbonus, scorebonuses=1;
   int pac_chomp;

   int speed_switch;                      // Options
   int opt_cheatmode, opt_soundon;
	int opt_nocrack;
	int esc_press;

   int gomess;                            // Messages
   char messtext[50];
	int longmess;

   int end_o_game;

   int objects_left;

   int ghost_mode, ghosts_eaten;          // Enemy Stuff

	char tempstr[100];

   struct sp
   {
      int spx[4];
      int spy[4];
   } start_positions[12];

   extern int stx, sty;

   extern LOADGAME nextloadgame;
	extern LOADGAME startlevelgame;

//          MAIN function
/************************
/  /  /  /  /  /  /  /  /
************************/
int main(int argc, char *argv[])
{
   time_t t;
   int menucode, gamecode;

	#ifndef ALLEGRO_WINDOWS // Doesn't work under windows
	if (argc>1 && (strcmp(argv[1],"help")==0 || strcmp(argv[1],"--help")==0))
	{
		printf("PacMan 3D\n"
		       "usage: pacman [OPTIONS]\n"
		       "  --nosound           turns off sound fx\n"
		       "  --notimer           turns off the timer that controls the speed\n"
		       "  --cheat             turns on cheat mode (see Cheating section)\n"
		       "  --nocrack           removes the bum cracks\n"
		       "  --volume n          change the sound volume; n = 0-255\n"
		       "  --windowed          run in a window\n"
		       "  --fullscreen        run full screen\n"
		       "  --video-width n     change the video width\n"
		       "  --video-height n    change the video height\n"
				 "  --reset-savegames   reset save games\n"
				 "  --reset-hiscores    reset hi-scores\n");
		return 0;
	}
	else if (argc>1 && (strcmp(argv[1],"info")==0 || strcmp(argv[1],"--info")==0))
	{
		printf("PacMan 3D\n"
		       "www.fifsoft.com/pacman\n"
		       "Type \"pacman --help\" for more info\n");
		return 0;
	}

	#ifndef ALLEGRO_DOS
	get_exec_dir(argv[0]);
	#endif
	#endif
	
   opt_cheatmode=0;
   opt_soundon=1;
	opt_nocrack=0;
   speed_switch=1;
   gomess=0;
	longmess=0;
   memset(messtext,' ',sizeof(messtext));
   messtext[0]='\0';

   check_ini_file();
   getcmdline(argc,argv);
   
   // Level 1
   start_positions[0].spx[0]=20;
   start_positions[0].spy[0]=20;
   start_positions[0].spx[1]=20;
   start_positions[0].spy[1]=19;
   start_positions[0].spx[2]=20;
   start_positions[0].spy[2]=18;
   start_positions[0].spx[3]=20;
   start_positions[0].spy[3]=17;
   // Level 2
   start_positions[1].spx[0]=74;
   start_positions[1].spy[0]=22;
   start_positions[1].spx[1]=78;
   start_positions[1].spy[1]=22;
   start_positions[1].spx[2]=78;
   start_positions[1].spy[2]=25;
   start_positions[1].spx[3]=74;
   start_positions[1].spy[3]=25;
   // Level 3
   start_positions[2].spx[0]=15;
   start_positions[2].spy[0]=17;
   start_positions[2].spx[1]=24;
   start_positions[2].spy[1]=17;
   start_positions[2].spx[2]=33;
   start_positions[2].spy[2]=17;
   start_positions[2].spx[3]=45;
   start_positions[2].spy[3]=15;
   // Level 4
   start_positions[3].spx[0]=44;
   start_positions[3].spy[0]=14;
   start_positions[3].spx[1]=44;
   start_positions[3].spy[1]=33;
   start_positions[3].spx[2]=67;
   start_positions[3].spy[2]=15;
   start_positions[3].spx[3]=81;
   start_positions[3].spy[3]=19;
   // Level 5
   start_positions[4].spx[0]=37;
   start_positions[4].spy[0]=12;
   start_positions[4].spx[1]=63;
   start_positions[4].spy[1]=12;
   start_positions[4].spx[2]=63;
   start_positions[4].spy[2]=38;
   start_positions[4].spx[3]=37;
   start_positions[4].spy[3]=38;
   // Level 6
   start_positions[5].spx[0]=5;
   start_positions[5].spy[0]=6;
   start_positions[5].spx[1]=8;
   start_positions[5].spy[1]=40;
   start_positions[5].spx[2]=93;
   start_positions[5].spy[2]=40;
   start_positions[5].spx[3]=86;
   start_positions[5].spy[3]=12;
   // Level 7
   start_positions[6].spx[0]=27;
   start_positions[6].spy[0]=14;
   start_positions[6].spx[1]=29;
   start_positions[6].spy[1]=14;
   start_positions[6].spx[2]=27;
   start_positions[6].spy[2]=16;
   start_positions[6].spx[3]=29;
   start_positions[6].spy[3]=16;
   // Level 8
   start_positions[7].spx[0]=16;
   start_positions[7].spy[0]=28;
   start_positions[7].spx[1]=28;
   start_positions[7].spy[1]=20;
   start_positions[7].spx[2]=34;
   start_positions[7].spy[2]=18;
   start_positions[7].spx[3]=69;
   start_positions[7].spy[3]=18;
   // Level 9
   start_positions[8].spx[0]=36;
   start_positions[8].spy[0]=34;
   start_positions[8].spx[1]=45;
   start_positions[8].spy[1]=43;
   start_positions[8].spx[2]=55;
   start_positions[8].spy[2]=8;
   start_positions[8].spx[3]=90;
   start_positions[8].spy[3]=5;
   // Level 10
   start_positions[9].spx[0]=78;
   start_positions[9].spy[0]=18;
   start_positions[9].spx[1]=60;
   start_positions[9].spy[1]=22;
   start_positions[9].spx[2]=38;
   start_positions[9].spy[2]=8;
   start_positions[9].spx[3]=21;
   start_positions[9].spy[3]=29;
   // Level 11
   start_positions[10].spx[0]=20;
   start_positions[10].spy[0]=9;
   start_positions[10].spx[1]=36;
   start_positions[10].spy[1]=19;
   start_positions[10].spx[2]=18;
   start_positions[10].spy[2]=40;
   start_positions[10].spx[3]=85;
   start_positions[10].spy[3]=9;
   // Level 12
   start_positions[11].spx[0]=15;
   start_positions[11].spy[0]=33;
   start_positions[11].spx[1]=15;
   start_positions[11].spy[1]=7;
   start_positions[11].spx[2]=50;
   start_positions[11].spy[2]=7;
   start_positions[11].spx[3]=52;
   start_positions[11].spy[3]=36;

   /*****/// Randomize

   srand((unsigned) time(&t));

   /*****/ // Init
   /*****/

#ifdef SHOWDISPLAY
   //clrscr();
   //textcolor(15);
   //textbackground(5);
   //gotoxy(1,1);
   //cprintf("                                    PacMan 3D                                 ");
   //textbackground(0);
	printf("PacMan 3D\n\n");
   printf("Initializing Allegro..\n");
   printf("Checking settings..\n");
   if (opt_cheatmode)
      printf("CHEAT MODE on\n");
   if (!opt_soundon)
      printf("SOUND is off\n");
   if (!speed_switch)
      printf("AUTO SPEED SETTING is off\n");
#endif

   allegro_init();
   install_timer();
   install_keyboard();
   reserve_voices(4,4);
   install_sound(DIGI_AUTODETECT, MIDI_NONE,"hi");

   text_mode(-1);

   /*****/ // Level Check
   /*****/
   
   /*for (c1=1;c1<=12;c1++)
   {
   	sprintf(tempstr,"data/lev%d.pac",c1);
	   if (!exists(tempstr))
	   {
	   	//sprintf(tempstr,"ERROR: Level file \"data/lev%d.pac\" does not exist.",c1);
			p3d_puts(tempstr);
			return 1;
		}
   }*/

   /*****/ // Load
   /*****/

#ifdef SHOWDISPLAY
   printf("Loading font data..\n");
#endif

   mwf = load_datafile("data/mwf.dat");             // Datafiles
   if (!mwf)
   {
      p3d_puts("Error loading MWF.DAT.");
      return 1;
   }

#ifdef SHOWDISPLAY
   printf("Loading sprites..\n");
#endif

   bg = load_datafile("data/bg.dat");
   if (!bg)
   {
      p3d_puts("Error loading BG.DAT.");
      return 1;
   }

   fl = load_datafile("data/fl.dat");
   if (!fl)
   {
      p3d_puts("Error loading FL.DAT.");
      return 1;
   }

#ifdef SHOWDISPLAY
   printf("Loading PacMan3D data..\n");
#endif

   pacdata = load_datafile("data/pacdata.dat");
   if (!pacdata)
   {
      p3d_puts("Error loading PACDATA.DAT.");
      return 1;
   }

#ifdef SHOWDISPLAY
   printf("Loading more sprites..\n");
#endif

   enempix = load_datafile("data/enem.dat");
   if (!enempix)
   {
      p3d_puts("Error loading ENEM.DAT.");
      return 1;
   }

   objectpix = load_datafile("data/obj.dat");
   if (!objectpix)
   {
      p3d_puts("Error loading OBJ.DAT.");
      return 1;
   }

#ifdef SHOWDISPLAY
   printf("Creating background and map images..\n");
#endif

   bk = create_bitmap(320,200);                // New Bitmaps
   if (!bk)
   {
      p3d_puts("Error creating bitmap.(bk)");
      return 1;
   }

   minimap = create_bitmap(100,50);
   if (!minimap)
   {
      p3d_puts("Error creating bitmap.(minimap)");
      return 1;
   }

   tempscore = create_bitmap(30,30);
   if (!tempscore)
   {
      p3d_puts("Error creating bitmap.(tempscore)");
      return 1;
   }

   tempscore2 = create_bitmap(250,100);
   if (!tempscore2)
   {
      p3d_puts("Error creating bitmap.(tempscore2)");
      return 1;
   }

#ifdef SHOWDISPLAY
   printf("Setting graphics mode..\n");
#endif

   if (set_gfx_mode(PACMAN_VID_MODE, PACMAN_VID_WIDTH, PACMAN_VID_HEIGHT, 0, 0)!=0)
	{
		//p3d_puts("Unable to enter video mode.");
		sprintf(messtext,"Unable to enter video mode: [%d] %d x %d",PACMAN_VID_MODE,PACMAN_VID_WIDTH,PACMAN_VID_HEIGHT);
		p3d_puts(messtext);
		return 1;
	}
   
   if (PACMAN_VID_WIDTH>320)
	   screenimage_left=(PACMAN_VID_WIDTH-320)/2;
	if (PACMAN_VID_HEIGHT>200)
		screenimage_top=(PACMAN_VID_HEIGHT-200)/2;

   now_in_vid_mode=1;
	set_window_title("PacMan 3D");
	
	run_intro(0);
	
	if (opt_nocrack)
		blot_out_bumcracks();

   /*****/ // Game
   /*****/

   LOCK_VARIABLE(speed_counter);
   LOCK_FUNCTION(increment_speed_counter);

   install_int_ex(increment_speed_counter, BPS_TO_TIMER(80));

   // Begin Game Loop

   // Game Codes
   // 0: Won
   // 1: Lost
   // 2: Quit
   // 3: Restart

   // Menu Codes (same as gamecodes):
   // 2: Quit
   // 3: Start/Restart

   menucode=menu(0);
   while(menucode!=2)
   {
      clear_keybuf();
      gamecode=game_sequence();
      if (gamecode!=2)
         end_game_feature(gamecode);
      if (gamecode!=3)
         do_scoreboard();
      if (gamecode!=3)
         menucode=menu(0);
   }

   /*****/ // Shutdown
   /*****/

   rest(1000);
   destroy_bitmap(bk);
   destroy_bitmap(minimap);
   destroy_bitmap(tempscore);
   destroy_bitmap(tempscore2);
   unload_datafile(mwf);
   unload_datafile(bg);
   unload_datafile(fl);
   unload_datafile(pacdata);
   unload_datafile(enempix);
   unload_datafile(objectpix);

   allegro_exit();

#ifdef SHOWDISPLAY
	printf("================\n");
	printf("PacMan 3D\n");
	printf("By FifSoft\n");
	printf("www.fifsoft.com/pacman\n");
#endif
   return 0;
}
END_OF_MAIN();


// GAME_SEQUENCE function
/************************
/  /  /  /  /  /  /  /  /
************************/
int game_sequence(void)
{
   int a;
   int starthere;

   starthere=reset_all();

   if (starthere<2)
   {
      run_intro(1);        // Level 1
      a=run_game(1);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==2)
   {
      run_intro(2);        // Level 2
      a=run_game(2);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==3)
   {
      run_intro(3);        // Level 3
      a=run_game(3);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==4)
   {
      run_intro(4);        // Level 4
      a=run_game(4);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==5)
   {
      run_intro(5);        // Level 5
      a=run_game(5);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==6)
   {
      run_intro(6);        // Level 6
      a=run_game(6);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==7)
   {
      run_intro(7);        // Level 7
      a=run_game(7);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==8)
   {
      run_intro(8);        // Level 8
      a=run_game(8);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==9)
   {
      run_intro(9);        // Level 9
      a=run_game(9);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==10)
   {
      run_intro(10);       // Level 10
      a=run_game(10);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==11)
   {
      run_intro(11);       // Level 11
      a=run_game(11);
      fade_out(5);
      clear_keybuf();
   }
   if (a==2 || starthere==12)
   {
      run_intro(12);       // Level 12
      a=run_game(12);
      fade_out(5);
      clear_keybuf();
   }

   if (a==2)
      return 0;
   else if (a==1)
      return 1;
   else if (a==3)
      return 3;
   else
      return 2;
}

//     RESET_ALL function
/************************
/  /  /  /  /  /  /  /  /
************************/
int reset_all(void)
{
   if (nextloadgame.onflag)
   {
      actscore=nextloadgame.score;
      pacscore=actscore;
      scorebonuses=(pacscore/5000)+1;
      paclives=nextloadgame.lives;
      nextloadgame.onflag=0;
      return nextloadgame.level;
   }
   else
   {
      actscore=0;
      pacscore=0;
      scorebonuses=1;
      paclives=3;
      return 0;
   }
}

//      RUN_GAME function
/************************
/  /  /  /  /  /  /  /  /
************************/
int run_game(int levno)
{
   int exitcode=4;  // Exit Code
   int m;
   int fade_flag;

   end_o_game=0;
   pacbonus=0;
   pacinv=0;
   ghost_mode=0;
   ghosts_eaten=0;
   curr_lev=levno;
   pac_chomp=0;
   fade_flag=1;
	esc_press=KEYPRESS_OK;
	strcpy(messtext,"PRESS F2 TO SAVE..");
	gomess=1;
	longmess=1;

   /*****/ // Initialise Objects
   /*****/

//                  int onflag;
//                  char name[30];
//                  int score;
//                  int lives;
//                  int level;

	startlevelgame.onflag=1;
	startlevelgame.name[0]='\0';
	startlevelgame.score=actscore;
	startlevelgame.lives=paclives;
	startlevelgame.level=levno;

   for (c1=0;c1<MAX_OBJECTS;c1++)
   {
      objects[c1].obx=0;
      objects[c1].oby=0;
      objects[c1].obon=0;
      objects[c1].obtype=0;
   }

   for (c1=0;c1<MAX_SCORES;c1++)
   {
      scores[c1].fsx=0;
      scores[c1].fsy=0;
      scores[c1].fson=0;
      scores[c1].fsscore=0;
   }

   /*****/ // Data
   /*****/

   if (load_level(pacmap,levno))
   {
		sprintf(tempstr,"Error loading level data (LEV%d.PAC)\n",levno);
		p3d_puts(tempstr);
      return 1;
   }

   x=stx-3;
   y=sty-7;
   xoff=0;
   yoff=0;
   go=0;
   h_dir=GO_DOWN;
   next_dir=0;
   mapx=1;
   mapy=1;

   if (x>25 && mapx!=x-25 && x<75)
      mapx=x-25;
   else if (x>=75)
      mapx=49;

   if (y>25 && mapy!=y-25 && y<35)
      mapy=y-25;
   else if (y>=35)
      mapy=9;

   /*****/// Initialize

   for (c1=0;c1<NO_ENEMIES;c1++)
   {
      enemies[c1].enx=start_positions[levno-1].spx[c1];
      enemies[c1].eny=start_positions[levno-1].spy[c1];
      enemies[c1].enxoff=0;
      enemies[c1].enyoff=0;
      enemies[c1].en_dir=GO_UP;
//      enemies[c1].en_go=(rand() % 4)+1;
      enemies[c1].en_go=GO_UP;
      enemies[c1].en_inv=0;
   }

   /*****/// Create mini map

   clear_to_color(minimap,254);
   for (c1=0;c1<50;c1++)
      for (c2=0;c2<100;c2++)
      {
         if (pacmap[c1][c2]>0 && pacmap[c1][c2]!=8) // #ere
            putpixel(minimap,c2,c1,48);
         else if (pacmap[c1][c2]==8)
            putpixel(minimap,c2,c1,192);
      }

   /*****/ // Game
   /*****/

//   play_sample(pacdata[PACSND01].dat, 255, 128, 1000, FALSE);
   PLAYSMP(pacdata[PACSND01].dat);

   speed_counter=0;
   while(exitcode==4)
   {

#if CALCMETHOD
      /*****/// Calc Oriented
      if (speed_switch)
      {
         while(speed_counter>0)
         {
            calculations();
            speed_counter--;

		      /*****/// Escape

				if (!key[KEY_ESC] && esc_press==KEYPRESS_DOWN)
					esc_press=KEYPRESS_OK;

				if (key[KEY_ESC] && esc_press==KEYPRESS_PRESS)
					esc_press=KEYPRESS_DOWN;

				if (key[KEY_ESC] && esc_press==KEYPRESS_OK)
					esc_press=KEYPRESS_PRESS;

				if (esc_press==KEYPRESS_PRESS)
		      {
				   m=menu(1);
		         clear_keybuf();
				   if (m!=0)
		         {
				      exitcode=2;
						if (m==3)
		               end_o_game=3;
				      else
						   end_o_game=0;
		         }
				   speed_counter=0;
					esc_press=KEYPRESS_PRESS;
		      }
         }
      }
      else
		{
         calculations();

	      /*****/// Escape

			if (!key[KEY_ESC] && esc_press==KEYPRESS_DOWN)
				esc_press=KEYPRESS_OK;

			if (key[KEY_ESC] && esc_press==KEYPRESS_PRESS)
				esc_press=KEYPRESS_DOWN;

			if (key[KEY_ESC] && esc_press==KEYPRESS_OK)
				esc_press=KEYPRESS_PRESS;

			if (esc_press==KEYPRESS_PRESS)
	      {
			   m=menu(1);
	         clear_keybuf();
			   if (m!=0)
	         {
			      exitcode=2;
					if (m==3)
	               end_o_game=3;
			      else
					   end_o_game=0;
	         }
			   speed_counter=0;
				esc_press=KEYPRESS_PRESS;
	      }
		}
      draw_stuff();
#else
      /*****/// Draw Oriented

      calculations();
      if (speed_switch)
      {
         while(speed_counter>0)
         {
            draw_stuff();
            speed_counter--;
         }
      }
      else
         draw_stuff();

      /*****/// Escape

      if (key[KEY_ESC])
      {
         m=menu(1);
         clear_keybuf();
         if (m!=0)
         {
            exitcode=2;
            if (m==3)
               end_o_game=3;
            else
               end_o_game=0;
         }
         speed_counter=0;
      }
#endif
      blit(bk, screen, 0, 0, screenimage_left, screenimage_top, 320, 200);

      // Fade in on first frame

      if (fade_flag)
      {
         fade_in(pacdata[thepal].dat,5);
         fade_flag=0;
         speed_counter=0;
      }

      // The End

      if (end_o_game)
         exitcode=2;
   }

   return end_o_game;
}


//    DRAW_STUFF function
/************************
/  /  /  /  /  /  /  /  /
************************/
void draw_stuff(void)
{
   static float tempx, tempy;
   static int j;

   clear_to_color(bk,2);

   /*****/ // Print Squares (exciting!)
   /*****/

   calc_offsets(&newxoff, &newyoff, xoff, yoff);

   /*****/// Frames.. for animated squares

//   frames++;
//   if (frames>100)
//      frames=0;

   /*****/
   /*****/ // PRINTING SQUARES NOW
   /*****/

   for (c1=0;c1<17;c1++)
      for (c2=beginx[c1];c2>endx[c1];c2--)
      {
         tempi=pacmap[c1+y][c2+x];
         if (tempi<0 && c1+y>0 && c2+x>0 && c1+y<50 && c2+x<100)
            draw_sprite(bk, fl[(tempi*-1)-1].dat, (int)((c2*16)+(c1*16)-newxoff-16),
             (int)((c1*16)-(c2*16)-newyoff+19));
         else if (tempi==0 || c1+y<=0 || c2+x<=0 || c1+y>=50 || c2+x>=100)
            draw_sprite(bk, fl[0].dat, (int)((c2*16)+(c1*16)-newxoff-16),
             (int)((c1*16)-(c2*16)-newyoff+19));
      }

   for (c1=0;c1<17;c1++)
      for (c2=beginx[c1];c2>endx[c1];c2--)
      {
         if (c1==8 && c2==3 && yoff>0)
            ;
         else if (!key[KEY_A])
         {                                 // *** Squares
            tempi=pacmap[c1+y][c2+x];
            if (tempi==8 && frames>50)
               tempi=9;

            if (c1+y>0 && c2+x>0 && c1+y<50 && c2+x<100 && tempi>0 && !key[KEY_G])
               draw_sprite(bk, bg[tempi].dat, (int)((c2*16)+(c1*16)-newxoff-16-sqoffset_x[tempi]),
                 (int)((c1*16)-(c2*16)-newyoff+19-sqoffset_y[tempi]));
//            else if (tempi<0 && c1+y>0 && c2+x>0 && c1+y<50 && c2+x<100)
//               draw_sprite(bk, fl[(tempi*-1)-1].dat, (int)((c2*16)+(c1*16)-newxoff-16),
//                 (int)((c1*16)-(c2*16)-newyoff+19));
//            else
//               draw_sprite(bk, fl[0].dat, (int)((c2*16)+(c1*16)-newxoff-16),
//                 (int)((c1*16)-(c2*16)-newyoff+19));

         /*****/// objects (new!)

            for (i=0;i<no_of_objects;i++)
            {
               if (objects[i].obon && objects[i].obx==x+c2 && objects[i].oby==y+c1)
                  draw_sprite(bk, objectpix[objects[i].obtype].dat, (int)((c2*16)+(c1*16)-newxoff-16),
                     (int)((c1*16)-(c2*16)-newyoff+19));
            }
         }

         /*****/// enemies

         for (i=0;i<NO_ENEMIES;i++)
         {
            if (enemies[i].en_dir!=enemies[i].en_go && enemies[i].en_go!=0)
               enemies[i].en_dir=enemies[i].en_go;

            if (enemies[i].enx==x+c2 && enemies[i].eny==y+c1)
            {
               if (enemies[i].en_dir==GO_DOWN || enemies[i].en_dir==GO_LEFT)
                  j=0;
               else
                  j=1;
               calc_offsets(&tempx, &tempy, enemies[i].enxoff, enemies[i].enyoff);

               if (enemies[i].en_dir==GO_UP || enemies[i].en_dir==GO_LEFT)
               {
                  if (ghost_mode>0)
                     draw_sprite_h_flip(bk, enempix[GHOST0+j].dat, (int)((c2*16)+(c1*16)-newxoff-16+tempx),
                       (int)((c1*16)-(c2*16)-newyoff+19+tempy));
                  else
                     draw_sprite_h_flip(bk, enempix[(i*2)+j].dat, (int)((c2*16)+(c1*16)-newxoff-16+tempx),
                       (int)((c1*16)-(c2*16)-newyoff+19+tempy));
               }
               else
               {
                  if (ghost_mode>0)
                     draw_sprite(bk, enempix[GHOST0+j].dat, (int)((c2*16)+(c1*16)-newxoff-16+tempx),
                       (int)((c1*16)-(c2*16)-newyoff+19+tempy));
                  else
                     draw_sprite(bk, enempix[(i*2)+j].dat, (int)((c2*16)+(c1*16)-newxoff-16+tempx),
                       (int)((c1*16)-(c2*16)-newyoff+19+tempy));
               }

               if (enemies[i].en_inv>0 && (enemies[i].en_inv<10 || (enemies[i].en_inv>20 &&
                   enemies[i].en_inv<30) || enemies[i].en_inv>40))
                  draw_sprite(bk, pacdata[GO].dat, (int)((c2*16)+(c1*16)-newxoff-26+tempx),
                    (int)((c1*16)-(c2*16)-newyoff+9+tempy));
            }
         }

         /*****/

         if (c1==7 && c2==3 && yoff>0)     // *** Block
            c2=endx[c1];

         if ((c1==7 && c2==3 && yoff==0) || (c1==8 && c2==4 && yoff>0))
         {
/*            if (yoff>0 && pacmap[y+8][x+3]==0)
               draw_sprite(bk, fl[0].dat, (int)(176-newxoff-16), (int)(80-newyoff+19));
            else if (yoff>0)
               draw_sprite(bk, fl[(pacmap[y+8][x+3]*-1)-1].dat, (int)(176-newxoff-16), (int)(80-newyoff+19));
*/
            if (h_dir==GO_DOWN || h_dir==GO_LEFT)
               i=0;
            else                                   // *** PacMan
               i=1;

            if (h_dir==GO_UP || h_dir==GO_LEFT)
            {
               if (pac_chomp>0 && i==0)
                  draw_sprite_h_flip(bk, pacdata[PAC03].dat, 144, 83);  // draw pac
               else
                  draw_sprite_h_flip(bk, pacdata[PAC01+i].dat, 144, 83);  // draw pac
            }
            else
            {
               if (pac_chomp>0 && i==0)
                  draw_sprite(bk, pacdata[PAC03].dat, 144, 83);  // draw pac
               else
                  draw_sprite(bk, pacdata[PAC01+i].dat, 144, 83);  // draw pac
            }

            if (yoff>0)             // *** Redraw Row
            {
               for (i=2;i>endx[7];i--)
               {
                  tempi=pacmap[y+7][i+x];
                  if (tempi==8 && frames>50)
                     tempi=9;

                  if (c1+y>0 && i+x>0 && c1+y<50 && i+x<100 && tempi>0)
                     draw_sprite(bk, bg[tempi].dat, (int)((i*16)+112-newxoff-16-sqoffset_x[tempi]),
                       (int)(112-(i*16)-newyoff+19-sqoffset_y[tempi]));

//                else if (tempi<0 && c1+y>0 && i+x>0 && c1+y<50 && i+x<100)
//                   draw_sprite(bk, fl[(tempi*-1)-1].dat, (int)((i*16)+112-newxoff-16),
//                     (int)(112-(i*16)-newyoff+19));
//                else
//                   draw_sprite(bk, fl[0].dat, (int)((i*16)+112-newxoff-16),
//                     (int)(112-(i*16)-newyoff+19));

                  for (c3=0;c3<no_of_objects;c3++) // Draw any objects in this row
                  {
                     if (objects[c3].obon && objects[c3].obx==i+x && objects[c3].oby==y+7)
                        draw_sprite(bk, objectpix[objects[c3].obtype].dat, (int)((i*16)+112-newxoff-16),
                           (int)(112-(i*16)-newyoff+19));
                  }
               }
            }
         }
      }

   /*****/
   /*****/
   /*****/

   if (pacinv>0 && (pacinv<10 || (pacinv>20 && pacinv<30) || pacinv>40))
      draw_sprite(bk, pacdata[GO].dat, 132, 70);

   if (pacbonus>0 && (pacbonus<10 || (pacbonus>20 && pacbonus<30) ||
                     (pacbonus>40 && pacbonus<50) || (pacbonus>60 && pacbonus<70) ||
                     pacbonus>80))
      draw_sprite(bk, pacdata[BONUS].dat, 100, 70);

   /*****/

   if (!key[KEY_N])
   {
      /*****/// Borders

      draw_sprite(bk, pacdata[BORD_T].dat, 0, 0);      // Top
      draw_sprite(bk, pacdata[BORD_B].dat, 0, 195);    // Bottom
      draw_sprite(bk, pacdata[BORD_L].dat, 0, 0);      // Left
      draw_sprite(bk, pacdata[BORD_R].dat, 315, 0);    // Right

      draw_sprite(bk, pacdata[BORD_MAP].dat, 265, 155);       // map window
      if (x>25 && mapx!=x-25 && x<75)
         mapx=x-25;
      if (y>25 && mapy!=y-25 && y<35)
         mapy=y-25;

      if (x>=75 && mapx!=49)
         mapx=49;
      if (y>=35 && mapy!=9)
         mapy=9;

      blit(minimap, bk, mapx, mapy, 266, 156, 50, 40);
      objects_left=0;
      for (c1=0;c1<no_of_objects;c1++)
      {
         if (objects[c1].obx>=mapx && objects[c1].obx<mapx+50 &&
             objects[c1].oby>=mapy && objects[c1].oby<mapy+40 &&
             objects[c1].obon)
            putpixel(bk, 266+objects[c1].obx-mapx, 156+objects[c1].oby-mapy, 49);
         if (objects[c1].obtype<=1 && objects[c1].obon && objects[c1].obtype==0)
            objects_left++;
      }
      putpixel(bk, 269+x-mapx, 163+y-mapy, 14);
      for (c1=0;c1<NO_ENEMIES;c1++)
      {
         if (enemies[c1].enx>=mapx && enemies[c1].enx<mapx+50 &&
             enemies[c1].eny>=mapy && enemies[c1].eny<mapy+40)
            putpixel(bk, 266+enemies[c1].enx-mapx, 156+enemies[c1].eny-mapy, 12);
      }
   }

   /*****/// Scores

   for (c1=0;c1<no_of_scores;c1++)
   {
      if (scores[c1].fson>0)
      {
         clear(tempscore);
         clear(tempscore2);
         textprintf(tempscore, mwf[0].dat, 1, 1, 14, "%d",scores[c1].fsscore);
         stretch_blit(tempscore, tempscore2, 0, 0, 30, 30, 0, 0, 30+scores[c1].fson, 30+scores[c1].fson);
         draw_sprite(bk, tempscore2, (((scores[c1].fsx-x)*16)+((scores[c1].fsy-y)*16)-newxoff-16),
                                     (((scores[c1].fsy-y)*16)-((scores[c1].fsx-x)*16)-newyoff+19));
      }
   }

   /*****/// Text Sextion

#if DEBUG
   textprintf(bk, mwf[0].dat, 1, 1, 12,  "xy=%d,%d --==--==-- map=%d,%d",x,y,mapx,mapy);
#endif
//      textprintf(bk, mwf[0].dat, 1, 1, 12,  "current square    [%d]",pacmap[y+6][x+3]);
//      textprintf(bk, mwf[0].dat, 1, 10, 12, "current square [%d][%d][%d]",pacmap[y+7][x+2],pacmap[y+7][x+3],pacmap[y+7][x+4]);
//      textprintf(bk, mwf[0].dat, 1, 20, 12, "current square    [%d]",pacmap[y+8][x+3]);
//   textprintf(bk, mwf[0].dat, 8, 55, 12, "off[%d,%d]x[%d]",enemies[0].enxoff,enemies[0].enyoff,enemies[0].enx);
//   textprintf(bk, mwf[0].dat, 8, 38, 12, "etemp[%d]",tempy_var);
//   textprintf(bk, mwf[0].dat, 8, 38, 12, "no_of_objects[%d][%d][%d]",no_of_objects,objects[0].obx,objects[0].oby);
/*
   if (speed_switch)
      textprintf(bk, mwf[0].dat, 8, 45, 12, "Auto");
   else
      textprintf(bk, mwf[0].dat, 8, 45, 12, "Off");
*/

   if (gomess>0)
   {
      gomess++;
      textprintf(bk, mwf[0].dat, 8, 45, 14, messtext);
      if (gomess>300 && longmess==0)
         gomess=0;
		else if (gomess>700)
		{
			gomess=0;
			longmess=0;
		}
   }

//   textprintf(bk, mwf[0].dat, 8, 22, 14, "%06ld [%d]",pacscore,no_of_scores);
   textprintf(bk, mwf[0].dat, 8, 22, 14, "%06ld",pacscore);
   textprintf(bk, mwf[0].dat, 8, 30, 14, "%d ORBS LEFT",objects_left);
   for (c1=0;c1<paclives;c1++)
   {
      if (ghost_mode>0 && (ghost_mode<900 || (ghost_mode/10)%2==0))
         draw_sprite(bk, pacdata[PACLIFE2].dat, 8+(c1*12), 8);
      else
         draw_sprite(bk, pacdata[PACLIFE].dat, 8+(c1*12), 8);
   }
   if (ghost_mode>0 && (ghost_mode<900 || (ghost_mode/10)%2==0))
      draw_sprite(bk, pacdata[GHOSTS].dat, 20+(paclives*12), 5);
}


//  CALCULATIONS function
/************************
/  /  /  /  /  /  /  /  /
************************/
void calculations(void)
{
   frames+=2;
   if (frames>100)
      frames=0;

   /*****/ // Calculations
   /*****/

/*   if (key[KEY_F1] && gomess==0)
   {
      gomess=1;
      if (speed_switch==0)
      {
         strcpy(messtext,"SPEED SETTING: AUTO");
         speed_switch=1;
         speed_counter=0;
      }
      else
      {
         strcpy(messtext,"SPEED SETTING: OFF");
         speed_switch=0;
         speed_counter=0;
      }
   }*/

   if (key[KEY_F1] && gomess==0)
   {
      gomess=1;
      if (opt_soundon)
      {
         strcpy(messtext,"SOUND OFF");
         opt_soundon=0;
      }
      else
      {
         strcpy(messtext,"SOUND ON");
         opt_soundon=1;
      }
   }

	if (key[KEY_F2] && gomess==0)
	{
		if (!save_game(curr_lev,1))
			esc_press=KEYPRESS_DOWN;
		clear(screen);
		set_palette(pacdata[thepal].dat);
		speed_counter=0;
	}

   if (h_dir!=go && go!=0)
      h_dir=go;

   /*****/// Invincible

   if (pacinv>0)
   {
      pacinv++;
      if (pacinv>80)
         pacinv=0;
   }

   /*****/// Keys

   if (key[KEY_L])
   {
      xoff=0;
      yoff=0;
   }

   /*****/// New Keys

   if ((key[KEY_DOWN] || key[KEY_3_PAD] || next_dir==GO_DOWN) && pacmap[y+8][x+3]<=0 && xoff==0)
   {
      go=GO_DOWN;
      if (xoff)
         xoff=0;
      if (next_dir)
         next_dir=0;
      if (yoff==0)
         yoff+=2;
   }

   if ((key[KEY_UP] || key[KEY_7_PAD] || next_dir==GO_UP) && pacmap[y+6][x+3]<=0 && xoff==0)
   {
      go=GO_UP;
      if (xoff)
         xoff=0;
      if (next_dir)
         next_dir=0;
      if (yoff==0)
      {
         yoff=BIGSIZE-2;
         y--;
      }
   }

   if ((key[KEY_RIGHT] || key[KEY_9_PAD] || next_dir==GO_RIGHT) && pacmap[y+7][x+4]<=0 && yoff==0) // || (yoff==1 && go!=0)))
   {
      go=GO_RIGHT;
      if (yoff)
         yoff=0;
      if (next_dir)
         next_dir=0;
      if (xoff==0)
         xoff+=2;
   }

   if ((key[KEY_LEFT] || key[KEY_1_PAD] || next_dir==GO_LEFT) && pacmap[y+7][x+2]<=0 && yoff==0) // || (yoff==1 && go!=0)))
   {
      go=GO_LEFT;
      if (yoff)
         yoff=0;
      if (next_dir)
         next_dir=0;
      if (xoff==0)
      {
         xoff=BIGSIZE-2;
         x--;
      }
   }

   /*****/// Pre-move keys

   if ((key[KEY_LEFT] || key[KEY_1_PAD]) && (go==GO_UP || go==GO_DOWN))           // left
   {
      next_dir=GO_LEFT;
      nextx=x;
      nexty=y;
   }

   if ((key[KEY_RIGHT] || key[KEY_9_PAD]) && (go==GO_UP || go==GO_DOWN))          // right
   {
      next_dir=GO_RIGHT;
      nextx=x;
      nexty=y;
   }

   if ((key[KEY_UP] || key[KEY_7_PAD]) && (go==GO_LEFT || go==GO_RIGHT))          // up
   {
      next_dir=GO_UP;
      nextx=x;
      nexty=y;
   }

   if ((key[KEY_DOWN] || key[KEY_3_PAD]) && (go==GO_LEFT || go==GO_RIGHT))        // down
   {
      next_dir=GO_DOWN;
      nextx=x;
      nexty=y;
   }

   if (next_dir && (nextx!=x || nexty!=y))
      next_dir=0;

   /*****/// Movements

   if (go==GO_DOWN) // && yoff>0)
      yoff+=2;
   if (go==GO_UP) // && yoff>0)
      yoff-=2;
   if (go==GO_RIGHT) // && xoff>0)
      xoff+=2;
   if (go==GO_LEFT) // && xoff>0)
      xoff-=2;

   /*****/// Stopping

   if (xoff>=BIGSIZE)         // right
   {
      x++;
      xoff=0;
      if (pacmap[y+7][x+4]>0)
         go=0;
      else
      {
         if (yoff)
            yoff=0;
      }
   }

   if (yoff>=BIGSIZE)         // down
   {
      y++;
      yoff=0;
      if (pacmap[y+8][x+3]>0)
         go=0;
      else
      {
         if (xoff)
            xoff=0;
      }
   }

   if (xoff<0)                // left
   {
      xoff=0;
      if (pacmap[y+7][x+2]>0)
         go=0;
      else
      {
         if (yoff)
            yoff=0;
         xoff=BIGSIZE-2;
         x--;
      }
   }

   if (yoff<0)                // up
   {
      yoff=0;
      if (pacmap[y+6][x+3]>0)
         go=0;
      else
      {
         if (xoff)
            xoff=0;
         yoff=BIGSIZE-2;
         y--;
      }
   }

   /*****/// Scoreboard

   if (actscore>pacscore)
      pacscore++;

   if (pacscore>=(scorebonuses*5000))
   {
      scorebonuses++;
      pacbonus=1;
      paclives++;
      //play_sample(pacdata[PACSND01].dat, 255, 128, 1000, FALSE);
      PLAYSMP(pacdata[PACSND01].dat);
   }

   if (pacbonus>0)
   {
      pacbonus++;
      if (pacbonus>90)
         pacbonus=0;
   }

   //if (actscore>pacscore+100)
   //   pacscore+=99;

   if (pac_chomp>0)
   {
      pac_chomp++;
      if (pac_chomp>5)
         pac_chomp=0;
   }

   /*****/// Enemies

   enemy_movements();

   /*****/// Scores

   score_control();

   /*****/// Objects

   for (c1=0;c1<no_of_objects;c1++)       // Collect Objects
   {
      if (objects[c1].obon && objects[c1].obx==x+3 && objects[c1].oby==y+7)
      {
         if (objects[c1].obtype==0)
         {
            objects[c1].obon=0;
            new_score(10,x+3,y+7);
//            play_sample(pacdata[PACSND03].dat, 255, 128, 1000, FALSE);
            PLAYSMP(pacdata[PACSND03].dat);
            if (objects_left==1)
               end_o_game=2;
            pac_chomp=1;
         }
         else if (objects[c1].obtype==1)
         {
            objects[c1].obon=0;
            new_score(50,x+3,y+7);
//            play_sample(pacdata[PACSND03].dat, 255, 128, 1000, FALSE);
            PLAYSMP(pacdata[PACSND03].dat);
            ghost_mode=1;
            pac_chomp=1;
         }
      }
   }

   /*****/// Ghost Mode

   if (ghost_mode>0)
   {
      ghost_mode++;
      if (ghost_mode>1000)
      {
         ghost_mode=0;
         ghosts_eaten=0;
      }
   }
}

/*****/// ** ** ** calc_offsets
/*******/
void calc_offsets(float *offx, float *offy, float xoffset, float yoffset)
{
   if (xoffset==yoffset)
   {
      *offy=0;
      *offx=hyp(xoffset);
   }
   else if (xoffset>yoffset)
   {
      *offy=antihyp(xoffset-yoffset);
      *offx=hyp(yoffset) + *offy;
      *offy=-*offy;
   }
   else if (xoffset<yoffset)
   {
      *offy=antihyp(yoffset-xoffset);
      *offx=hyp(xoffset) + *offy;
   }
}


/*****/// ** ** ** enemy_movements
/*******/
void enemy_movements(void)
{
   static int e1, etemp;

   for (e1=0;e1<NO_ENEMIES;e1++)
   {
      /*****/// Been Bitten (pacman has)
		// working here  (abs())
      if (pacinv==0 && ((enemies[e1].enx==x+3 && enemies[e1].eny==y+7)
          || ((enemies[e1].enx==x+2 || enemies[e1].enx==x+4) && abs(xoff-enemies[e1].enxoff)<12 && enemies[e1].eny==y+7)
          || ((enemies[e1].eny==y+6 || enemies[e1].eny==y+8) && abs(yoff-enemies[e1].enyoff)<12 && enemies[e1].enx==x+3)))
      //if (pacinv==0 && enemies[e1].enx==x+3 && enemies[e1].eny==y+7)
      {
         if (paclives>0 && ghost_mode==0)
         {
            paclives--;
            x=stx-3;
            y=sty-7;
            h_dir=GO_DOWN;
            go=0;
            xoff=0;
            yoff=0;
            //play_sample(pacdata[PACSND02].dat, 255, 128, 1000, FALSE);
            PLAYSMP(pacdata[PACSND02].dat);
            //actscore+=100;
            mapx=1;
            mapy=1;
            pacinv=1;
         }
         else if (ghost_mode==0)
         {
            end_o_game=1;
            //play_sample(pacdata[PACSND02].dat, 255, 128, 1000, FALSE);
            PLAYSMP(pacdata[PACSND02].dat);
         }
         else if (enemies[e1].en_inv==0)  // time to eat ghosts
         {
            ghosts_eaten++;
            enemies[e1].enx=start_positions[curr_lev-1].spx[e1];
            enemies[e1].eny=start_positions[curr_lev-1].spy[e1];
            enemies[e1].en_dir=GO_UP;
            enemies[e1].en_go=GO_UP;
            enemies[e1].enxoff=0;
            enemies[e1].enyoff=0;
            enemies[e1].en_inv=1;
            new_score(ghosts_eaten*500,x+3,y+7);
            //play_sample(pacdata[PACSND02].dat, 255, 128, 1000, FALSE);
            PLAYSMP(pacdata[PACSND02].dat);
            pac_chomp=1;
         }
      }

      if (enemies[e1].en_inv>0)
      {
         enemies[e1].en_inv++;
         if (enemies[e1].en_inv>50)
            enemies[e1].en_inv=0;
      }

      /*****/ //  GO UP
      /*****/

      if (enemies[e1].en_go==GO_UP)
         enemies[e1].enyoff-=2;
      if (enemies[e1].en_go==GO_DOWN)
         enemies[e1].enyoff+=2;
      if (enemies[e1].en_go==GO_LEFT)
         enemies[e1].enxoff-=2;
      if (enemies[e1].en_go==GO_RIGHT)
         enemies[e1].enxoff+=2;

      if (enemies[e1].enyoff<0)
      {
         if (pacmap[enemies[e1].eny-1][enemies[e1].enx]>0)
         {
            enemies[e1].enyoff=0;
            etemp=(rand() % 2) + 3;
            if (pacmap[enemies[e1].eny][enemies[e1].enx+1]<=0 && etemp==GO_RIGHT)
               enemies[e1].en_go=GO_RIGHT;
            else if (pacmap[enemies[e1].eny][enemies[e1].enx-1]<=0 && etemp==GO_LEFT)
            {
               enemies[e1].en_go=GO_LEFT;
               enemies[e1].enxoff=BIGSIZE;
               enemies[e1].enx--;
            }
            else if (pacmap[enemies[e1].eny][enemies[e1].enx+1]<=0)
               enemies[e1].en_go=GO_RIGHT;
            else if (pacmap[enemies[e1].eny][enemies[e1].enx-1]<=0)
            {
               enemies[e1].en_go=GO_LEFT;
               enemies[e1].enxoff=BIGSIZE;
               enemies[e1].enx--;
            }
            else
               enemies[e1].en_go=GO_DOWN;
         }
         else
         {
            if (pacmap[enemies[e1].eny][enemies[e1].enx-1]<=0 ||
              pacmap[enemies[e1].eny][enemies[e1].enx+1]<=0)
            {
               etemp=(rand() % 4) + 1;
               if (etemp!=GO_UP && etemp!=GO_DOWN)
               {
                  if (pacmap[enemies[e1].eny][enemies[e1].enx+1]<=0 && etemp==GO_RIGHT)
                  {
                     enemies[e1].en_go=GO_RIGHT;
                     enemies[e1].enyoff=0;
                  }
                  else if (pacmap[enemies[e1].eny][enemies[e1].enx-1]<=0 && etemp==GO_LEFT)
                  {
                     enemies[e1].en_go=GO_LEFT;
                     enemies[e1].enyoff=0;
                     enemies[e1].enxoff=BIGSIZE;
                     enemies[e1].enx--;
                  }
               }
               else
               {
                  enemies[e1].enyoff=BIGSIZE;
                  enemies[e1].eny--;
               }
            }
            else
            {
               enemies[e1].enyoff=BIGSIZE;
               enemies[e1].eny--;
            }
         }
      }

      /*****/ //  GO DOWN
      /*****/

      if (enemies[e1].enyoff>BIGSIZE)
      {
         enemies[e1].enyoff=0;
         enemies[e1].eny++;

         if (pacmap[enemies[e1].eny+1][enemies[e1].enx]>0)
         {
            etemp=(rand() % 2) + 3;
            if (pacmap[enemies[e1].eny][enemies[e1].enx+1]<=0 && etemp==GO_RIGHT)
               enemies[e1].en_go=GO_RIGHT;
            else if (pacmap[enemies[e1].eny][enemies[e1].enx-1]<=0 && etemp==GO_LEFT)
            {
               enemies[e1].en_go=GO_LEFT;
               enemies[e1].enxoff=BIGSIZE;
               enemies[e1].enx--;
            }
            else if (pacmap[enemies[e1].eny][enemies[e1].enx+1]<=0)
               enemies[e1].en_go=GO_RIGHT;
            else if (pacmap[enemies[e1].eny][enemies[e1].enx-1]<=0)
            {
               enemies[e1].en_go=GO_LEFT;
               enemies[e1].enxoff=BIGSIZE;
               enemies[e1].enx--;
            }
            else
            {
               enemies[e1].en_go=GO_UP;
               enemies[e1].enyoff=BIGSIZE;
               enemies[e1].eny--;
            }
         }
         else
         {
            if (pacmap[enemies[e1].eny][enemies[e1].enx-1]<=0 ||
              pacmap[enemies[e1].eny][enemies[e1].enx+1]<=0)
            {
               etemp=(rand() % 4) + 1;
               if (etemp!=GO_UP && etemp!=GO_DOWN)
               {
                  if (pacmap[enemies[e1].eny][enemies[e1].enx+1]<=0 && etemp==GO_RIGHT)
                  {
                     enemies[e1].en_go=GO_RIGHT;
                     enemies[e1].enyoff=0;
                  }
                  else if (pacmap[enemies[e1].eny][enemies[e1].enx-1]<=0 && etemp==GO_LEFT)
                  {
                     enemies[e1].en_go=GO_LEFT;
                     enemies[e1].enyoff=0;
                     enemies[e1].enxoff=BIGSIZE;
                     enemies[e1].enx--;
                  }
               }
            }
         }
      }

      /*****/ //  GO LEFT
      /*****/

      if (enemies[e1].enxoff<0)
      {
         if (pacmap[enemies[e1].eny][enemies[e1].enx-1]>0) // ok.. stopped
         {
            enemies[e1].enxoff=0;
            etemp=(rand() % 2) + 1;

            if (pacmap[enemies[e1].eny+1][enemies[e1].enx]<=0 && etemp==GO_DOWN)
               enemies[e1].en_go=GO_DOWN;
            else if (pacmap[enemies[e1].eny-1][enemies[e1].enx]<=0 && etemp==GO_UP)
            {
               enemies[e1].en_go=GO_UP;
               enemies[e1].enyoff=BIGSIZE;
               enemies[e1].eny--;
            }
            else if (pacmap[enemies[e1].eny+1][enemies[e1].enx]<=0)
               enemies[e1].en_go=GO_DOWN;
            else if (pacmap[enemies[e1].eny-1][enemies[e1].enx]<=0)
            {
               enemies[e1].en_go=GO_UP;
               enemies[e1].enyoff=BIGSIZE;
               enemies[e1].eny--;
            }
            else
               enemies[e1].en_go=GO_RIGHT;
         }
         else
         {
            if (pacmap[enemies[e1].eny-1][enemies[e1].enx]<=0 ||
              pacmap[enemies[e1].eny+1][enemies[e1].enx]<=0)
            {
               etemp=(rand() % 4) + 1;
               if (etemp!=GO_LEFT && etemp!=GO_RIGHT)
               {
                  if (pacmap[enemies[e1].eny+1][enemies[e1].enx]<=0 && etemp==GO_DOWN)
                  {
                     enemies[e1].en_go=GO_DOWN;
                     enemies[e1].enxoff=0;
                  }
                  else if (pacmap[enemies[e1].eny-1][enemies[e1].enx]<=0 && etemp==GO_UP)
                  {
                     enemies[e1].en_go=GO_UP;
                     enemies[e1].enxoff=0;
                     enemies[e1].enyoff=BIGSIZE;
                     enemies[e1].eny--;
                  }
               }
               else
               {
                  enemies[e1].enxoff=BIGSIZE;
                  enemies[e1].enx--;
               }
            }
            else
            {
               enemies[e1].enxoff=BIGSIZE;
               enemies[e1].enx--;
            }
         }
      }

      /*****/ //  GO RIGHT
      /*****/

      if (enemies[e1].enxoff>BIGSIZE)
      {
         enemies[e1].enxoff=0;
         enemies[e1].enx++;

         if (pacmap[enemies[e1].eny][enemies[e1].enx+1]>0)
         {
            etemp=(rand() % 2) + 1;
            if (pacmap[enemies[e1].eny+1][enemies[e1].enx]<=0 && etemp==GO_DOWN)
               enemies[e1].en_go=GO_DOWN;
            else if (pacmap[enemies[e1].eny-1][enemies[e1].enx]<=0 && etemp==GO_UP)
            {
               enemies[e1].en_go=GO_UP;
               enemies[e1].enyoff=BIGSIZE;
               enemies[e1].eny--;
            }
            else if (pacmap[enemies[e1].eny+1][enemies[e1].enx]<=0)
               enemies[e1].en_go=GO_DOWN;
            else if (pacmap[enemies[e1].eny-1][enemies[e1].enx]<=0)
            {
               enemies[e1].en_go=GO_UP;
               enemies[e1].enyoff=BIGSIZE;
               enemies[e1].eny--;
            }
            else
            {
               enemies[e1].en_go=GO_LEFT;
               enemies[e1].enxoff=BIGSIZE;
               enemies[e1].enx--;
            }
         }
         else
         {
            /*****/// Options up or down

            if (pacmap[enemies[e1].eny-1][enemies[e1].enx]<=0 ||
              pacmap[enemies[e1].eny+1][enemies[e1].enx]<=0)
            {
               etemp=(rand() % 4) + 1;
               if (etemp!=GO_LEFT && etemp!=GO_RIGHT)
               {
                  if (pacmap[enemies[e1].eny+1][enemies[e1].enx]<=0 && etemp==GO_DOWN)
                  {
                     enemies[e1].en_go=GO_DOWN;
                     enemies[e1].enxoff=0;
                  }
                  else if (pacmap[enemies[e1].eny-1][enemies[e1].enx]<=0 && etemp==GO_UP)
                  {
                     enemies[e1].en_go=GO_UP;
                     enemies[e1].enxoff=0;
                     enemies[e1].enyoff=BIGSIZE;
                     enemies[e1].eny--;
                  }
               }
            }
         }
      }
   }

   tempy_var=etemp;        // temp

   /*****/// Cheatin' Section!!!

   if (opt_cheatmode)
   {
      if (key[KEY_Z])
         end_o_game=2;

      if (key[KEY_X])
      {
         enemies[0].en_inv=1;
         enemies[1].en_inv=1;
         enemies[2].en_inv=1;
         enemies[3].en_inv=1;
      }

      if (key[KEY_C])
         ghost_mode=1;

      /*****/ // Well..
      /*****/

      /*if (key[KEY_W])
      {
         pacscore+=100;
         actscore+=100;
      }*/

      /*****/// Pause

      if (key[KEY_O])
      {
         remove_keyboard();
			#ifdef ALLEGRO_WINDOWS
         getch();
         getch();
			#else
			getchar();
			#endif
         install_keyboard();
         speed_counter=0;
      }
   }

   /*****/// Pause

   if (key[KEY_P])
   {
      draw_sprite(screen,pacdata[PAUSE].dat,screenimage_left+110,screenimage_top+82);
      //play_sample(pacdata[PACSND04].dat, 255, 128, 1000, FALSE);
      PLAYSMP(pacdata[PACSND04].dat);
      clear_keybuf();
      readkey();
      //play_sample(pacdata[PACSND06].dat, 255, 128, 1000, FALSE);
      PLAYSMP(pacdata[PACSND06].dat);
      clear_keybuf();
      rest(100);
      speed_counter=0;
   }
}

/*****/// ** ** ** New Score
/*******/

void new_score(int amt, int nsx, int nsy)
{
   actscore+=amt;
   if (amt>=500)
      pacscore+=amt-100;
   scores[no_of_scores].fsx=nsx;
   scores[no_of_scores].fsy=nsy;
   scores[no_of_scores].fson=1;
   scores[no_of_scores].fsscore=amt;
   no_of_scores++;
}

/*****/// ** ** ** Score Control
/*******/

void score_control(void)
{
   static int sc;

   if (no_of_scores>0 && scores[no_of_scores-1].fson==0)
      no_of_scores--;

   if (no_of_scores>MAX_SCORES-2)
   {
      no_of_scores=10;
      for (sc=40;sc<50;sc++)
      {
         scores[sc-40].fson=scores[sc].fson;
         scores[sc].fson=0;
         scores[sc-40].fsx=scores[sc].fsx;
         scores[sc-40].fsy=scores[sc].fsy;
         scores[sc-40].fsscore=scores[sc].fsscore;
      }
   }

   for (sc=0;sc<no_of_scores;sc++)
   {
      if (scores[sc].fson>0)
      {
         scores[sc].fson+=2;
         if (scores[sc].fson>120)
            scores[sc].fson=0;
      }
   }
}

void p3d_puts(char *str)
{
#ifdef ALLEGRO_WINDOWS
	if (!now_in_vid_mode)
		win_error_message(str);
#else
	printf("%s\n",str);
#endif
}

