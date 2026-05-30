#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>

enum class transactionType {
    None = 0,
    deposit = 1,
    withdraw = 2
};

std::string transactionTypeToString(transactionType type) {
    if (type == transactionType::deposit) return "deposit";
    if (type == transactionType::withdraw) return "withdraw";
    return "unknown";
}

class Transaction {
    int amount {};
    transactionType type = transactionType::None;
public:
    void deposit(int value) {
        this->amount = value;
        type = transactionType::deposit;
    }
    void withdraw(int value) {
        this->amount = value;
        type = transactionType::withdraw;
    }
    friend std::ostream &operator<<(std::ostream &os, const Transaction &transaction);
};

int balance = 0;
std::vector<Transaction> transactions;

std::ostream &operator<<(std::ostream &os, const Transaction &transaction) {
    os << "Type: " << transactionTypeToString(transaction.type) << ", amount: " << transaction.amount;
    return os;
}

std::string createHttpResponse(const std::string &body, int status, const std::string &statusText) {
    const std::string fullBody = body + "\n";
    return "HTTP/1.1 " + std::to_string(status) + " " + statusText + "\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: " + std::to_string(fullBody.size()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Connection: close\r\n"
           "\r\n" + fullBody;
}

void handleClient(const int clientSocket) {
    char buffer[4096] = {0};
    read(clientSocket, buffer, sizeof(buffer));
    std::string request(buffer);
    std::string response;
    if (request.find("POST /deposit") != std::string::npos) {
        std::string body = request.substr(request.find("\r\n\r\n") + 4);

        try {
            int amount = std::stoi(body);
            if (amount <= 0) {
                response = createHttpResponse("Amount for deposit must be positive\n", 200, "OK");
            } else {
                balance += amount;
                Transaction transaction;
                transaction.deposit(amount);
                transactions.push_back(transaction);
                response = createHttpResponse(
                    std::to_string(amount) + " deposited successfully\n", 200, "OK");
            }
        }
        catch (const std::exception&) {
            response = createHttpResponse(
                "Invalid deposit amount\n", 400, "Bad Request");
        }
    }
    else if (request.find("POST /withdraw") != std::string::npos) {
        std::string body = request.substr(request.find("\r\n\r\n") + 4);
        try {
            int amount = std::stoi(body);
            if (amount <= 0) {
                response = createHttpResponse("Invalid withdrawal amount\n", 400, "Bad Request");
            }
            else if (balance < amount) {
                response = createHttpResponse("Insufficient funds\n", 400, "Bad Request");
            }
            else {
                balance -= amount;
                Transaction transaction;
                transaction.withdraw(amount);
                transactions.push_back(transaction);
                response = createHttpResponse(
                    std::to_string(amount) + " withdrawn successfully\n", 200, "OK");
            }
        }
        catch (const std::exception&) {
            response = createHttpResponse(
                "Invalid withdrawal amount\n", 400, "Bad Request");
        }
    }
    else if (request.find("GET /balance") != std::string::npos) {
        response = createHttpResponse("Balance: " + std::to_string(balance), 200, "OK");
    }
    else if (request.find("GET /transactions") != std::string::npos) {
        std::stringstream ss;
        for (auto transaction : transactions) {
            ss << transaction << std::endl;
        }
        response = createHttpResponse("Transactions:\n" + ss.str(), 200, "OK");
    }
    send(clientSocket, response.c_str(), response.size(), 0);
    close(clientSocket);
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    int port = 8080;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);
    std::cout << "Server running on port " << port << std::endl;
    while (true) {
        socklen_t addrlen = sizeof(serverAddress);
        int clientSocket = accept(
            serverSocket,
            (struct sockaddr*)&serverAddress,
            &addrlen
        );
        if (clientSocket < 0) continue;
        handleClient(clientSocket);
    }
    return 0;
}