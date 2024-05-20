#ifndef CONTROLLER_H
#define CONTROLLER_H

// Structure to hold user input data
struct user_input_data {
    int throttle;
    int rotational_thrust;
};

// Structure to hold server response data
struct server_response {
    char state[256];
    char condition[256];
    char terrain[256];
};

// Thread functions
void* user_input_thread(void* arg);
void* server_comm_thread(void* arg);
void* dashboard_comm_thread(void* arg);
void* data_logging_thread(void* arg);

#endif // CONTROLLER_H

