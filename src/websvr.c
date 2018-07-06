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

/* ROM 2.4 Integrated Web Server - Version 1.0
 *
 * This is my first major snippet... Please be kind. ;-)
 * Copyright 1998 -- Defiant -- Rob Siemborski -- mud@towers.crusoe.net
 *
 * Many thanks to Russ and the rest of the developers of ROM for creating
 * such an excellent codebase to program on.
 *
 * If you use this code on your mud, I simply ask that you place my name
 * someplace in the credits.  You can put it where you feel it is
 * appropriate.
 *
 * I offer no guarantee that this will work on any mud except my own, and
 * if you can't get it to work, please don't bother me.  I wrote and tested
 * this only on a Linux 2.0.30 system.  Comments about bugs, are, however,
 * appreciated.
 *
 * Now... On to the installation!
 */

/*
 * Insanity v0.9a pre-release Modifications
 * By Chris Fewtrell (Trax) <C.J.Fewtrell@bcs.org.uk>
 *
 * - Added functionailiy for Secure Web server pages, using standard HTTP
 *   Basic authentication, comparing with pass list generated with command
 *   from within the MUD itself. 
 * - Started work on web interface to help files, allowing them to be browsed
 *   from a web browser rather than being in MUD to read them.
 * - Seperated out the HTTP codes and content type to seperate functions
 *   (intending to allow more than HTML to be served via this)
 * - Adjusted the descriptor handling to prevent anyone from prematurely
 *   stopping a transfer causing a fd exception and the system to exit()
 * - Created a sorta "virtual" web directory for the webserver files to be
 *   actually served. This contains the usual images dir if any images are
 *   needed to be served from a central repository rather than generated.
 *   Be warned though! It WON'T follow any symlinks, I'll add that later
 *   with the stat function.. (maybe :) 
 * - Including a IMC web module to add the functionaility of the IMC webserver
 *   code directly into the mud itself for use here, preventing the need for
 *   the seperate server and client processes (I know it sorta depends on the
 *   mud be FE never stays down for long..)
 *
 * Future Possbile additions:
 * - Access to general boards though web interface, prolly prevent posting but
 *   being able to browse and read notes to 'all' would be allowed
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>

/* #include "h/merc.h" */
#include "mud.h"
#include "web.h"

#define SECURE_WEB      "../web/staff_html"
#define SECURE_URL	"/staffarea"   /* The secure URL. http://mud.is.here:5502SECURE_URL */
#define WEB_IMAGES	"../web/images"   /* for http://mud.is.here:port/images/blah.stuff */
#define AUTH_DOMAIN	"YakTalk Staff Area" /* Secure Area Description (tell me where this is used) */
#define MAXDATA 1024
#define WEB_PASS_FILE	SYSTEM_DIR "webpass.dat"   /* Web Password Storage */

void Base64Decode( char *bufcoded, unsigned char *bufplain, int outbufsize );

typedef struct web_descriptor WEB_DESCRIPTOR;

struct web_descriptor
{
   int fd;
   char request[MAXDATA * 2];
   struct sockaddr_in their_addr;
   socklen_t sin_size;
   WEB_DESCRIPTOR *next;
   bool valid;
   bool keepalive;
};

typedef struct web_password WEB_PASS;

struct web_password
{
   WEB_PASS *next;
   char *username;
   char *password;
};


WEB_PASS *web_passwords;
WEB_DESCRIPTOR *web_desc_free;

void web_colourconv( char *buffer, const char *txt );


/*
 * Content type stuff
 * This should let us use multiple filetypes
 * behind the server (graphics, html, text etc..)
 * all based on suffix matching
 */

#define CONTENT_HTML	1
#define CONTENT_TEXT	2
#define CONTENT_GIF	3
#define CONTENT_JPEG	4
#define CONTENT_GZIP	5
#define CONTENT_WAV	6
#define CONTENT_VRML	7
#define CONTENT_CLASS   8

struct type_data
{
   char *suffix;
   int type;
};

struct type_data content_types[] = {
   {".html", CONTENT_HTML},
   {".htm", CONTENT_HTML},
   {".gif", CONTENT_GIF},
   {".txt", CONTENT_TEXT},
   {".text", CONTENT_TEXT},
   {".jpg", CONTENT_JPEG},
   {".jpeg", CONTENT_JPEG},
   {".gz", CONTENT_GZIP},
   {".gzip", CONTENT_GZIP},
   {".wav", CONTENT_WAV},
   {".wrl", CONTENT_VRML},
   {".class", CONTENT_CLASS},

   {"", CONTENT_TEXT}
};



/* FUNCTION DEFS */
int send_buf( int fd, const char *buf );
void handle_web_request( WEB_DESCRIPTOR * wdesc );
void handle_web_who_request( WEB_DESCRIPTOR * wdesc );
void handle_wwwlist_request( WEB_DESCRIPTOR * wdesc );
void handle_web_main( WEB_DESCRIPTOR * wdesc );
void handle_web_unfound( WEB_DESCRIPTOR * wdesc );
void handle_web_wizlist( WEB_DESCRIPTOR * wdesc );
void handle_images( WEB_DESCRIPTOR * wdesc, char *path );
void handle_web_about( WEB_DESCRIPTOR * wdesc );
void handle_who_routine( WEB_DESCRIPTOR * wdesc );
void show_web_file( char *filename, WEB_DESCRIPTOR * wdesc );

bool check_web_pass( char *username, char *password );
void handle_secure_web( WEB_DESCRIPTOR * wdesc, char *username, char *password, char *path );
void save_webpass( void );

WEB_DESCRIPTOR *new_web_desc( void );
void free_web_desc( WEB_DESCRIPTOR * desc );

/* The mark of the end of a HTTP/1.x request */
const char ENDREQUEST[5] = { 13, 10, 13, 10, 0 };  /* (CRLFCRLF) */

/* Externs */
int top_web_desc;

/* Locals */
WEB_DESCRIPTOR *web_descs;
int sockfd;

bool init_web( int port )
{
   struct sockaddr_in my_addr;
   char buf[1024];

   web_descs = NULL;
   web_desc_free = NULL;

   sprintf( buf, "Attaching Internal Web Server to Port %d", port );
   log_string( buf );

   if( ( sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
   {
      log_string( "----> Web Server: Error getting socket" );
      perror( "web-socket" );
      sprintf( buf, "Web server (%d) : Failed initialization - Error getting socket", port );
// wiznet(buf, NULL, NULL, WIZ_WEB, 0, 0);
      return FALSE;
   }

   my_addr.sin_family = AF_INET;
   my_addr.sin_port = htons( port );
   my_addr.sin_addr.s_addr = htons( INADDR_ANY );
   bzero( &( my_addr.sin_zero ), 8 );

   if( ( bind( sockfd, ( struct sockaddr * )&my_addr, sizeof( struct sockaddr ) ) ) == -1 )
   {
      log_string( "----> Web Server: Error binding socket" );
      perror( "web-bind" );
      sprintf( buf, "Web server (%d) : Failed initialization - Error binding socket", port );
// wiznet(buf, NULL, NULL, WIZ_WEB, 0, 0);
      return FALSE;
   }

   /*
    * Only listen for 5 connects at once, do we really need more? 
    */
   listen( sockfd, 5 );

   sprintf( buf, "Web server: Initalization complete. Servering on port %d.", port );
//    wiznet(buf, NULL, NULL, WIZ_WEB, 0, 0);

//    sys_data.webup = TRUE;

   return TRUE;
}

struct timeval ZERO_TIME = { 0, 0 };

void handle_web( void )
{
   int max_fd;
   WEB_DESCRIPTOR *current, *prev = NULL, *next;
   fd_set readfds;

   FD_ZERO( &readfds );
   FD_SET( sockfd, &readfds );

   /*
    * it *will* be atleast sockfd 
    */
   max_fd = sockfd;

   /*
    * add in all the current web descriptors 
    */
   for( current = web_descs; current; current = current->next )
   {
      FD_SET( current->fd, &readfds );
      if( max_fd < current->fd )
         max_fd = current->fd;
   }

   /*
    * Wait for ONE descriptor to have activity 
    */
   select( max_fd + 1, &readfds, NULL, NULL, &ZERO_TIME );

   if( FD_ISSET( sockfd, &readfds ) )
   {
      /*
       * NEW CONNECTION -- INIT & ADD TO LIST 
       */

      current = new_web_desc(  );
      current->sin_size = sizeof( struct sockaddr_in );
      current->request[0] = '\0';

      if( ( current->fd = accept( sockfd, ( struct sockaddr * )&( current->their_addr ), &( current->sin_size ) ) ) == -1 )
      {
         log_string( "----> Web Server: Error accepting connection" );
         perror( "web-accept" );
         free_web_desc( current );
         FD_CLR( sockfd, &readfds );
         return;
      }

      current->next = web_descs;
      web_descs = current;

      /*
       * END ADDING NEW DESC 
       */
   }

   /*
    * DATA IN! 
    */
   for( prev = NULL, current = web_descs; current != NULL; current = next )
   {
      next = current->next;

      if( FD_ISSET( current->fd, &readfds ) )   /* We Got Data! */
      {
         char buf[MAXDATA];
         int numbytes;

         if( ( numbytes = read( current->fd, buf, sizeof( buf ) ) ) == -1 )
         {
            perror( "web-read" );
            if( prev )
               prev->next = next;
            else
               web_descs = next;

            free_web_desc( current );
            continue;
         }

         buf[numbytes] = '\0';

         strcat( current->request, buf );
      }

      prev = current;
   }  /* DONE WITH DATA IN */

   /*
    * DATA OUT 
    */
   /*
    * Hmm we want to delay this if possible, to prevent it prematurely 
    */
   for( prev = NULL, current = web_descs; current != NULL; current = next )
   {
      next = current->next;

      if( strstr( current->request, "HTTP/1." ) /* 1.x request (vernum on FIRST LINE) */
          && strstr( current->request, ENDREQUEST ) )
      {
         handle_web_request( current );
      }
      else if( !strstr( current->request, "HTTP/1." ) && strchr( current->request, '\n' ) )  /* HTTP/0.9 (no ver number) */
      {
         handle_web_request( current );
      }
      else if( !str_cmp( current->request, "IMC" ) )
      {
         if( current->keepalive )
         {
            prev = current;
            continue;
         }
      }
      else
      {
         prev = current;
         continue;   /* Don't have full request yet! */
      }

      if( current->keepalive )
      {
         prev = current;
         continue;
      }

      close( current->fd );

      if( !prev )
      {
         web_descs = current->next;
      }
      else
      {
         prev->next = current->next;
      }

      free_web_desc( current );
   }  /* END DATA-OUT */
}

/* Generic Utility Function */

int send_buf( int fd, const char *buf )
{
   return send( fd, buf, strlen( buf ), 0 );
}

int determine_type( char *path )
{
   int i;

   for( i = 0; *content_types[i].suffix; i++ )
   {
      if( !str_suffix( content_types[i].suffix, path ) )
         return content_types[i].type;
   }

   /*
    * If we dunno, we'll use plain text then 
    */
   return CONTENT_TEXT;
}

void send_200OK( WEB_DESCRIPTOR * wdesc )
{
   send_buf( wdesc->fd, "HTTP/1.1 200 OK\n" );
}

void send_404UNFOUND( WEB_DESCRIPTOR * wdesc )
{
   send_buf( wdesc->fd, "HTTP/1.1 404 Not Found\n" );
}

void send_401UNAUTHORISED( WEB_DESCRIPTOR * wdesc, char *realm )
{
   char buf[MAX_INPUT_LENGTH];

   sprintf( buf, "WWW-Authenticate: Basic realm=\"%s\"\n", realm );

   send_buf( wdesc->fd, "HTTP/1.1 401 Unauthorised\n" );
   send_buf( wdesc->fd, buf );
}

void send_content( WEB_DESCRIPTOR * wdesc, int type )
{
   switch ( type )
   {
      case CONTENT_HTML:
         send_buf( wdesc->fd, "Content-type: text/html\n\n" );
         break;
      default:
      case CONTENT_TEXT:
         send_buf( wdesc->fd, "Content-type: text/plain\n\n" );
         break;
      case CONTENT_GIF:
         send_buf( wdesc->fd, "Content-type: image/gif\n\n" );
         break;
      case CONTENT_WAV:
         send_buf( wdesc->fd, "Content-type: audio/x-wav\n\n" );
         break;
      case CONTENT_GZIP:
         send_buf( wdesc->fd, "Content-type: application/x-zip-compressed\n\n" );
         break;
      case CONTENT_VRML:
         send_buf( wdesc->fd, "Content-type: x-world/x-vrml\n\n" );
         break;
      case CONTENT_CLASS:
/*    	    send_buf(wdesc->fd,"Content-type: application/octect-stream\n\n"); */
         send_buf( wdesc->fd, "Content-type: application/octet-stream\n\n" );
         break;

   }
}

void handle_web_request( WEB_DESCRIPTOR * wdesc )
{
   char buf[MAX_STRING_LENGTH];
   char path[MAX_STRING_LENGTH];
   char *stuff;
   int addr;

   stuff = one_argument( wdesc->request, path );
   one_argument( stuff, path );

   /*
    * process request 
    */
   /*
    * are we using HTTP/1.x? If so, write out header stuff.. 
    */
   if( !strstr( wdesc->request, "GET" ) )
   {
      send_buf( wdesc->fd, "HTTP/1.1 501 Not Implemented" );
      return;
   }
   else if( strstr( wdesc->request, "HTTP/1." ) )
   {
      /*
       * Check for and handle secure area access 
       */
      if( !str_prefix( SECURE_URL, path ) )
      {
         char *where;
         char encoded[MAX_INPUT_LENGTH];
         char username[MAX_INPUT_LENGTH];
         char *password = "";

         username[0] = '\0';
         encoded[0] = '\0';

         where = strstr( stuff, "Authorization: Basic" );

         if( !where )
            send_401UNAUTHORISED( wdesc, AUTH_DOMAIN );
         else
         {
            where += strlen( "Authorization: Basic" );

            where++;
            for( password = encoded; *where && !isspace( *where ); where++, password++ )
               *password = *where;

            *password = '\0';

            Base64Decode( encoded, (unsigned char *)username, MAX_INPUT_LENGTH );

            for( password = username; *password && *password != ':'; password++ );

            if( *password == ':' )
            {
               *password = '\0';
               password++;
            }
         }

         if( !check_web_pass( username, password ) )
         {
            handle_secure_web( wdesc, "", "", path + strlen( SECURE_URL ) );
            return;
         }
         else
         {
            handle_secure_web( wdesc, username, password, path + strlen( SECURE_URL ) );
            return;
         }
      }
   }

   addr = ntohl( wdesc->their_addr.sin_addr.s_addr );

   sprintf( buf, "Web - Request for %s recived from %d.%d.%d.%d",
            path, ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF, ( addr >> 8 ) & 0xFF, ( addr ) & 0xFF );
//        wiznet(buf,NULL,NULL,WIZ_WEB,0,0);

   /*
    * Handle the actual request 
    */
   if( !str_cmp( path, "/wholist" ) || !str_cmp( path, "/wholist.html" ) )
   {
      handle_web_who_request( wdesc );
      log_string( "Counter Hit on Telnet Interface Who List." );
   }
   else if( !str_cmp( path, "/" ) || !str_cmp( path, "/index.html" ) )
   {
      handle_web_main( wdesc );
      log_string( "Counter Hit on Telnet Interface Index." );
   }
   else if( !str_cmp( path, "/wizlist" ) || !str_cmp( path, "/wizlist.html" ) )
   {
      handle_web_wizlist( wdesc );
      log_string( "Counter Hit on Telnet Interface Wizlist." );
   }
/*
	else if(!str_cmp(path, "/about") || !str_cmp(path, "/about.html"))
	    handle_web_about(wdesc);
	else if(!str_prefix("/images/", path))
	    handle_images(wdesc, path+strlen("/images"));
	else if(!str_prefix(SECURE_URL, path))
	    handle_secure_web(wdesc, "", "", path);
	else if(!str_prefix("/wwwlist", path))
	    handle_wwwlist_request(wdesc);
*/
   else
      handle_web_unfound( wdesc );
}

void shutdown_web( void )
{
   WEB_DESCRIPTOR *current, *next;

   /*
    * Close All Current Connections 
    */
   for( current = web_descs; current != NULL; current = next )
   {
      next = current->next;
      close( current->fd );
      free_web_desc( current );
   }

   /*
    * Stop Listening 
    */
   close( sockfd );

//     sys_data.webup = FALSE;
}

/* point 1 */

void handle_web_main( WEB_DESCRIPTOR * wdesc )
{
   send_200OK( wdesc );
   send_content( wdesc, CONTENT_HTML );

   show_web_file( PUB_INDEX, wdesc );
}

/* point 2 */

void handle_web_unfound( WEB_DESCRIPTOR * wdesc )
{
   send_404UNFOUND( wdesc );
   send_content( wdesc, CONTENT_HTML );

   show_web_file( PUB_ERROR, wdesc );

   return;
}

/* point 3 */

void handle_web_wizlist( WEB_DESCRIPTOR * wdesc )
{
   char buf[MAX_STRING_LENGTH];
   char colbuf[2 * MAX_STRING_LENGTH];
   FILE *fp;
   int num = 0;
   char let;

   send_200OK( wdesc );
   send_content( wdesc, CONTENT_HTML );

   show_web_file( PUB_WIZLIST_H, wdesc );

   if( ( fp = fopen( WEBWIZ_FILE, "r" ) ) != NULL )
   {
      while( !feof( fp ) )
      {
         while( ( let = fgetc( fp ) ) != EOF && num < ( MAX_STRING_LENGTH - 2 ) )
         {
            if( let != '\r' )
               buf[num++] = let;
         }

      }
      buf[num] = '\0';
      fclose( fp );
   }
   else
      sprintf( buf, "Error opening Staff list Data file<br>\r\n" );

   web_colourconv( colbuf, buf );

   send_buf( wdesc->fd, colbuf );

   show_web_file( PUB_WIZLIST_F, wdesc );
}


/*
 * Many thanks to Altrag who contributed this function! --GW
 */
char *text2html( const char *ip )
{
   static struct
   {
      const char *text;
      const char *html;
      int tlen, hlen;
   } convert_table[] =
   {
      {
      "<", "&lt;"},
      {
      ">", "&gt;"},
      {
      "&", "&amp;"},
      {
      "\"", "&quot;"},
      {
      " ", "&nbsp;"},
      {
   NULL, NULL}};

   static char buf[MAX_STRING_LENGTH * 2];   /* Safety here .. --GW */
   char *bp = buf;
   int i;

   if( !convert_table[0].tlen )
   {
      for( i = 0; convert_table[i].text; ++i )
      {
         convert_table[i].tlen = strlen( convert_table[i].text );
         convert_table[i].hlen = strlen( convert_table[i].html );
      }
   }
   while( *ip )
   {
      for( i = 0; convert_table[i].text; ++i )
         if( !strncmp( ip, convert_table[i].text, convert_table[i].tlen ) )
            break;
      if( convert_table[i].text )
      {
         strcpy( bp, convert_table[i].html );
         bp += convert_table[i].hlen;
         ip += convert_table[i].tlen;
      }
      else
         *bp++ = *ip++;
   }
   *bp = '\0';
   return buf;
}

char *parse_quotes( char *arg )
{
   int str;

   for( str = 0; arg[str] != '\0'; str++ )
   {
      if( arg[str] == '*' )
         arg[str] = '"';
   }

   return arg;
}

/* point 4 */

void handle_web_who_request( WEB_DESCRIPTOR * wdesc )
{
   send_200OK( wdesc );
   send_content( wdesc, CONTENT_HTML );

   show_web_file( PUB_WHOLIST_H, wdesc );

   do_who( NULL, "" );
   handle_who_routine( wdesc );

   show_web_file( PUB_WHOLIST_F, wdesc );
}

/* point 5 */

void handle_wwwlist_request( WEB_DESCRIPTOR * wdesc )
{
   send_200OK( wdesc );
   send_content( wdesc, CONTENT_HTML );

   show_web_file( PUB_WWWLIST_H, wdesc );

   do_who( NULL, "www" );
   handle_who_routine( wdesc );

   show_web_file( PUB_WWWLIST_F, wdesc );
}

void handle_web_about( WEB_DESCRIPTOR * wdesc )
{
   send_200OK( wdesc );
   send_content( wdesc, CONTENT_HTML );

   show_web_file( PUB_ABOUT, wdesc );
}

WEB_DESCRIPTOR *new_web_desc( void )
{
   WEB_DESCRIPTOR *desc;

   if( web_desc_free == NULL )
   {
      CREATE( desc, WEB_DESCRIPTOR, 1 );
      top_web_desc++;
   }
   else
   {
      desc = web_desc_free;
      web_desc_free = web_desc_free->next;
   }

   desc->keepalive = FALSE;
   desc->next = NULL;

   return desc;
}

void free_web_desc( WEB_DESCRIPTOR * desc )
{
   desc->next = web_desc_free;
   web_desc_free = desc;
}

void handle_images( WEB_DESCRIPTOR * wdesc, char *path )
{
   char buf[MAX_STRING_LENGTH];
   char file[MAX_INPUT_LENGTH];
   int type, fd;
   void *buffer;

   if( !str_cmp( path, "" ) || !str_cmp( path, "/" ) )
      sprintf( file, "%s%s", WEB_IMAGES, "/index.html" );
   else
      sprintf( file, "%s%s", WEB_IMAGES, path );

   if( file[strlen( file ) - 2] == '/' )
   {
      strcat( file, "index.html" );
   }

   /*
    * Work out the filetype so we know what we are doing 
    */
   type = determine_type( file );

   if( ( fd = open( file, O_RDONLY | O_NONBLOCK ) ) == -1 )
   {
      send_404UNFOUND( wdesc );
      send_content( wdesc, CONTENT_HTML );

      sprintf( buf,
               "<HTML><HEAD>\n"
               "<TITLE>Telnet Interface -- URL Not Found</TITLE>\n"
               "</HEAD>\r\n"
               "<center><b><font size=+3>Telnet Interface</b></font>\n"
               "<br><b><font size=+3>URL Not Found</b></font>\n"
               "<P>The URL that you requested could not be found.\r\n" "</center></body></html>\n" );

   }
   else
   {
      int readlen = 0;

      buffer = malloc( 1024 );
      send_200OK( wdesc );
      send_content( wdesc, type );

      while( ( readlen = read( fd, buffer, 1024 ) ) > 0 )
         send( wdesc->fd, buffer, readlen, 0 );

      close( fd );
      free( buffer );
   }
   return;
}

void handle_secure_web( WEB_DESCRIPTOR * wdesc, char *username, char *password, char *path )
{
   char file[MAX_INPUT_LENGTH];
   int type, fd;
   void *buffer;

   if( username[0] == '\0' || password[0] == '\0' )
   {
      send_401UNAUTHORISED( wdesc, "YakTalk Staff Area" );
      return;
   }


   if( !str_cmp( path, "" ) || !str_cmp( path, "/" ) )
      sprintf( file, "%s%s", SECURE_WEB, "/index.html" );
   else
      sprintf( file, "%s%s", SECURE_WEB, path );

   if( file[strlen( file ) - 2] == '/' )
   {
      strcat( file, "index.html" );
   }

   /*
    * Work out the filetype so we know what we are doing 
    */
   type = determine_type( file );

   if( ( fd = open( file, O_RDONLY | O_NONBLOCK ) ) == -1 )
   {
      send_404UNFOUND( wdesc );
      send_content( wdesc, CONTENT_HTML );

      show_web_file( STA_ERROR, wdesc );
   }
   else
   {
      int readlen = 0;

      buffer = malloc( 1024 );
      send_200OK( wdesc );
      send_content( wdesc, type );

      while( ( readlen = read( fd, buffer, 1024 ) ) > 0 )
         send( wdesc->fd, buffer, readlen, 0 );

      close( fd );
      free( buffer );
   }
   return;
}

bool check_web_pass( char *username, char *password )
{
   WEB_PASS *current;

   for( current = web_passwords; current; current = current->next )
      if( !str_cmp( current->username, username ) )
         if( !str_cmp( current->password, sha256_crypt( password ) ) )
            return TRUE;

   return FALSE;
}

bool change_web_pass( char *username, char *newpass )
{
   WEB_PASS *current, *new;

   for( current = web_passwords; current; current = current->next )
      if( !str_cmp( current->username, username ) )
         break;

   if( !current )
   {
      log_string( "Creating new webpass entry..." );

      CREATE( new, WEB_PASS, 1 );
      new->username = str_dup( username );
      new->password = str_dup( "" );

      new->next = web_passwords;
      web_passwords = new;

   }
   else
      new = current;

   STRFREE( new->password );

   new->password = str_dup( sha256_crypt( newpass ) );

   save_webpass(  );

   return TRUE;
}

void do_changewebpass( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];

   argument = one_argument( argument, buf );

   if( change_web_pass( ch->name, buf ) )
   {
      send_to_char( "Web password set.\r\n", ch );
   }
   else
   {
      send_to_char( "There was a problem setting the web password.\r\n", ch );
   }

   return;
}

void save_webpass(  )
{
   FILE *fpWebPass;
   WEB_PASS *current;

   if( ( fpWebPass = fopen( WEB_PASS_FILE, "w" ) ) )
   {
      log_string( "Saving web passes" );

      for( current = web_passwords; current; current = current->next )
         fprintf( fpWebPass, "WebPass %s~\n%s~\n", current->username, current->password );
      fprintf( fpWebPass, "$0\n" );

      fclose( fpWebPass );
      fpWebPass = NULL;
   }
}

void load_webpass(  )
{
   FILE *fpWebPass;

   if( ( fpWebPass = fopen( WEB_PASS_FILE, "r" ) ) )
   {
      WEB_PASS *new;
      char *word;

      for( word = fread_word( fpWebPass ); str_cmp( word, "$0" ); )
      {
         CREATE( new, WEB_PASS, 1 );
         new->username = fread_string( fpWebPass );
         new->password = fread_string( fpWebPass );

         new->next = web_passwords;
         web_passwords = new;
         word = fread_word( fpWebPass );

      }

      fclose( fpWebPass );

   }
}

void release_web_desc( int desc )
{
   WEB_DESCRIPTOR *current;

   for( current = web_descs; current; current = current->next )
   {
      if( current->fd == desc )
      {
         current->keepalive = FALSE;
         return;
      }
   }
}

int web_colour( char type, char *string )
{
   char code[50];
   char *p = '\0';

   switch ( type )
   {
      default:
//     sprintf( code, "" );
         break;
      case 'x':
         sprintf( code, "</font><font color=#000000>" );
         break;
      case 'b':
         sprintf( code, "</font><font color=#00007F>" );
         break;
      case 'c':
         sprintf( code, "</font><font color=#007F7F>" );
         break;
      case 'g':
         sprintf( code, "</font><font color=#007F00>" );
         break;
      case 'r':
         sprintf( code, "</font><font color=#7F0000>" );
         break;
      case 'w':
         sprintf( code, "</font><font color=#BFBFBF>" );
         break;
      case 'y':
         sprintf( code, "</font><font color=#FFFF00>" );
         break;
      case 'Y':
         sprintf( code, "</font><font color=#FFFF00>" );
         break;
      case 'B':
         sprintf( code, "</font><font color=#0000FF>" );
         break;
      case 'C':
         sprintf( code, "</font><font color=#00FFFF>" );
         break;
      case 'G':
         sprintf( code, "</font><font color=#00FF00>" );
         break;
      case 'R':
         sprintf( code, "</font><font color=#FF0000>" );
         break;
      case 'W':
         sprintf( code, "</font><font color=#FFFFFF>" );
         break;
      case 'z':
         sprintf( code, "</font><font color=#7F7F7F>" );
         break;
      case 'o':
         sprintf( code, "</font><font color=#FFFF00>" );
         break;
      case 'O':
         sprintf( code, "</font><font color=#7F7F00>" );
         break;
      case 'p':
         sprintf( code, "</font><font color=#7F007F>" );
         break;
      case 'P':
         sprintf( code, "</font><font color=#FF00FF>" );
         break;
      case '/':
         sprintf( code, "<br>" );
         break;
      case '{':
         sprintf( code, "%c", '{' );
         break;
      case '-':
         sprintf( code, "%c", '~' );
         break;
   }

   p = code;
   while( *p != '\0' )
   {
      *string = *p++;
      *++string = '\0';
   }

   return ( strlen( code ) );
}


void web_colourconv( char *buffer, const char *txt )
{
   const char *point;
   int skip = 0;

   if( txt )
   {
      for( point = txt; *point; point++ )
      {
         if( *point == '&' )
         {
            point++;
            skip = web_colour( *point, buffer );
            while( skip-- > 0 )
               ++buffer;
            continue;
         }
         *buffer = *point;
         *++buffer = '\0';
      }
      *buffer = '\0';
   }
   return;
}

/*
 *	This was added because of webmasters complaining on how they don't
 *	know how to code.  So web.h was added as well as extra directories
 *	in ../web (public_html and staff_html).     [readme.txt in ../web]
 *
 *	The file:  *.tih means 'Telnet Interface Header' (for beginning
 *	the html files) and *.tif 'Telnet Interface Footer' (for ending
 *	the html files).  The middle is filled in with generated code.
 *	
 *	-- Christopher Aaron Haslage (Yakkov) -- 6/3/99 (No Help Needed)
 */

void show_web_file( char *filename, WEB_DESCRIPTOR * wdesc )
{
   char buf[MAX_STRING_LENGTH];
   FILE *fp;
   int num = 0;
   /*
    * char *word; 
    */
   char let;


   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      while( !feof( fp ) )
      {
/******************* 
    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;
            
        switch ( UPPER(word[0]) )
        {
	case '<':
*******************/
/*
    	if ( !strcmp( word, "<!-- CODE -->"))
 	{
 	word = feof ( fp ) ? "End" : fread_word( fp );
 
        sprintf(buf, "\r\n<P><font color=red>\n"
 		"CODE IS INSERTED HERE!\n"
		"</font><font color=white><P>\r\n");
 	send_buf(wdesc->fd, buf);		
 	}
*/
         while( ( let = fgetc( fp ) ) != EOF && num < ( MAX_STRING_LENGTH - 2 ) )
         {
            if( let != '\r' )
               buf[num++] = let;
         }

      }
      buf[num] = '\0';
      fclose( fp );
   }
   else

      sprintf( buf, "\r\n<P><font color=red>\n"
               "ERROR: Missing or corrupted file in the Telnet Interface!\n" "</font><font color=white><P>\r\n" );

   send_buf( wdesc->fd, buf );

}

void handle_who_routine( WEB_DESCRIPTOR * wdesc )
{
   FILE *fp;
   char buf[MAX_STRING_LENGTH], col_buf[MAX_STRING_LENGTH];
   int c;
   int num = 0;

   if( ( fp = fopen( WHO_FILE, "r" ) ) != NULL )
   {
      while( !feof( fp ) )
      {
         while( ( buf[num] = fgetc( fp ) ) != EOF
                && buf[num] != '\n' && buf[num] != '\r' && num < ( MAX_STRING_LENGTH - 2 ) )
            num++;
         c = fgetc( fp );
         if( ( c != '\n' && c != '\r' ) || c == buf[num] )
            ungetc( c, fp );
         buf[num++] = '\n';
         buf[num] = '\0';

         if( strlen( buf ) > 32000 )
         {
            bug( "Strlen Greater then 32000: show_file", 0 );
            buf[32000] = '\0';
         }
         num = 0;
         web_colourconv( col_buf, buf );
         send_buf( wdesc->fd, col_buf );
//      send_buf(wdesc->fd,"<BR>");  
      }
      fclose( fp );
   }
   return;
}
