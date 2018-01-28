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
char* dest = "null";



void send_To_Server();

int checkServerReturnedCode(char* buf) {

    char server_returned_code[4]="   ";
    memcpy(server_returned_code, buf,strlen(server_returned_code));
    int code=0;
    code=atoi(server_returned_code);


    //checking if the code is valid
    if(code<200 ||code>399) {
        //printf("code number:%d\n",code);
        printf("ERROR:%s\n",buf);
    }

    return code;

}

char* readFromMailFile(char* file_name) {
    ifstream t;
    int length;
    t.open(file_name);
    t.seekg(0, ios::end);
    length = t.tellg();           // this is the length
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
    for(int i =1; i<4; i++) {
        if(argv[i] == NULL) {
            cout<<"Invalid parameters";
            exit(0);
        }
    }
    if(!strstr(argv[1], ":")) {
        cout<<"Incorrect formatting: Please provide port with : "<<endl;
    }
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

    send_To_Server();
}

void send_To_Server() {

    char buf[TMP_LENGTH];

    while(1) {
        string data;
        getline(cin, data);
        memset(&buf, 0, sizeof(buf));//clear the buffer
        strcpy(buf, data.c_str());
        //cin>>buf;
        printf("%s", c);
        printf("%s\n", buf);

        if(strstr(buf, "HELO") || strstr(buf, "helo")) {
            gethostname(myHostName,TMP_LENGTH);
            strcat(buf, " ");
            strcat(buf,myHostName);
            strcat(buf,"\n");

            send(clientSockt, (char*)&buf, strlen(buf), 0);
            printf("%s",c);
            printf("%s",buf);

            bzero(buf,TMP_LENGTH);
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            checkServerReturnedCode(buf);
            cout<<s;
            cout<<buf<<"\n";

        }

        else if(strstr(buf, "MAIL FROM") || strstr(buf, "mail from")) {

            strcat(buf, ":");
            //getlogin_r(from, TMP_LENGTH);
            char from[TMP_LENGTH];
            memset(&from, 0, sizeof(from));
            strcpy(from,getenv("USER"));
            strcat(from, "@");
            strcat(from, myHostName);

            strcat(buf,from);
            strcat(buf,"\n");
            send(clientSockt, (char*)&buf, strlen(buf), 0);

            printf("%s",c);
            printf("%s",buf);

            bzero(buf,TMP_LENGTH);
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            checkServerReturnedCode(buf);
            cout<<s;
            cout<<buf<<"\n";

        }

        else if(strstr(buf, "RCPT TO") || strstr(buf, "rcpt to")) {
            strcat(buf, ":");
            dest = receiver;
            strcat(buf,dest);
            strcat(buf,"\n");
            if(!strstr(buf, "@")){
            cout<<"Incorrect Formattting of email address\n";
            continue;
            }
            send(clientSockt, (char*)&buf, strlen(buf), 0);
            printf("%s",c);
            printf("%s",buf);


            bzero(buf,TMP_LENGTH);
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            checkServerReturnedCode(buf);
            cout<<s;
            cout<<buf<<"\n";

        }

        else if(strstr(buf, "DATA") || strstr(buf, "data")) {

            send(clientSockt, (char*)&buf, strlen(buf), 0);
            printf("%s",c);
            printf("%s\n",buf);

            bzero(buf,TMP_LENGTH);
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            if(checkServerReturnedCode(buf) != 354){
            continue;
            }
            cout<<s;
            cout<<buf<<"\n";

            //sending the body
            char header[TMP_LENGTH] = "To:";
            strcat(header,dest);
            strcat(header,"\nFrom:");
            char* from = getenv("USER");
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
            bzero(buf,TMP_LENGTH);
            strcpy(buf,header);
            strcat(buf,"\n");
            send(clientSockt, (char*)&buf, strlen(buf), 0);
            printf("%s",c);
            printf("%s",buf);

            //sending the msg body
            bzero(buf,TMP_LENGTH);
            strcpy(buf,readFromMailFile(file_name_mail));
            send(clientSockt, (char*)&buf, strlen(buf), 0);
            cout<<buf<<"\n";
            printf("\n");

            bzero(buf,TMP_LENGTH);
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            checkServerReturnedCode(buf);
            cout<<s;
            cout<<buf<<"\n";


            strcpy(buf, "\r\n.\r\n");
            send(clientSockt, (char*)&buf, strlen(buf), 0);
            cout<<buf<<"\n";


        }

        else if(strstr(buf, "QUIT") || strstr(buf, "quit")) {
            send(clientSockt, (char*)&buf, strlen(buf), 0);
            printf("%s",c);
            cout<<buf <<endl;

            bzero(buf,TMP_LENGTH);
            recv(clientSockt, (char*)&buf, sizeof(buf), 0);
            checkServerReturnedCode(buf);
            cout<<s;
            cout<<buf<<"\n";
            //closing connection
            close(clientSockt);
            cout << "Connection closed" << endl;
            exit(0);

        }

        else {
            cout<<"500  Command not implemented\n";
        }

    }

}
