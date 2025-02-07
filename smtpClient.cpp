#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#define SOCKET int               // On UNIX/Linux, sockets are just integers
#define INVALID_SOCKET -1        // Value indicating socket creation failed
#define SOCKET_ERROR -1          // Value indicating a socket operation failed
#define SOCKET_ERROR_CODE errno  // On UNIX/Linux, errors are stored in errno
#define CLOSE_SOCKET(s) close(s) // Maps to the close() function on UNIX/Linux
#define INIT_SOCKETS()           // Empty macro (needed only for Windows)
#define CLEANUP_SOCKETS()        // Empty macro (needed only for Windows)

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
#include <string>
#include "base64.h"
#include <fstream>
#include <sstream>

#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

void sendSMTPCommand(SSL *ssl, const std::string &command, const std::string &expectedResponse)
{
    SSL_write(ssl, command.c_str(), command.size());
    char response[1024] = {0};
    SSL_read(ssl, response, sizeof(response) - 1);
    std::cout << "Server Response: " << response;
    if (expectedResponse != "" && std::string(response).find(expectedResponse) == std::string::npos)
    {
        throw std::runtime_error("Unexpected SMTP response.");
    }
}

std::string readFile(const std::string &filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

int main()
{

    // SMTP server details
    const char *smtpServer = "smtp.gmail.com";
    const int smtpPort = 465; // SMTP over SSL

    // Sender credentials
    const std::string emailSender = "jooohnng@gmail.com";
    const std::string emailPassword = "zidu dags yrrm kpkv";

    // Recipients
    const std::string emailReceiver = "jooohn.eth@gmail.com";
    const std::vector<std::string> ccRecipients = {"snehamrzzn@gmail.com"};
    const std::vector<std::string> bccRecipients = {"john@mantle.xyz"};

    const std::string subject = "Test Email with Attachment";
    const std::string body = "This is a test email sent using Winsock and OpenSSL.\n\nPlease find the attached file.";

    try
    {
        // Create socket
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET)
        {
            throw std::runtime_error("Socket creation failed.");
        }

        // Resolve SMTP server address
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(smtpPort);
        struct hostent *host = gethostbyname(smtpServer);
        memcpy(&serverAddr.sin_addr, host->h_addr_list[0], host->h_length);

        // Connect to the SMTP server
        if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            close(sock);
            throw std::runtime_error("Connection to SMTP server failed.");
        }

        // Initialize OpenSSL
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
        if (!ctx)
        {
            close(sock);
            throw std::runtime_error("Failed to create SSL context.");
        }

        // Create SSL structure and connect it to the socket
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sock);
        if (SSL_connect(ssl) <= 0)
        {
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(sock);
            throw std::runtime_error("SSL connection failed.");
        }

        // SMTP Communication
        char buffer[1024];
        SSL_read(ssl, buffer, sizeof(buffer) - 1); // Read initial response

        sendSMTPCommand(ssl, "EHLO localhost\r\n", "250");
        sendSMTPCommand(ssl, "AUTH LOGIN\r\n", "334");

        // Encode credentials in Base64
        std::string base64Email = base64_encode(reinterpret_cast<const unsigned char *>(emailSender.c_str()), emailSender.size());
        std::string base64Password = base64_encode(reinterpret_cast<const unsigned char *>(emailPassword.c_str()), emailPassword.size());

        sendSMTPCommand(ssl, base64Email + "\r\n", "334");
        sendSMTPCommand(ssl, base64Password + "\r\n", "235");

        // Send MAIL FROM and RCPT TO commands for all recipients
        sendSMTPCommand(ssl, "MAIL FROM:<" + emailSender + ">\r\n", "250");

        // Primary recipient
        sendSMTPCommand(ssl, "RCPT TO:<" + emailReceiver + ">\r\n", "250");

        // CC recipients
        for (const auto &cc : ccRecipients)
        {
            sendSMTPCommand(ssl, "RCPT TO:<" + cc + ">\r\n", "250");
        }

        // BCC recipients
        for (const auto &bcc : bccRecipients)
        {
            sendSMTPCommand(ssl, "RCPT TO:<" + bcc + ">\r\n", "250");
        }

        // Construct CC header field
        std::string ccHeader;
        if (!ccRecipients.empty())
        {
            ccHeader = "Cc: ";
            for (size_t i = 0; i < ccRecipients.size(); ++i)
            {
                ccHeader += ccRecipients[i];
                if (i < ccRecipients.size() - 1)
                {
                    ccHeader += ", ";
                }
            }
            ccHeader += "\r\n";
        }

        // File to attach (relative path)
        std::string filepath = "./test.txt"; // Replace with your file path
        std::string filename = filepath;     // For relative path, use the filepath as filename

        // Read and encode the file
        std::string fileContent = readFile(filepath);
        std::string encodedFile = base64_encode(reinterpret_cast<const unsigned char *>(fileContent.c_str()),
                                                fileContent.length());

        // Construct email with attachment
        std::string emailContent = "From: " + emailSender + "\r\n"
                                                            "To: " +
                                   emailReceiver + "\r\n" +
                                   ccHeader + // CC recipients visible in header
                                   "Subject: " + subject + "\r\n"
                                                           "MIME-Version: 1.0\r\n"
                                                           "Content-Type: multipart/mixed; boundary=\"boundary\"\r\n\r\n"
                                                           "--boundary\r\n"
                                                           "Content-Type: text/plain; charset=utf-8\r\n\r\n" +
                                   body + "\r\n\r\n"
                                          "--boundary\r\n"
                                          "Content-Type: application/octet-stream; name=\"" +
                                   filename + "\"\r\n"
                                              "Content-Transfer-Encoding: base64\r\n"
                                              "Content-Disposition: attachment; filename=\"" +
                                   filename + "\"\r\n\r\n" +
                                   encodedFile + "\r\n\r\n"
                                                 "--boundary--\r\n.\r\n";

        // Send email
        sendSMTPCommand(ssl, "DATA\r\n", "354");
        sendSMTPCommand(ssl, emailContent, "250");

        // Quit
        sendSMTPCommand(ssl, "QUIT\r\n", "221");

        // Cleanup
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sock);

        std::cout << "Email sent successfully.\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}