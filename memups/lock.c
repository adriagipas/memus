/*
 * Copyright 2022-2025 Adrià Giménez Pastor.
 *
 * This file is part of adriagipas/memus.
 *
 * adriagipas/memus is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * adriagipas/memus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  lock.c - Implementació de 'lock.h'.
 *
 */

#include <dbus/dbus.h>
#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "lock.h"
#include "session.h"




/*********/
/* ESTAT */
/*********/

static DBusConnection *_con;
static gchar *_ifname;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
create_ifname (
               const char *name
               )
{

  char *tmp,*p;
  GString *buf;
  

  // Copia el nom canviant caràcters no vàlids per _
  tmp= g_strdup ( name );
  for ( p= tmp; *p != '\0'; ++p )
    if ( *p != '_' && (*p<'A' || *p>'Z') &&
         (*p<'a' || *p>'z') && (*p<'0' || *p>'9') )
      *p= '_';

  // Afegeix .interface
  buf= g_string_new ( tmp );
  g_string_append ( buf, ".interface" );
  _ifname= g_string_free_and_steal ( buf );
  g_free ( tmp );
  
} // end create_ifname


static void
add_match (void)
{

  DBusError err;
  GString *buf;

  
  dbus_error_init ( &err );
  buf= g_string_new ( NULL );
  g_string_printf ( buf, "type='signal',interface='%s'", _ifname );
  dbus_bus_add_match ( _con, (const char *) buf->str, &err );
  if ( dbus_error_is_set ( &err ) )
    warning ( "no s'ha pogut registrar en D-BUS la regla: %s",
              (const char *) buf->str );
  dbus_connection_flush ( _con );
  g_string_free ( buf, TRUE );
  dbus_error_free ( &err );
  
} // end add_match


static void
send_signal (void)
{

  DBusMessage *signal;
  

  // Prepara
  signal= NULL;
  
  // Crea la senyal
  signal= dbus_message_new_signal ( "/memu/PS/object",
                                    (const char *) _ifname,
                                    "ShowWin" );
  if ( signal == NULL )
    {
      warning ( "No s'ha pogut emetre la senyal D-BUS: no s'ha"
                " pogut crear la senyal" );
      goto error;
    }
  
  // Envia.
  if ( !dbus_connection_send ( _con, signal, NULL ) )
    {
      warning ( "No s'ha pogut emetre la senyal D-BUS: error en l'enviament" );
      goto error;
    }
  dbus_connection_flush ( _con );
  
  // Allibera memòria.
  dbus_message_unref ( signal );
  
  return;
  
 error:
  if ( signal != NULL ) dbus_message_unref ( signal );
  
} // end send_signal


static gboolean
create_name (
             const gchar    *name,
             const bool      signal,
             const gboolean  verbose,
             DBusError      *err
             )
{
  
  int res;
  gboolean ret;
  
  
  res= dbus_bus_request_name ( _con, (const char *) name,
                               DBUS_NAME_FLAG_DO_NOT_QUEUE,
                               err );
  if ( res == -1 )
    {
      if ( dbus_error_is_set ( err ) )
        error ( "error a l'obtindre el nom de D-BUS: %s - %s",
                err->name, err->message );
      else
        error ( "error a l'obtindre el nom de D-BUS" );
      ret= FALSE;
    }
  else ret= (res==DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
  
  // Verbose
  if ( verbose )
    {
      if ( ret )
        fprintf ( stderr, "Creat nom D-BUS: %s\n", name );
      else
        fprintf ( stderr, "Nom D-BUS ja en ús: %s\n", name );
    }

  // Senyal
  if ( signal )
    {
      create_ifname ( name );
      if ( ret ) add_match ();
      else send_signal ();
    }
  
  return ret;
  
} // end create_name


static gchar *
get_name (
          const gboolean is_base
          )
{

  GString *buf;
  const gchar *sname;
  gchar *ret;
  

  sname= session_get_name ();
  buf= g_string_new ( "memus.PS" );
  if ( sname != NULL )
    g_string_append_printf ( buf, ".%s", sname );
  if ( !is_base )
    g_string_append ( buf, ".subname" );
  ret= g_string_free_and_steal ( buf );
  
  return ret;
  
} // end get_name




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_lock (void)
{

  if ( _ifname != NULL )
    {
      g_free ( _ifname );
      _ifname= NULL;
    }
  dbus_connection_unref ( _con );
  
} // end close_lock


gboolean
init_lock (
           const gboolean verbose
           )
{

  gchar *name;
  DBusError err;
  gboolean ret;
  
  
  // Prepara.
  _ifname= NULL;
  dbus_error_init ( &err );
  
  // Es connecta al bus
  _con= dbus_bus_get ( DBUS_BUS_SESSION, &err );
  if ( _con == NULL )
    {
      if ( dbus_error_is_set ( &err ) )
        error ( "no s'ha pogut establir connexió amb D-BUS: %s - %s",
                err.name, err.message );
      else
        error ( "no s'ha pogut establir connexió amb D-BUS" );
    }
  
  // Crea el nom base.
  name= get_name ( TRUE );
  ret= create_name ( name, true, verbose, &err );
  g_free ( name );
  
  // Crea el subnom
  // En aquest cas sols es pot executar quan és l'única executant-se
  if ( ret )
    {
      name= get_name ( FALSE );
      ret= create_name ( name, false, verbose, &err );
      g_free ( name );
    }
  
  // Allibera memòria
  dbus_error_free ( &err );
  
  return ret;
  
} // end init_lock


bool
lock_check_signals (void)
{

  DBusMessage *message;
  bool ret;

  
  ret= false;
  do {
    dbus_connection_read_write ( _con, 0 );
    message= dbus_connection_pop_message ( _con );
    if ( message != NULL )
      {
        if ( dbus_message_is_signal ( message, _ifname, "ShowWin" ) )
          ret= true;
        dbus_message_unref ( message );
      }
  } while ( message != NULL );
  
  return ret;
  
} // end lock_check_signals
