#include<iostream>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string>
#include<string.h>
#include<unistd.h>
#include<vector>
#include<fstream>

void error(const char* msg){
    perror(msg);
    exit(0);
}

int main(int argc, char* argv[])
{
    if(argc < 2){
        std::cout << "Usage: " << argv[0] << " Port" << std::endl;
        error("Exiting");
    }

    //Setting server_fd, connected_fd and address struct
    int portno = atoi(argv[1]), server_fd, connected_fd;
    struct sockaddr_in address;
    int addresslen = sizeof(address);

    //Retrieving socket file-descriptor
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        error("ERROR creating socket");

    //IPV4 address, and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(portno);

    //Binding address to socket
    if(bind(server_fd, (struct sockaddr*) &address, sizeof(address)) < 0){
        error("Error binding address to socket");
    }

    //Socket for handshaking clients
    if(listen(server_fd, 3) < 0){
        error("Listen failed");
    }

    //Buffer to recieve, reply request, stat to check for file
    char request[256];
    char reply[100];
    struct stat st;

    //As long as server is running
    while(true){
        //Accepting client connection
        std::cout << "Waiting to accept incomming request" << std::endl;
        if((connected_fd = accept(server_fd, (struct sockaddr*) &address, (socklen_t*)&addresslen)) < 0){
            error("Accept failed");
        }

        //Retrieving request from client
        if(read(connected_fd, request, 255) < 0)
            error("Failed reading from connected socket");

        //Checking if file exists to reply client with file size or nack.
        if(stat(request, &st) == 0){
            int size_bytes = st.st_size;
            sprintf(reply, "%i", size_bytes);
            send(connected_fd, reply, strlen(reply)+1, 0);
            std::cout << "File requested: " << request << std::endl;

            //Sending file in 1000 byte chunks
            const int CHUNKSIZE = 1000;
            std::vector<char> filebuffer(CHUNKSIZE, 0);
            std::ifstream ifile(request, std::ifstream::binary);

            //Until requested file ends
            while(1){
                ifile.read(filebuffer.data(), CHUNKSIZE);
                //If file at end, get read size with gcount, else it must be chunksize
                std::streamsize size = ((ifile)? CHUNKSIZE: ifile.gcount());
                std::cout << "Sending " << size << "Bytes" << std::endl;
                send(connected_fd, filebuffer.data(), size, 0);
                //If file at end, break
                if(!ifile)
                    break;
            }
        }else{
            std::cout << "Client requested nonvalid file" << std::endl;
            std::string reply = "No such file could be accessed";
            send(connected_fd, reply.c_str(), reply.length()+1, 0);
        }
    }
    return 0;
}
