#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

#define SERVER_ADDR "3.248.203.233"
#define SERVER_PORT 8080
#define WEATHER_SERVER_NAME "api.openweathermap.org"
#define WEATHER_SERVER_ADDR "176.9.63.49"
#define WEATHER_SERVER_PORT 80

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    char **body;
    char **cookies;
    char *query_params;

    // Ex 1: GET dummy from main server
    printf("=============================================== 1 ===================================================\n\n");
    // Open a tcp connection with the main server
    sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(SERVER_ADDR, "/api/v1/dummy", NULL, NULL, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("%s\n", response);
    free(message);
    free(response);
    close_connection(sockfd);

    // =================================================================================================================
    printf("\n============================================= 2 ===================================================\n\n");
    // Ex 2: POST dummy and print response from main server
    sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    body = calloc(2, sizeof(char*));
    body[0] = calloc(LINELEN, sizeof(char));
    sprintf(body[0], "key=value");
    body[1] = calloc(LINELEN, sizeof(char));
    sprintf(body[1], "test=true");

    message = compute_post_request(
        SERVER_ADDR, "/api/v1/dummy", "application/x-www-form-urlencoded", body, 2, NULL, 0
    );
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("%s\n", response);
    free(message);
    free(response);
    close_connection(sockfd);

    // =================================================================================================================
    printf("\n============================================= 3 ===================================================\n\n");

    // Ex 3: Login into main server /api/v1/auth/login
    sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    memset(body[0], 0, LINELEN);
    sprintf(body[0], "username=student");
    memset(body[1], 0, LINELEN);
    sprintf(body[1], "password=student");

    message = compute_post_request(
        SERVER_ADDR, "/api/v1/auth/login", "application/x-www-form-urlencoded", body, 2, NULL, 0
    );
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("%s\n", response);

    // Extract all cookies from the response and store them for the next ex
    cookies = calloc(1, sizeof(char*));
    int cookies_size = 0;
    int cookies_capacity = 1;
    
    char *search_ptr = response;
    // Search for "Set-Cookie: " in the response and extract cookie values
    while ((search_ptr = strstr(search_ptr, "Set-Cookie: ")) != NULL) {
        search_ptr += strlen("Set-Cookie: "); // Move the pointer to the start of the cookie value
        
        // Find where the cookie value ends (at ';' or at the end of the line)
        char *end_ptr = strchr(search_ptr, ';');
        char *line_end = strchr(search_ptr, '\r');
        
        // Choose the closest terminator
        if (end_ptr == NULL || (line_end != NULL && line_end < end_ptr)) {
            end_ptr = line_end;
        }

        if (end_ptr != NULL) {
            int cookie_len = end_ptr - search_ptr;
            if (cookies_size >= cookies_capacity) {
                cookies_capacity *= 2;
                cookies = realloc(cookies, cookies_capacity * sizeof(char*));
            }
            cookies[cookies_size] = calloc(cookie_len + 1, sizeof(char));
            memcpy(cookies[cookies_size], search_ptr, cookie_len);
            cookies[cookies_size][cookie_len] = '\0';
            
            printf("Extracted Cookie: %s\n", cookies[cookies_size]);
            cookies_size++;
        }
    }

    free(message);
    free(response);
    close_connection(sockfd);

    // =================================================================================================================
    printf("\n============================================= 4 ===================================================\n\n");

    // Ex 4: GET weather key from main server /api/v1/weather/key
    sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    message = compute_get_request(SERVER_ADDR, "/api/v1/weather/key", NULL, cookies, cookies_size);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("%s\n", response);

    // Extract the weather key from the response
    char weather_key[LINELEN] = {0};
    char *json_start = basic_extract_json_response(response);
    if (json_start != NULL) {
        char *weather_key_start = strchr(json_start, ':');
        // Move the pointer to the start of the weather key value (after the `:"`)
        weather_key_start += 2;
        char *weather_key_end = strchr(weather_key_start, '"');
        strncpy(weather_key, weather_key_start, weather_key_end - weather_key_start);
        printf("\nExtracted Weather Key: %s\n", weather_key);
    }

    free(message);
    free(response);
    close_connection(sockfd);

    // =================================================================================================================
    printf("\n============================================= 5 ===================================================\n\n");

    // Ex 5: GET weather data from OpenWeather API
    const char latitude[] = "47.01";
    const char longitude[] = "28.84";

    sockfd = open_connection(WEATHER_SERVER_ADDR, WEATHER_SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    query_params = calloc(LINELEN, sizeof(char));
    sprintf(query_params, "lat=%s&lon=%s&appid=%s", latitude, longitude, weather_key);

    message = compute_get_request(WEATHER_SERVER_ADDR, "/data/2.5/weather", query_params, cookies, cookies_size);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("%s\n", response);

    // Prepare the weather data to be sent to the main server in the next ex
    free(body[1]);
    memset(body[0], 0, LINELEN);
    json_start = basic_extract_json_response(response);
    if (json_start != NULL) {
        strncpy(body[0], json_start, LINELEN - 1);  // Leave space for null terminator
    }

    free(message);
    free(response);

    close_connection(sockfd);

    // =================================================================================================================
    printf("\n============================================= 6 ===================================================\n\n");

    // Ex 6: POST weather data for verification to main server
    sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    message = compute_post_request(
        SERVER_ADDR, "/api/v1/dummy", "application/x-www-form-urlencoded", body, 1, NULL, 0
    );
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("%s\n", response);
    free(message);
    free(response);
    close_connection(sockfd);

    // =================================================================================================================
    printf("\n============================================= 7 ===================================================\n\n");

    // Ex 7: Logout from main server
    sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    message = compute_get_request(SERVER_ADDR, "/api/v1/auth/logout", NULL, cookies, cookies_size);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("%s\n", response);

    free(message);
    free(response);
    close_connection(sockfd);

    // TODO BONUS: make the main server return "Already logged in!"
    return 0;
}
