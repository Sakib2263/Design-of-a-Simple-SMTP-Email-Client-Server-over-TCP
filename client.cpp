#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <ctime>

#define TMP_LENGTH 1024

using namespace std;

int clientSockt;
char s[] = "server: ";
char c[] = "client: ";
char* file_name_mail;
char* receiver = "";
char* mail_subject;
char myHostName[TMP_LENGTH];
char* from= getenv(" USER");
char* dest = "";


void send_To_Server(char* buf);

int checkServerReturnedCode(char* buf)
{

    char server_returned_code[4]="   ";
    memcpy(server_returned_code, buf,strlen(server_returned_code));
    int code=0;
    code=atoi(server_returned_code);


    //checking if the code is valid
    if(code<200 ||code>399)
    {
        printf("code number:%d\n",code);
        printf("ERROR:%s\n",buf);
    }

    return code;

}

char* readFromMailFile(char* file_name) {
    ifstream t;
    int length;
    t.open(file_name);
    t.seekg(0, ios::end);
    length = t.tellg();           // report location (this is the length)
    t.seekg(0, ios::beg);    // go back to the beginning
    char* buffer = new char[length];    // allocate memory for a buffer of appropriate dimension
    t.read(buffer, length);       // read the whole file into the buffer
    t.close();
    return buffer;
}


//Client side
int main(int argc, char *argv[]) {

    if(argc != 4) {
        cerr << "Please provide valid arguments" << endl;
        exit(0);
    } //temporarily using localhost
    //char *serverIp = argv[1];
    char serverIp[] = "127.0.0.1";
    receiver = strtok (argv[1],":");
    int port = atoi(strtok(NULL, ":"));
    mail_subject = argv[2];
    file_name_mail = argv[3];

    //create a message buffer
    char msg[1500];
    //setup a socket and connection tools
    struct hostent* host = gethostbyname(serverIp);
    sockaddr_in sendSockAddr;
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr =
    inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);
    clientSockt = socket(AF_INET, SOCK_STREAM, 0);
    //try to connect...
    int status = connect(clientSockt,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0) {
        cout<<"Error connecting to socket!"<<endl;
        return 0;
    }
    cout << "Connected to the server!" << endl;

     while(1)
    {
        cout << ">";
        string data;
        getline(cin, data);
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, data.c_str());
        send_To_Server(msg);
        cout<<"bugs";
        //send(clientSd, (char*)&msg, strlen(msg), 0);

        //memset(&buf, 0, sizeof(buf));//clear the buffer
        //recv(clientSd, (char*)&msg, sizeof(msg), 0);
    }
}

void send_To_Server(char* buf) {
    char buf1[TMP_LENGTH];
     if(strstr(buf, "HELO") || strstr(buf, "helo"))
        {
            gethostname(myHostName,TMP_LENGTH);
            strcat(buf, " ");
            strcat(buf,myHostName);
            strcat(buf,"\n");

            send(clientSockt, (char*)&buf, strlen(buf), 0);
            printf("%s",c);
            printf("%s",buf);

            memset(&buf1, 0, sizeof(buf1));//clear the buffer
            recv(clientSockt, (char*)&buf1, sizeof(buf1), 0);
            checkServerReturnedCode(buf1);
            cout<<s;
            cout<<buf1<<"\n";

        }

        else if(strstr(buf, "MAIL FROM") || strstr(buf, "mail from"))
        {

            strcat(buf, ":");
            //getlogin_r(from, TMP_LENGTH);
            strcat(from, "@");
            strcat(from, myHostName);

            strcat(buf,from);
            strcat(buf,"\n");
            send(clientSockt, (char*)&buf, strlen(buf), 0);

            printf("%s",c);
            printf("%s",buf);

            memset(&buf, 0, sizeof(buf));//clear the buffer
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            checkServerReturnedCode(buf);
            cout<<s;
            cout<<buf<<"\n";

        }

        else if(strstr(buf, "RCPT TO") || strstr(buf, "rcpt to"))
        {
            strcat(buf, ":");
            dest = receiver;
            strcat(buf,dest);
            strcat(buf,"\n");
            send(clientSockt, (char*)&buf, strlen(buf), 0);
            printf("%s",c);
            printf("%s",buf);


            memset(&buf, 0, sizeof(buf));//clear the buffer
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            checkServerReturnedCode(buf);
            cout<<s;
            cout<<buf<<"\n";

        }

        else if(strstr(buf, "DATA") || strstr(buf, "data"))
        {

            send(clientSockt, (char*)&buf, strlen(buf), 0);
            printf("%s",c);
            printf("%s",buf);

            memset(&buf, 0, sizeof(buf));//clear the buffer
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            checkServerReturnedCode(buf);
            cout<<s;
            cout<<buf<<"\n";

            if(checkServerReturnedCode(buf) == 354){

                 //sending the body
            char header[TMP_LENGTH] = "To:";
            strcat(header,dest);
            strcat(header,"\nFrom:");
            strcat(header,from);
            strcat(header,"\nSubject:");
            strcat(header, mail_subject);
            strcat(header,"\nDate:");
            char text[100];
            time_t now = time(NULL);
            struct tm *t = localtime(&now);


            strftime(text, sizeof(text)-1, "%d %m %Y %H:%M", t);
            strcat(header, text);

            //sending the headers
            memset(&buf, 0, sizeof(buf));//clear the buffer
            strcpy(buf,header);
            strcat(buf,"\n");
            send(clientSockt, (char*)&buf, strlen(buf), 0);
            printf("%s",c);
            printf("%s",buf);

            //sending the msg body
            memset(&buf, 0, sizeof(buf));//clear the buffer
            strcpy(buf,readFromMailFile(file_name_mail));
            send(clientSockt, (char*)&buf, strlen(buf), 0);
            cout<<buf<<"\n";
            printf("\n");

            memset(&buf, 0, sizeof(buf));//clear the buffer
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            checkServerReturnedCode(buf);
            cout<<s;
            cout<<buf<<"\n";


            strcpy(buf, "\r\n.\r\n");
            send(clientSockt, (char*)&buf, strlen(buf), 0);

            }


        }

        else if(strstr(buf, "QUIT") || strstr(buf, "quit"))
        {
            send(clientSockt, (char*)&buf, strlen(buf), 0);
            printf("%s",c);
            cout<<buf <<endl;

            memset(&buf, 0, sizeof(buf));//clear the buffer
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            if(checkServerReturnedCode(buf) == 221){
                cout<<s;
            cout<<buf<<"\n";
            //closing connection
            close(clientSockt);
            cout << "Connection closed" << endl;
            exit(0);
            }

        }

        else{
        cout<<"invalid\n";
        }
}
