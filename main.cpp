#include <iostream>
#include <string>
#include <curl/curl.h>
#include "include/json.hpp"
#include <chrono>
#include <thread>
#include <future>

using json = nlohmann::json;

// Function to handle the response from the cURL request
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// General function to send a cURL request with optional access token
std::string sendRequest(const std::string& url, const json& payload, const std::string& accessToken = "") {
    std::string readBuffer;
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);  // Set the HTTP method to POST

        // Set the request payload
        std::string jsonStr = payload.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

        // Set headers, including Authorization if accessToken is provided
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!accessToken.empty()) {
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set up the write callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Enable HTTP/2 for improved performance
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 60L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 30L);

        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Free Resources
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return readBuffer;
}

// Function to get the access token
std::string getAccessToken(const std::string& clientId, const std::string& clientSecret) {
    json payload = {
        {"id", 0},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"scope", "session:apiconsole-c5i26ds6dsr expires:2592000"},
            {"client_id", clientId},
            {"client_secret", clientSecret}
        }},
        {"jsonrpc", "2.0"}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/public/auth", payload);
    auto responseJson = json::parse(response);

    // Retrieve the access token from the response
    if (responseJson.contains("result") && responseJson["result"].contains("access_token")) {
        return responseJson["result"]["access_token"];
    } else {
        std::cerr << "Failed to retrieve access token." << std::endl;
        return "";
    }
}

// Function to place an order
void placeOrder(const std::string& price, const std::string& accessToken, const std::string& amount, const std::string& instrument) {
    auto start = std::chrono::high_resolution_clock::now();
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/buy"},
        {"params", {
            {"instrument_name", instrument},
            {"type", "limit"},
            {"price", price},
            {"amount", amount}
        }},
        {"id", 1}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/buy", payload, accessToken);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end - start;

    std::cout << "Place Order Response: " << response << std::endl;
    std::cout << "Order Placement Latency: " << latency.count() << " seconds" << std::endl;
}

// Function to cancel an order
void cancelOrder(const std::string& accessToken, const std::string& orderID) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/cancel"},
        {"params", {{"order_id", orderID}}},
        {"id", 6}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/cancel", payload,accessToken);
    std::cout << "Cancel Order Response: " << response << std::endl;
}

// Function to modify an order
void modifyOrder(const std::string& accessToken, const std::string& orderId, double newPrice, double newAmount) {
    // Create the payload for modifying an order
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/edit"},
        {"params", {
            {"order_id", orderId},
            {"price", newPrice},
            {"amount", newAmount}
        }},
        {"id", 2}
    };

    // Send the request
    std::string response = sendRequest("https://test.deribit.com/api/v2/private/edit", payload, accessToken);
    auto responseJson = json::parse(response);

    // Check for errors in the response
    if (responseJson.contains("error")) {
        std::cerr << "Error modifying order: " << responseJson["error"]["message"] << std::endl;
        if (responseJson["error"]["data"].contains("reason")) {
            std::cerr << "Reason: " << responseJson["error"]["data"]["reason"] << std::endl;
        }
        return; // Exit the function
    }

    // Handle successful response
    if (responseJson.contains("result")) {
        std::cout << "Order modified successfully: " << responseJson["result"] << std::endl;
    } else {
        std::cerr << "Unexpected response format while modifying order." << std::endl;
    }
}
// Function to retrieve the order book
void getOrderBook(const std::string& accessToken, const std::string& instrument) {
    auto start = std::chrono::high_resolution_clock::now();

    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "public/get_order_book"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 15}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_order_book",payload, accessToken);

    auto responseJson = json::parse(response);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end -start;

    if (responseJson.contains("result")) {
        std::cout << "Order Book: " << responseJson["result"] << std::endl;
    } else {
        std::cerr << "Error: Could not retrieve order book." << std::endl;
    }

    std::cout << "Market Data Processing Latency: " << latency.count() << " seconds" << std::endl;

      std::cout << "Order Book for " << instrument << ":\n\n";
            
            // Printing best bid and ask
            std::cout << "Best Bid Price: " << responseJson["result"]["best_bid_price"] << ", Amount: " << responseJson["result"]["best_bid_amount"] << '\n';
            std::cout << "Best Ask Price: " << responseJson["result"]["best_ask_price"] << ", Amount: " << responseJson["result"]["best_ask_amount"] << '\n';

            // Printing bids and asks in detail
            std::cout << "Asks:\n";
            for (const auto& ask : responseJson["result"]["asks"]) {
                std::cout << "Price: " << ask[0] << ", Amount: " << ask[1] << '\n';
            }

            std::cout << "\nBids:\n";
            for (const auto& bid : responseJson["result"]["bids"]) {
                std::cout << "Price: " << bid[0] << ", Amount: " << bid[1] << '\n';
            }

            // Additional information
            std::cout << "\nMark Price: " << responseJson["result"]["mark_price"] << '\n';
            std::cout << "Open Interest: " << responseJson["result"]["open_interest"] << '\n';
            std::cout << "Timestamp: " << responseJson["result"]["timestamp"] << '\n';
}

// Function to get position details of a specific instrument
void getPosition(const std::string& accessToken, const std::string& instrument) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/get_position"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 20}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_position", payload, accessToken);
    auto responseJson = json::parse(response);
    
    // Parse and print position details if available
    if (responseJson.contains("result")) {
                std::cout << "Position Details for " << instrument << ":\n\n";
                auto result = responseJson["result"];
                std::cout << "Estimated Liquidation Price: " << result["estimated_liquidation_price"] << '\n';
                std::cout << "Size Currency: " << result["size_currency"] << '\n';
                std::cout << "Realized Funding: " << result["realized_funding"] << '\n';
                std::cout << "Total Profit Loss: " << result["total_profit_loss"] << '\n';
                std::cout << "Realized Profit Loss: " << result["realized_profit_loss"] << '\n';
                std::cout << "Floating Profit Loss: " << result["floating_profit_loss"] << '\n';
                std::cout << "Leverage: " << result["leverage"] << '\n';
                std::cout << "Average Price: " << result["average_price"] << '\n';
                std::cout << "Delta: " << result["delta"] << '\n';
                std::cout << "Interest Value: " << result["interest_value"] << '\n';
                std::cout << "Mark Price: " << result["mark_price"] << '\n';
                std::cout << "Settlement Price: " << result["settlement_price"] << '\n';
                std::cout << "Index Price: " << result["index_price"] << '\n';
                std::cout << "Direction: " << result["direction"] << '\n';
                std::cout << "Open Orders Margin: " << result["open_orders_margin"] << '\n';
                std::cout << "Initial Margin: " << result["initial_margin"] << '\n';
                std::cout << "Maintenance Margin: " << result["maintenance_margin"] << '\n';
                std::cout << "Kind: " << result["kind"] << '\n';
                std::cout << "Size: " << result["size"] << '\n';
    } else {
        std::cerr << "Error: Could not retrieve position data." << std::endl;
    }
}

// Function to print all open orders with instrument, order ID, price, and amount
void getOpenOrders(const std::string& accessToken) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/get_open_orders"},
        {"params", {{"kind", "future"}, {"type", "limit"}}},
        {"id", 25}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_open_orders", payload, accessToken);
    auto responseJson = json::parse(response);

    // Check if the response contains the "result" array
    if (responseJson.contains("result")) {
        std::cout << "Open Orders:\n\n";
        for (const auto& order : responseJson["result"]) {
            std::string instrument = order["instrument_name"];
            std::string orderId = order["order_id"];
            double price = order["price"];
            double amount = order["amount"];

            std::cout << "Instrument: " << instrument << ", Order ID: " << orderId
                      << ", Price: " << price << ", Amount: " << amount << '\n';
        }
    } else {
        std::cerr << "Error: Could not retrieve open orders." << std::endl;
    }
}

void tradingLoop(const std::string& accessToken) {
    auto start = std::chrono::high_resolution_clock::now();

    placeOrder("92500", accessToken, "10", "BTC-PERPETUAL");
    getOrderBook(accessToken, "BTC-PERPETUAL");

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end - start;

    std::cout << "End-to-End Trading Loop Latency: " << latency.count() << " seconds" << std::endl;
}

void performParallelTasks(const std::string& accessToken) {
    auto task1 = std::async(std::launch::async, [&]() { placeOrder("3300", accessToken, "10", "ETH-PERPETUAL"); });
    auto task2 = std::async(std::launch::async, [&]() { getOrderBook(accessToken, "BTC-PERPETUAL"); });
    auto task3 = std::async(std::launch::async, [&]() { placeOrder("3300", accessToken, "10", "ETH-PERPETUAL"); });
    auto task4 = std::async(std::launch::async, [&]() { getOrderBook(accessToken, "BTC-PERPETUAL"); });

    task1.get();
    task2.get();
}

void menuDrivenSystem(const std::string& accessToken) {
    int choice;
    do {
        std::cout << "\nMenu:\n";
        std::cout << "1. Place Order\n";
        std::cout << "2. Modify Order\n";
        std::cout << "3. Cancel Order\n";
        std::cout << "4. Get Order Book\n";
        std::cout << "5. Get Position\n";
        std::cout << "6. Get Open Orders\n";
        std::cout << "7. Perform Parallel Tasks\n";
        std::cout << "8. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1: {
                std::string price, amount, instrumentName;
                std::cout << "Enter price: ";
                std::cin >> price;
                std::cout << "Enter amount: ";
                std::cin >> amount;
                std::cout << "Enter instrument name: ";
                std::cin >> instrumentName;
                placeOrder(price, accessToken, amount, instrumentName);
                break;
            }
            case 2: {
                std::string orderId;
                double newPrice, newAmount;
                std::cout << "Enter order ID: ";
                std::cin >> orderId;
                std::cout << "Enter new price: ";
                std::cin >> newPrice;
                std::cout << "Enter new amount: ";
                std::cin >> newAmount;
                try {
                    modifyOrder(accessToken, orderId, newPrice, newAmount);
                } catch (const std::exception& e) {
                    std::cerr << "Error modifying order: " << e.what() << std::endl;
                }
                break;
            }
            case 3: {
                std::string orderId;
                std::cout << "Enter order ID: ";
                std::cin >> orderId;
                try {
                    cancelOrder(accessToken, orderId);
                } catch (const std::exception& e) {
                    std::cerr << "Error cancelling order: " << e.what() << std::endl;
                }
                break;
            }
            case 4: {
                std::string instrumentName;
                std::cout << "Enter instrument name: ";
                std::cin >> instrumentName;
                getOrderBook(accessToken, instrumentName);
                break;
            }
            case 5: {
                std::string instrumentName;
                std::cout << "Enter instrument name: ";
                std::cin >> instrumentName;
                getPosition(accessToken, instrumentName);
                break;
            }
            case 6: {
                getOpenOrders(accessToken);
                break;
            }
            case 7: {
                performParallelTasks(accessToken);
                break;
            }
            case 8: {
                std::cout << "Exiting...\n";
                break;
            }
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 8);
}

int main() {
    // Replace with your actual client credentials
    std::string clientId = "n6Vgsl7Q";
    std::string clientSecret = "J6tMvbiejsCV_4zdGRpegv5FK0y152KsZ3sjdvTz8k4";

    // Retrieve the access token
    std::string accessToken = getAccessToken(clientId, clientSecret);

    if (!accessToken.empty()) {
        performParallelTasks(accessToken);
        
        //placeOrder("95000", accessToken, "10","BTC-PERPETUAL");

        //cancelOrder(accessToken,"29257473891");

        //modifyOrder(accessToken,"29231860132",30,30);

       // getOrderBook(accessToken,"BTC-PERPETUAL");

        std::cout<<"\n";

        getPosition(accessToken, "BTC-PERPETUAL");
        
        getOpenOrders(accessToken);
         menuDrivenSystem(accessToken);
        tradingLoop(accessToken);
    
    } else {
        std::cerr << "Unable to obtain access token, aborting operations." << std::endl;
    }

    return 0;
}
