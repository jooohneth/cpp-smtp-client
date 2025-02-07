#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

using namespace std;

int main()
{
    // Initialize socket
    int ServerSocket;
    ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ServerSocket < 0)
    {
        cout << "Socket creation failed" << endl;
        return 0;
    }

    // Bind socket to address
    struct sockaddr_in SvrAddr;
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_addr.s_addr = INADDR_ANY;
    SvrAddr.sin_port = htons(27000);
    if (bind(ServerSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)) < 0)
    {
        cout << "Bind failed" << endl;
        close(ServerSocket);
        return 0;
    }

    // Listen on socket
    if (listen(ServerSocket, 1) < 0)
    {
        cout << "Listen failed" << endl;
        close(ServerSocket);
        return 0;
    }

    cout << "Waiting for client connection\n"
         << endl;

    // Accept connection from client
    int ConnectionSocket;
    if ((ConnectionSocket = accept(ServerSocket, NULL, NULL)) < 0)
    {
        cout << "Accept failed" << endl;
        close(ServerSocket);
        return 0;
    }

    cout << "Connection Established" << endl;

    while (1)
    {
        char RxBuffer[128] = {};
        recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
        cout << "Msg Rx: " << RxBuffer << endl;
    }

    close(ConnectionSocket); // Close incoming socket
    close(ServerSocket);     // Close server socket
}