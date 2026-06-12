FROM gcc:latest AS builder

WORKDIR /app

COPY . /app/

RUN g++ src/taskmanager.cpp -o src/taskmanager

FROM debian:stable-slim

WORKDIR /app

COPY --from=builder /app/src/taskmanager ./src/taskmanager

CMD ["./src/taskmanager"]
