# Compiler and flags
CC = g++
CFLAGS = -I./tcp_server -I./tcp_client  # Include paths for server and client headers
LDFLAGS =  # Add any linker flags if needed

# Source files for the server and client
SRCS_SERVER = tcp_server/server.cpp tcp_server/ThreadPool.cpp tcp_server/Server.cpp tcp_client/Client.cpp tcp_server/utils/ServerUtils.cpp
SRCS_CLIENT = tcp_client/client.cpp tcp_client/Client.cpp tcp_server/Server.cpp tcp_server/ThreadPool.cpp tcp_server/utils/ServerUtils.cpp

# Object files
OBJS_SERVER = $(SRCS_SERVER:.cpp=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.cpp=.o)

# Final executables
EXEC_SERVER = server
EXEC_CLIENT = client

# Default target: all
all: $(EXEC_SERVER) $(EXEC_CLIENT)

# Compile and link the server executable
$(EXEC_SERVER): $(OBJS_SERVER)
	$(CC) $(OBJS_SERVER) $(CFLAGS) -o $(EXEC_SERVER)

# Compile and link the client executable
$(EXEC_CLIENT): $(OBJS_CLIENT)
	$(CC) $(OBJS_CLIENT) $(CFLAGS) -o $(EXEC_CLIENT)

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

# Clean up object files and executables
clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(EXEC_SERVER) $(EXEC_CLIENT)