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
#include <ostream>
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
    // char msgType_;
    uint64_t timestamp_;
    uint64_t orderId_;
    uint32_t token_;
    char orderType_;
    uint32_t price_;
    uint32_t quantity_;

    explicit orderMsg(const char *buffer) {
        *this = *reinterpret_cast<const orderMsg *>(buffer);
    }


    // void display() const {
    //     std::cout << msgType_ << "\t" << timestamp_ << "\t" << orderId_ << "\t" << token_ << "\t" << orderType_ << "\t"
    //             << price_ << "\t"
    //             << quantity_ << std::endl;
    // }

    friend std::ostream &operator<<(std::ostream &os, const orderMsg &obj) {
        return os
               // << "msgType_: " << obj.msgType_
               << " timestamp_: " << obj.timestamp_
               << " orderId_: " << obj.orderId_
               << " token_: " << obj.token_
               << " orderType_: " << obj.orderType_
               << " price_: " << obj.price_
               << " quantity_: " << obj.quantity_;
    }
};

class tradeMsg {
public:
    // char msgType_;
    uint64_t timestamp_;
    uint64_t buyOrderId_;
    uint64_t sellOrderId_;
    uint32_t token_;
    uint32_t tradePrice_;
    uint32_t quantity_;


    void getMessage(tradeMsg &trade, const char *buffer) {
        *this = *reinterpret_cast<const tradeMsg *>(buffer);
    }

    // void display() const {
    //     std::cout << msgType_ << "\t" << timestamp_ << "\t" << buyOrderId_ << "\t" << sellOrderId_ << "\t" << token_ <<
    //             "\t" << tradePrice_ << "\t"
    //             << quantity_ << std::endl;
    // }
    friend std::ostream &operator<<(std::ostream &os, const tradeMsg &obj) {
        return os
               // << "msgType_: " << obj.msgType_
               << " timestamp_: " << obj.timestamp_
               << " buyOrderId_: " << obj.buyOrderId_
               << " sellOrderId_: " << obj.sellOrderId_
               << " token_: " << obj.token_
               << " tradePrice_: " << obj.tradePrice_
               << " quantity_: " << obj.quantity_;
    }
};

class OrderBook {
    static bool BidCompare(const u_int32_t &a, const u_int32_t &b) {
        return a > b;
    }

    static bool AskCompare(const u_int32_t &a, const u_int32_t &b) {
        return a < b;
    }

    using BidComparator = decltype(&OrderBook::BidCompare);
    using AskComparator = decltype(&OrderBook::AskCompare);

    std::multimap<u_int32_t, orderMsg, BidComparator> BidTable;
    std::multimap<u_int32_t, orderMsg, AskComparator> AskTable;

public:
    OrderBook()
        : BidTable(&OrderBook::BidCompare), AskTable(&OrderBook::AskCompare) {
    }

    void addOrder(const orderMsg &order, char msgType) {
        if (order.orderType_ == 'B') {
            if (msgType == 'N') {
                BidTable.insert(std::make_pair(order.price_, order));
            } else if (msgType == 'M') {
                auto range = BidTable.equal_range(order.price_);
                bool found = false;
                if (range.first != range.second) {
                    for (auto it = range.first; it != range.second; ++it) {
                        if (it->second.orderId_ == order.orderId_) {
                            if (it->second.quantity_ > order.quantity_) {
                                it->second.quantity_ = order.quantity_;
                            } else {
                                BidTable.erase(it);
                                BidTable.insert(std::make_pair(order.price_, order));
                            }
                            found = true;
                            break;
                        }
                    }
                    if (found == false) {
                        BidTable.insert(std::make_pair(order.price_, order));
                    }
                } else {
                    //std::cout << "No order found to modify but still added a new order as per protocol" << std::endl;
                }
            } else if (msgType == 'X') {
                auto range = BidTable.equal_range(order.price_);
                bool found = false;
                for (auto it = range.first; it != range.second; ++it) {
                    if (it->second.orderId_ == order.orderId_) {
                        BidTable.erase(it);
                        found = true;
                        break;
                    }
                }
                if (found == false) {
                    //std::cout << "No order found to deleted" << std::endl;
                }
            }
        } else {
            if (msgType == 'N') {
                AskTable.insert(std::make_pair(order.price_, order));
            } else if (msgType == 'M') {
                auto range = AskTable.equal_range(order.price_);
                bool found = false;
                if (range.first != range.second) {
                    for (auto it = range.first; it != range.second; ++it) {
                        if (it->second.orderId_ == order.orderId_) {
                            if (it->second.quantity_ > order.quantity_) {
                                it->second.quantity_ = order.quantity_;
                            } else {
                                AskTable.erase(it);
                                AskTable.insert(std::make_pair(order.price_, order));
                            }
                            found = true;
                            break;
                        }
                    }
                    if (found == false) {
                        AskTable.insert(std::make_pair(order.price_, order));
                    }
                } else {
                    //std::cout << "No order found to modify but still added a new order as per protocol" << std::endl;
                }
            } else if (msgType == 'X') {
                auto range = AskTable.equal_range(order.price_);
                if (range.first != range.second) {
                    for (auto it = range.first; it != range.second; ++it) {
                        if (it->second.orderId_ == order.orderId_) {
                            AskTable.erase(it);
                            break;
                        }
                    }
                } else {
                    //std::cout << "No order found to delete" << std::endl;
                }
            }
        }
    }

    void matchTrade(const tradeMsg &trade) {
        auto range = BidTable.equal_range(trade.tradePrice_);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second.orderId_ == trade.buyOrderId_) {
                if (it->second.quantity_ >= trade.quantity_) {
                    it->second.quantity_ -= trade.quantity_;
                    if (it->second.quantity_ == 0) {
                        BidTable.erase(it);
                    }
                    break;
                }
            }
        }
        range = AskTable.equal_range(trade.tradePrice_);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second.orderId_ == trade.sellOrderId_) {
                if (it->second.quantity_ >= trade.quantity_) {
                    it->second.quantity_ -= trade.quantity_;
                    if (it->second.quantity_ == 0) {
                        AskTable.erase(it);
                    }
                    break;
                }
            }
        }
    }

    void display() const {
        auto it = this->BidTable.begin();
        auto it2 = this->AskTable.begin();
        std::cout << "Bid Table" << std::endl;
        while (it != this->BidTable.end()) {
            std::cout << it->second.orderId_ << "\t" << it->second.price_ << "\t" << it->second.quantity_ << std::endl;
            it++;
        }
        std::cout << "Ask Table" << std::endl;
        while (it2 != this->AskTable.end()) {
            std::cout << it2->second.orderId_ << "\t" << it2->second.price_ << "\t" << it2->second.quantity_ <<
                    std::endl;
            ++it2;
        }
    }
};


int main() {
    //std::cout<< sizeof(header)<< std::endl;
    std::ifstream file("/home/vaibhav/cm_stream_3", std::ios::binary);
    header msgHeader;
    OrderBook orderBook;

    char buffer[100];
    int count = 1e8;
    while (true and count-- != 0) {
        file.read(reinterpret_cast<char *>(&msgHeader), sizeof(header));
        if (file.gcount() != sizeof(header)) {
            std::cerr << "Error reading header!" << std::endl;
            break;
        }
        // msgHeader.display();
        // file.seekg(-1, std::ios::cur);
        file.read(buffer, msgHeader.msg_len_ - 9);
        if (file.gcount() != msgHeader.msg_len_ - 9) {
            std::cerr << "Error reading message data!" << std::endl;
            break;
        }

        if (msgHeader.msg_len_ == 38) {
            orderMsg order(buffer);
            // order.getMessage(order, buffer);
            orderBook.addOrder(order, msgHeader.msgType_);
            // std::cout << order << std::endl;
        }
        if (msgHeader.msg_len_ == 45) {
            tradeMsg trade;
            trade.getMessage(trade, buffer);
            // std::cout << trade << std::endl;
            orderBook.matchTrade(trade);
        }
    }
    orderBook.display();
    return 0;
}
