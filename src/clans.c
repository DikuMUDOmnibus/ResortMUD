/****************************************************************************
 * ResortMUD 4.0 Beta by Ntanel, Garinan, Badastaz, Josh, Digifuzz, Senir,  *
 * Kratas, Scion, Shogar and Tagith.  Special thanks to Thoric, Nivek,      *
 * Altrag, Arlorn, Justice, Samson, Dace, HyperEye and Yakkov.              *
 ****************************************************************************
 * Copyright (C) 1996 - 2001 Haslage Net Electronics: MudWorld              *
 * of Lorain, Ohio - ALL RIGHTS RESERVED                                    *
 * The text and pictures of this publication, or any part thereof, may not  *
 * be reproduced or transmitted in any form or by any means, electronic or  *
 * mechanical, includes photocopying, recording, storage in a information   *
 * retrieval system, or otherwise, without the prior written or e-mail      *
 * consent from the publisher.                                              *
 ****************************************************************************
 * GREETING must mention ResortMUD programmers and the help file named      *
 * CREDITS must remain completely intact as listed in the SMAUG license.    *
 ****************************************************************************/

/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 *			     Special clan module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"



#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];

CLAN_DATA *first_clan;
CLAN_DATA *last_clan;
COUNCIL_DATA *first_council;
COUNCIL_DATA *last_council;

/* local routines */
void fread_clan args( ( CLAN_DATA * clan, FILE * fp ) );
bool load_clan_file args( ( char *clanfile ) );
void write_clan_list args( ( void ) );

void fread_council args( ( COUNCIL_DATA * council, FILE * fp ) );
bool load_council_file args( ( char *councilfile ) );
void write_council_list args( ( void ) );

/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( char *name )
{
   CLAN_DATA *clan;

   for( clan = first_clan; clan; clan = clan->next )
      if( !str_cmp( name, clan->name ) )
         return clan;
   return NULL;
}

COUNCIL_DATA *get_council( char *name )
{
   COUNCIL_DATA *council;

   for( council = first_council; council; council = council->next )
      if( !str_cmp( name, council->name ) )
         return council;
   return NULL;
}

void write_clan_list(  )
{
   CLAN_DATA *tclan;
   FILE *fpout;
   char filename[256];

   sprintf( filename, "%s%s", CLAN_DIR, CLAN_LIST );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open clan.lst for writing!\r\n", 0 );
      return;
   }
   for( tclan = first_clan; tclan; tclan = tclan->next )
      fprintf( fpout, "%s\n", tclan->filename );
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

void write_council_list(  )
{
   COUNCIL_DATA *tcouncil;
   FILE *fpout;
   char filename[256];

   sprintf( filename, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open council.lst for writing!\r\n", 0 );
      return;
   }
   for( tcouncil = first_council; tcouncil; tcouncil = tcouncil->next )
      fprintf( fpout, "%s\n", tcouncil->filename );
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

/*
 * Save a clan's data to its data file
 */
void save_clan( CLAN_DATA * clan )
{
   FILE *fp;
   char filename[256];

   if( !clan )
   {
      bug( "save_clan: null clan pointer!", 0 );
      return;
   }

   if( !clan->filename || clan->filename[0] == '\0' )
   {
      bug( "save_clan: %s has no filename", clan->name );
      return;
   }

   sprintf( filename, "%s%s", CLAN_DIR, clan->filename );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_clan: fopen", 0 );
      perror( filename );
   }
   else
   {
      fprintf( fp, "#CLAN\n" );
      if( clan->name && clan->name[0] != '\0' )
         fprintf( fp, "Name         %s~\n", clan->name );
      if( clan->filename && clan->filename[0] != '\0' )
         fprintf( fp, "Filename     %s~\n", clan->filename );
      if( clan->motto && clan->motto[0] != '\0' )
         fprintf( fp, "Motto        %s~\n", clan->motto );
      if( clan->description && clan->description[0] != '\0' )
         fprintf( fp, "Description  %s~\n", clan->description );
      if( clan->deity && clan->deity[0] != '\0' )
         fprintf( fp, "Deity        %s~\n", clan->deity );
      if( clan->leader && clan->leader[0] != '\0' )
         fprintf( fp, "Leader       %s~\n", clan->leader );
      if( clan->number1 && clan->number1[0] != '\0' )
         fprintf( fp, "NumberOne    %s~\n", clan->number1 );
      if( clan->number2 && clan->number2[0] != '\0' )
         fprintf( fp, "NumberTwo    %s~\n", clan->number2 );
      if( clan->badge && clan->badge[0] != '\0' )
         fprintf( fp, "Badge        %s~\n", clan->badge );
      fprintf( fp, "PKillRangeNew   %d %d %d %d %d %d %d\n",
               clan->pkills[0], clan->pkills[1], clan->pkills[2],
               clan->pkills[3], clan->pkills[4], clan->pkills[5], clan->pkills[6] );
      fprintf( fp, "PDeathRangeNew  %d %d %d %d %d %d %d\n",
               clan->pdeaths[0], clan->pdeaths[1], clan->pdeaths[2],
               clan->pdeaths[3], clan->pdeaths[4], clan->pdeaths[5], clan->pdeaths[6] );
      fprintf( fp, "MKills       %d\n", clan->mkills );
      fprintf( fp, "MDeaths      %d\n", clan->mdeaths );
      fprintf( fp, "IllegalPK    %d\n", clan->illegal_pk );
      fprintf( fp, "Score        %d\n", clan->score );
      fprintf( fp, "Type         %d\n", clan->clan_type );
      fprintf( fp, "Class        %d\n", clan->class );
      fprintf( fp, "Favour       %d\n", clan->favour );
      fprintf( fp, "Strikes      %d\n", clan->strikes );
      fprintf( fp, "Members      %d\n", clan->members );
      fprintf( fp, "MemLimit     %d\n", clan->mem_limit );
      fprintf( fp, "Alignment    %d\n", clan->alignment );
      fprintf( fp, "Board        %d\n", clan->board );
      fprintf( fp, "ClanObjOne   %d\n", clan->clanobj1 );
      fprintf( fp, "ClanObjTwo   %d\n", clan->clanobj2 );
      fprintf( fp, "ClanObjThree %d\n", clan->clanobj3 );
      fprintf( fp, "ClanObjFour  %d\n", clan->clanobj4 );
      fprintf( fp, "ClanObjFive  %d\n", clan->clanobj5 );
      fprintf( fp, "Recall       %d\n", clan->recall );
      fprintf( fp, "Storeroom    %d\n", clan->storeroom );
      fprintf( fp, "GuardOne     %d\n", clan->guard1 );
      fprintf( fp, "GuardTwo     %d\n", clan->guard2 );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
      fclose( fp );
      fp = NULL;
   }
   return;
}

/*
 * Save a council's data to its data file
 */
void save_council( COUNCIL_DATA * council )
{
   FILE *fp;
   char filename[256];

   if( !council )
   {
      bug( "save_council: null council pointer!", 0 );
      return;
   }

   if( !council->filename || council->filename[0] == '\0' )
   {
      bug( "save_council: %s has no filename", council->name );
      return;
   }

   sprintf( filename, "%s%s", COUNCIL_DIR, council->filename );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_council: fopen", 0 );
      perror( filename );
   }
   else
   {
      fprintf( fp, "#COUNCIL\n" );
      if( council->name && council->name[0] != '\0' )
         fprintf( fp, "Name         %s~\n", council->name );
      if( council->filename && council->filename[0] != '\0' )
         fprintf( fp, "Filename     %s~\n", council->filename );
      if( council->description && council->description[0] != '\0' )
         fprintf( fp, "Description  %s~\n", council->description );
      if( council->head && council->head[0] != '\0' )
         fprintf( fp, "Head         %s~\n", council->head );
      if( council->head2 && council->head2[0] != '\0' )
         fprintf( fp, "Head2        %s~\n", council->head2 );
      fprintf( fp, "Members      %d\n", council->members );
      fprintf( fp, "Board        %d\n", council->board );
      fprintf( fp, "Meeting      %d\n", council->meeting );
      if( council->powers && council->powers[0] != '\0' )
         fprintf( fp, "Powers       %s~\n", council->powers );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
      fclose( fp );
      fp = NULL;
   }
   return;
}

/*
 * Read in actual clan data.
 */
/*
 * Reads in PKill and PDeath still for backward compatibility but now it
 * should be written to PKillRange and PDeathRange for multiple level pkill
 * tracking support. --Shaddai
 * Added a hardcoded limit memlimit to the amount of members a clan can 
 * have set using setclan.  --Shaddai
 */

void fread_clan( CLAN_DATA * clan, FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;

   clan->mem_limit = 0; /* Set up defaults */
   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Alignment", clan->alignment, fread_number( fp ) );
            break;

         case 'B':
            KEY( "Badge", clan->badge, fread_string( fp ) );
            KEY( "Board", clan->board, fread_number( fp ) );
            break;

         case 'C':
            KEY( "ClanObjOne", clan->clanobj1, fread_number( fp ) );
            KEY( "ClanObjTwo", clan->clanobj2, fread_number( fp ) );
            KEY( "ClanObjThree", clan->clanobj3, fread_number( fp ) );
            KEY( "ClanObjFour", clan->clanobj4, fread_number( fp ) );
            KEY( "ClanObjFive", clan->clanobj5, fread_number( fp ) );
            KEY( "Class", clan->class, fread_number( fp ) );
            break;

         case 'D':
            KEY( "Deity", clan->deity, fread_string( fp ) );
            KEY( "Description", clan->description, fread_string( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !clan->name )
                  clan->name = STRALLOC( "" );
               if( !clan->leader )
                  clan->leader = STRALLOC( "" );
               if( !clan->description )
                  clan->description = STRALLOC( "" );
               if( !clan->motto )
                  clan->motto = STRALLOC( "" );
               if( !clan->number1 )
                  clan->number1 = STRALLOC( "" );
               if( !clan->number2 )
                  clan->number2 = STRALLOC( "" );
               if( !clan->deity )
                  clan->deity = STRALLOC( "" );
               if( !clan->badge )
                  clan->badge = STRALLOC( "" );
               return;
            }
            break;

         case 'F':
            KEY( "Favour", clan->favour, fread_number( fp ) );
            KEY( "Filename", clan->filename, fread_string_nohash( fp ) );

         case 'G':
            KEY( "GuardOne", clan->guard1, fread_number( fp ) );
            KEY( "GuardTwo", clan->guard2, fread_number( fp ) );
            break;

         case 'I':
            KEY( "IllegalPK", clan->illegal_pk, fread_number( fp ) );
            break;

         case 'L':
            KEY( "Leader", clan->leader, fread_string( fp ) );
            break;

         case 'M':
            KEY( "MDeaths", clan->mdeaths, fread_number( fp ) );
            KEY( "Members", clan->members, fread_number( fp ) );
            KEY( "MemLimit", clan->mem_limit, fread_number( fp ) );
            KEY( "MKills", clan->mkills, fread_number( fp ) );
            KEY( "Motto", clan->motto, fread_string( fp ) );
            break;

         case 'N':
            KEY( "Name", clan->name, fread_string( fp ) );
            KEY( "NumberOne", clan->number1, fread_string( fp ) );
            KEY( "NumberTwo", clan->number2, fread_string( fp ) );
            break;

         case 'P':
            KEY( "PDeaths", clan->pdeaths[6], fread_number( fp ) );
            KEY( "PKills", clan->pkills[6], fread_number( fp ) );
            /*
             * Addition of New Ranges 
             */
            if( !str_cmp( word, "PDeathRange" ) )
            {
               fMatch = TRUE;
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
            }
            if( !str_cmp( word, "PDeathRangeNew" ) )
            {
               fMatch = TRUE;
               clan->pdeaths[0] = fread_number( fp );
               clan->pdeaths[1] = fread_number( fp );
               clan->pdeaths[2] = fread_number( fp );
               clan->pdeaths[3] = fread_number( fp );
               clan->pdeaths[4] = fread_number( fp );
               clan->pdeaths[5] = fread_number( fp );
               clan->pdeaths[6] = fread_number( fp );
            }
            if( !str_cmp( word, "PKillRangeNew" ) )
            {
               fMatch = TRUE;
               clan->pkills[0] = fread_number( fp );
               clan->pkills[1] = fread_number( fp );
               clan->pkills[2] = fread_number( fp );
               clan->pkills[3] = fread_number( fp );
               clan->pkills[4] = fread_number( fp );
               clan->pkills[5] = fread_number( fp );
               clan->pkills[6] = fread_number( fp );
            }
            if( !str_cmp( word, "PKillRange" ) )
            {
               fMatch = TRUE;
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
            }
            break;

         case 'R':
            KEY( "Recall", clan->recall, fread_number( fp ) );
            break;

         case 'S':
            KEY( "Score", clan->score, fread_number( fp ) );
            KEY( "Strikes", clan->strikes, fread_number( fp ) );
            KEY( "Storeroom", clan->storeroom, fread_number( fp ) );
            break;

         case 'T':
            KEY( "Type", clan->clan_type, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_clan: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

/*
 * Read in actual council data.
 */
void fread_council( COUNCIL_DATA * council, FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'B':
            KEY( "Board", council->board, fread_number( fp ) );
            break;

         case 'D':
            KEY( "Description", council->description, fread_string( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !council->name )
                  council->name = STRALLOC( "" );
               if( !council->description )
                  council->description = STRALLOC( "" );
               if( !council->powers )
                  council->powers = STRALLOC( "" );
               return;
            }
            break;

         case 'F':
            KEY( "Filename", council->filename, fread_string_nohash( fp ) );
            break;

         case 'H':
            KEY( "Head", council->head, fread_string( fp ) );
            KEY( "Head2", council->head2, fread_string( fp ) );
            break;

         case 'M':
            KEY( "Members", council->members, fread_number( fp ) );
            KEY( "Meeting", council->meeting, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", council->name, fread_string( fp ) );
            break;

         case 'P':
            KEY( "Powers", council->powers, fread_string( fp ) );
            break;
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_council: no match: %s", word );
         bug( buf, 0 );
      }
   }
}


/*
 * Load a clan file
 */

bool load_clan_file( char *clanfile )
{
   char filename[256];
   CLAN_DATA *clan;
   FILE *fp;
   bool found;

   CREATE( clan, CLAN_DATA, 1 );

   /*
    * Make sure we default these to 0 --Shaddai 
    */
   clan->pkills[0] = 0;
   clan->pkills[1] = 0;
   clan->pkills[2] = 0;
   clan->pkills[3] = 0;
   clan->pkills[4] = 0;
   clan->pkills[5] = 0;
   clan->pkills[6] = 0;
   clan->pdeaths[0] = 0;
   clan->pdeaths[1] = 0;
   clan->pdeaths[2] = 0;
   clan->pdeaths[3] = 0;
   clan->pdeaths[4] = 0;
   clan->pdeaths[5] = 0;
   clan->pdeaths[6] = 0;

   found = FALSE;
   sprintf( filename, "%s%s", CLAN_DIR, clanfile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {

      found = TRUE;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_clan_file: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "CLAN" ) )
         {
            fread_clan( clan, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            char buf[MAX_STRING_LENGTH];

            sprintf( buf, "Load_clan_file: bad section: %s.", word );
            bug( buf, 0 );
            break;
         }
      }
      fclose( fp );
   }

   if( found )
   {
      ROOM_INDEX_DATA *storeroom;

      LINK( clan, first_clan, last_clan, next, prev );

      if( clan->storeroom == 0 || ( storeroom = get_room_index( clan->storeroom ) ) == NULL )
      {
         log_string( "Storeroom not found" );
         return found;
      }

      sprintf( filename, "%s%s.vault", CLAN_DIR, clan->filename );
      if( ( fp = fopen( filename, "r" ) ) != NULL )
      {
         int iNest;
         bool found;
         OBJ_DATA *tobj, *tobj_next;

         log_string( "Loading clan storage room" );
         rset_supermob( storeroom );
         for( iNest = 0; iNest < MAX_NEST; iNest++ )
            rgObjNest[iNest] = NULL;

         found = TRUE;
         for( ;; )
         {
            char letter;
            char *word;

            letter = fread_letter( fp );
            if( letter == '*' )
            {
               fread_to_eol( fp );
               continue;
            }

            if( letter != '#' )
            {
               bug( "Load_clan_vault: # not found.", 0 );
               bug( clan->name, 0 );
               break;
            }

            word = fread_word( fp );
            if( !str_cmp( word, "OBJECT" ) ) /* Objects  */
               fread_obj( supermob, fp, OS_CARRY );
            else if( !str_cmp( word, "END" ) )  /* Done     */
               break;
            else
            {
               bug( "Load_clan_vault: bad section.", 0 );
               bug( clan->name, 0 );
               break;
            }
         }
         fclose( fp );
         for( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
         {
            tobj_next = tobj->next_content;
            obj_from_char( tobj );
            obj_to_room( tobj, storeroom );
         }
         release_supermob(  );
      }
      else
         log_string( "Cannot open clan vault" );
   }
   else
      DISPOSE( clan );

   return found;
}

/*
 * Load a council file
 */

bool load_council_file( char *councilfile )
{
   char filename[256];
   COUNCIL_DATA *council;
   FILE *fp;
   bool found;

   CREATE( council, COUNCIL_DATA, 1 );

   found = FALSE;
   sprintf( filename, "%s%s", COUNCIL_DIR, councilfile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {

      found = TRUE;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_council_file: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "COUNCIL" ) )
         {
            fread_council( council, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_council_file: bad section.", 0 );
            break;
         }
      }
      fclose( fp );
   }

   if( found )
      LINK( council, first_council, last_council, next, prev );

   else
      DISPOSE( council );

   return found;
}

/*
 * Load in all the clan files.
 */
void load_clans(  )
{
   FILE *fpList;
   char *filename;
   char clanlist[256];
   char buf[MAX_STRING_LENGTH];


   first_clan = NULL;
   last_clan = NULL;

   log_string( "Loading clans..." );

   sprintf( clanlist, "%s%s", CLAN_DIR, CLAN_LIST );
   if( ( fpList = fopen( clanlist, "r" ) ) == NULL )
   {
      perror( clanlist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      log_string( filename );
      if( filename[0] == '$' )
         break;

      if( !load_clan_file( filename ) )
      {
         sprintf( buf, "Cannot load clan file: %s", filename );
         bug( buf, 0 );
      }
   }
   fclose( fpList );
   fpList = NULL;
   log_string( " Done clans " );
   return;
}

/*
 * Load in all the council files.
 */
void load_councils(  )
{
   FILE *fpList;
   char *filename;
   char councillist[256];
   char buf[MAX_STRING_LENGTH];


   first_council = NULL;
   last_council = NULL;

   log_string( "Loading councils..." );

   sprintf( councillist, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
   if( ( fpList = fopen( councillist, "r" ) ) == NULL )
   {
      perror( councillist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      log_string( filename );
      if( filename[0] == '$' )
         break;

      if( !load_council_file( filename ) )
      {
         sprintf( buf, "Cannot load council file: %s", filename );
         bug( buf, 0 );
      }
   }
   fclose( fpList );
   fpList = NULL;
   log_string( " Done councils " );
   return;
}

void do_make( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   CLAN_DATA *clan;
   int level;


   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( str_cmp( ch->name, clan->leader )
       && str_cmp( ch->name, clan->deity ) && ( clan->clan_type != CLAN_GUILD || str_cmp( ch->name, clan->number1 ) ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      char buf[MAX_STRING_LENGTH];





      send_to_char( "You can make the following items:\r\n", ch );



      sprintf( buf, "[ " );

      pObjIndex = get_obj_index( clan->clanobj1 );

      strcat( buf, ( pObjIndex ? pObjIndex->name : "" ) );

      strcat( buf, " ]\r\n[ " );



      pObjIndex = get_obj_index( clan->clanobj2 );

      strcat( buf, ( pObjIndex ? pObjIndex->name : "" ) );

      strcat( buf, " ]\r\n[ " );



      pObjIndex = get_obj_index( clan->clanobj3 );

      strcat( buf, ( pObjIndex ? pObjIndex->name : "" ) );

      strcat( buf, " ]\r\n[ " );



      pObjIndex = get_obj_index( clan->clanobj4 );

      strcat( buf, ( pObjIndex ? pObjIndex->name : "" ) );

      strcat( buf, " ]\r\n[ " );



      pObjIndex = get_obj_index( clan->clanobj5 );

      strcat( buf, ( pObjIndex ? pObjIndex->name : "" ) );

      strcat( buf, " ]\r\n" );



      send_to_char( buf, ch );



      return;

   }

   pObjIndex = get_obj_index( clan->clanobj1 );
   level = 40;


   if( !pObjIndex || !is_name( arg, pObjIndex->name ) )
   {
      pObjIndex = get_obj_index( clan->clanobj2 );
      level = 45;
   }
   if( !pObjIndex || !is_name( arg, pObjIndex->name ) )
   {
      pObjIndex = get_obj_index( clan->clanobj3 );
      level = 50;
   }
   if( !pObjIndex || !is_name( arg, pObjIndex->name ) )
   {
      pObjIndex = get_obj_index( clan->clanobj4 );
      level = 35;
   }
   if( !pObjIndex || !is_name( arg, pObjIndex->name ) )
   {
      pObjIndex = get_obj_index( clan->clanobj5 );
      level = 1;
   }

   if( !pObjIndex || !is_name( arg, pObjIndex->name ) )
   {
      send_to_char( "You don't know how to make that.\r\n", ch );
      return;
   }

   obj = create_object( pObjIndex, level );
   xSET_BIT( obj->extra_flags, ITEM_CLANOBJECT );
   if( CAN_WEAR( obj, ITEM_TAKE ) )
      obj = obj_to_char( obj, ch );
   else
      obj = obj_to_room( obj, ch->in_room );
   act( AT_MAGIC, "$n makes $p!", ch, obj, NULL, TO_ROOM );
   act( AT_MAGIC, "You make $p!", ch, obj, NULL, TO_CHAR );
   return;
}



void do_induct( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   CLAN_DATA *clan;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "induct", ch->pcdata->bestowments ) )
       || !str_cmp( ch->name, clan->deity )
       || !str_cmp( ch->name, clan->leader )
       || !str_cmp( ch->name, clan->number1 )
       || !str_cmp( ch->name, clan->number2 ) || IS_SET( ch->pcdata->flags, PCFLAG_INDUCTOR ) )
      ;
   else
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Induct whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "You can't induct such a godly presence.\r\n", ch );
      return;
   }

   if( clan->clan_type == CLAN_GUILD )
   {
      if( victim->class != clan->class )
      {
         send_to_char( "This player's will is not in accordance with your guild.\r\n", ch );
         return;
      }
   }
   else
   {
      if( victim->level < 10 )
      {
         send_to_char( "This player is not worthy of joining yet.\r\n", ch );
         return;
      }

      if( victim->level > ch->level )
      {
         send_to_char( "This player is too powerful for you to induct.\r\n", ch );
         return;
      }
   }

   if( victim->pcdata->clan )
   {
      if( victim->pcdata->clan->clan_type == CLAN_ORDER )
      {
         if( victim->pcdata->clan == clan )
            send_to_char( "This player already belongs to your order!\r\n", ch );
         else
            send_to_char( "This player already belongs to an order!\r\n", ch );
         return;
      }
      else if( victim->pcdata->clan->clan_type == CLAN_GUILD )
      {
         if( victim->pcdata->clan == clan )
            send_to_char( "This player already belongs to your guild!\r\n", ch );
         else
            send_to_char( "This player already belongs to a guild!\r\n", ch );
         return;
      }
      else
      {
         if( victim->pcdata->clan == clan )
            send_to_char( "This player already belongs to your clan!\r\n", ch );
         else
            send_to_char( "This player already belongs to a clan!\r\n", ch );
         return;
      }
   }
   if( clan->mem_limit && clan->members >= clan->mem_limit )
   {
      send_to_char( "Your clan is too big to induct anymore players.\r\n", ch );
      return;
   }
   clan->members++;
   if( clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_GUILD )
      SET_BIT( victim->speaks, LANG_CLAN );

/*    if ( clan->clan_type != CLAN_NOKILL && clan->clan_type != CLAN_ORDER
    &&   clan->clan_type != CLAN_GUILD )
      SET_BIT( victim->pcdata->flags, PCFLAG_DEADLY ); -- Scion */

   if( clan->clan_type != CLAN_GUILD && clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_NOKILL )
   {
      int sn;

      for( sn = 0; sn < top_sn; sn++ )
      {
         if( skill_table[sn]->guild == clan->class && skill_table[sn]->name != NULL )
         {
            victim->pcdata->learned[sn] = GET_ADEPT( victim, sn );
            ch_printf( victim, "%s instructs you in the ways of %s.\r\n", ch->name, skill_table[sn]->name );
         }
      }
   }

   victim->pcdata->clan = clan;
   STRFREE( victim->pcdata->clan_name );
   victim->pcdata->clan_name = QUICKLINK( clan->name );
   act( AT_MAGIC, "You induct $N into $t", ch, clan->name, victim, TO_CHAR );
   act( AT_MAGIC, "$n inducts $N into $t", ch, clan->name, victim, TO_NOTVICT );
   act( AT_MAGIC, "$n inducts you into $t", ch, clan->name, victim, TO_VICT );
   sprintf( buf, "%s has been inducted into %s!", victim->name, clan->name );
   do_info( ch, buf );
   save_char_obj( victim );
   save_clan( clan );
   return;
}

void do_council_induct( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   COUNCIL_DATA *council;
   char buf[MAX_STRING_LENGTH];
   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   council = get_council( arg2 );

   if( !council )
   {
      send_to_char( "No such council.\r\n", ch );
      return;
   }

   if( ( council->head == NULL || str_cmp( ch->name, council->head ) )
       && ( council->head2 == NULL || str_cmp( ch->name, council->head2 ) ) && str_cmp( council->name, "mortal council" ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }


   if( arg[0] == '\0' )
   {
      send_to_char( "Induct whom into that council?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

/*    if ( victim->level < LEVEL_NEOPHYTE )
    {
	send_to_char( "This player is not worthy of joining any council yet.\r\n", ch );
	return;
    }
*/


   if( council != ch->pcdata->council && council != ch->pcdata->council2 )
   {
      send_to_char( "You cannot induct players into a council you do not SIT on!\r\n", ch );
      return;
   }

   if( victim->pcdata->council )
   {
      if( victim->pcdata->council2 )
      {
         send_to_char( "This player already belongs to 2 councils!\r\n", ch );
         return;
      }
      council->members++;
      victim->pcdata->council2 = council;
      STRFREE( victim->pcdata->council2_name );
      victim->pcdata->council2_name = QUICKLINK( council->name );
      act( AT_MAGIC, "You induct $N into $t", ch, council->name, victim, TO_CHAR );
      act( AT_MAGIC, "$n inducts $N into $t", ch, council->name, victim, TO_ROOM );
      act( AT_MAGIC, "$n inducts you into $t", ch, council->name, victim, TO_VICT );
      save_char_obj( victim );
      save_council( council );
      return;
   }

   council->members++;
   victim->pcdata->council = council;
   STRFREE( victim->pcdata->council_name );
   victim->pcdata->council_name = QUICKLINK( council->name );
   act( AT_MAGIC, "You induct $N into $t", ch, council->name, victim, TO_CHAR );
   act( AT_MAGIC, "$n inducts $N into $t", ch, council->name, victim, TO_ROOM );
   act( AT_MAGIC, "$n inducts you into $t", ch, council->name, victim, TO_VICT );
   sprintf( buf, "%s has been inducted into %s!", victim->name, council->name );
   do_info( ch, buf );
   save_char_obj( victim );
   save_council( council );
   return;
}

void do_outcast( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   CLAN_DATA *clan;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "outcast", ch->pcdata->bestowments ) )
       || !str_cmp( ch->name, clan->deity )
       || !str_cmp( ch->name, clan->leader )
       || !str_cmp( ch->name, clan->number1 )
       || !str_cmp( ch->name, clan->number2 ) || IS_SET( ch->pcdata->flags, PCFLAG_INDUCTOR ) )
      ;
   else
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Outcast whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      if( ch->pcdata->clan->clan_type == CLAN_ORDER )
      {
         send_to_char( "Kick yourself out of your own order?\r\n", ch );
         return;
      }
      else if( ch->pcdata->clan->clan_type == CLAN_GUILD )
      {
         send_to_char( "Kick yourself out of your own guild?\r\n", ch );
         return;
      }
      else
      {
         send_to_char( "Kick yourself out of your own clan?\r\n", ch );
         return;
      }
   }

   if( victim->level > ch->level )
   {
      send_to_char( "This player is too powerful for you to outcast.\r\n", ch );
      return;
   }

   if( victim->pcdata->clan != ch->pcdata->clan )
   {
      if( ch->pcdata->clan->clan_type == CLAN_ORDER )
      {
         send_to_char( "This player does not belong to your order!\r\n", ch );
         return;
      }
      else if( ch->pcdata->clan->clan_type == CLAN_GUILD )
      {
         send_to_char( "This player does not belong to your guild!\r\n", ch );
         return;
      }
      else
      {
         send_to_char( "This player does not belong to your clan!\r\n", ch );
         return;
      }
   }

   if( clan->clan_type != CLAN_GUILD && clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_NOKILL )
   {
      int sn;

      for( sn = 0; sn < top_sn; sn++ )
         if( skill_table[sn]->guild == victim->pcdata->clan->class && skill_table[sn]->name != NULL )
         {
            victim->pcdata->learned[sn] = 0;
            ch_printf( victim, "You forget the ways of %s.\r\n", skill_table[sn]->name );
         }
   }

   if( victim->speaking & LANG_CLAN )
      victim->speaking = LANG_COMMON;
   REMOVE_BIT( victim->speaks, LANG_CLAN );
   --clan->members;
   if( clan->members < 0 )
      clan->members = 0;
   if( !str_cmp( victim->name, ch->pcdata->clan->number1 ) )
   {
      STRFREE( ch->pcdata->clan->number1 );
      ch->pcdata->clan->number1 = STRALLOC( "" );
   }
   if( !str_cmp( victim->name, ch->pcdata->clan->number2 ) )
   {
      STRFREE( ch->pcdata->clan->number2 );
      ch->pcdata->clan->number2 = STRALLOC( "" );
   }
   if( !str_cmp( victim->name, ch->pcdata->clan->deity ) )
   {
      STRFREE( ch->pcdata->clan->deity );
      ch->pcdata->clan->deity = STRALLOC( "" );
   }
   victim->pcdata->clan = NULL;
   STRFREE( victim->pcdata->clan_name );
   victim->pcdata->clan_name = STRALLOC( "" );
   act( AT_MAGIC, "You outcast $N from $t", ch, clan->name, victim, TO_CHAR );
   act( AT_MAGIC, "$n outcasts $N from $t", ch, clan->name, victim, TO_ROOM );
   act( AT_MAGIC, "$n outcasts you from $t", ch, clan->name, victim, TO_VICT );
   if( clan->clan_type != CLAN_GUILD )
   {
      sprintf( buf, "%s has been outcast from %s!", victim->name, clan->name );
      do_info( ch, buf );
   }

/* Outcast flag setting removed by Narn.  It's useless now that deadlies
   remain deadly even on being cast out of a clan.
*/
/*    if ( clan->clan_type != CLAN_GUILD )
	xSET_BIT(victim->act, PLR_OUTCAST);
*/
   save_char_obj( victim );   /* clan gets saved when pfile is saved */
   save_clan( clan );
   return;
}

void do_council_outcast( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   COUNCIL_DATA *council;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   council = get_council( arg2 );
   if( arg[0] == '\0' )
   {
      send_to_char( "Outcast whom from your council?\r\n", ch );
      return;
   }

   if( !council )
   {
      send_to_char( "That is not a council\r\n", ch );
      return;
   }
   if( ( council->head == NULL || str_cmp( ch->name, council->head ) )
       && ( council->head2 == NULL || str_cmp( ch->name, council->head2 ) ) && str_cmp( council->name, "mortal council" ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Kick yourself out of your own council?\r\n", ch );
      return;
   }

   if( council != ch->pcdata->council && council != ch->pcdata->council2 )
   {
      send_to_char( "You can only outcast from YOUR councils!\r\n", ch );
      return;
   }
   if( victim->pcdata->council != council )
   {
      if( victim->pcdata->council2 != council )
      {
         send_to_char( "This player does not belong to THAT council!\r\n", ch );
         return;
      }
      else
      {
         --council->members;
         if( council->members < 0 )
            council->members = 0;
         victim->pcdata->council2 = NULL;
         STRFREE( victim->pcdata->council2_name );
         victim->pcdata->council2_name = STRALLOC( "" );
         act( AT_MAGIC, "You outcast $N from $t", ch, council->name, victim, TO_CHAR );
         act( AT_MAGIC, "$n outcasts $N from $t", ch, council->name, victim, TO_ROOM );
         act( AT_MAGIC, "$n outcasts you from $t", ch, council->name, victim, TO_VICT );
         sprintf( buf, "%s has been outcasted from %s", victim->name, council->name );
         do_info( ch, buf );
         save_char_obj( victim );
         save_council( council );
         return;
      }
   }
   if( council->head2 && !str_cmp( victim->name, ch->pcdata->council->head2 ) )
   {
      STRFREE( ch->pcdata->council->head2 );
      ch->pcdata->council->head2 = NULL;
   }
   --council->members;
   if( council->members < 0 )
      council->members = 0;
   victim->pcdata->council2 = NULL;
   STRFREE( victim->pcdata->council2_name );
   victim->pcdata->council2_name = STRALLOC( "" );
   act( AT_MAGIC, "You outcast $N from $t", ch, council->name, victim, TO_CHAR );
   act( AT_MAGIC, "$n outcasts $N from $t", ch, council->name, victim, TO_ROOM );
   act( AT_MAGIC, "$n outcasts you from $t", ch, council->name, victim, TO_VICT );
   save_char_obj( victim );
   save_council( council );
   return;
}

void do_setclan( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CLAN_DATA *clan;

   set_char_color( AT_PLAIN, ch );
   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Usage: setclan <clan> <field> <deity|leader|number1|number2> <player>\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( " deity leader number1 number2\r\n", ch );
      send_to_char( " members board recall storage guard1 guard2\r\n", ch );
      send_to_char( " align (not functional) memlimit", ch );
      send_to_char( " obj1 obj2 obj3 obj4 obj5\r\n", ch );
      if( get_trust( ch ) >= LEVEL_GOD )
      {
         send_to_char( " name filename motto desc\r\n", ch );
         send_to_char( " favour strikes type class\r\n", ch );
      }
      if( get_trust( ch ) >= LEVEL_IMPLEMENTOR )
         send_to_char( " pkill1-7 pdeath1-7\r\n", ch );
      return;
   }

   clan = get_clan( arg1 );
   if( !clan )
   {
      send_to_char( "No such clan.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "deity" ) )
   {
      STRFREE( clan->deity );
      clan->deity = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "leader" ) )
   {
      STRFREE( clan->leader );
      clan->leader = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "number1" ) )
   {
      STRFREE( clan->number1 );
      clan->number1 = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "number2" ) )
   {
      STRFREE( clan->number2 );
      clan->number2 = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "badge" ) )
   {
      STRFREE( clan->badge );
      clan->badge = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "board" ) )
   {
      clan->board = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "memlimit" ) )
   {
      clan->mem_limit = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "members" ) )
   {
      clan->members = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "recall" ) )
   {
      clan->recall = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "storage" ) )
   {
      clan->storeroom = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "obj1" ) )
   {
      clan->clanobj1 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "obj2" ) )
   {
      clan->clanobj2 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "obj3" ) )
   {
      clan->clanobj3 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "obj4" ) )
   {
      clan->clanobj4 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "obj5" ) )
   {
      clan->clanobj5 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "guard1" ) )
   {
      clan->guard1 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "guard2" ) )
   {
      clan->guard2 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( get_trust( ch ) < LEVEL_GOD )
   {
      do_setclan( ch, "" );
      return;
   }
   if( !str_cmp( arg2, "align" ) )
   {
      clan->alignment = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "type" ) )
   {
      if( !str_cmp( argument, "order" ) )
         clan->clan_type = CLAN_ORDER;
      else if( !str_cmp( argument, "guild" ) )
         clan->clan_type = CLAN_GUILD;
      else
         clan->clan_type = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "class" ) )
   {
      clan->class = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "name" ) )
   {
      STRFREE( clan->name );
      clan->name = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "filename" ) )
   {
      if( clan->filename && clan->filename[0] != '\0' )
         DISPOSE( clan->filename );
      clan->filename = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      write_clan_list(  );
      return;
   }
   if( !str_cmp( arg2, "motto" ) )
   {
      STRFREE( clan->motto );
      clan->motto = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( !str_cmp( arg2, "desc" ) )
   {
      STRFREE( clan->description );
      clan->description = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }
   if( get_trust( ch ) < LEVEL_IMPLEMENTOR )
   {
      do_setclan( ch, "" );
      return;
   }
   if( !str_prefix( "pkill", arg2 ) )
   {
      int temp_value;
      if( !str_cmp( arg2, "pkill1" ) )
         temp_value = 0;
      else if( !str_cmp( arg2, "pkill2" ) )
         temp_value = 1;
      else if( !str_cmp( arg2, "pkill3" ) )
         temp_value = 2;
      else if( !str_cmp( arg2, "pkill4" ) )
         temp_value = 3;
      else if( !str_cmp( arg2, "pkill5" ) )
         temp_value = 4;
      else if( !str_cmp( arg2, "pkill6" ) )
         temp_value = 5;
      else if( !str_cmp( arg2, "pkill7" ) )
         temp_value = 6;
      else
      {
         do_setclan( ch, "" );
         return;
      }
      clan->pkills[temp_value] = atoi( argument );
      send_to_char( "Ok.\r\n", ch );
      return;
   }
   if( !str_prefix( "pdeath", arg2 ) )
   {
      int temp_value;
      if( !str_cmp( arg2, "pdeath1" ) )
         temp_value = 0;
      else if( !str_cmp( arg2, "pdeath2" ) )
         temp_value = 1;
      else if( !str_cmp( arg2, "pdeath3" ) )
         temp_value = 2;
      else if( !str_cmp( arg2, "pdeath4" ) )
         temp_value = 3;
      else if( !str_cmp( arg2, "pdeath5" ) )
         temp_value = 4;
      else if( !str_cmp( arg2, "pdeath6" ) )
         temp_value = 5;
      else if( !str_cmp( arg2, "pdeath7" ) )
         temp_value = 6;
      else
      {
         do_setclan( ch, "" );
         return;
      }
      clan->pdeaths[temp_value] = atoi( argument );
      send_to_char( "Ok.\r\n", ch );
      return;
   }
   do_setclan( ch, "" );
   return;
}

void do_setcouncil( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   COUNCIL_DATA *council;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Usage: setcouncil <council> <field> <value>\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( " head head2 members board meeting\r\n", ch );
      if( get_trust( ch ) >= LEVEL_GOD )
         send_to_char( " name filename desc\r\n", ch );
      if( get_trust( ch ) >= LEVEL_SUB_IMPLEM )
         send_to_char( " powers\r\n", ch );
      return;
   }

   council = get_council( arg1 );
   if( !council )
   {
      send_to_char( "No such council.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "head" ) )
   {
      STRFREE( council->head );
      council->head = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   if( !str_cmp( arg2, "head2" ) )
   {
      if( council->head2 != NULL )
         STRFREE( council->head2 );
      if( !str_cmp( argument, "none" ) || !str_cmp( argument, "clear" ) )
         council->head2 = NULL;
      else
         council->head2 = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }
   if( !str_cmp( arg2, "board" ) )
   {
      council->board = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }
   if( !str_cmp( arg2, "members" ) )
   {
      council->members = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }
   if( !str_cmp( arg2, "meeting" ) )
   {
      council->meeting = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }
   if( get_trust( ch ) < LEVEL_GOD )
   {
      do_setcouncil( ch, "" );
      return;
   }
   if( !str_cmp( arg2, "name" ) )
   {
      STRFREE( council->name );
      council->name = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }
   if( !str_cmp( arg2, "filename" ) )
   {
      if( council->filename && council->filename[0] != '\0' )
         DISPOSE( council->filename );
      council->filename = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      write_council_list(  );
      return;
   }
   if( !str_cmp( arg2, "desc" ) )
   {
      STRFREE( council->description );
      council->description = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }
   if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
   {
      do_setcouncil( ch, "" );
      return;
   }
   if( !str_cmp( arg2, "powers" ) )
   {
      STRFREE( council->powers );
      council->powers = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   do_setcouncil( ch, "" );
   return;
}

/*
 * Added multiple levels on pkills and pdeaths. -- Shaddai
 */

void do_showclan( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: showclan <clan>\r\n", ch );
      return;
   }

   clan = get_clan( argument );
   if( !clan )
   {
      send_to_char( "No such clan, guild or order.\r\n", ch );
      return;
   }

   ch_printf_color( ch, "\r\n&w%s     : &W%s\t\t&wBadge : &W%s\r\n&wFilename : &W%s\r\n&wMotto    : &W%s\r\n",
                    clan->clan_type == CLAN_ORDER ? "Order" :
                    ( clan->clan_type == CLAN_GUILD ? "Guild" : "Clan" ),
                    clan->name, clan->badge ? clan->badge : "(not set)", clan->filename, clan->motto );
   ch_printf_color( ch, "&wDesc     : &W%s\r\n&wDeity    : &W%s\r\n&wLeader   : &W%s\r\n",
                    clan->description, clan->deity, clan->leader );
   ch_printf_color( ch, "&wNumber1  : &W%s\r\n&wNumber2  : &W%s\r\n", clan->number1, clan->number2 );
   ch_printf_color( ch,
                    "&wPKills   : &w1-9:&W%-3d &w10-14:&W%-3d &w15-19:&W%-3d &w20-29:&W%-3d &w30-39:&W%-3d &w40-49:&W%-3d &w50:&W%-3d\r\n",
                    clan->pkills[0], clan->pkills[1], clan->pkills[2], clan->pkills[3], clan->pkills[4], clan->pkills[5],
                    clan->pkills[6] );
   ch_printf_color( ch,
                    "&wPDeaths  : &w1-9:&W%-3d &w10-14:&W%-3d &w15-19:&W%-3d &w20-29:&W%-3d &w30-39:&W%-3d &w40-49:&W%-3d &w50:&W%-3d\r\n",
                    clan->pdeaths[0], clan->pdeaths[1], clan->pdeaths[2], clan->pdeaths[3], clan->pdeaths[4],
                    clan->pdeaths[5], clan->pdeaths[6] );
   ch_printf_color( ch, "&wIllegalPK: &W%-6d\r\n", clan->illegal_pk );
   ch_printf_color( ch, "&wMKills   : &W%-6d   &wMDeaths: &W%-6d\r\n", clan->mkills, clan->mdeaths );
   ch_printf_color( ch, "&wScore    : &W%-6d   &wFavor  : &W%-6d   &wStrikes: &W%d\r\n",
                    clan->score, clan->favour, clan->strikes );
   ch_printf_color( ch, "&wMembers  : &W%-6d   &wMLimit : &W%-6d   &wAlign  : &W%-6d",
                    clan->members, clan->mem_limit, clan->alignment );
   if( clan->clan_type == CLAN_GUILD )
      ch_printf_color( ch, "&wClass  : &W%d &w(&W%s&w)", clan->class, npc_class[clan->class] );
   send_to_char( "\r\n", ch );
   ch_printf_color( ch, "&wBoard    : &W%-5d    &wRecall : &W%-5d    &wStorage: &W%-5d\r\n",
                    clan->board, clan->recall, clan->storeroom );
   ch_printf_color( ch, "&wGuard1   : &W%-5d    &wGuard2 : &W%-5d\r\n", clan->guard1, clan->guard2 );
   ch_printf_color( ch, "&wObj1( &W%d &w)  Obj2( &W%d &w)  Obj3( &W%d &w)  Obj4( &W%d &w)  Obj5( &W%d &w)\r\n",
                    clan->clanobj1, clan->clanobj2, clan->clanobj3, clan->clanobj4, clan->clanobj5 );
   return;
}

void do_showcouncil( CHAR_DATA * ch, char *argument )
{
   COUNCIL_DATA *council;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: showcouncil <council>\r\n", ch );
      return;
   }

   council = get_council( argument );
   if( !council )
   {
      send_to_char( "No such council.\r\n", ch );
      return;
   }

   ch_printf_color( ch, "\r\n&wCouncil :  &W%s\r\n&wFilename:  &W%s\r\n", council->name, council->filename );
   ch_printf_color( ch, "&wHead:      &W%s\r\n", council->head );
   ch_printf_color( ch, "&wHead2:     &W%s\r\n", council->head2 );
   ch_printf_color( ch, "&wMembers:   &W%-d\r\n", council->members );
   ch_printf_color( ch, "&wBoard:     &W%-5d\r\n&wMeeting:   &W%-5d\r\n&wPowers:    &W%s\r\n",
                    council->board, council->meeting, council->powers );
   ch_printf_color( ch, "&wDescription:\r\n&W%s\r\n", council->description );
   return;
}

void do_makeclan( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;

   set_char_color( AT_IMMORT, ch );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: makeclan <clan name>\r\n", ch );
      return;
   }

/*  Otherwise it would be possible to create multiple clans
    with the same name. Bad thing... */

   set_char_color( AT_PLAIN, ch );
   clan = get_clan( argument );
   if( clan )
   {
      send_to_char( "There is already a clan with that name.\r\n", ch );
      return;
   }

   CREATE( clan, CLAN_DATA, 1 );
   LINK( clan, first_clan, last_clan, next, prev );

   clan->name = STRALLOC( argument );
   /*
    * Let's refix this, STRALLOC shouldn't be used for the 'filename'
    * member without changing load_clan() and do_setclan() to employ hashstrings too... 
    */
   clan->filename = str_dup( "" );
   clan->motto = STRALLOC( "" );
   clan->description = STRALLOC( "" );
   clan->deity = STRALLOC( "" );
   clan->leader = STRALLOC( "" );
   clan->number1 = STRALLOC( "" );
   clan->number2 = STRALLOC( "" );
   clan->badge = STRALLOC( "" );
}

void do_makecouncil( CHAR_DATA * ch, char *argument )
{
   char filename[256];
   COUNCIL_DATA *council;
   bool found;

   set_char_color( AT_IMMORT, ch );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: makecouncil <council name>\r\n", ch );
      return;
   }

   found = FALSE;
   sprintf( filename, "%s%s", COUNCIL_DIR, strlower( argument ) );

   CREATE( council, COUNCIL_DATA, 1 );
   LINK( council, first_council, last_council, next, prev );
   council->name = STRALLOC( argument );
   council->head = STRALLOC( "" );
   council->head2 = NULL;
   council->powers = STRALLOC( "" );
}

/*
 * Added multiple level pkill and pdeath support. --Shaddai
 */

void do_clans( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;
   int count = 0;

   if( argument[0] == '\0' )
   {
      set_char_color( AT_BLOOD, ch );
      send_to_char
         ( "\r\nClan          Deity         Leader           Pkills:    Avatar      Other\r\n_________________________________________________________________________\r\n\r\n",
           ch );
      for( clan = first_clan; clan; clan = clan->next )
      {
         if( clan->clan_type == CLAN_ORDER || clan->clan_type == CLAN_GUILD )
            continue;
         set_char_color( AT_GREY, ch );
         ch_printf( ch, "%-13s %-13s %-13s", clan->name, clan->deity, clan->leader );
         set_char_color( AT_BLOOD, ch );
         ch_printf( ch, "                %5d      %5d\r\n",
                    clan->pkills[6], ( clan->pkills[2] + clan->pkills[3] + clan->pkills[4] + clan->pkills[5] ) );
         count++;
      }
      set_char_color( AT_BLOOD, ch );
      if( !count )
         send_to_char( "There are no Clans currently formed.\r\n", ch );
      else
         send_to_char
            ( "_________________________________________________________________________\r\n\r\nUse 'clans <clan>' for detailed information and a breakdown of victories.\r\n",
              ch );
      return;
   }

   clan = get_clan( argument );
   if( !clan || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_ORDER )
   {
      set_char_color( AT_BLOOD, ch );
      send_to_char( "No such clan.\r\n", ch );
      return;
   }
   set_char_color( AT_BLOOD, ch );
   ch_printf( ch, "\r\n%s, '%s'\r\n\r\n", clan->name, clan->motto );
   set_char_color( AT_GREY, ch );
   send_to_char_color( "Victories:&w\r\n", ch );
   ch_printf_color( ch,
                    "    &w15-19...  &r%-4d\r\n    &w20-29...  &r%-4d\r\n    &w30-39...  &r%-4d\r\n    &w40-49...  &r%-4d\r\n",
                    clan->pkills[2], clan->pkills[3], clan->pkills[4], clan->pkills[5] );
   ch_printf_color( ch, "   &wAvatar...  &r%-4d\r\n", clan->pkills[6] );
   set_char_color( AT_GREY, ch );
   ch_printf( ch, "Clan Leader:  %s\r\nNumber One :  %s\r\nNumber Two :  %s\r\nClan Deity :  %s\r\n",
              clan->leader, clan->number1, clan->number2, clan->deity );
   if( !str_cmp( ch->name, clan->deity )
       || !str_cmp( ch->name, clan->leader ) || !str_cmp( ch->name, clan->number1 ) || !str_cmp( ch->name, clan->number2 ) )
      ch_printf( ch, "Members    :  %d\r\n", clan->members );
   set_char_color( AT_BLOOD, ch );
   ch_printf( ch, "\r\nDescription:  %s\r\n", clan->description );
   return;
}

void do_orders( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *order;
   int count = 0;

   if( argument[0] == '\0' )
   {
      set_char_color( AT_DGREEN, ch );
      send_to_char
         ( "\r\nOrder            Deity          Leader           Mkills      Mdeaths\r\n____________________________________________________________________\r\n\r\n",
           ch );
      set_char_color( AT_GREEN, ch );
      for( order = first_clan; order; order = order->next )
         if( order->clan_type == CLAN_ORDER )
         {
            ch_printf( ch, "%-16s %-14s %-14s   %-7d       %5d\r\n",
                       order->name, order->deity, order->leader, order->mkills, order->mdeaths );
            count++;
         }
      set_char_color( AT_DGREEN, ch );
      if( !count )
         send_to_char( "There are no Orders currently formed.\r\n", ch );
      else
         send_to_char
            ( "____________________________________________________________________\r\n\r\nUse 'orders <order>' for more detailed information.\r\n",
              ch );
      return;
   }

   order = get_clan( argument );
   if( !order || order->clan_type != CLAN_ORDER )
   {
      set_char_color( AT_DGREEN, ch );
      send_to_char( "No such Order.\r\n", ch );
      return;
   }

   set_char_color( AT_DGREEN, ch );
   ch_printf( ch, "\r\nOrder of %s\r\n'%s'\r\n\r\n", order->name, order->motto );
   set_char_color( AT_GREEN, ch );
   ch_printf( ch, "Deity      :  %s\r\nLeader     :  %s\r\nNumber One :  %s\r\nNumber Two :  %s\r\n",
              order->deity, order->leader, order->number1, order->number2 );
   if( !str_cmp( ch->name, order->deity )
       || !str_cmp( ch->name, order->leader )
       || !str_cmp( ch->name, order->number1 ) || !str_cmp( ch->name, order->number2 ) )
      ch_printf( ch, "Members    :  %d\r\n", order->members );
   set_char_color( AT_DGREEN, ch );
   ch_printf( ch, "\r\nDescription:\r\n%s\r\n", order->description );
   return;
}

void do_councils( CHAR_DATA * ch, char *argument )
{
   COUNCIL_DATA *council;

   set_char_color( AT_CYAN, ch );
   if( !first_council )
   {
      send_to_char( "There are no councils currently formed.\r\n", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char_color( "\r\n&cTitle                    Head\r\n", ch );
      for( council = first_council; council; council = council->next )
      {
         if( council->head2 != NULL )
            ch_printf_color( ch, "&w%-24s %s and %s\r\n", council->name, council->head, council->head2 );
         else
            ch_printf_color( ch, "&w%-24s %-14s\r\n", council->name, council->head );
      }
      send_to_char_color( "&cUse 'councils <name of council>' for more detailed information.\r\n", ch );
      return;
   }
   council = get_council( argument );
   if( !council )
   {
      send_to_char_color( "&cNo such council exists...\r\n", ch );
      return;
   }
   ch_printf_color( ch, "&c\r\n%s\r\n", council->name );
   if( council->head2 == NULL )
      ch_printf_color( ch, "&cHead:     &w%s\r\n&cMembers:  &w%d\r\n", council->head, council->members );
   else
      ch_printf_color( ch, "&cCo-Heads:     &w%s &cand &w%s\r\n&cMembers:  &w%d\r\n",
                       council->head, council->head2, council->members );
   ch_printf_color( ch, "&cDescription:\r\n&w%s\r\n", council->description );
   return;
}


void do_guilds( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *guild;
   int count = 0;

   if( argument[0] == '\0' )
   {
      set_char_color( AT_HUNGRY, ch );
      send_to_char
         ( "\r\nGuild                 Leader             Mkills       Mdeaths\r\n_____________________________________________________________\r\n\r\n",
           ch );
      set_char_color( AT_YELLOW, ch );
      for( guild = first_clan; guild; guild = guild->next )
         if( guild->clan_type == CLAN_GUILD )
         {
            ++count;
            ch_printf( ch, "%-20s   %-14s     %-6d       %6d\r\n", guild->name, guild->leader, guild->mkills,
                       guild->mdeaths );
         }
      set_char_color( AT_HUNGRY, ch );
      if( !count )
         send_to_char( "There are no guilds currently formed.\r\n", ch );
      else
         send_to_char
            ( "_____________________________________________________________\r\n\r\nUse 'guilds <guild>' for more detailed information.\r\n",
              ch );
      return;
   }

   guild = get_clan( argument );
   if( !guild || guild->clan_type != CLAN_GUILD )
   {
      set_char_color( AT_HUNGRY, ch );
      send_to_char( "No such Guild.\r\n", ch );
      return;
   }
   set_char_color( AT_HUNGRY, ch );
   ch_printf( ch, "\r\n%s\r\n", guild->name );
   set_char_color( AT_YELLOW, ch );
   ch_printf( ch, "Leader:    %s\r\nNumber 1:  %s\r\nNumber 2:  %s\r\nMotto:     %s\r\n",
              guild->leader, guild->number1, guild->number2, guild->motto );
   if( !str_cmp( ch->name, guild->deity )
       || !str_cmp( ch->name, guild->leader )
       || !str_cmp( ch->name, guild->number1 ) || !str_cmp( ch->name, guild->number2 ) )
      ch_printf( ch, "Members:   %d\r\n", guild->members );
   set_char_color( AT_HUNGRY, ch );
   ch_printf( ch, "Guild Description:\r\n%s\r\n", guild->description );
   return;
}

void do_victories( CHAR_DATA * ch, char *argument )
{
   char filename[256];

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   if( ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD )
   {
      sprintf( filename, "%s%s.record", CLAN_DIR, ch->pcdata->clan->name );
      set_pager_color( AT_PURPLE, ch );
      if( !str_cmp( ch->name, ch->pcdata->clan->leader ) && !str_cmp( argument, "clean" ) )
      {
         FILE *fp = fopen( filename, "w" );
         if( fp )
            fclose( fp );
         send_to_pager( "\r\nVictories ledger has been cleared.\r\n", ch );
         return;
      }
      else
      {
         send_to_pager( "\r\nLVL  Character       LVL  Character\r\n", ch );
         show_file( ch, filename );
         return;
      }
   }
   else
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
}

void do_shove( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int exit_dir;
   EXIT_DATA *pexit;
   CHAR_DATA *victim;
   bool nogo;
   ROOM_INDEX_DATA *to_room;
   int schance = 0;
   int race_bonus = 0;

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   if( arg[0] == '\0' )
   {
      send_to_char( "Shove whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You shove yourself around, to no avail.\r\n", ch );
      return;
   }

   if( ch->level - victim->level > 10 || victim->level - ch->level > 10 )
   {
      send_to_char( "There is too great an experience difference for you to even bother.\r\n", ch );
      return;
   }

   if( ( victim->position ) != POS_STANDING )
   {
      act( AT_PLAIN, "$N isn't standing up.", ch, NULL, victim, TO_CHAR );
      return;
   }

   if( arg2[0] == '\0' )
   {
      send_to_char( "Shove them in which direction?\r\n", ch );
      return;
   }

   exit_dir = get_dir( arg2 );
   if( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) && get_timer( victim, TIMER_SHOVEDRAG ) <= 0 )
   {
      send_to_char( "That character cannot be shoved right now.\r\n", ch );
      return;
   }
   victim->position = POS_SHOVE;
   nogo = FALSE;
   if( ( pexit = get_exit( ch->in_room, exit_dir ) ) == NULL )
      nogo = TRUE;
   else
      if( IS_SET( pexit->exit_info, EX_CLOSED )
          && ( !IS_AFFECTED( victim, AFF_PASS_DOOR ) || IS_SET( pexit->exit_info, EX_NOPASSDOOR ) ) )
      nogo = TRUE;
   if( nogo )
   {
      send_to_char( "There's no exit in that direction.\r\n", ch );
      victim->position = POS_STANDING;
      return;
   }
   to_room = pexit->to_room;
   if( IS_SET( to_room->room_flags, ROOM_DEATH ) )
   {
      send_to_char( "You cannot shove someone into a death trap.\r\n", ch );
      victim->position = POS_STANDING;
      return;
   }

   if( ch->in_room->area != to_room->area && !in_hard_range( victim, to_room->area ) )
   {
      send_to_char( "That character cannot enter that area.\r\n", ch );
      victim->position = POS_STANDING;
      return;
   }

   /* Check for class, assign percentage based on that. */
   if( ch->class == CLASS_WARRIOR )
      schance = 70;
   if( ch->class == CLASS_VAMPIRE )
      schance = 65;
   if( ch->class == CLASS_RANGER )
      schance = 60;
   if( ch->class == CLASS_DRUID )
      schance = 45;
   if( ch->class == CLASS_CLERIC )
      schance = 35;
   if( ch->class == CLASS_THIEF )
      schance = 30;
   if( ch->class == CLASS_MAGE )
      schance = 15;
   if( ch->class == CLASS_AUGURER )
      schance = 20;
   if( ch->class == CLASS_PALADIN )
      schance = 55;

/* Add 3 points to chance for every str point above 15, subtract for below 15 */

   schance += ( ( get_curr_str( ch ) - 15 ) * 3 );

   schance += ( ch->level - victim->level );

   /* Check for race, adjust percentage based on that. */
   if( ch->race == RACE_ELF )
      race_bonus = -3;

   if( ch->race == RACE_DWARF )
      race_bonus = 3;

   if( ch->race == RACE_HALFLING )
      race_bonus = -5;

   if( ch->race == RACE_PIXIE )
      race_bonus = -7;

   if( ch->race == RACE_HALF_OGRE )
      race_bonus = 5;

   if( ch->race == RACE_HALF_ORC )
      race_bonus = 7;

   if( ch->race == RACE_HALF_TROLL )
      race_bonus = 10;

   if( ch->race == RACE_HALF_ELF )
      race_bonus = -2;

   if( ch->race == RACE_GITH )
      race_bonus = -2;

   if( ch->race == RACE_PIXIE )
      race_bonus = -7;

   if( ch->race == RACE_DROW )
      race_bonus = 1;

   if( ch->race == RACE_SEA_ELF )
      race_bonus = -1;

   if( ch->race == RACE_LIZARDMAN )
      race_bonus = 4;

   if( ch->race == RACE_GNOME )
      race_bonus = -2;

   schance += race_bonus;

   if( schance < number_percent(  ) )
   {
      send_to_char( "You failed.\r\n", ch );
      victim->position = POS_STANDING;
      return;
   }
   act( AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR );
   act( AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT );
   move_char( victim, get_exit( ch->in_room, exit_dir ), 0 );
   if( !char_died( victim ) )
      victim->position = POS_STANDING;
   WAIT_STATE( ch, 12 );
   /*
    * Remove protection from shove/drag if char shoves -- Blodkai 
    */
   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) && get_timer( ch, TIMER_SHOVEDRAG ) <= 0 )
      add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );
}

void do_drag( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int exit_dir;
   CHAR_DATA *victim;
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *to_room;
   bool nogo;
   int schance = 0;
   int race_bonus = 0;

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Only players can drag.\r\n", ch );
      return;
   }

   if( arg[0] == '\0' )
   {
      send_to_char( "Drag whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You take yourself by the scruff of your neck, but go nowhere.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can only drag players.\r\n", ch );
      return;
   }

   if( !xIS_SET( victim->act, PLR_SHOVEDRAG ) )
   {
      send_to_char( "That character doesn't seem to appreciate your attentions.\r\n", ch );
      return;
   }

   if( victim->fighting )
   {
      send_to_char( "You try, but can't get close enough.\r\n", ch );
      return;
   }

   if( victim->position > 3 )
   {
      send_to_char( "They don't seem to need your assistance.\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' )
   {
      send_to_char( "Drag them in which direction?\r\n", ch );
      return;
   }

   if( ch->level - victim->level > 10 || victim->level - ch->level > 10 )
   {
      send_to_char( "There is too great an experience difference for you to even bother.\r\n", ch );
      return;
   }

   exit_dir = get_dir( arg2 );

   if( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) && get_timer( victim, TIMER_SHOVEDRAG ) <= 0 )
   {
      send_to_char( "That character cannot be dragged right now.\r\n", ch );
      return;
   }

   nogo = FALSE;
   if( ( pexit = get_exit( ch->in_room, exit_dir ) ) == NULL )
      nogo = TRUE;
   else
      if( IS_SET( pexit->exit_info, EX_CLOSED )
          && ( !IS_AFFECTED( victim, AFF_PASS_DOOR ) || IS_SET( pexit->exit_info, EX_NOPASSDOOR ) ) )
      nogo = TRUE;
   if( nogo )
   {
      send_to_char( "There's no exit in that direction.\r\n", ch );
      return;
   }

   to_room = pexit->to_room;
   if( IS_SET( to_room->room_flags, ROOM_DEATH ) )
   {
      send_to_char( "You cannot drag someone into a death trap.\r\n", ch );
      return;
   }

   if( ch->in_room->area != to_room->area && !in_hard_range( victim, to_room->area ) )
   {
      send_to_char( "That character cannot enter that area.\r\n", ch );
      victim->position = POS_STANDING;
      return;
   }

   /* Check for class, assign percentage based on that. */
   if( ch->class == CLASS_WARRIOR )
      schance = 70;
   if( ch->class == CLASS_VAMPIRE )
      schance = 65;
   if( ch->class == CLASS_RANGER )
      schance = 60;
   if( ch->class == CLASS_DRUID )
      schance = 45;
   if( ch->class == CLASS_CLERIC )
      schance = 35;
   if( ch->class == CLASS_THIEF )
      schance = 30;
   if( ch->class == CLASS_MAGE )
      schance = 15;
   if( ch->class == CLASS_AUGURER )
      schance = 20;
   if( ch->class == CLASS_PALADIN )
      schance = 55;

/* Add 3 points to chance for every str point above 15, subtract for below 15 */

   schance += ( ( get_curr_str( ch ) - 15 ) * 3 );

   schance += ( ch->level - victim->level );

/* Check for race, adjust percentage based on that. */
   if( ch->race == RACE_ELF )
      race_bonus = -3;

   if( ch->race == RACE_DWARF )
      race_bonus = 3;

   if( ch->race == RACE_HALFLING )
      race_bonus = -5;

   if( ch->race == RACE_PIXIE )
      race_bonus = -7;

   if( ch->race == RACE_HALF_OGRE )
      race_bonus = 5;

   if( ch->race == RACE_HALF_ORC )
      race_bonus = 7;

   if( ch->race == RACE_HALF_TROLL )
      race_bonus = 10;

   if( ch->race == RACE_HALF_ELF )
      race_bonus = -2;

   if( ch->race == RACE_GITH )
      race_bonus = -2;

   if( ch->race == RACE_PIXIE )
      race_bonus = -7;

   if( ch->race == RACE_DROW )
      race_bonus = 1;

   if( ch->race == RACE_SEA_ELF )
      race_bonus = -1;

   if( ch->race == RACE_LIZARDMAN )
      race_bonus = 4;

   if( ch->race == RACE_GNOME )
      race_bonus = -2;

   schance += race_bonus;

   if( schance < number_percent(  ) )
   {
      send_to_char( "You failed.\r\n", ch );
      victim->position = POS_STANDING;
      return;
   }
   if( victim->position < POS_STANDING )
   {
      short temp;

      temp = victim->position;
      victim->position = POS_DRAG;
      act( AT_ACTION, "You drag $M into the next room.", ch, NULL, victim, TO_CHAR );
      act( AT_ACTION, "$n grabs your hair and drags you.", ch, NULL, victim, TO_VICT );
      move_char( victim, get_exit( ch->in_room, exit_dir ), 0 );
      if( !char_died( victim ) )
         victim->position = temp;
/* Move ch to the room too.. they are doing dragging - Scryn */
      move_char( ch, get_exit( ch->in_room, exit_dir ), 0 );
      WAIT_STATE( ch, 12 );
      return;
   }
   send_to_char( "You cannot do that to someone who is standing.\r\n", ch );
   return;
}
