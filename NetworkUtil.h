/*
 *      NetworkUtil.h contains methods that both GalactiCombatServer and NetworkManagerClient use.
 */

#ifndef __NetworkUtil_h
#define __NetworkUtil_h

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#include "SDL.h"
#include "SDL_net.h"
#else
#include "SDL/SDL.h"
#include "SDL/SDL_net.h"
#endif

//#define ERROR 0xff
#define TIMEOUT 5000 //five seconds

#define TCP_PORT 5172

// Used for the Packet struct
#define CONNECTION 10
#define STATE 11
#define INFO 12
#define PLAYERINPUT 13
#define PLAYERROTATION 14
#define READY 15
#define WALLS 16
#define SCORE 17

struct Packet {
    int type;
    char* message;
};
typedef struct Packet Packet; // allows Packet struct to be used in C

namespace NetworkUtil {
    
    /*
     *           charArrayToPacket(char*):
     *           This method takes a char array and parses it into a Packet.
     *           The idea is that you can take the return value of TCPReceive() and use
     *           this method to turn it into a usable Packet struct.
     *
     *           It's not very robust and doesn't check to make sure that the argument can actually
     *           be converted into a Packet; it just assumes that it can. You may or may not
     *           want to FIXME.
     * 
     *           As another NOTE, this method mallocs() a char* for the Packet.message.
     *           When you release the packet, you need to free(packet.message).
     *           Otherwise, you WILL have memory problems. Use with caution.
     */
    inline Packet charArrayToPacket(char* msg)
    {
        //std::cout << "Entering charArrayToPacket" << std::endl;
        Packet pack;
        
        char* typeFinder = (char*)malloc(3);
        memmove(typeFinder, msg, 2);
        *(typeFinder+3) = 0;
        int type = atoi(typeFinder);
        pack.type = type;
        
        int messageLength = strlen(msg) + 1;
        if(type == PLAYERINPUT)
            messageLength = 3;
        char* messageFinder =  (char*)malloc(messageLength-2);
        
        for(int i = 2; i < messageLength; ++i) {
            messageFinder[i-2] = msg[i];
        }
        pack.message = messageFinder;
        
        free(typeFinder);
        
        //std::cout << "Exiting charArrayToPacket" << std::endl;
        return pack;
    }
    
    /*
     *           PacketToCharArray(Packet):
     *           This method takes a Packet and converts it to an
     *           array of chars. This makes it so that you can use it as the second
     *           argument in TCPSend().
     * 
     *           NOTE: This method mallocs(). 
     *           You need to free() when done with the charArray.
     *           Otherwise, you WILL have memory problems. Use with caution.
     *
     *           -pack: the Packet to be converted
     *
     *           returns: the converted Packet
     */
    inline char* PacketToCharArray(Packet pack)
    {
        //std::cout << "Entering PacketToCharArray" << std::endl;
        int messageLength = strlen(pack.message) + 1;
        if(pack.type == PLAYERINPUT)
            messageLength = 1;
        
        char* charArray = (char*)malloc(2 + messageLength);
        sprintf(charArray, "%02d", pack.type);
        memmove(charArray+2, pack.message, messageLength);
        
        //std::cout << "Exiting PacketToCharArray" << std::endl;
        return charArray;
    }
    
    
    /*
     *        TCPReceive(TCPsocket, char**):
     *        This method listens for a message on the specified socket.
     *        This was copied from the method getMsg() from tcputil.h in the SDLNet example code.
     * 
     *         NOTE: This method mallocs(). 
     *         You need to free(buf) when you're done with it.
     *         Otherwise, you WILL have memory problems. Use with caution.
     * 
     *        -sock: the socket to listen on
     *        -buf: a pointer to a block of memory where the data can be stored
     */
    inline char* TCPReceive(TCPsocket sock, char **buf)
    {
        //std::cout << "Entering TCPReceive" << std::endl;
        Uint32 len,result;
        static char *_buf;
        
        // allow for a NULL buf, use a static internal one...
        if(!buf)
            buf=&_buf;
        
        // free the old buffer 
        if(*buf)
            free(*buf);
        *buf=NULL;
            
        // receive the length of the string message
        result=SDLNet_TCP_Recv(sock,&len,sizeof(len));
        if(result<sizeof(len)) {   
            if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
            std::cerr << "SDLNet_TCP_Recv done goofed: " << SDLNet_GetError() << std::endl;
            return(NULL);
        }   
        
        // swap byte order to our local order
        len=SDL_SwapBE32(len);
        
        // check if anything is strange, like a zero length buffer
        if(!len)
            return(NULL);
        
        // allocate the buffer memory
        *buf=(char*)malloc(len);
        if(!(*buf))
            return(NULL);
        
        // get the string buffer over the socket
        result = SDLNet_TCP_Recv(sock,*buf,len);
        if(result < len) {   
            if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
            std::cerr << "SDLNet_TCP_Recv done goofed: " << SDLNet_GetError() << std::endl;
            free(*buf);
            buf=NULL;
        }   
        
        // return the new buffer
        //std::cout << "Exiting TCPReceive" << std::endl;
        return(*buf);
    }
    
    /*
     *        TCPSend(TCPsocket, char*):
     *        This method is basically copy-pasted from the SDLNet example code.    
     *        The method copied is putMsg() from tcputil.h.
     *        
     *        -sock: the TCPsocket through which to send the data
     *        -buf: the data to be sent
     * 
     *        returns: the number of bytes sent, or 0 if error, or 1 if the second argument is empty
     */
    inline int TCPSend(TCPsocket sock, char *buf)
    {
        //std::cout << "Entering TCPSend" << std::endl;
        Uint32 len,result;
        
        if(!buf || !strlen(buf))
            return(1);
        
        // determine the length of the string 
        len=strlen(buf)+1; // add one for the terminating NULL 
        // change endianness to network order 
        len=SDL_SwapBE32(len);
        
        // send the length of the string 
        result=SDLNet_TCP_Send(sock,&len,sizeof(len));
        
        if(result<sizeof(len)) {
            std::cerr << "SDLNet_TCP_Send done goofed: " << std::endl;
            if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
            std::cerr << "\t- " << SDLNet_GetError() << std::endl;
            return(0);
        }
        
        // revert to our local byte order
        len=SDL_SwapBE32(len);
        
        // send the buffer, with the NULL as well
        result=SDLNet_TCP_Send(sock,buf,len);
        
        if(result<len) {
            std::cerr << "SDLNet_TCP_Send done goofed: " << std::endl;
            if(SDLNet_GetError() && strlen(SDLNet_GetError())) // sometimes blank!
            std::cerr << "\t- " << SDLNet_GetError() << std::endl;
            return(0);
        }
        
        //std::cout << "Exiting TCPSend" << std::endl;
        return(result);
    }
    
    
    /*
     *           UDPSend(UDPsocket, int, UDPpacket*):
     *           Wrapper function for SDLNet_UDP_Send. If using channels, ensure that the
     *           UDPpacket argument has that information in the "channel" field.
     *           -sock: The UDPsocket to send the packet through.
     *           -channel: The socket's channel to send through. Use -1 if not using channels.
     *           -outgoing: The UDPpacket to send.
     */
    inline int UDPSend(UDPsocket sock, int channel, UDPpacket *outgoing)
    {
        //std::cout << "Entering UDPSend" << std::endl;
        int numSent = SDLNet_UDP_Send(sock, channel, outgoing);
        if(!numSent)
            std::cerr << "SDLNet_UDP_Send done goofed: " << SDLNet_GetError() << std::endl;
        //std::cout << "Exiting UDPSend" << std::endl;
        return numSent;
    }
    
    /*
     *           UDPReceive(UDPsocket, UDPpacket, Uint32, Uint8, int):
     *           Wrapper function for SDLNet_UDP_Recv.
     *           -sock: The UDPsocket to listen on.
     *           -in: The incoming UDPpacket.
     */
    inline int UDPReceive(UDPsocket sock, UDPpacket *in)
    {
        //std::cout << "Entering UDPReceive" << std::endl;
        int received = SDLNet_UDP_Recv(sock, in);
        if(received < 0)
            std::cerr << "SDLNet_UDP_Recv done goofed: " << SDLNet_GetError() << std::endl;
        //std::cout << "Exiting UDPReceive" << std::endl;
        return received;
    }
    
    inline UDPpacket *AllocPacket(int size)
    {
        //std::cout << "Entering AllocPacket" << std::endl;
        UDPpacket *UDPpack = SDLNet_AllocPacket(size);
        if(!UDPpack)
            std::cerr << "SDLNet_AllocPack done goofed: " << SDLNet_GetError() << std::endl;
        //std::cout << "Exiting AllocPacket" << std::endl;
        return UDPpack;
    }
    inline int UDPBind(UDPsocket sock, int channel, IPaddress *address)
    {
        //std::cout << "Entering UDPBind" << std::endl;
        int result = SDLNet_UDP_Bind(sock, channel, address);
        if (result < 0)
            std::cerr << "SDLNet_UDP_Bind done goofed: " << SDLNet_GetError() << std::endl;
        //std::cout << "Exiting UDPBind" << std::endl;
        return result;
    }
    
    inline int ResolveHost(IPaddress *address, char *host, Uint16 port)
    {
        int result = SDLNet_ResolveHost(address, host, port);
        if(result == -1)
            std::cerr<<"SDLNet_ResolveHost done goofed: "<<SDLNet_GetError()<<std::endl;
        return result;
    }
    
    inline TCPsocket TCPOpen(IPaddress *ip)
    {
        TCPsocket result = SDLNet_TCP_Open(ip);
        if(!result)
            std::cerr<<"SDLNet_TCP_Open done goofed: "<<SDLNet_GetError()<<std::endl;
        return result;
    }
    
    inline UDPsocket UDPOpen(Uint16 port)
    {
        UDPsocket result = SDLNet_UDP_Open(port);
        if(!result)
            std::cerr<<"SDLNet_UDP_Open done goofed: "<<SDLNet_GetError()<<std::endl;
        return result;
    }
    
    inline int CheckSockets(SDLNet_SocketSet set, Uint32 timeout)
    {
        int result = SDLNet_CheckSockets(set, timeout);
        if(result == -1)
            std::cerr<<"SDLNet_CheckSockets done goofed: "<<SDLNet_GetError()<<std::endl;
        return result;
    }
}

#endif //#ifndef __NetworkUtil_h
