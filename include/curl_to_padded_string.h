#ifndef CURL_TO_PADDED_STRING_H
#define CURL_TO_PADDED_STRING_H

/**
 * @brief Header file for loading URLs into padded strings using curl and simdjson.
 *
 * This namespace provides utilities to fetch content from URLs and prepare it for
 * efficient JSON parsing with simdjson.
 *
 * Functions:
 * - size_t get_actual_payload_size(const std::string& url):
 *   Returns the total size in bytes of the content at the given URL.
 *   Returns 0 on failure or if curl initialization fails.
 *
 * - std::expected<simdjson::padded_string, std::runtime_error> load_url(const std::string& url):
 *   Downloads the content from the URL and returns it as a padded_string suitable for simdjson parsing.
 *   Returns an unexpected with runtime_error on failure.
 */

#include <string>
#include <expected>
#include <stdexcept>
#include <curl/curl.h>
#include <simdjson.h>

namespace curl_to_padded_string {
/// @brief Get the actual payload size of a URL without downloading the full content.
/// 
/// This function performs a full request to determine the content size
/// by using a write callback that counts bytes without storing them.
/// 
/// @param url The URL string to check (must be a valid HTTP/HTTPS URL).
/// @return The size in bytes of the content at the URL, or 0 if the request fails
///         or curl initialization fails.
inline size_t get_actual_payload_size(const std::string& url);

/// @brief Load the content of a URL into a padded string optimized for JSON parsing.
/// 
/// Downloads the full content from the URL and stores it in a simdjson padded_string,
/// which provides the necessary padding for efficient SIMD JSON parsing.
/// 
/// @param url The URL string to load (must be a valid HTTP/HTTPS URL).
/// @return An expected containing:
///         - On success: A padded_string with the downloaded content
///         - On failure: An unexpected with a runtime_error describing the issue
inline std::expected<simdjson::padded_string, std::runtime_error> load_url(const std::string& url);






/////////
// Implementation details below. Users should only need the declarations above.
/////////
namespace internal {
static size_t count_callback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t len = size * nmemb;
    size_t* count = static_cast<size_t*>(userdata);
    *count += len;
    return len; // Return len to tell curl we "processed" it
}

static size_t builder_callback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* bldr = static_cast<simdjson::padded_string_builder*>(userdata);
    size_t total = size * nmemb;
    bool ok = bldr->append(static_cast<const char*>(ptr), total);
    return ok ? total : 0;
}
}

inline size_t get_actual_payload_size(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return 0;

    size_t total_bytes = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Write callback that simply increments the counter
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, internal::count_callback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &total_bytes);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK) ? total_bytes : 0;
}

inline std::expected<simdjson::padded_string, std::runtime_error> load_url(const std::string& url) {
    simdjson::padded_string_builder builder;

    CURL* curl = curl_easy_init();
    if (!curl) {
     return std::unexpected(std::runtime_error("Failed to initialize curl"));
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 15000L); // 15 seconds

    // Write callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, internal::builder_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &builder);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return std::unexpected(std::runtime_error(curl_easy_strerror(res)));
    }
    //

    return builder.convert();
}


} // namespace curl_to_padded_string

#endif // CURL_TO_PADDED_STRING_H