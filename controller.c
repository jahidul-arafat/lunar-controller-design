#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <ncurses.h>
#include "include/controller.h"

// Define semaphores
sem_t sem_user_input, sem_server_comm, sem_dashboard_comm, sem_data_logging;

// Shared resources
struct user_input_data input_data;
struct server_response server_data;
FILE* log_file;

int main() {
    printf("Welcome to the Lunar Lander Controller!\n\n");
    printf("Steps to run the system:\n");
    printf("1. Start the Lunar Lander server: java -jar LunarLander.jar\n");
    printf("2. Start the dashboard: java -jar LanderDash.jar\n");
    printf("3. Press Enter here to start the controller...\n\n");
    getchar();

    // Initialize semaphores
    sem_init(&sem_user_input, 0, 1);
    sem_init(&sem_server_comm, 0, 1);
    sem_init(&sem_dashboard_comm, 0, 1);
    sem_init(&sem_data_logging, 0, 1);

    // Create threads
    pthread_t tid_user_input, tid_server_comm, tid_dashboard_comm, tid_data_logging;
    pthread_create(&tid_user_input, NULL, user_input_thread, NULL);
    pthread_create(&tid_server_comm, NULL, server_comm_thread, NULL);
    pthread_create(&tid_dashboard_comm, NULL, dashboard_comm_thread, NULL);
    pthread_create(&tid_data_logging, NULL, data_logging_thread, NULL);

    // Join threads
    pthread_join(tid_user_input, NULL);
    pthread_join(tid_server_comm, NULL);
    pthread_join(tid_dashboard_comm, NULL);
    pthread_join(tid_data_logging, NULL);

    // Destroy semaphores
    sem_destroy(&sem_user_input);
    sem_destroy(&sem_server_comm);
    sem_destroy(&sem_dashboard_comm);
    sem_destroy(&sem_data_logging);

    return 0;
}

// User input thread function
void* user_input_thread(void* arg) {
    initscr();
    cbreak();
    noecho();
    timeout(0);
    int ch;

    while (1) {
        sem_wait(&sem_user_input);
        ch = getch();
        if (ch == 'w') {
            input_data.throttle += 10;
            printw("Throttle increased to %d\n", input_data.throttle);
        } else if (ch == 's') {
            input_data.throttle -= 10;
            printw("Throttle decreased to %d\n", input_data.throttle);
        } else if (ch == 'a') {
            input_data.rotational_thrust -= 10;
            printw("Rotational thrust decreased to %d\n", input_data.rotational_thrust);
        } else if (ch == 'd') {
            input_data.rotational_thrust += 10;
            printw("Rotational thrust increased to %d\n", input_data.rotational_thrust);
        }
        refresh();
        sem_post(&sem_user_input);
        usleep(200000); // 5 times per second
    }
    endwin();
    return NULL;
}

// Server communication thread function
void* server_comm_thread(void* arg) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(65200); // Example port
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

    while (1) {
        sem_wait(&sem_server_comm);
        // Send request to server
        sendto(sockfd, "State:?", 7, 0, (const struct sockaddr*)&server_addr, sizeof(server_addr));
        // Receive response from server
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        buffer[n] = '\0';
        // Parse and store server response in server_data
        sscanf(buffer, "State:%255s Condition:%255s Terrain:%255s", server_data.state, server_data.condition, server_data.terrain);
        printf("Server -> State: %s, Condition: %s, Terrain: %s\n", server_data.state, server_data.condition, server_data.terrain);
        sem_post(&sem_server_comm);
        usleep(200000); // 5 times per second
    }
    close(sockfd);
    return NULL;
}

// Dashboard communication thread function
void* dashboard_comm_thread(void* arg) {
    int sockfd;
    struct sockaddr_in dashboard_addr;
    char buffer[1024];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&dashboard_addr, 0, sizeof(dashboard_addr));
    dashboard_addr.sin_family = AF_INET;
    dashboard_addr.sin_port = htons(65250); // Example port
    dashboard_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

    while (1) {
        sem_wait(&sem_dashboard_comm);
        // Prepare and send update message to dashboard
        sprintf(buffer, "State=%s;Condition=%s;Terrain=%s", server_data.state, server_data.condition, server_data.terrain);
        sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr*)&dashboard_addr, sizeof(dashboard_addr));
        printf("Dashboard <- %s\n", buffer);
        sem_post(&sem_dashboard_comm);
        usleep(200000); // 5 times per second
    }
    close(sockfd);
    return NULL;
}

// Data logging thread function
void* data_logging_thread(void* arg) {
    log_file = fopen("lander_log.csv", "w");
    fprintf(log_file, "Timestamp,Throttle,RotationalThrust,State,Condition,Terrain\n");

    while (1) {
        sem_wait(&sem_data_logging);
        // Write log data
        fprintf(log_file, "%ld,%d,%d,%s,%s,%s\n", time(NULL), input_data.throttle, input_data.rotational_thrust, server_data.state, server_data.condition, server_data.terrain);
        fflush(log_file);
        sem_post(&sem_data_logging);
        usleep(200000); // 5 times per second
    }
    fclose(log_file);
    return NULL;
}

