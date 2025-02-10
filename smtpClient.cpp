// g++ -std=c++17 -Wall -g -o ./run/smtpClient smtpClient.cpp lib/base64.cpp -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto

// including POSIX socket API
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

// including OpenSSL library
#include <openssl/ssl.h>
#include <openssl/err.h>

// including standard libraries
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "lib/base64.h"

#define EMAIL_SENDER "jooohnng@gmail.com"
#define EMAIL_PASSWORD "zidu dags yrrm kpkv"
#define EMAIL_RECEIVER "jooohn.eth@gmail.com"
#define EMAIL_CC_RECIPIENTS {"snehamrzzn@gmail.com", "dn269234@gmail.com"}
#define EMAIL_BCC_RECIPIENTS {"john@mantle.xyz"}
#define FILEPATH "./attachment/boyss.jpg"

#define SUBJECT "THE GREATEST EMAIL OF ALL TIME"
#define BODY "JUST A BOY \nhttps://justaboy.us "

// function to send SMTP command
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

// function to read file
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

// Add this function to determine MIME type
std::string getMimeType(const std::string &filename)
{
    // Get file extension
    size_t dot_pos = filename.find_last_of(".");
    if (dot_pos == std::string::npos)
        return "application/octet-stream";

    std::string ext = filename.substr(dot_pos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Common MIME types
    if (ext == ".txt")
        return "text/plain";
    if (ext == ".pdf")
        return "application/pdf";
    if (ext == ".jpg" || ext == ".jpeg")
        return "image/jpeg";
    if (ext == ".png")
        return "image/png";
    if (ext == ".gif")
        return "image/gif";
    if (ext == ".doc" || ext == ".docx")
        return "application/msword";
    if (ext == ".xls" || ext == ".xlsx")
        return "application/vnd.ms-excel";
    if (ext == ".zip")
        return "application/zip";
    if (ext == ".mp3")
        return "audio/mpeg";
    if (ext == ".mp4")
        return "video/mp4";

    // Default to binary stream if unknown
    return "application/octet-stream";
}

// main function
int main()
{

    // SMTP server details
    const char *smtpServer = "smtp.gmail.com";
    const int smtpPort = 465; // SMTP over SSL

    // Sender credentials
    const std::string emailSender = EMAIL_SENDER;
    const std::string emailPassword = EMAIL_PASSWORD;

    // Recipients
    const std::string emailReceiver = EMAIL_RECEIVER;
    const std::vector<std::string> ccRecipients = EMAIL_CC_RECIPIENTS;
    const std::vector<std::string> bccRecipients = EMAIL_BCC_RECIPIENTS;

    const std::string subject = SUBJECT;
    const std::string body = BODY;

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

        // Send encoded credentials
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

        // Construct CC header field, note: not including BCC recipients
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
        std::string filepath = FILEPATH; // Replace with your file path
        std::string filename = filepath; // For relative path, use the filepath as filename

        // Read and encode the file
        std::string fileContent = readFile(filepath);
        std::string encodedFile = base64_encode(reinterpret_cast<const unsigned char *>(fileContent.c_str()),
                                                fileContent.length());

        // Modify the email content construction part
        std::string emailContent = "From: " + emailSender + "\r\n"
                                                            "To: " +
                                   emailReceiver + "\r\n" +
                                   ccHeader +
                                   "Subject: " + subject + "\r\n"
                                                           "MIME-Version: 1.0\r\n"
                                                           "Content-Type: multipart/mixed; boundary=\"boundary\"\r\n\r\n"
                                                           "--boundary\r\n"
                                                           "Content-Type: text/plain; charset=utf-8\r\n\r\n" +
                                   body + "\r\n\r\n"
                                          "--boundary\r\n"
                                          "Content-Type: " +
                                   getMimeType(filename) + "; name=\"" + filename + "\"\r\n"
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