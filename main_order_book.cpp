// DEBUG: g++ -std=c++11 -IBOOST_PATH order_book.cpp -pedantic -Wall 
// RELEASE: g++ -std=c++11 -IBOOST_PATH order_book.cpp -DNDEBUG

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <assert.h>
#include <stdint.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/member.hpp>

namespace bmi = boost::multi_index;

class OrderBook {
    private:
        struct order {int32_t id; double price;};
        
        /* orders contained within a multi_index of two views:              
         *   view 0: hashmap of order ID  
         *           (allows for O(1) insert/erase of orders) 
         *   view 1: multiset of prices, ordered by descending price                  
         *           (allows for O(lg n) access to current maximum price)   */
        typedef bmi::multi_index_container<order, 
                    bmi::indexed_by<
                        bmi::hashed_unique<bmi::member<order, int32_t, &order::id>>,
                        bmi::ordered_non_unique<
                            bmi::member<order, double, &order::price>, 
                            std::greater<double>
                        >
                    >
                > orderbook_data;
        
        orderbook_data orders;
        
        const double dNaN = std::numeric_limits<double>::quiet_NaN();
        
        uint64_t current_timestamp = 0;   
        uint64_t total_nonempty_time = 0;
        double timeweighted_maxprice_sum = 0.0;
        
        void update_stats(uint64_t new_timestamp) {
            // only update for time intervals during which order book was non-empty
            if (!orders.empty()) { 
                uint64_t time_interval = new_timestamp - current_timestamp; 
         
                if (time_interval > 0) {
                    total_nonempty_time += time_interval;  
                    timeweighted_maxprice_sum += double(time_interval) * get_highest_price();
                }
            }
            current_timestamp = new_timestamp;
        }
  
    public:
        void insert(uint64_t time_stamp, int32_t order_id, double price) {
            // first update stats up until new time stamp, then insert new entry
            update_stats(time_stamp);  
            orders.insert({order_id, price});
        }
  
        void erase(uint64_t time_stamp, int32_t order_id) {
            // first update stats up until this new time stamp, then erase entry
            update_stats(time_stamp);
            orders.erase(order_id); 
        }
  
        double get_highest_price() {
            // access current max price directly from first element in view 1
            return !orders.empty() ? orders.get<1>().begin()->price : dNaN;
        }
  
        double get_time_average_of_maxprices() {
            return total_nonempty_time > 0 ? timeweighted_maxprice_sum / double(total_nonempty_time) : dNaN;
        }
};

int main(int argc, char** argv) {
    const char INSERT_OP = 'I', ERASE_OP = 'E';
 
    OrderBook order_book;

    assert(argc > 0);
    std::ifstream input_file(argv[1]);
    assert(input_file);  

    std::string line;
    while (std::getline(input_file, line)) {
        uint64_t time_stamp; 
        int32_t order_id;  
        char operation;     
      
        std::istringstream iss(line);
        iss >> time_stamp >> operation >> order_id;

        if (operation == INSERT_OP) {
            double price;  // additional column 'price' for inserts only
            iss >> price; 
            order_book.insert(time_stamp, order_id, price);   
        }
        else if (operation == ERASE_OP) {
            order_book.erase(time_stamp, order_id);
        }
    }

    input_file.close();

    std::cout << "time-averaged highest price: "
              << order_book.get_time_average_of_maxprices() << std::endl;
   
    return 0;
}
