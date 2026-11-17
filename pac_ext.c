/***********************************/
/*
         P A C M A N   3 D

   Extra Functions

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
#include <math.h>
#include <string.h>
#include <allegro.h>

#ifndef ALLEGRO_WINDOWS
#ifndef ALLEGRO_DOS
#include <unistd.h>
#endif
#endif

#include "data/pacdata.h"
#include "data/enem.h"

#define PLAYSMP(x) if (opt_soundon) play_sample(x, PACMAN_SOUND_VOL, 128, 1000, FALSE)

#define KEYPRESS_OK    0
#define KEYPRESS_PRESS 1
#define KEYPRESS_DOWN  2

void load_scoreboard(void);
void save_scoreboard(void);
int get_sentence(int sx, int sy, int smax, char *rstr);
int load_game(void);
int save_game(int level_no, int savefrom);
void getcmdline(int vArgc, char **vArgv);

int stx, sty;
extern DATAFILE *pacdata;
extern DATAFILE *mwf;
extern DATAFILE *enempix;
extern int no_of_objects;
extern int actscore;
extern int paclives;

extern int opt_cheatmode;    // Options
extern int opt_soundon;
extern int opt_nocrack;
extern int speed_switch;

extern int PACMAN_VID_MODE;
extern int PACMAN_SOUND_VOL;
extern int PACMAN_VID_WIDTH;
extern int PACMAN_VID_HEIGHT;

extern int screenimage_left;
extern int screenimage_top;

extern volatile int speed_counter;

struct obje {
               int obx;
               int oby;
               int obon;
               int obtype;
            };

extern struct obje objects[];

typedef struct {
                  char name[30];
                  long score;
               } SINGLESCORE;

typedef struct {
                  SINGLESCORE hiscorer[10];
               } FULLSCOREBOARD;

FULLSCOREBOARD scoreboard;

typedef struct {
                  int onflag;
                  char name[30];
                  int score;
                  int lives;
                  int level;
               } LOADGAME;

typedef struct {
                  int useflag;
                  LOADGAME games[10];
               } LOADGAMEFILE;

LOADGAME nextloadgame;
LOADGAME startlevelgame;

void reset_savegames(void);
void reset_hiscores(void);

void load_game_file(LOADGAMEFILE *lg);
void save_game_file(LOADGAMEFILE *lg);
void print_all_games(LOADGAMEFILE *lg, int thespot);

/*****/ // HYPOT FUNCTIONS
/*****/

float hyp(float a)
{
   float hypot;
   hypot=sqrt((a*a)*2);
   return hypot;
}

float antihyp(float hypot)
{
   float a;
   a=sqrt((hypot*hypot)/2);
   return a;
}

//     RUN_INTRO function
/************************
/  /  /  /  /  /  /  /  /
************************/
int run_intro(int intro_no)
{
   DATAFILE *titlebmp;
   DATAFILE *titlepal;
   unsigned char k;
   int counter, go_c;
   char temp1[10];

   titlebmp=NULL;
   titlepal=NULL;

   if (intro_no>0)
   {
      sprintf(temp1,"LEVEL%d",intro_no);
      titlebmp=load_datafile_object("data/titles.dat",temp1);
      sprintf(temp1,"L%dPAL",intro_no);
      titlepal=load_datafile_object("data/titles.dat",temp1);
   }

   if (intro_no==0)
   {
      set_palette(black_palette);
      blit(pacdata[TITLE1].dat,screen,0,0,screenimage_left,screenimage_top,320,200);
      fade_in(pacdata[TITLE1PAL].dat,5);
      readkey();
      fade_out(5);
      blit(pacdata[TITLE2].dat,screen,0,0,screenimage_left,screenimage_top,320,200);
      fade_in(pacdata[TITLE2PAL].dat,5);
   }
   else
   {
      if (titlebmp==NULL)
      {
         clear(screen);
         textprintf(screen,mwf[0].dat,screenimage_left+105,screenimage_top+84,15,"UNABLE TO LOAD BITMAP.");
         fade_in(pacdata[thepal].dat,5);
      }
      else
      {
         blit(titlebmp->dat,screen,0,0,screenimage_left,screenimage_top,320,200);
         fade_in(titlepal->dat,5);
      }
   }

   if (intro_no>0)
   {
      rectfill(screen,screenimage_left+90,screenimage_top+180,screenimage_left+230,screenimage_top+194,1);
      rect(screen,screenimage_left+90,screenimage_top+180,screenimage_left+230,screenimage_top+194,15);
      textprintf(screen,mwf[0].dat,screenimage_left+103,screenimage_top+184,15,"PRESS [F2] TO SAVE GAME");
   }

   clear_keybuf();
   k=0;
   counter=80;

   if (intro_no>0)
	{
		go_c=0;
		while (go_c==0)
		{
	      k=(readkey() >> 8);
			if (k==KEY_F2)
			{
		      go_c=save_game(intro_no,0);
				if (!go_c)
				{
			      if (titlebmp==NULL)
			      {
						clear(screen);
						set_palette(pacdata[thepal].dat);
			         textprintf(screen,mwf[0].dat,screenimage_left+105,screenimage_top+84,15,"UNABLE TO LOAD BITMAP.");
			      }
			      else
			      {
						clear(screen);
						set_palette(titlepal->dat);
			         blit(titlebmp->dat,screen,0,0,screenimage_left,screenimage_top,320,200);
			      }
			      rectfill(screen,screenimage_left+90,screenimage_top+180,screenimage_left+230,screenimage_top+194,1);
			      rect(screen,screenimage_left+90,screenimage_top+180,screenimage_left+230,screenimage_top+194,15);
			      textprintf(screen,mwf[0].dat,screenimage_left+103,screenimage_top+184,15,"PRESS [F2] TO SAVE GAME");
				}
			}
			else
				go_c=1;
		}
	}
   else
   {
      while(!keypressed())
      {
         if (counter==100)
            draw_sprite(screen,pacdata[COINS].dat,screenimage_left+97,screenimage_top+88);
         counter++;
         rest(10);
         if (counter>200)
         {
            counter=0;
            blit(pacdata[TITLE2].dat,screen,0,0,screenimage_left,screenimage_top,320,200);
         }
      }
   }

   fade_out(5);

   clear(screen);
   if (titlebmp!=NULL)
      unload_datafile_object(titlebmp);
   if (titlepal!=NULL)
      unload_datafile_object(titlepal);
   return 0;
}

// END_GAME_FEATURE function
/***************************
/  /  /  /  /  /  /  /  / /
***************************/
void end_game_feature(int howitended)
{
   int newx, newy;
   BITMAP *newbk;

   if (howitended==1)
   {
      clear(screen);
      blit(pacdata[WINBG].dat,screen,0,0,screenimage_left,screenimage_top,320,200);
      fade_in(pacdata[thepal].dat,5);

      /*****/ // Losing Sequence
      /*****/

      speed_counter=0;
      newbk=create_bitmap(320,200);

      if (newbk!=NULL)
      {
         newx=10;
			newy=202;
			while (newy>(-36))
         {
            while(speed_counter>0)
            {
	            newx+=2;
					newy-=2;
               speed_counter--;
            }
            clear(newbk);
            blit(pacdata[WINBG].dat,newbk,0,0,0,0,320,200);
            draw_sprite(newbk,pacdata[PAC02].dat,newx,newy);
            blit(newbk,screen,0,0,screenimage_left,screenimage_top,320,200);
         }
      }

      destroy_bitmap(newbk);
      draw_sprite(screen,pacdata[YOULOSE].dat,screenimage_left+77,screenimage_top+80);

      /*****/
   }
   else if (howitended==0)
   {
      clear(screen);
      blit(pacdata[WINBG].dat,screen,0,0,screenimage_left,screenimage_top,320,200);
      fade_in(pacdata[thepal].dat,5);

      /*****/ // Winning Sequence
      /*****/

      speed_counter=0;
      newbk=create_bitmap(320,200);

      if (newbk!=NULL)
      {
         newx=10;
			newy=-36;
			while (newy<202)
         {
            while(speed_counter>0)
            {
	            newx+=2;
					newy+=2;
               speed_counter--;
            }
            clear(newbk);
            blit(pacdata[WINBG].dat,newbk,0,0,0,0,320,200);
            if (((newx/4) % 2)==0)
               draw_sprite(newbk,pacdata[PAC01].dat,newx,newy);
            else
               draw_sprite(newbk,pacdata[PAC03].dat,newx,newy);
            blit(newbk,screen,0,0,screenimage_left,screenimage_top,320,200);
         }
      }

      destroy_bitmap(newbk);

//      textprintf(screen, mwf[0].dat, 140, 90, 14, "YOU WIN");
      draw_sprite(screen,pacdata[YOUWIN].dat,screenimage_left+85,screenimage_top+80);
      textprintf(screen, mwf[0].dat,screenimage_left+100,screenimage_top+120,9,"YOUR FINAL SCORE: %06ld",(long)actscore);

      /*****/
   }

   if (howitended==1 || howitended==0)
   {
      readkey();
//      play_sample(pacdata[PACSND06].dat, 255, 128, 1000, FALSE);
      PLAYSMP(pacdata[PACSND06].dat);
      fade_out(5);
   }
}

//    LOAD_LEVEL function
/************************
/  /  /  /  /  /  /  /  /
************************/
int load_level(char lmap[][102], int levno)
{
   FILE *fp;
   int f1, f2;
   char temp1, temp2;
   unsigned char t;

   fp=NULL;
   if (levno==1)
      fp=fopen("data/lev1.pac","rb");
   else if (levno==2)
      fp=fopen("data/lev2.pac","rb");
   else if (levno==3)
      fp=fopen("data/lev3.pac","rb");
   else if (levno==4)
      fp=fopen("data/lev4.pac","rb");
   else if (levno==5)
      fp=fopen("data/lev5.pac","rb");
   else if (levno==6)
      fp=fopen("data/lev6.pac","rb");
   else if (levno==7)
      fp=fopen("data/lev7.pac","rb");
   else if (levno==8)
      fp=fopen("data/lev8.pac","rb");
   else if (levno==9)
      fp=fopen("data/lev9.pac","rb");
   else if (levno==10)
      fp=fopen("data/lev10.pac","rb");
   else if (levno==11)
      fp=fopen("data/lev11.pac","rb");
   else if (levno==12)
      fp=fopen("data/lev12.pac","rb");
   if (fp==NULL)
      return 1;

   no_of_objects=0;

   temp1=fgetc(fp);   
   temp2=fgetc(fp);
   stx=fgetc(fp);
   sty=fgetc(fp);

   if (temp1!=114 || temp2!='p')
      return 1;

   for (f1=0;f1<50;f1++)
   {
      for (f2=0;f2<100;f2++)
      {
         t=fgetc(fp);
         if (t==100 || t==101 || t==102 || t==103)
         {
            objects[no_of_objects].obx=f2;
            objects[no_of_objects].oby=f1;
            if (t==100 || t==101)
               objects[no_of_objects].obtype=0;
            else
               objects[no_of_objects].obtype=1;
            objects[no_of_objects].obon=1;
            lmap[f1][f2]=0;
            if (t==100 || t==102)
               lmap[f1][f2]=0;
            if (t==101 || t==103)
               lmap[f1][f2]=-2;
            no_of_objects++;
         }
         else
            lmap[f1][f2]=t;
      }
   }

   fclose(fp);
   return 0;
}

/*****/// Load Scoreboard

void load_scoreboard(void)
{
   FILE *fp;
   int c1;

   fp=fopen("hiscore.dat","rb");
   if (fp==NULL)
   {
      memset(&scoreboard,' ',sizeof(scoreboard));
      for (c1=0;c1<10;c1++)
      {
         strcpy(scoreboard.hiscorer[c1].name,"fif");
         scoreboard.hiscorer[c1].score=1000;
      }
   }
   else
   {
      fread(&scoreboard,sizeof(scoreboard),1,fp);
      fclose(fp);
   }
}

/*****/// Save Scoreboard

void save_scoreboard(void)
{
   FILE *fp;

   fp=fopen("hiscore.dat","wb");
   if (fp!=NULL)
   {
      fwrite(&scoreboard,sizeof(scoreboard),1,fp);
      fclose(fp);
   }
}

/*****/ // Menu Func
/*****/

int menu(int mcode)
{
   int exitcode;
   int spot;
   char mch;

   // Menu Codes:
   // 2: Quit
   // 3: Start

   spot=1;
   exitcode=0;
   clear_keybuf();

   if (mcode==0)
   {
      clear(screen);
      set_palette(black_pallete);
      blit(pacdata[MENUBG].dat,screen,0,0,screenimage_left,screenimage_top,320,200);
      fade_in(pacdata[MENUPAL].dat,5);
   }
   else
//      play_sample(pacdata[PACSND06].dat, 255, 128, 1000, FALSE);
      PLAYSMP(pacdata[PACSND06].dat);

   while(exitcode==0)
   {
      draw_sprite(screen,pacdata[M1A+(spot==1)].dat,screenimage_left+110,screenimage_top+40);
      draw_sprite(screen,pacdata[M2A+(spot==2)].dat,screenimage_left+110,screenimage_top+80);
      draw_sprite(screen,pacdata[M3A+(spot==3)].dat,screenimage_left+110,screenimage_top+120);
      draw_sprite(screen,pacdata[M4A+(spot==4)].dat,screenimage_left+110,screenimage_top+160);
      mch=(readkey() >> 8);

      if (mch==KEY_ENTER)
      {
         if (spot!=3 || mcode==1)
         {
            PLAYSMP(pacdata[PACSND05].dat);
            if (spot==1)
               exitcode=3;
            else if (spot==2)
            {
               exitcode=load_game();
               if (mcode==1)
               {
                  clear(screen);
	               set_palette(pacdata[thepal].dat);
						if (exitcode==0)
			            return 0;
               }
               else if (exitcode==0)
               {
                  clear(screen);
                  set_palette(pacdata[MENUPAL].dat);
                  blit(pacdata[MENUBG].dat,screen,0,0,screenimage_left,screenimage_top,320,200);
               }
            }
            else if (spot==4)
               exitcode=2;
            else if (spot==3)
               return 0;
         }
         else
//            play_sample(pacdata[PACSND05].dat, 255, 128, 1000, FALSE);
            PLAYSMP(pacdata[PACSND05].dat);
      }
      else if (mch==KEY_ESC && mcode==0)
      {
//         play_sample(pacdata[PACSND06].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND06].dat);
         exitcode=2;
      }
      else if (mch==KEY_ESC && mcode==1)
      {
//         play_sample(pacdata[PACSND06].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND06].dat);
         return 0;
      }
      else if (mch==KEY_UP)
      {
//         play_sample(pacdata[PACSND04].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND04].dat);
         spot--;
         if (spot<1)
            spot=4;
      }
      else if (mch==KEY_DOWN)
      {
//         play_sample(pacdata[PACSND04].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND04].dat);
         spot++;
         if (spot>4)
            spot=1;
      }
   }

   if (mcode==0)
   {
      fade_out(5);
      clear(screen);
   }

   return exitcode;
}

/*****/ // Sore Board
/*****/

void do_scoreboard(void)
{
   char tempname[30];
   int c1, notthistime;

   load_scoreboard();
   clear_keybuf();
   clear(screen);

   set_palette(black_pallete);
   blit(pacdata[SCOREBG].dat,screen,0,0,screenimage_left,screenimage_top,320,200);

   if (actscore>scoreboard.hiscorer[9].score)
   {
      fade_in(pacdata[SCOREBGPAL].dat,5);
      textprintf(screen, mwf[0].dat, screenimage_left+120, screenimage_top+80, 15, "CONGRATULATIONS!");
      textprintf(screen, mwf[0].dat, screenimage_left+100, screenimage_top+90, 15, "YOU HAVE MADE A HISCORE!");
      textprintf(screen, mwf[0].dat, screenimage_left+120, screenimage_top+100, 15, "Enter Your Name:");
      rectfill(screen,screenimage_left+80,screenimage_top+110,screenimage_left+240,screenimage_top+130,1);
      rect(screen,screenimage_left+80,screenimage_top+110,screenimage_left+240,screenimage_top+130,15);
      text_mode(1);
      notthistime=get_sentence(screenimage_left+90,screenimage_top+118,26,tempname);
      text_mode(-1);
      clear_keybuf();

      if (!notthistime)
      {
         c1=8;
         while(actscore>scoreboard.hiscorer[c1].score && c1>=0)
         {
            scoreboard.hiscorer[c1+1].score=scoreboard.hiscorer[c1].score;
            strcpy(scoreboard.hiscorer[c1+1].name,scoreboard.hiscorer[c1].name);
            c1--;
         }

         scoreboard.hiscorer[c1+1].score=actscore;
         strcpy(scoreboard.hiscorer[c1+1].name,tempname);

         save_scoreboard();
      }

      fade_out(5);
      blit(pacdata[SCOREBG].dat,screen,0,0,screenimage_left,screenimage_top,320,200);
   }

   textprintf(screen, mwf[0].dat, screenimage_left+40, screenimage_top+20, 15, "PACMAN 3D SCOREBOARD");
   for (c1=0;c1<10;c1++)
   {
      textprintf(screen, mwf[0].dat, screenimage_left+40, screenimage_top+45+c1*15, 15, "%s",scoreboard.hiscorer[c1].name);
      textprintf(screen, mwf[0].dat, screenimage_left+250, screenimage_top+45+c1*15, 15, "%06ld",scoreboard.hiscorer[c1].score);
   }
   fade_in(pacdata[SCOREBGPAL].dat,5);
   readkey();
   fade_out(5);
   clear(screen);
}

/*****/ // GET sentence
/*****/

int get_sentence(int sx, int sy, int smax, char *rstr)
{
   char temp1[100];
   unsigned char k;
   int leftpos;

   leftpos=0;
   memset(temp1,' ',sizeof(temp1));
   temp1[0]='\0';
   textprintf(screen, mwf[0].dat, sx, sy, 15, "%s_  ", temp1);

   k=readkey();
   if (k==0)
      k=readkey();
   while(k!=13)
   {
      if (k>31 && k<128 && leftpos<smax)
      {
         temp1[leftpos]=k;
         temp1[leftpos+1]='\0';
         leftpos++;
      }
      else if (k==8 && leftpos>0)
      {
         temp1[leftpos-1]='\0';
         leftpos--;
      }
      else if (k==27)
      {
         strcpy(rstr,temp1);
         return 1;
      }

      textprintf(screen, mwf[0].dat, sx, sy, 15, "%s_  ", temp1);

      k=readkey();
      if (k==0)
         k=readkey();
   }
   strcpy(rstr,temp1);
   return 0;
}

/*****/// LoadGame
/*****/

int load_game(void)
{
   LOADGAMEFILE lgfile;
   unsigned char k;
   int exitcode, spot, c1;

   load_game_file(&lgfile);

   clear(screen);
   set_palette(pacdata[SCOREBGPAL].dat);
   blit(pacdata[SCOREBG].dat,screen,0,0,screenimage_left,screenimage_top,320,200);

   textprintf(screen,mwf[0].dat,screenimage_left+20,screenimage_top+10,15,"LOAD GAME");
   exitcode=-1;
   spot=1;
   clear_keybuf();
   for (c1=0;c1<10;c1++)
   {
      rectfill(screen, screenimage_left+20, screenimage_top+45+c1*15, screenimage_left+180, screenimage_top+55+c1*15, 1);
      rectfill(screen, screenimage_left+220, screenimage_top+45+c1*15, screenimage_left+300, screenimage_top+55+c1*15, 1);
      rect(screen, screenimage_left+20, screenimage_top+45+c1*15, screenimage_left+180, screenimage_top+55+c1*15, 15);
      rect(screen, screenimage_left+220, screenimage_top+45+c1*15, screenimage_left+300, screenimage_top+55+c1*15, 15);
      if (c1==spot-1)
      {
         if (lgfile.games[c1].onflag)
         {
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+c1*15, 9, "%s",lgfile.games[c1].name);
            textprintf(screen, mwf[0].dat, screenimage_left+230, screenimage_top+47+c1*15, 9, "Level %d",lgfile.games[c1].level);
         }
         else
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+c1*15, 9, "[EMPTY]");
      }
      else
      {
         if (lgfile.games[c1].onflag)
         {
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+c1*15, 15, "%s",lgfile.games[c1].name);
            textprintf(screen, mwf[0].dat, screenimage_left+230, screenimage_top+47+c1*15, 15, "Level %d",lgfile.games[c1].level);
         }
         else
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+c1*15, 15, "[EMPTY]");
      }
   }

   while(exitcode==-1)
   {
      k=(readkey() >> 8);

      if (k==KEY_ENTER && lgfile.games[spot-1].onflag)
      {
//         play_sample(pacdata[PACSND05].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND05].dat);
         strcpy(nextloadgame.name,lgfile.games[spot-1].name);
         nextloadgame.score=lgfile.games[spot-1].score;
         nextloadgame.level=lgfile.games[spot-1].level;
         nextloadgame.lives=lgfile.games[spot-1].lives;
         nextloadgame.onflag=1;
         exitcode=3;
      }
      else if (k==KEY_ENTER)
      {
//         play_sample(pacdata[PACSND06].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND06].dat);
      }
      else if (k==KEY_ESC)
      {
//         play_sample(pacdata[PACSND06].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND06].dat);
         exitcode=0;
      }
      else if (k==KEY_UP)
      {
         spot--;
         if (spot<1)
            spot=10;
         print_all_games(&lgfile,spot);
//         play_sample(pacdata[PACSND04].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND04].dat);
      }
      else if (k==KEY_DOWN)
      {
         spot++;
         if (spot>10)
            spot=1;
         print_all_games(&lgfile,spot);
//         play_sample(pacdata[PACSND04].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND04].dat);
      }
   }
   return exitcode;
}

/*****/ // Save Game
/*****/

int save_game(int level_no, int savefrom)
{
   LOADGAMEFILE lgfile;
   unsigned char k;
   int exitcode, spot, c1;
   char temp1[40];

   load_game_file(&lgfile);

   PLAYSMP(pacdata[PACSND05].dat);

   clear(screen);
   set_palette(pacdata[SCOREBGPAL].dat);
   blit(pacdata[SCOREBG].dat,screen,0,0,screenimage_left,screenimage_top,320,200);

   textprintf(screen,mwf[0].dat,screenimage_left+20,screenimage_top+10,15,"SAVE GAME");
   exitcode=-1;
   spot=1;
   clear_keybuf();
   for (c1=0;c1<10;c1++)
   {
      rectfill(screen, screenimage_left+20, screenimage_top+45+c1*15, screenimage_left+180, screenimage_top+55+c1*15, 1);
      rectfill(screen, screenimage_left+220, screenimage_top+45+c1*15, screenimage_left+300, screenimage_top+55+c1*15, 1);
      rect(screen, screenimage_left+20, screenimage_top+45+c1*15, screenimage_left+180, screenimage_top+55+c1*15, 15);
      rect(screen, screenimage_left+220, screenimage_top+45+c1*15, screenimage_left+300, screenimage_top+55+c1*15, 15);
      if (c1==spot-1)
      {
         if (lgfile.games[c1].onflag)
         {
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+c1*15, 9, "%s",lgfile.games[c1].name);
            textprintf(screen, mwf[0].dat, screenimage_left+230, screenimage_top+47+c1*15, 9, "Level %d",lgfile.games[c1].level);
         }
         else
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+c1*15, 9, "[EMPTY]");
      }
      else
      {
         if (lgfile.games[c1].onflag)
         {
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+c1*15, 15, "%s",lgfile.games[c1].name);
            textprintf(screen, mwf[0].dat, screenimage_left+230, screenimage_top+47+c1*15, 15, "Level %d",lgfile.games[c1].level);
         }
         else
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+c1*15, 15, "[EMPTY]");
      }
   }

   text_mode(1);

   while(exitcode==-1)
   {
      k=(readkey() >> 8);

      if (k==KEY_ENTER)
      {
         textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+(spot-1)*15, 15, "                          ");
         if (get_sentence(screenimage_left+30,screenimage_top+47+(spot-1)*15,26,temp1))
         {
//            play_sample(pacdata[PACSND06].dat, 255, 128, 1000, FALSE);
            PLAYSMP(pacdata[PACSND06].dat);
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+(spot-1)*15, 15, "                          ");
            print_all_games(&lgfile,spot);
         }
         else
         {
//            play_sample(pacdata[PACSND05].dat, 255, 128, 1000, FALSE);
            PLAYSMP(pacdata[PACSND05].dat);
            strcpy(lgfile.games[spot-1].name,temp1);
            
				if (savefrom==0)
				{
					lgfile.games[spot-1].level=level_no;
		         lgfile.games[spot-1].score=actscore;
			      lgfile.games[spot-1].lives=paclives;
				   lgfile.games[spot-1].onflag=1;
				}
				else
				{
					lgfile.games[spot-1].level=level_no;
		         lgfile.games[spot-1].score=startlevelgame.score;
			      lgfile.games[spot-1].lives=startlevelgame.lives;
				   lgfile.games[spot-1].onflag=1;
				}
            
				save_game_file(&lgfile);
            exitcode=1;
         }
      }
      else if (k==KEY_ESC)
      {
//         play_sample(pacdata[PACSND06].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND06].dat);
         exitcode=0;
      }
      else if (k==KEY_UP)
      {
         spot--;
         if (spot<1)
            spot=10;
         print_all_games(&lgfile,spot);
//         play_sample(pacdata[PACSND04].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND04].dat);
      }
      else if (k==KEY_DOWN)
      {
         spot++;
         if (spot>10)
            spot=1;
         print_all_games(&lgfile,spot);
//         play_sample(pacdata[PACSND04].dat, 255, 128, 1000, FALSE);
         PLAYSMP(pacdata[PACSND04].dat);
      }
   }

   text_mode(-1);
	return exitcode;
}

void load_game_file(LOADGAMEFILE *lg)
{
   FILE *fp;

   fp=fopen("save.dat","rb");
   if (fp==NULL)
   {
      memset(lg,0,sizeof(LOADGAMEFILE));
   }
   else
   {
      fread(lg,sizeof(LOADGAMEFILE),1,fp);
      fclose(fp);
   }
}

void save_game_file(LOADGAMEFILE *lg)
{
   FILE *fp;

   fp=fopen("save.dat","wb");
   if (fp!=NULL)
   {
      fwrite(lg,sizeof(LOADGAMEFILE),1,fp);
      fclose(fp);
   }
}

void print_all_games(LOADGAMEFILE *lg, int thespot)
{
   int p1;

   for (p1=0;p1<10;p1++)
   {
      if (p1==thespot-1)
      {
         if (lg->games[p1].onflag)
         {
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+p1*15, 9, "%s",lg->games[p1].name);
            textprintf(screen, mwf[0].dat, screenimage_left+230, screenimage_top+47+p1*15, 9, "Level %d",lg->games[p1].level);
         }
         else
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+p1*15, 9, "[EMPTY]");
      }
      else
      {
         if (lg->games[p1].onflag)
         {
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+p1*15, 15, "%s",lg->games[p1].name);
            textprintf(screen, mwf[0].dat, screenimage_left+230, screenimage_top+47+p1*15, 15, "Level %d",lg->games[p1].level);
         }
         else
            textprintf(screen, mwf[0].dat, screenimage_left+30, screenimage_top+47+p1*15, 15, "[EMPTY]");
      }
   }
}

/*****/ // CHECK INI FILE
/*****/

void check_ini_file(void)
{
	// Look for PACMAN.INI
	
	FILE *fp;
	char line[1000], *list[500]; //, *ptr;
	int c1, list_num; //, found;
	
	for (c1=0;c1<500;c1++)
		list[c1]=NULL;
	list_num=0;
	
	fp=fopen("pacman.ini","r");
	if (fp)
	{
		line[0]='\0';
		fgets(line,999,fp);

		while (!feof(fp) && list_num<499)
		{
			if (strlen(line)>0 && line[strlen(line)-1]=='\n')
				line[strlen(line)-1]='\0';
				
			if (*line!='\0' && *line!='#' && list_num<500)
			{
				list[list_num]=(char*)malloc(sizeof(char)*(strlen(line)+1));
				strcpy(list[list_num],line);
				list_num++;
			}

			line[0]='\0';
			fgets(line,999,fp);
		}

		if (strlen(line)>0 && line[strlen(line)-1]=='\n')
			line[strlen(line)-1]='\0';
			
		if (strlen(line)>0 && *line!='\0' && *line!='#' && list_num<500)
		{
			list[list_num]=(char*)malloc(sizeof(char)*(strlen(line)+1));
			strcpy(list[list_num],line);
			list_num++;
		}

		fclose(fp);
	}
	
	if (list_num>0)
	{
		getcmdline(list_num,list);
		for (c1=0;c1>list_num;c1++)
		{
			//fprintf(stderr,"[%s]\n",list[c1]);
			free(list[c1]);
		}
	}
}

/*****/ // GetCmdLine
/*****/

void getcmdline(int vArgc, char **vArgv)
{
	char *ptr;
   int c1;

   for (c1=0;c1<vArgc;c1++)
   {
		ptr=vArgv[c1];
		if (*ptr=='-' && *(ptr+1)=='-' && *(ptr+2)!='\0')
			ptr+=2;
	
      if (stricmp(ptr,"cheat")==0)
         opt_cheatmode=1;
      if (stricmp(ptr,"nosound")==0)
         opt_soundon=0;
      if (stricmp(ptr,"notimer")==0)
         speed_switch=0;
      if (stricmp(ptr,"nocrack")==0) // blot 'em out!
         opt_nocrack=1;
         
      // NEW!
      
      if (stricmp(ptr,"volume")==0)
      {
      	c1++;
      	if (c1<vArgc)
      		PACMAN_SOUND_VOL=atoi(vArgv[c1]);
      }

		// Video Options

      if (stricmp(ptr,"windowed")==0)
         PACMAN_VID_MODE=GFX_AUTODETECT_WINDOWED;
      if (stricmp(ptr,"fullscreen")==0)
         PACMAN_VID_MODE=GFX_AUTODETECT_FULLSCREEN;

      if (stricmp(ptr,"video-width")==0)
      {
      	c1++;
      	if (c1<vArgc)
      		PACMAN_VID_WIDTH=atoi(vArgv[c1]);
      }
      if (stricmp(ptr,"video-height")==0)
      {
      	c1++;
      	if (c1<vArgc)
      		PACMAN_VID_HEIGHT=atoi(vArgv[c1]);
      }

		// Reset Data
      
      if (stricmp(ptr,"reset-savegames")==0)
      {
      	reset_savegames();
      }

      if (stricmp(ptr,"reset-hiscores")==0)
      {
      	reset_hiscores();
      }
   }
}

// No more gaps!

void blot(BITMAP *bl, int bx, int by)
{
	int n;
	
	if (bx==0)
		n=getpixel(bl,bx,by);
	else
		n=getpixel(bl,bx-1,by);

	putpixel(bl,bx,by,n);
}

void blot_em_out(BITMAP *b)
{
	blot(b,9,17);
	blot(b,13,17);
	blot(b,10,18);
	blot(b,12,18);
	blot(b,11,19);
	blot(b,11,20);
	blot(b,10,21);
	blot(b,10,22);
	blot(b,10,23);
	blot(b,11,23);
	blot(b,11,24);
	blot(b,11,25);
	blot(b,12,26);
	blot(b,13,27);
	blot(b,14,28);
	blot(b,15,29);
}

void blot_out_bumcracks(void)
{
	blot_em_out(pacdata[PAC02].dat);
	blot_em_out(enempix[EN1_1].dat);
	blot_em_out(enempix[EN2_1].dat);
	blot_em_out(enempix[EN3_1].dat);
	blot_em_out(enempix[EN4_1].dat);
	blot_em_out(enempix[GHOST1].dat);
}

// Change directory to where executable is stored

#ifndef ALLEGRO_WINDOWS
#ifndef ALLEGRO_DOS
int get_exec_dir(char *str)
{
	char tmp[500], *e, *ptr;
	int n;

	// Get file name
		
	e=getenv("PWD");
	if (!e || strlen(e)<=0)
		return 0;
	
	if (strlen(str)>0 && *str=='/')
		sprintf(tmp,"%s",str);
	else
		sprintf(tmp,"%s/%s",e,str);
	
	// Get path
		
	ptr=tmp+strlen(tmp);
	while (ptr>tmp && *ptr!='/')
		ptr--;
	if (ptr>tmp)
		*ptr='\0';

	n=chdir(tmp);
	if (n<0)
		return 0;
	return 1;
}
#endif
#endif

// Reset Save Games

void reset_savegames(void)
{
   FILE *fp;
   LOADGAMEFILE lg;

   fp=fopen("save.dat","wb");
   if (fp)
   {
      memset(&lg,0,sizeof(LOADGAMEFILE));
      fwrite(&lg,sizeof(LOADGAMEFILE),1,fp);
      fclose(fp);

	   #ifndef ALLEGRO_WINDOWS
   	printf("Savegames reset.\n");
   	#endif
   }
}

// Reset Hi Scores

void reset_hiscores(void)
{
   FILE *fp;
   int c1;

   fp=fopen("hiscore.dat","wb");
   if (fp)
   {
      memset(&scoreboard,' ',sizeof(scoreboard));
      for (c1=0;c1<10;c1++)
      {
         strcpy(scoreboard.hiscorer[c1].name,"fif");
         scoreboard.hiscorer[c1].score=1000;
      }
      fwrite(&scoreboard,sizeof(scoreboard),1,fp);
      fclose(fp);

	   #ifndef ALLEGRO_WINDOWS
   	printf("Scoreboard reset.\n");
   	#endif
   }
}

