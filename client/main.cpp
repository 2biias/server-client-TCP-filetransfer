#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string>
#include<string.h>
#include<iostream>
#include<unistd.h>
#include<arpa/inet.h>
#include<vector>
#include<fstream>


bool stringisnumber(const std::string& s)
{
    //Iterate over string while pointing at number and not at end
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void error(const char *msg){
    perror(msg);
    exit(0);
}

int main(int argc, char const *argv[])
{
    if(argc < 4){
        std::cout << "Usage: " << argv[0] << " hostname port file" << std::endl;
        exit(0);
    }

    //Setting port number, socketfd, host entry and address struct
    int portno = atoi(argv[2]), socketfd = 0, n=0;
    struct sockaddr_in server_address;

    //Retrieving socket file-descriptor
    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("ERROR opening socket");

    //IPV4 address, and port
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portno);

    //Converting hostname to address
    if(inet_pton(AF_INET, argv[1], &server_address.sin_addr) < 0){
        error("ERROR converting hostname to address type");
    }

    //Connecting to passed hostname and port
    if(connect(socketfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0){
        std::cout << "Error connecting to host: " << argv[1] << ":" << portno << std::endl;
        error("Closing");
    }

    //file(path) on server
    char filePath[strlen(argv[3])+1];
    strcpy(filePath, argv[3]);

    //Requesting file
    if((n = write(socketfd, filePath, sizeof(filePath))) < 0){
        error("Error writing to socket");
    }

    //Getting reply from server
    char reply[256];
    if((n = read(socketfd, reply, 255)) < 0){
        error("Error reading from socket");
    }
    std::cout << "Reply from server:\n" << reply << std::endl;

    //If reply is a number (file size), get the whole file
    if(stringisnumber(reply)){
        std::cout << "Getting file from server" << std::endl;

        int bytes_left = std::stoi(reply);
        int bytes = 0;
        const int CHUNKSIZE = 1000;
        std::vector<char> filebuffer(CHUNKSIZE, 0);
        std::ofstream ofs;
        std::string fileplacement = get_current_dir_name();
        fileplacement.append("/myFile");

        //Make entry for streaming bytes into fileplacement
        ofs.open(fileplacement, std::ofstream::out | std::ofstream::binary);

        //While theres still bytes left, read in chunks of maximum 1000 bytes
        while(bytes_left > 0){
            bytes = read(socketfd, filebuffer.data(), CHUNKSIZE);
            ofs.write(filebuffer.data(), bytes);
            std::cout << "Bytes received: " << bytes << std::endl;
            bytes_left=bytes_left-bytes;
        }
        ofs.close();
        std::cout << "Filed recieved as 'myFile' in calling path: " << fileplacement << std::endl;
    }

    close(socketfd);

    return 0;
}
