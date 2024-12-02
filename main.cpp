#include <iostream>
#include<map>
#include<string>
#include<vector>
#include<algorithm>
#include<functional>
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<cctype>
#include<cmath>
#include<climits>
#include <cstdint>
#include <fstream>
#pragma pack(1)

struct header {
    uint64_t rxTimestamp_;
    int16_t msg_len_;
    int16_t streamId_;
    int32_t seqNo_;
    char msgType_;

    void display() const {
        std::cout << rxTimestamp_ << "\t" << msg_len_ << "\t" << streamId_ << "\t" << seqNo_ << "\t" << msgType_ <<
                std::endl;
    }
};

class orderMsg {
public:
    char msgType_;
    uint64_t timestamp_;
    uint64_t orderId_;
    uint32_t token_;
    char orderType_;
    uint32_t price_;
    uint32_t quantity_;

    void getMessage(orderMsg &order, const char *buffer) {
        order.msgType_ = buffer[0];

        // Reverse the bytes (convert from little-endian to big-endian)
        unsigned char reversed_bytes[8];
        for (int i = 0; i < 8; ++i) {
            reversed_bytes[i] = buffer[8 - i];
        }

        // Combine the reversed bytes into an integer
        order.timestamp_ = 0;
        for (int i = 0; i < 8; ++i) {
            timestamp_ |= (static_cast<uint64_t>(reversed_bytes[i]) << (8 * (7 - i)));
        }

        for (int i = 0; i < 8; ++i) {
            reversed_bytes[i] = buffer[16 - i];
        }
        order.orderId_ = 0;
        for (int i = 0; i < 8; ++i) {
            orderId_ |= (static_cast<uint64_t>(reversed_bytes[i]) << (8 * (7 - i)));
        }

        unsigned char reversed_bytes2[4];
        for (int i = 0; i < 4; ++i) {
            reversed_bytes2[i] = buffer[20 - i];
        }
        order.token_ = 0;
        for (int i = 0; i < 4; ++i) {
            token_ |= (static_cast<uint32_t>(reversed_bytes2[i]) << (8 * (3 - i)));
        }
        order.orderType_ = buffer[21];

        for (int i = 0; i < 4; ++i) {
            reversed_bytes2[i] = buffer[25 - i];
        }
        order.price_ = 0;
        for (int i = 0; i < 4; ++i) {
            price_ |= (static_cast<uint32_t>(reversed_bytes2[i]) << (8 * (3 - i)));
        }
        price_ = price_ / 100;

        for (int i = 0; i < 4; ++i) {
            reversed_bytes2[i] = buffer[29 - i];
        }
        order.quantity_ = 0;
        for (int i = 0; i < 4; ++i) {
            quantity_ |= (static_cast<uint32_t>(reversed_bytes2[i]) << (8 * (3 - i)));
        }
    }

    void display() const {
        std::cout << msgType_ << "\t" << timestamp_ << "\t" << orderId_ << "\t" << token_ << "\t" << orderType_ << "\t"
                << price_ << "\t"
                << quantity_ << std::endl;
    }
};

class tradeMsg {
public:
    char msgType_;
    uint64_t timestamp_;
    uint64_t buyOrderId_;
    uint64_t sellOrderId_;
    uint32_t token_;
    uint32_t tradePrice_;
    uint32_t quantity_;

    void getMessage(tradeMsg &trade, const char *buffer) {
        msgType_ = buffer[0];
        // Reverse the bytes (convert from little-endian to big-endian)
        unsigned char reversed_bytes[8];
        for (int i = 0; i < 8; ++i) {
            reversed_bytes[i] = buffer[8 - i];
        }
        // Combine the reversed bytes into an integer
        timestamp_ = 0;
        for (int i = 0; i < 8; ++i) {
            timestamp_ |= (static_cast<uint64_t>(reversed_bytes[i]) << (8 * (7 - i)));
        }

        for (int i = 0; i < 8; ++i) {
            reversed_bytes[i] = buffer[16 - i];
        }
        buyOrderId_ = 0;
        for (int i = 0; i < 8; ++i) {
            buyOrderId_ |= (static_cast<uint64_t>(reversed_bytes[i]) << (8 * (7 - i)));
        }

        for (int i = 0; i < 8; ++i) {
            reversed_bytes[i] = buffer[24 - i];
        }
        sellOrderId_ = 0;
        for (int i = 0; i < 8; ++i) {
            sellOrderId_ |= (static_cast<uint64_t>(reversed_bytes[i]) << (8 * (7 - i)));
        }

        unsigned char reversed_bytes2[4];
        for (int i = 0; i < 4; ++i) {
            reversed_bytes2[i] = buffer[28 - i];
        }
        token_ = 0;
        for (int i = 0; i < 4; ++i) {
            token_ |= (static_cast<uint32_t>(reversed_bytes2[i]) << (8 * (3 - i)));
        }

        for (int i = 0; i < 4; ++i) {
            reversed_bytes2[i] = buffer[32 - i];
        }
        tradePrice_ = 0;
        for (int i = 0; i < 4; ++i) {
            tradePrice_ |= (static_cast<uint32_t>(reversed_bytes2[i]) << (8 * (3 - i)));
        }
        tradePrice_ = tradePrice_ / 100;

        for (int i = 0; i < 4; ++i) {
            reversed_bytes2[i] = buffer[36 - i];
        }
        quantity_ = 0;
        for (int i = 0; i < 4; ++i) {
            quantity_ |= (static_cast<uint32_t>(reversed_bytes2[i]) << (8 * (3 - i)));
        }
    }

    void display() const {
        std::cout << msgType_ << "\t" << timestamp_ << "\t" << buyOrderId_ << "\t" << sellOrderId_ << "\t" << token_ <<
                "\t" << tradePrice_ << "\t"
                << quantity_ << std::endl;
    }
};

class OrderBook {
    static bool BidCompare(const orderMsg &a, const orderMsg &b) {
        if (a.price_ != b.price_) {
            return a.price_ > b.price_;
        }
        return a.timestamp_ < b.timestamp_;
    }

    static bool AskCompare(const orderMsg &a, const orderMsg &b) {
        if (a.price_ != b.price_) {
            return a.price_ < b.price_;
        }
        return a.timestamp_ < b.timestamp_;
    }

    std::multimap<u_int32_t, orderMsg, decltype(&OrderBook::BidCompare)> BidTable;
    std::multimap<u_int32_t, orderMsg, decltype(&OrderBook::AskCompare)> AskTable;

public:
    OrderBook()
        : BidTable(&OrderBook::BidCompare), AskTable(&OrderBook::AskCompare) {
    }
    void addOrder(const orderMsg &order) {
        if (order.orderType_ == 'B') {
            if (order.msgType_=='N') {
                BidTable.insert(std::make_pair(order.price_, order));
            }
            else if (order.msgType_=='M') {
             
            }
        } else {
            AskTable.insert(std::make_pair(order.price_, order));
        }
    }
    void display() const {
        std::cout << "Bid Table" << std::endl;
    }
};


int main() {
    //std::cout<< sizeof(header)<< std::endl;
    std::ifstream file("/home/vaibhav/cm_stream_3", std::ios::binary);
    header msgHeader;

    char buffer[100];
    int count = 100;
    while (true and count-- > 0) {
        file.read(reinterpret_cast<char *>(&msgHeader), sizeof(header));
        if (file.gcount() != sizeof(header)) {
            std::cerr << "Error reading header!" << std::endl;
            break;
        }
        msgHeader.display();
        file.seekg(-1, std::ios::cur);
        file.read(buffer, msgHeader.msg_len_ - 8);
        if (file.gcount() != msgHeader.msg_len_ - 8) {
            std::cerr << "Error reading message data!" << std::endl;
            break;
        }

        if (msgHeader.msg_len_ == 38) {
            orderMsg order;
            order.getMessage(order, buffer);
            order.display();
        }
        if (msgHeader.msg_len_ == 45) {
            tradeMsg trade;
            trade.getMessage(trade, buffer);
            trade.display();
        }
    }
    return 0;
}
