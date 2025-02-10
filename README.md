# SMTP Server in C++

This is a school assignment, that shills "JUST A BOY" by PetraVoice.
Note: this assumes you're running this from a UNIX-based OS.

## How to run?

Note: For the sake of dev experience I changed the original code provided.

1. Clone the repo:

```shell
    git clone https://github.com/jooohneth/cpp-smtp-client
```

2. I'm running on Mac, a UNIX based system. So before we being you'll probably need OpenSSL to encrypt messages, because they are so important u know.

```shell
    brew install openssl@3
```

if you don't have "brew" installed, see this [resource](https://brew.sh/) and run command below:

```shell
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

3. Configure the Server

   ```cpp
   // -----------------------SENDER----------------------------

    #define EMAIL_SENDER "jooohnng@gmail.com" // ----> YOUR EMAIL
    #define EMAIL_PASSWORD "\***\* \*\*** \***\* \*\***" // ----> YOUR APP PASSWORD

    // -----------------------RECIPIENTS------------------------

    #define EMAIL_RECEIVER "jooohn.eth@gmail.com" // ----> RECIPIENT EMAIL
    #define EMAIL_CC_RECIPIENTS {"snehamrzzn@gmail.com", "dn269234@gmail.com", "alikh.og00@gmail.com"} // ----> CC RECIPIENTS
    #define EMAIL_BCC_RECIPIENTS {"john@mantle.xyz"} // ----> BCC RECIPIENTS

    // -----------------------CONTENT----------------------------

    #define SUBJECT "THE GREATEST EMAIL OF ALL TIME" // ----> SUBJECT
    #define BODY "JUST A BOY \nhttps://justaboy.us" // ----> BODY

    // -----------------------FILE PATH--------------------------

    #define FILEPATH "./attachment/boyss.jpg" // ----> FILE PATH

   ```

   - Sender: an email address from which the email will be sent

   - Password: this is the App Password, that can be obtained in your Google Account. [See Guide](https://support.google.com/accounts/answer/185833?hl=en).

   - Receiver: primary email address that receives the email

   - CC and BCC: email address that will also receive a copy of the email. BCC'd addresses will be hidden to everyone else.

   - Subject: you know what it is... make it good!

   - Body: straighforward... make it even better pls.

   - Filepath: include the path to a file that you want to attach to the email. Could be of any type. Place the file you want to attach inside the "/attachment" folder. (Optional)

4. Compile the server:

```shell
    g++ -std=c++17 -Wall -g -o ./run/smtpClient smtpClient.cpp lib/base64.cpp -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto
```

Note: this assumes you're using g++ to compile your C++ code. See this [resource](https://stackoverflow.com/questions/2122425/how-do-i-install-g-on-macos-x) to learn to install G++.

5. Finally, run the server

```shell
    ./run/smtpClient
```
