#include "curl_to_padded_string.h"
#include <iostream>
#include <cassert>

int main() {
    std::string url = "https://httpbin.org/json";
    size_t size = curl_to_padded_string::get_actual_payload_size(url);
    std::cout << "Actual payload size from " << url << " is " << size << " bytes." << std::endl;
    auto result = curl_to_padded_string::load_url(url);
    if (!result) {
        std::cerr << "Error: " << result.error().what() << std::endl;
        return EXIT_FAILURE;
    }
    auto& padded = result.value();
    std::cout << "Loaded " << padded.size() << " bytes from " << url << std::endl;
    std::cout << "Test passed!" << std::endl;
    return EXIT_SUCCESS;
}