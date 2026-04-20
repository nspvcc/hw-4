FROM gcc:latest
WORKDIR /app
COPY Main.cpp .
RUN g++ -O3 -pthread Main.cpp -o collatz
CMD ["./collatz"]