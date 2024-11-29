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
#pragma pack(push, 1)

struct header {
    uint64_t rxTimestamp_;
    int16_t msg_len_;
    int16_t streamId_;
    int32_t seqno_;
    char msgType_;

    void display() const {
        std::cout << rxTimestamp_ << "\t" << msg_len_ << "\t" << streamId_ << "\t" << seqno_ << "\t" << msgType_ << std::endl;
    }
};

#pragma pack(pop)

class Order {
public:
    uint64_t rxTimestamp_;
    short streamId_;
    short seqNumber_;
    char msgType_;
    long timestamp_;
    double orderId_;
    int token_;
    char orderType_;
    int price_;
    int quantity_;


    Order(long ts, short streamId_, short seqNumber_, char msgtyp, long times, double id, int token, char ordertype,
          int price, int quantity)
        : rxTimestamp_(ts), streamId_(streamId_), seqNumber_(seqNumber_), msgType_(msgtyp), timestamp_(times),
          orderId_(id), token_(token), orderType_(ordertype), price_(price), quantity_(quantity) {
    }


    void display() const {
        std::cout << rxTimestamp_ << "\t" << timestamp_ << "\t" << orderId_ << "\t" << token_ << "\t" << price_ << "\t"
                << quantity_ << std::endl;
    }


    bool operator<(const Order &other) const {
        if (price_ == other.price_) {
            return timestamp_ < other.timestamp_;
        }
        return price_ > other.price_;
    }


    bool operator>(const Order &other) const {
        if (price_ == other.price_) {
            return timestamp_ > other.timestamp_;
        }
        return price_ < other.price_;
    }
};

class BidTable {
private:
    // std::map where key = price and value = Order
    std::map<double, Order, std::greater<> > bids;

public:
    // Add a new bid order
    void addOrder(long ts, short streamId_, short seqNumber_, char msgtyp, long times, double id, int token,
                  char ordertype, int price, int quantity) {
       // bids[price] = Order(ts, streamId_, seqNumber_, msgtyp, times, id, token, ordertype, price, quantity);
        display();
    }

    // Delete an order by order_id
    void deleteOrder(int order_id) {
        for (auto it = bids.begin(); it != bids.end(); ++it) {
            if (it->second.orderId_ == order_id) {
                bids.erase(it);
                break;
            }
        }
        display();
    }

    // Modify an existing order
    void modifyOrder(long ts, short streamId_, short seqNumber_, char msgtyp, long times, double id, int token,
                     char ordertype, int price, int quantity) {
        deleteOrder(id);
        addOrder(ts, streamId_, seqNumber_, msgtyp, times, id, token, ordertype, price, quantity);
    }

    // Display the bid table (sorted by price descending)
    void display() const {
        std::cout << "Bid Table (Sorted by Price Descending):\n";
        std::cout << "RxTimestamp\tStreamID\tSeqNumber\tTimestamp\tOrderID\tToken\tOrderType\tPrice\tQuantity\n";
        for (const auto &order: bids) {
            order.second.display();
        }
        std::cout << "------------------------\n";
    }
};


class AskTable {
private:
    // std::map where key = price and value = Order
    std::map<double, Order> asks;

public:
    // Add a new ask order
    void addOrder(long ts, short streamId_, short seqNumber_, char msgtyp, long times, double id, int token,
                  char ordertype, int price, int quantity) {
        //checks for add?
       // asks[price] = Order(ts, streamId_, seqNumber_, msgtyp, times, id, token, ordertype, price, quantity);
        display();
    }

    // Delete an order by order_id
    void deleteOrder(int order_id) {
        for (auto it = asks.begin(); it != asks.end(); ++it) {
            if (it->second.orderId_ == order_id) {
                asks.erase(it);
                break;
            }
        }
        display();
    }

    // Modify an existing order
    void modifyOrder(long ts, short streamId_, short seqNumber_, char msgtyp, long times, double id, int token,
                     char ordertype, int price, int quantity) {
        deleteOrder(id); // Remove the old order
        addOrder(ts, streamId_, seqNumber_, msgtyp, times, id, token, ordertype, price, quantity);
        // Add the modified order
    }

    // Display the ask table (sorted by price ascending)
    void display() const {
        std::cout << "Ask Table (Sorted by Price Ascending):\n";
        std::cout << "RxTimestamp\tStreamID\tSeqNumber\tTimestamp\tOrderID\tToken\tOrderType\tPrice\tQuantity\n";
        for (const auto &order: asks) {
            order.second.display();
        }
        std::cout << "------------------------\n";
    }
};

void processMessage(BidTable &bid_table, AskTable &ask_table, std::pmr::vector<char> &message) {
    int index = 0;
    short streamId_ = static_cast<short>(static_cast<uint8_t>(message[index])) |
                      (static_cast<short>(static_cast<uint8_t>(message[index + 1])) << 8);
    index += 2;
    int seqNumber_ = static_cast<int>(static_cast<uint8_t>(message[index])) |
                     (static_cast<int>(static_cast<uint8_t>(message[index + 1])) << 8) |
                     (static_cast<int>(static_cast<uint8_t>(message[index + 2])) << 16) |
                     (static_cast<int>(static_cast<uint8_t>(message[index + 3])) << 24);
    index += 4;
    char msgType_ = static_cast<char>(static_cast<uint8_t>(message[index]));
    index += 1;

}

int main() {
    //std::cout<< sizeof(header)<< std::endl;
    std::ifstream file("/home/vaibhav/cm_stream_3", std::ios::binary);
    header msgHeader;
    char buffer[100];
    int count=100;
    while (true and count-->0 ) {
        file.read(reinterpret_cast<char*>(&msgHeader),sizeof(header));
        if (file.gcount() !=sizeof(header)) {
            std::cerr << "Error reading header!" << std::endl;
            break;
        }
        msgHeader.display();
        file.read(buffer,msgHeader.msg_len_-9);
        if (file.gcount() !=msgHeader.msg_len_-9) {
            std::cerr << "Error reading message data!" << std::endl;
            break;
        }
    }

    return 0;
    BidTable bidTable;
    AskTable askTable;
    // multiple




    if (!file) {
        std::cerr << "Error opening file!" << std::endl;
        return 0;
    }


    while (true) {
        uint8_t lengthBytes[2];
        file.read(reinterpret_cast<char *>(&lengthBytes), sizeof(lengthBytes));

        // Check if we reached the end of the file
        if (file.eof()) {
            break;
        }

        // Convert the 2 bytes to a short value (little-endian format)
        uint16_t messageLength = static_cast<uint16_t>(lengthBytes[0]) | (static_cast<uint16_t>(lengthBytes[1]) << 8);

        file.seekg(2, std::ios::cur);
        messageLength -= 2;
        // Read the actual message based on the length
        std::vector<char> message(messageLength);
        file.read(message.data(), messageLength);

        // Check for read errors
        if (file.fail()) {
            std::cerr << "Error reading message data!" << std::endl;
            break;
        }
       // processMessage(bidTable, askTable, message);
    }

    return 0;
}
