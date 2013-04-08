/*
 * This program has been developed by Luca Tringali, using code from
 * Fredi Machado <https://github.com/Fredi> for the IRC client.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <map>
#include <algorithm>
#include "Thread.h"
#include "IRCClient.h"
#include "Command.cpp"


#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#endif



volatile bool running;

void signalHandler(int signal)
{
    running = false;
}

ConsoleCommandHandler commandHandler;

void msgCommand(std::string arguments, IRCClient* client)
{
    std::string to = arguments.substr(0, arguments.find(" "));
    std::string text = arguments.substr(arguments.find(" ") + 1);

    std::cout << "To " + to + ": " + text << std::endl;
    client->SendIRC("PRIVMSG " + to + " :" + text);
};

void joinCommand(std::string channel, IRCClient* client)
{
    if (channel[0] != '#')
        channel = "#" + channel;

    client->SendIRC("JOIN " + channel);
}

void partCommand(std::string channel, IRCClient* client)
{
    if (channel[0] != '#')
        channel = "#" + channel;

    client->SendIRC("PART " + channel);
}

void ctcpCommand(std::string arguments, IRCClient* client)
{
    std::string to = arguments.substr(0, arguments.find(" "));
    std::string text = arguments.substr(arguments.find(" ") + 1);

    std::transform(text.begin(), text.end(), text.begin(), towupper);

    client->SendIRC("PRIVMSG " + to + " :\001" + text + "\001");
}

void breakTime( int seconds)
{
    clock_t temp;
    temp = clock () + seconds * CLOCKS_PER_SEC ;
    while (clock() < temp) {}
}

std::string httpRequestF( std::string myrequest) {

#ifdef _WIN32
    //  On Windows

    string request;
    string response;
    int resp_leng;

    char buffer[BUFFERSIZE];
    struct sockaddr_in serveraddr;
    int sock;

    WSADATA wsaData;
    char *ipaddress = "66.171.248.178";
    int port = 80;

    //request+= "GET http://bot.whatismyipaddress.com/ http/1.1\r\nHOST: bot.whatismyipaddress.com\r\n";
    //request+="\r\n";
    request = myrequest;

    //init winsock
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
        std::cout << "WSAStartup() failed" << std::endl;

    //open socket
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        std::cout << "socket() failed" << std::endl;

    //connect
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family      = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ipaddress);
    serveraddr.sin_port        = htons((unsigned short) port);
    if (connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        cout << "connect() failed" << endl;

    //send request
    if (send(sock, request.c_str(), request.length(), 0) != request.length())
        cout << "send() sent a different number of bytes than expected" << endl;

    //get response
    response = "";
    resp_leng= BUFFERSIZE;
    while (resp_leng == BUFFERSIZE)
    {
        resp_leng= recv(sock, (char*)&buffer, BUFFERSIZE, 0);
        if (resp_leng>0)
            response+= string(buffer).substr(0,resp_leng);
        //note: download lag is not handled in this code
    }

    //display response
    //cout << response << endl;
    std::string answer (response);

    //disconnect
    //closesocket(sock);

    //cleanup
    WSACleanup();
    return answer;
#else

    int s, error;
    struct sockaddr_in addr;

    if((s = socket(AF_INET,SOCK_STREAM,0))<0)
    {
        std::cout<<"Error 01: creating socket failed!\n";
        //close(s);
        return "error";
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_aton("66.171.248.178",&addr.sin_addr);  //this should be the ip of the server I'm contacting

    error = connect(s,(sockaddr*)&addr,sizeof(addr));
    if(error!=0)
    {
        std::cout<<"Error 02: conecting to server failed!\n";
        //close(s);
        return "error";
    }

    //char msg[] = "GET http://bot.whatismyipaddress.com/ http/1.1\nHOST: bot.whatismyipaddress.com\n\n";

    char answ[1024];
    std::string answer;
    //send(s,msg,sizeof(msg),0);
    send(s,myrequest.c_str(),myrequest.length(),0);

    //while(recv(s,answ,1024,0)!=0)
    //cout<<answ<<endl;
    while(recv(s,answ,1024,0)!=0){
        answer.append(answ);
    }
    return answer;
#endif


}


ThreadReturn inputThread(void* client)
{
    std::string command;

    commandHandler.AddCommand("msg", 2, &msgCommand);
    commandHandler.AddCommand("join", 1, &joinCommand);
    commandHandler.AddCommand("part", 1, &partCommand);
    commandHandler.AddCommand("ctcp", 2, &ctcpCommand);


    breakTime(20);  //wait 30 seconds
    commandHandler.ParseCommand("/join #dottorfeudo aragosta", (IRCClient*)client);


    while(true)
    {
        getline(std::cin, command);
        if (command == "")
            continue;

        if (command[0] == '/')
            commandHandler.ParseCommand(command, (IRCClient*)client);
        else
            ((IRCClient*)client)->SendIRC(command);

        if (command == "quit")
            break;
    }

#ifdef _WIN32
    _endthread();
#else
    pthread_exit(NULL);
#endif
}


int main(int argc, char* argv[])
{
    std::string nick("MyIRCClient");
    std::string user("IRCClient");
    int port = 6667;
    std::string host;

    if (argc < 3)
    {
        //std::cout << "Insuficient parameters: host port [nick] [user]" << std::endl;
        //adding my data
        host = "openirc.snt.utwente.nl";   // IRCnet non controlla nomi utente e canali: openirc.snt.utwente.nl porta 6667



        std::string myOldIP ("255.255.255.255");
        while (1==1) {

            std::string answer = httpRequestF("GET http://bot.whatismyipaddress.com/ http/1.1\nHOST: bot.whatismyipaddress.com\n\n");

            //cout<<answer.c_str()<<endl;
            //   answer = answT;
            int f = 0;
            char numbers[11] = "0123456789";
            for (int i = 0; i< answer.length(); i++) {
                if ((answer.c_str()[i] == numbers[0]) || (answer.c_str()[i] == numbers[1])|| (answer.c_str()[i] == numbers[2]) || (answer.c_str()[i] == numbers[3]) ||(answer.c_str()[i] == numbers[4]) || (answer.c_str()[i] == numbers[5]) || (answer.c_str()[i] == numbers[6]) || (answer.c_str()[i] == numbers[7])  || (answer.c_str()[i] == numbers[8])  || (answer.c_str()[i] == numbers[9])) f = i;
            }
            int i = answer.find_last_of("\n")+1;
            std::string myIP (answer.substr(i,f-i+1));


            //check if the IP is correct
            std::cout << "Possible IP:" <<myIP.c_str()<< "|" << std::endl;
            if (myIP.compare(myOldIP) == 0) {
                myOldIP.assign(myIP);
                break;
            } else{
                myOldIP.assign(myIP);
            }
        }

        std::cout << "My real IP:" <<myOldIP.c_str()<< "|" << std::endl;
        std::cout << "\n Ready?\n";



        //close(s);


        nick = "m";
        nick.append(myOldIP);
        while (nick.rfind(".")!=std::string::npos) {
            nick.replace (nick.rfind("."),1,"l");
        }
        nick.append("_");
        user = nick;
    } else {

        host = argv[1];
        port = atoi(argv[2]);
        if (argc >= 4)
            nick = argv[3];
        if (argc >= 5)
            user = argv[4];

    }
    IRCClient client;

    client.Debug(true);

    // Start the input thread
    Thread thread;
    thread.Start(&inputThread, &client);

    if (client.InitSocket())
    {
        std::cout << "Socket initialized. Connecting..." << std::endl;

        if (client.Connect(host.c_str(), port))
        {
            std::cout << "Connected. Loggin in..." << std::endl;

            if (client.Login(nick, user))
            {
                std::cout << "Logged." << std::endl;

                running = true;
                signal(SIGINT, signalHandler);

                while (client.Connected() && running)
                    client.ReceiveData();
            }

            if (client.Connected())
                client.Disconnect();

            std::cout << "Disconnected." << std::endl;
        }
    }
}
