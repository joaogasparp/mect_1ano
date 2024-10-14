#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#ifndef SERVER
#define SERVER 0
#endif

#define max_message_size 7u
#define n_samples 8193
#define max_iterations 256

typedef struct
{
    unsigned int message_size;
    unsigned int data[max_message_size];
} message_t;

static int write_data(int socket_fd, void *data, size_t data_size)
{
    struct timeval time_out;
    fd_set w_mask, e_mask;
    ssize_t bytes_written;

    while (data_size > (size_t)0)
    {
        FD_ZERO(&w_mask);
        FD_SET(socket_fd, &w_mask);
        FD_ZERO(&e_mask);
        FD_SET(socket_fd, &e_mask);
        time_out.tv_sec = 10; // 10 seconds
        time_out.tv_usec = 0;
        (void)select(socket_fd + 1, NULL, &w_mask, &e_mask, &time_out); // block if no data can be sent
        if (FD_ISSET(socket_fd, &w_mask) == 0)
            return -1; // either error or time out
        bytes_written = send(socket_fd, data, data_size, 0);
        if (bytes_written <= (ssize_t)0)
            return -2; // write error
        data += bytes_written;
        data_size -= bytes_written;
    }
    return 0; // ok
}

static int read_data(int socket_fd, void *data, size_t data_size)
{
    struct timeval time_out;
    fd_set r_mask, e_mask;
    ssize_t bytes_read;

    while (data_size > (size_t)0)
    {
        FD_ZERO(&r_mask);
        FD_SET(socket_fd, &r_mask);
        FD_ZERO(&e_mask);
        FD_SET(socket_fd, &e_mask);
        time_out.tv_sec = 10; // 10 seconds
        time_out.tv_usec = 0;
        (void)select(socket_fd + 1, &r_mask, NULL, &e_mask, &time_out); // block if no data can be read
        if (FD_ISSET(socket_fd, &r_mask) == 0)
            return -1; // either error or time out
        bytes_read = recv(socket_fd, data, data_size, 0);
        if (bytes_read <= (ssize_t)0)
            return -2; // read error
        data += bytes_read;
        data_size -= bytes_read;
    }
    return 0; // ok
}

static int send_message(int socket_fd, message_t *m)
{
    return write_data(socket_fd, m, sizeof(message_t));
}

static int receive_message(int socket_fd, message_t *m)
{
    return read_data(socket_fd, m, sizeof(message_t));
}

static void close_socket(int socket_fd)
{
    if (close(socket_fd) < 0)
    {
        perror("close_socket(): close");
        exit(1);
    }
}

#if SERVER != 0
static int setup_server(int port_number)
{
    struct sockaddr_in peer_address;
    int listen_fd;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("setup_server(): socket");
        exit(1);
    }
    peer_address.sin_family = AF_INET;
    peer_address.sin_addr.s_addr = htonl(INADDR_ANY);
    peer_address.sin_port = htons(port_number);
    if (bind(listen_fd, (struct sockaddr *)&peer_address, sizeof(peer_address)) == -1)
    {
        perror("setup_server(): bind");
        exit(1);
    }
    if (listen(listen_fd, 2) == -1)
    {
        perror("setup_server(): listen");
        exit(1);
    }
    return listen_fd;
}

static int get_connection(int listen_fd, char connection_ipv4_address[32])
{
    struct sockaddr_in peer_address;
    socklen_t peer_length;
    int connection_fd;

    do
    {
        peer_length = (socklen_t)sizeof(peer_address);
        connection_fd = accept(listen_fd, (struct sockaddr *)&peer_address, &peer_length);
    } while (connection_fd < 0);
    if (connection_ipv4_address != NULL)
        sprintf(connection_ipv4_address, "%.30s", inet_ntoa(peer_address.sin_addr));
    return connection_fd;
}

void alarm_signal_handler(int dummy)
{
    exit(1);
}

int main(int argc, char **argv)
{
    int port_number, listen_fd, connection_fd, total_count = 0;
    char connection_ipv4_address[32];
    message_t m;

    if (argc != 2)
    {
    usage:
        fprintf(stderr, "usage: %s port_number  # 49152 <= port_number <= 65535\n", argv[0]);
        exit(1);
    }
    port_number = atoi(argv[1]);
    if (port_number < 49152 || port_number > 65535)
        goto usage;

    (void)signal(SIGALRM, alarm_signal_handler);
    (void)alarm(15u * 60u);

    listen_fd = setup_server(port_number);
    printf("Server is running and waiting for clients...\n");

    while (1)
    {
        connection_fd = get_connection(listen_fd, connection_ipv4_address);
        printf("Connected to client: %s\n", connection_ipv4_address);

        if (receive_message(connection_fd, &m) < 0)
        {
            perror("receive_message");
            close_socket(connection_fd);
            continue;
        }

        if (m.message_size == 1)
        {
            int start_idx = m.data[0];
            int end_idx = m.data[1];
            printf("Received work request for range %d to %d\n", start_idx, end_idx);

            // Send the range to the client
            m.message_size = 2;
            m.data[0] = start_idx;
            m.data[1] = end_idx;
            if (send_message(connection_fd, &m) < 0)
            {
                perror("send_message");
                close_socket(connection_fd);
                continue;
            }

            // Receive the result from the client
            if (receive_message(connection_fd, &m) < 0)
            {
                perror("receive_message");
                close_socket(connection_fd);
                continue;
            }

            if (m.message_size == 1)
            {
                int count = m.data[0];
                total_count += count;
                printf("Received result: %d\n", count);
            }
        }

        close_socket(connection_fd);
    }

    close_socket(listen_fd);
    printf("Total count: %d\n", total_count);
    return 0;
}
#endif

#if SERVER == 0
static int connect_to_server(char *ip_address, int port_number)
{
    struct sockaddr_in peer_address;
    int connection_fd;
    in_addr_t address;

    connection_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connection_fd == -1)
    {
        perror("connect_to_server(): socket");
        exit(1);
    }
    address = inet_addr(ip_address);
    if (address == (in_addr_t)-1)
    {
        perror("connect_to_server(): inet_addr");
        exit(1);
    }
    peer_address.sin_family = AF_INET;
    peer_address.sin_addr.s_addr = address;
    peer_address.sin_port = htons(port_number);
    if (connect(connection_fd, (struct sockaddr *)&peer_address, (socklen_t)sizeof(struct sockaddr)) == -1)
    {
        perror("connect_to_server(): connect");
        exit(1);
    }
    return connection_fd;
}

int main(int argc, char **argv)
{
    char *server_address;
    int port_number, connection_fd;
    message_t m;

    if (argc != 3)
    {
    usage:
        fprintf(stderr, "usage: %s server-address port_number  # 49152 <= port_number <= 65535\n", argv[0]);
        exit(1);
    }
    server_address = argv[1];
    port_number = atoi(argv[2]);
    if (port_number < 49152 || port_number > 65535)
        goto usage;

    connection_fd = connect_to_server(server_address, port_number);
    printf("Connected to server: %s\n", server_address);

    // Request work from the server
    m.message_size = 1;
    m.data[0] = 0;         // Start index
    m.data[1] = n_samples; // End index
    if (send_message(connection_fd, &m) < 0)
    {
        perror("send_message");
        close_socket(connection_fd);
        exit(1);
    }

    // Receive the range from the server
    if (receive_message(connection_fd, &m) < 0)
    {
        perror("receive_message");
        close_socket(connection_fd);
        exit(1);
    }

    if (m.message_size == 2)
    {
        int start_idx = m.data[0];
        int end_idx = m.data[1];
        printf("Received range: %d to %d\n", start_idx, end_idx);

        // Perform the Mandelbrot calculation
        int count = 0;
        double c_re_min = -2.05;
        double c_re_max = +0.55;
        double c_im_min = -1.3;
        double c_im_max = +1.3;

        for (int re_idx = start_idx; re_idx < end_idx; re_idx++)
        {
            double c_re = c_re_min + (double)re_idx * ((c_re_max - c_re_min) / (double)n_samples);
            for (int im_idx = 0; im_idx < n_samples; im_idx++)
            {
                double c_im = c_im_min + (double)im_idx * ((c_im_max - c_im_min) / (double)n_samples);
                int n_iter = 0;
                double z_re = 0.0;
                double z_im = 0.0;
                while (n_iter < max_iterations && z_re * z_re + z_im * z_im <= 4.0)
                {
                    double tmp = z_re * z_re - z_im * z_im;
                    z_im = 2.0 * z_re * z_im + c_im;
                    z_re = tmp + c_re;
                    n_iter++;
                }
                if (n_iter < max_iterations)
                    count++;
            }
        }

        // Send the result back to the server
        m.message_size = 1;
        m.data[0] = count;
        if (send_message(connection_fd, &m) < 0)
        {
            perror("send_message");
            close_socket(connection_fd);
            exit(1);
        }
        printf("Sent result: %d\n", count);
    }

    close_socket(connection_fd);
    return 0;
}
#endif
