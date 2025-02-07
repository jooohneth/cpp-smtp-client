// ... existing code ...
// Replace Windows-specific headers with POSIX headers
#include <sys/socket.h> // POSIX library -- Unix-based alt. to Winsock2
#include <netinet/in.h> // POSIX library -- Unix-based alt. to Winsock2
#include <arpa/inet.h>  // POSIX library -- Unix-based alt. to Winsock2
#include <unistd.h>     // POSIX library -- Unix-based alt. to Winsock2
#include <iostream>

using namespace std;

int main()
{
    // Remove Windows-specific initialization (no WSAStartup needed)

    // Initialize socket (syntax is similar but slightly different)
    int ClientSocket;
    ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSocket < 0)
    {
        cout << "Socket creation failed" << endl;
        return 0;
    }

    // Connect socket to specified server
    struct sockaddr_in SvrAddr;
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(27000);
    SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(ClientSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)) < 0)

        cout << "Connection failed" << endl;
    close(ClientSocket);
    return 0;
}

while (1)
{
    char TxBuffer[128] = {};
    cout << "Enter a String to transmit" << endl;
    cin >> TxBuffer;
    send(ClientSocket, TxBuffer, sizeof(TxBuffer), 0);
}

// Use close() instead of closesocket()
close(ClientSocket);
// Remove WSACleanup as it's not needed
}