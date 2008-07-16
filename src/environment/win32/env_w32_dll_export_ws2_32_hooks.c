/********************************************************************************
 *                               libemu
 *
 *                    - x86 shellcode emulation -
 *
 *
 * Copyright (C) 2007  Paul Baecher & Markus Koetter
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * 
 *             contact nepenthesdev@users.sourceforge.net  
 *
 *******************************************************************************/

#include <stdlib.h>
#include <string.h>

// for the socket hooks
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../../config.h"
#include "emu/emu.h"
#include "emu/emu_log.h"
#include "emu/emu_memory.h"
#include "emu/emu_cpu.h"
#include "emu/emu_cpu_data.h"
#include "emu/emu_cpu_stack.h"
#include "emu/emu_hashtable.h"
#include "emu/emu_string.h"
#include "emu/environment/emu_env.h"
#include "emu/environment/emu_profile.h"
#include "emu/environment/win32/emu_env_w32.h"
#include "emu/environment/win32/emu_env_w32_dll.h"
#include "emu/environment/win32/emu_env_w32_dll_export.h"
#include "emu/environment/win32/env_w32_dll_export_ws2_32_hooks.h"

int32_t	env_w32_hook_accept(struct emu_env *env, struct emu_env_hook *hook)
{
	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);

/*SOCKET accept(
  SOCKET s,
  struct sockaddr* addr,
  int* addrlen
);*/

	emu_profile_function_add(env->profile, "accept");

	uint32_t s;
	POP_DWORD(c, &s);
	emu_profile_argument_add_int(env->profile, "SOCKET", "s", s);

	uint32_t addr;
	POP_DWORD(c, &addr);
	emu_profile_argument_add_ptr(env->profile, "sockaddr *", "addr", addr);
	emu_profile_argument_struct_start(env->profile, "", "");
	emu_profile_argument_struct_end(env->profile);

	uint32_t addrlen;
	POP_DWORD(c, &addrlen);
	emu_profile_argument_add_ptr(env->profile, "int", "addrlen", addrlen);
	emu_profile_argument_add_none(env->profile);

	logDebug(env->emu, "accept(s=%i, addr=%x, addrlen=%i);\n", s, addr, addrlen);

	uint32_t returnvalue;
	if ( hook->hook.win->userhook != NULL )
	{
		struct sockaddr sa;
		returnvalue = hook->hook.win->userhook(env, hook, 
											   s,
											   &sa,
											   &addrlen);
	}else
	{
		returnvalue	= 68;
	}
	emu_cpu_reg32_set(c, eax, returnvalue);
	emu_profile_function_returnvalue_int_set(env->profile, "SOCKET", returnvalue);

	emu_cpu_eip_set(c, eip_save);
	return 0;
}

int32_t	env_w32_hook_bind(struct emu_env *env, struct emu_env_hook *hook)
{
	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);


/*int bind(
  SOCKET s,
  const struct sockaddr* name,
  int namelen
); 
*/

	emu_profile_function_add(env->profile, "bind");
	uint32_t s;
	POP_DWORD(c, &s);
	emu_profile_argument_add_int(env->profile, "SOCKET", "s", s);


	uint32_t name;
	POP_DWORD(c, &name);
	
	

	struct sockaddr sa;
	
	emu_memory_read_block(emu_memory_get(env->emu), name, &sa, sizeof(struct sockaddr));
	if (sa.sa_family == AF_INET)
	{
		struct sockaddr_in *si = (struct sockaddr_in *)&sa;
		emu_profile_argument_add_ptr(env->profile, "sockaddr_in *", "name", name);
		emu_profile_argument_struct_start(env->profile, "", "");
		emu_profile_argument_add_short(env->profile, "short", "sin_family", si->sin_family);
		emu_profile_argument_add_port(env->profile, "unsigned short", "sin_port", si->sin_port);
		emu_profile_argument_struct_start(env->profile, "in_addr", "sin_addr");
		emu_profile_argument_add_ip(env->profile, "unsigned long", "s_addr", si->sin_addr.s_addr);
		emu_profile_argument_struct_end(env->profile);
		emu_profile_argument_add_string(env->profile, "char", "sin_zero", "       ");
		emu_profile_argument_struct_end(env->profile);

	}else
	{
		emu_profile_argument_struct_start(env->profile, "sockaddr *", "name");
		emu_profile_argument_struct_end(env->profile);
	}



	uint32_t namelen;
	POP_DWORD(c, &namelen);
	emu_profile_argument_add_int(env->profile, "int", "namelen", namelen);

	logDebug(env->emu, "bind(s=%i, name=%x, namelen=%i\n", s, name, namelen);


	uint32_t returnvalue;
	if ( hook->hook.win->userhook != NULL )
	{
		returnvalue = hook->hook.win->userhook(env, hook, 
											   s,
											   &sa,
											   namelen);
	}else
	{
		returnvalue	= 0;
	}
	emu_cpu_reg32_set(c, eax, returnvalue);
	emu_profile_function_returnvalue_int_set(env->profile, "int", returnvalue);


	emu_cpu_eip_set(c, eip_save);
	return 0;
}


int32_t	env_w32_hook_closesocket(struct emu_env *env, struct emu_env_hook *hook)
{
	logDebug(env->emu, "Hook me Captain Cook!\n");
	logDebug(env->emu, "%s:%i %s\n",__FILE__,__LINE__,__FUNCTION__);

	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);
/*
int closesocket(
  SOCKET s
);
*/
	emu_profile_function_add(env->profile, "closesocket");
	uint32_t s;
	POP_DWORD(c, &s);
	emu_profile_argument_add_int(env->profile, "SOCKET", "s", s);

	uint32_t returnvalue;
	if ( hook->hook.win->userhook != NULL )
	{
		returnvalue = hook->hook.win->userhook(env, hook, 
											   s);
	}else
	{
		returnvalue	= 0;
	}
	emu_cpu_reg32_set(c, eax, returnvalue);
	emu_profile_function_returnvalue_int_set(env->profile, "int", returnvalue);

	emu_cpu_eip_set(c, eip_save);
	return 0;
}



int32_t	env_w32_hook_connect(struct emu_env *env, struct emu_env_hook *hook)
{
	logDebug(env->emu, "Hook me Captain Cook!\n");
	logDebug(env->emu, "%s:%i %s\n",__FILE__,__LINE__,__FUNCTION__);

	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);
/*
int connect(
  SOCKET s,
  const struct sockaddr* name,
  int namelen
)
*/
	emu_profile_function_add(env->profile, "connect");
	uint32_t s;
	POP_DWORD(c, &s);
	emu_profile_argument_add_int(env->profile, "SOCKET", "s", s);

	uint32_t name;
	POP_DWORD(c, &name);
	struct sockaddr sa;

	emu_memory_read_block(emu_memory_get(env->emu), name, &sa, sizeof(struct sockaddr));
	if (sa.sa_family == AF_INET)
	{
		struct sockaddr_in *si = (struct sockaddr_in *)&sa;
		emu_profile_argument_add_ptr(env->profile, "sockaddr_in *", "name", name);
		emu_profile_argument_struct_start(env->profile, "", "");
		emu_profile_argument_add_short(env->profile, "short", "sin_family", si->sin_family);
		emu_profile_argument_add_port(env->profile, "unsigned short", "sin_port", si->sin_port);
		emu_profile_argument_struct_start(env->profile, "in_addr", "sin_addr");
		emu_profile_argument_add_ip(env->profile, "unsigned long", "s_addr", si->sin_addr.s_addr);
		emu_profile_argument_struct_end(env->profile);
		emu_profile_argument_add_string(env->profile, "char", "sin_zero", "       ");
		emu_profile_argument_struct_end(env->profile);

	}else
	{
		emu_profile_argument_struct_start(env->profile, "sockaddr *", "name");
		emu_profile_argument_struct_end(env->profile);
	}

	uint32_t namelen;
	POP_DWORD(c, &namelen);
	emu_profile_argument_add_int(env->profile, "int", "namelen", namelen);


	uint32_t returnvalue;
	if ( hook->hook.win->userhook != NULL )
	{
		returnvalue = hook->hook.win->userhook(env, hook, 
											   s,
											   &sa,
											   namelen);
	}else
	{
		returnvalue	= 0;
	}
	emu_cpu_reg32_set(c, eax, returnvalue);
	emu_profile_function_returnvalue_int_set(env->profile, "int", returnvalue);

	emu_cpu_eip_set(c, eip_save);
	return 0;
}

int32_t	env_w32_hook_listen(struct emu_env *env, struct emu_env_hook *hook)
{
	logDebug(env->emu, "Hook me Captain Cook!\n");
	logDebug(env->emu, "%s:%i %s\n",__FILE__,__LINE__,__FUNCTION__);

	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);

/*int listen(
  SOCKET s,
  int backlog
);
*/
	emu_profile_function_add(env->profile, "listen");


	uint32_t s;
	POP_DWORD(c, &s);
	emu_profile_argument_add_int(env->profile, "SOCKET", "s", s);

	uint32_t backlog;
	POP_DWORD(c, &backlog);
	emu_profile_argument_add_int(env->profile, "int", "backlog", backlog);

	logDebug(env->emu, "listen(s=%i,  backlog=%i)\n", s,  backlog);


	uint32_t returnvalue;
	if ( hook->hook.win->userhook != NULL )
	{
		returnvalue = hook->hook.win->userhook(env, hook, 
											   s,
											   backlog);
	}else
	{
		returnvalue	= 0;
	}

	emu_cpu_reg32_set(c, eax, returnvalue);
	emu_profile_function_returnvalue_int_set(env->profile, "int", returnvalue);

	emu_cpu_eip_set(c, eip_save);
	return 0;
}

int32_t	env_w32_hook_recv(struct emu_env *env, struct emu_env_hook *hook)
{
	logDebug(env->emu, "Hook me Captain Cook!\n");
	logDebug(env->emu, "%s:%i %s\n",__FILE__,__LINE__,__FUNCTION__);

	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);
/*
int recv(
  SOCKET s,
  char* buf,
  int len,
  int flags
);
*/
	emu_profile_function_add(env->profile, "recv");

	uint32_t s;
	POP_DWORD(c, &s);
	emu_profile_argument_add_int(env->profile, "SOCKET", "s", s);

	uint32_t buf;
	POP_DWORD(c, &buf);
	emu_profile_argument_add_ptr(env->profile, "char *", "buf", buf);
	emu_profile_argument_add_none(env->profile);

	uint32_t len;
	POP_DWORD(c, &len);
	emu_profile_argument_add_int(env->profile, "int", "len", len);

	uint32_t flags;
	POP_DWORD(c, &flags);
	emu_profile_argument_add_int(env->profile, "int", "flags", flags);

	uint32_t xlen = len;
	char *buffer = (char *)malloc(len);
	memset(buffer, 0, len);

	uint32_t returnvalue;
	if ( hook->hook.win->userhook != NULL )
	{
		returnvalue = hook->hook.win->userhook(env, hook, 
											   s,
											   buffer,
											   len,
											   flags);
	}else
	{
		if (rand()%100 < 40)
			returnvalue = 0;
		else
			returnvalue = len;
	}
	emu_cpu_reg32_set(c, eax, returnvalue);
	emu_profile_function_returnvalue_int_set(env->profile, "int", returnvalue);

	
	logDebug(env->emu, "recv(%i, 0x%08x, %i) == %i \n", s, buf, xlen, (int32_t)len);
	if ((int32_t)returnvalue > 0)
		emu_memory_write_block(emu_memory_get(env->emu), buf, buffer, len);
	free(buffer);


	emu_cpu_eip_set(c, eip_save);
	
	return 0;
}


int32_t	env_w32_hook_send(struct emu_env *env, struct emu_env_hook *hook)
{
	logDebug(env->emu, "Hook me Captain Cook!\n");
	logDebug(env->emu, "%s:%i %s\n",__FILE__,__LINE__,__FUNCTION__);

	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);
/*
int send(
  SOCKET s,
  const char* buf,
  int len,
  int flags
);
*/
	uint32_t s;
	POP_DWORD(c, &s);

	uint32_t buf;
	POP_DWORD(c, &buf);

	uint32_t len;
	POP_DWORD(c, &len);

	uint32_t flags;
	POP_DWORD(c, &flags);

	char *buffer = (char *)malloc(len);
	logDebug(env->emu, "send(%i, 0x%08x, %i,  %i)\n", s, buf, len, flags);
	emu_memory_read_block(emu_memory_get(env->emu), buf, buffer, len);


	uint32_t returnvalue;
	if ( hook->hook.win->userhook != NULL )
	{
		returnvalue = hook->hook.win->userhook(env, hook, 
											   s,
											   buffer,
											   len,
											   flags);
	}
	else
	{
		returnvalue = len;
	}
	emu_cpu_reg32_set(c, eax, returnvalue);
	emu_profile_function_returnvalue_int_set(env->profile, "int", returnvalue);

	free(buffer);

	emu_cpu_eip_set(c, eip_save);
	return 0;
}


int32_t	env_w32_hook_sendto(struct emu_env *env, struct emu_env_hook *hook)
{
	logDebug(env->emu, "Hook me Captain Cook!\n");
	logDebug(env->emu, "%s:%i %s\n",__FILE__,__LINE__,__FUNCTION__);

	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);
/*
int sendto(
  SOCKET s,
  const char* buf,
  int len,
  int flags,
  const struct sockaddr* to,
  int tolen
);
*/
	emu_profile_function_add(env->profile, "sendto");

	uint32_t s;
	POP_DWORD(c, &s);
	emu_profile_argument_add_int(env->profile, "SOCKET", "s", s);

	uint32_t p_buf;
	POP_DWORD(c, &p_buf);
	emu_profile_argument_add_ptr(env->profile, "const char *", "buf", p_buf);
	emu_profile_argument_add_none(env->profile);
//

	uint32_t len;
	POP_DWORD(c, &len);

	char *buffer = (char *)malloc(len);
	emu_memory_read_block(emu_memory_get(env->emu), p_buf, buffer, len);

//	emu_profile_argument_add_bytea(env->profile, "", "", (unsigned char *)buffer, (uint32_t)len);
	emu_profile_argument_add_int(env->profile, "int", "len", len);

	uint32_t flags;
	POP_DWORD(c, &flags);
	emu_profile_argument_add_int(env->profile, "int", "flags", flags);

	uint32_t p_to;
	POP_DWORD(c, &p_to);

	struct sockaddr sa;
	emu_memory_read_block(emu_memory_get(env->emu), p_to, &sa, sizeof(struct sockaddr));
	if (sa.sa_family == AF_INET)
	{
		struct sockaddr_in *si = (struct sockaddr_in *)&sa;
		emu_profile_argument_add_ptr(env->profile, "sockaddr_in *", "name", p_to);
		emu_profile_argument_struct_start(env->profile, "", "");
		emu_profile_argument_add_short(env->profile, "short", "sin_family", si->sin_family);
		emu_profile_argument_add_port(env->profile, "unsigned short", "sin_port", si->sin_port);
		emu_profile_argument_struct_start(env->profile, "in_addr", "sin_addr");
		emu_profile_argument_add_ip(env->profile, "unsigned long", "s_addr", si->sin_addr.s_addr);
		emu_profile_argument_struct_end(env->profile);
		emu_profile_argument_add_string(env->profile, "char", "sin_zero", "       ");
		emu_profile_argument_struct_end(env->profile);

	}else
	{
		emu_profile_argument_struct_start(env->profile, "sockaddr *", "name");
		emu_profile_argument_struct_end(env->profile);
	}


	uint32_t tolen;
	POP_DWORD(c, &tolen);
	emu_profile_argument_add_int(env->profile, "int", "tolen", tolen);


	uint32_t returnvalue;
	if ( hook->hook.win->userhook != NULL )
	{
		returnvalue = hook->hook.win->userhook(env, hook, 
											   s,
											   buffer,
											   len,
											   &sa,
											   tolen);
	}else
	{
			returnvalue = len;
	}
	emu_cpu_reg32_set(c, eax, returnvalue);
	emu_profile_function_returnvalue_int_set(env->profile, "int", returnvalue);


	free(buffer);

	logDebug(env->emu, "eip_save is %08x\n",  eip_save);
	emu_cpu_eip_set(c, eip_save);
	return 0;
}

int32_t	env_w32_hook_socket(struct emu_env *env, struct emu_env_hook *hook)
{
	logDebug(env->emu, "Hook me Captain Cook!\n");
	logDebug(env->emu, "%s:%i %s\n",__FILE__,__LINE__,__FUNCTION__);

	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);
/*
SOCKET WSAAPI socket(
  int af,
  int type,
  int protocol
);
*/

	emu_profile_function_add(env->profile, "socket");

	uint32_t af;
	POP_DWORD(c, &af);
	emu_profile_argument_add_int(env->profile, "int", "af", af);

	uint32_t type;
	POP_DWORD(c, &type);
	emu_profile_argument_add_int(env->profile, "int", "type", type);

	uint32_t protocol;
	POP_DWORD(c, &protocol);
	emu_profile_argument_add_int(env->profile, "int", "protocol", protocol);


	uint32_t returnvalue;
	if ( hook->hook.win->userhook != NULL )
	{
		returnvalue = hook->hook.win->userhook(env, hook, 
											   af,
											   type,
											   protocol);
	}else
	{
			returnvalue = 65;
	}
	emu_cpu_reg32_set(c, eax, returnvalue);
	emu_profile_function_returnvalue_int_set(env->profile, "SOCKET", returnvalue);

	emu_cpu_eip_set(c, eip_save);
	return 0;
}

int32_t	env_w32_hook_WSASocketA(struct emu_env *env, struct emu_env_hook *hook)
{
	logDebug(env->emu, "Hook me Captain Cook!\n");
	logDebug(env->emu, "%s:%i %s\n",__FILE__,__LINE__,__FUNCTION__);

	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);


/* SOCKET WSASocket(
  int af,
  int type,
  int protocol,
  LPWSAPROTOCOL_INFO lpProtocolInfo,
  GROUP g,
  DWORD dwFlags
); */

	emu_profile_function_add(env->profile, "WSASocket");

	uint32_t af;
	POP_DWORD(c, &af);
	emu_profile_argument_add_int(env->profile, "int", "af", af);



	uint32_t type;
	POP_DWORD(c, &type);
	emu_profile_argument_add_int(env->profile, "int", "type", type);

	uint32_t protocol;
	POP_DWORD(c, &protocol);
	emu_profile_argument_add_int(env->profile, "int", "protocol", protocol);

	uint32_t protocolinfo;
	POP_DWORD(c, &protocolinfo);
	emu_profile_argument_add_int(env->profile, "LPWSAPROTOCOL_INFO", "lpProtocolInfo", protocolinfo);


	uint32_t group;
	POP_DWORD(c, &group);
	emu_profile_argument_add_int(env->profile, "GROUP", "g", group);


	uint32_t flags;
	POP_DWORD(c, &flags);
	emu_profile_argument_add_int(env->profile, "DWORD", "dwFlags", flags);


	logDebug(env->emu, "SOCKET WSASocket(af=%i, type=%i, protocol=%i, lpProtocolInfo=%x, group=%i, dwFlags=%i);\n",
		   af, type, protocol, protocolinfo, group,  flags);


	uint32_t returnvalue;
	if ( hook->hook.win->userhook != NULL )
	{
		returnvalue = hook->hook.win->userhook(env, hook, 
											   af,
											   type,
											   protocol,
											   NULL,
											   NULL,
											   NULL);
	}else
	{
		returnvalue = 66;
	}
	emu_cpu_reg32_set(c, eax, returnvalue);
	emu_profile_function_returnvalue_int_set(env->profile, "SOCKET", returnvalue);

	emu_cpu_eip_set(c, eip_save);
	return 0;
}


int32_t	env_w32_hook_WSAStartup(struct emu_env *env, struct emu_env_hook *hook)
{
	logDebug(env->emu, "Hook me Captain Cook!\n");
	logDebug(env->emu, "%s:%i %s\n",__FILE__,__LINE__,__FUNCTION__);

	struct emu_cpu *c = emu_cpu_get(env->emu);

	uint32_t eip_save;

	POP_DWORD(c, &eip_save);

/*
int WSAStartup(
  WORD wVersionRequested,
  LPWSADATA lpWSAData
);
*/
	emu_profile_function_add(env->profile, "WSAStartup");
	
	uint32_t wsaversionreq;
	POP_DWORD(c, &wsaversionreq);
	logDebug(env->emu, "WSAStartup version %x\n", wsaversionreq);
	emu_profile_argument_add_int(env->profile, "WORD", "wVersionRequested", wsaversionreq);

	uint32_t wsadata;
	POP_DWORD(c, &wsadata);
	emu_profile_argument_add_int(env->profile, "LPWSADATA", "lpWSAData", wsadata);


	emu_profile_function_returnvalue_int_set(env->profile, "int", 0);
	emu_cpu_reg32_set(c, eax, 0x0);

	emu_cpu_eip_set(c, eip_save);
	return 0;
}

