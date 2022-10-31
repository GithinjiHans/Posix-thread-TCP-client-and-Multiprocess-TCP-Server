#include <array>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <wait.h>
#include <map>

std::string outerBinary;

std::string encode(std::string str){
    std::string unique = "";
    std::string encodedString = "";
    for(int i=0; i< str.length(); i++){
        if(isalpha(str[i])){
            int match =0;
            for(int j=0; j < unique.length(); j++){
                if(unique.at(j)==str.at(i)){
                    match =1;
                    break;
                }
            }
            if(match == 0){
              unique += str[i];
            }
        }
    }
    int freq[unique.length()];
    for(int i=0; i< unique.length(); i++){
        freq[i] = 0;
        for(int j=0; j< str.length(); j++){
            if(unique.at(i)==str.at(j)){
                freq[i]++;
            }
        }
    }
    double freqFraction[unique.length()];
    for(int i=0; i< unique.length(); i++){
        freqFraction[i] = (double)freq[i]/str.length();
    }
    double encoding[unique.length()];
    double sum = 0;
    for(int i=0; i < unique.length(); i++){
        encoding[i] = sum + freqFraction[i]*0.5;
        sum += freqFraction[i];
    }
    std::string binaryString[unique.length()];
    // CONVERT each encoding to binary
    for(int i=0; i < unique.length(); i++){
        double temp = encoding[i];
        std::string binary = "";
        while(temp > 0){
            if(temp >= 0.5){
                binary += "1";
                temp -= 0.5;
            }
            else{
                binary += "0";
            }
            temp *= 2;
        }
        // TRUNCATE the binary string to 4 bits
        if(binary.length() > 4){
            binary = binary.substr(0,4);
        }
        else{
            while(binary.length() < 4){
                binary += "0";
            }
        }
        binaryString[i] = binary;
        // printf("%c: %s \n",unique.at(i),binaryString[i].c_str());
    }
    std::string newBinary = "";
    std::string s = "Symbol ";
    std::string c = " Code ";
    for(int i=0; i < unique.length(); i++){
        newBinary+= s + unique[i] + ',' + c +binaryString[i] + "\n";
    }
    outerBinary=newBinary;
    return outerBinary;
}

int main(int argc, char** argv)
{
    int port = 0;
    if (argc == 2)
    {
        port = atoi(argv[1]);
    }
    else
    {
        std::cout << "Usage: " << argv[0] << " port" << std::endl;
        return 1;
    }

    // create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        std::cout << "Error creating socket" << std::endl;
        return 1;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // bind the socket to our local address and port
    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        std::cout << "Error binding socket" << std::endl;
        return 1;
    }

    // start listening, allowing a queue of up to 5 pending connection
    listen(sock, 5);

    // accept connection from client
    struct sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    int clientSock = accept(sock, (struct sockaddr*)&client, &clientSize);
    if (clientSock < 0)
    {
        std::cout << "Error accepting connection" << std::endl;
        return 1;
    }
 int pid = fork();
    if (pid == 0)
    {
        // child process
        char  buffer[1024];
        memset(buffer, 0, 1024);
        int n = 0;
        while((n=recv(clientSock, buffer, 1024, 0)) > 0)
        {
            // encode the string
            std::string encodedString = encode(buffer);
            send(clientSock, encodedString.c_str(), encodedString.length(), 0);
            memset(buffer, 0, 1024);
        }
        if (n < 0)
        {
            std::cout << "Error reading from socket" << std::endl;
            return 1;
        }
    }
    else
    {
        // parent process
        // wait for child process to finish
        wait(NULL);
        // close the socket
        close(sock);
    }

return 0;
}